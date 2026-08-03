#include "Core/Application.h"

#include "UI/PauseMenu.h"
#include "UI/SettingsMenu.h"

#include <random>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>

namespace
{
    constexpr unsigned int ClientWidth =
        1280;

    constexpr unsigned int ClientHeight =
        720;

    constexpr std::uint16_t
        LocalNetworkPort =
        27015;

    constexpr double
        PlayerInputSendInterval =
        1.0 / 60.0;

    constexpr std::uint32_t
        SnapshotTickInterval =
        2;

    constexpr double
        RemoteInputTimeout =
        0.25;

    constexpr double
        ConnectionRecoveryDuration =
        30.0;

    constexpr double
        ReconnectAttemptInterval =
        1.0;
}

namespace Echo
{
    std::uint64_t
        Application::GenerateRuntimeIdentifier()
    {
        static std::random_device entropy;

        static std::mt19937_64 generator(
            (
                static_cast<std::uint64_t>(
                    entropy()
                    ) <<
                32u
                ) ^
            static_cast<std::uint64_t>(
                entropy()
                )
        );

        std::uint64_t identifier = 0;

        while (identifier == 0)
        {
            identifier =
                generator();
        }

        return identifier;
    }

    Application::Application()
        : m_window(
            m_keyboard,
            m_mouse,
            ClientWidth,
            ClientHeight,
            L"EchoGame"
        ),
        m_graphics(
            m_window.GetHandle(),
            ClientWidth,
            ClientHeight
        ),
        m_quadRenderer(
            m_graphics
        ),
        m_textRenderer(
            m_graphics
        )
    {
        m_localPlayerId =
            GenerateRuntimeIdentifier();
    }

    int Application::Run()
    {

        constexpr double fixedDeltaTime =
            1.0 / 60.0;

        constexpr double maximumFrameTime =
            0.25;

        double accumulatedTime = 0.0;

        m_clock.Reset();

        UpdateMenuTitle();

        while (!m_exitRequested &&
            m_window.ProcessMessages())
        {
            unsigned int resizedWidth = 0;
            unsigned int resizedHeight = 0;

            if (m_window.ConsumeResize(
                resizedWidth,
                resizedHeight))
            {
                m_graphics.Resize(
                    resizedWidth,
                    resizedHeight
                );

                m_aspectRatio =
                    static_cast<float>(resizedWidth) /
                    static_cast<float>(resizedHeight);
            }

            // Limit unusually large time jumps.
            // This can happen after pausing in the debugger.
            const double frameTime = std::min(
                m_clock.Tick(),
                maximumFrameTime
            );

            HandleApplicationInput();

            m_networkSession.Update();

            if (m_networkSession.
                ConsumeConnected())
            {
                HandleNetworkConnected();
            }

            if (m_networkSession.
                ConsumeConnectionLost())
            {
                HandleNetworkConnectionLost();
            }

            if (m_networkGamePhase ==
                NetworkGamePhase::
                ConnectionRecovery)
            {
                UpdateConnectionRecovery(
                    frameTime
                );
            }

            if (m_networkGamePhase ==
                NetworkGamePhase::
                HandshakingHost ||
                m_networkGamePhase ==
                NetworkGamePhase::
                HandshakingClient)
            {
                UpdateNetworkHandshake();
            }

            if (m_networkGamePhase ==
                NetworkGamePhase::
                SynchronizingHost ||
                m_networkGamePhase ==
                NetworkGamePhase::
                SynchronizingClient)
            {
                UpdateNetworkSynchronization();
            }

            const bool networkGameplayRunning =
                m_networkGamePhase ==
                NetworkGamePhase::Running;

            const bool shouldUpdateGameplayCamera =
                m_applicationState ==
                ApplicationState::LocalGame ||
                (
                    (
                        m_applicationState ==
                        ApplicationState::HostGame ||
                        m_applicationState ==
                        ApplicationState::JoinGame
                        ) &&
                    networkGameplayRunning
                    );

            if (shouldUpdateGameplayCamera)
            {
                m_gameplayCameraController.Update(
                    m_gameplayCamera,
                    m_gameSession,
                    GetCameraPlayerIndex(),
                    m_mouse,
                    m_window,
                    m_aspectRatio,
                    frameTime
                );
            }

            if (m_applicationState ==
                ApplicationState::JoinGame &&
                networkGameplayRunning &&
                m_networkSession.IsConnected())
            {
                m_playerInputSendAccumulator +=
                    frameTime;

                if (m_playerInputSendAccumulator >=
                    PlayerInputSendInterval)
                {
                    m_playerInputSendAccumulator =
                        std::fmod(
                            m_playerInputSendAccumulator,
                            PlayerInputSendInterval
                        );

                    NetworkPlayerInput localInput =
                        BuildLocalNetworkInput();

                    localInput.inputSequence =
                        m_nextLocalInputSequence;

                    localInput.clientTick =
                        m_clientTick;

                    ++m_nextLocalInputSequence;
                    ++m_clientTick;

                    m_networkSession.QueuePlayerInput(
                        localInput
                    );
                }

                NetworkWorldSnapshot receivedSnapshot{};

                if (m_networkSession.
                    TryConsumeWorldSnapshot(
                        receivedSnapshot
                    ))
                {
                    ApplyWorldSnapshot(
                        receivedSnapshot
                    );
                }
            }
            else
            {
                m_playerInputSendAccumulator =
                    0.0;
            }

            if (m_applicationState ==
                ApplicationState::JoinGame &&
                networkGameplayRunning &&
                m_networkSession.IsConnected() &&
                m_hasReceivedWorldSnapshot)
            {
                UpdateRemoteWorldPresentation(
                    frameTime
                );
            }

            if (m_applicationState ==
                ApplicationState::HostGame &&
                networkGameplayRunning &&
                m_networkSession.IsConnected())
            {
                NetworkPlayerInput receivedInput{};

                if (m_networkSession.
                    TryConsumePlayerInput(
                        receivedInput
                    ))
                {
                    m_latestRemotePlayerInput =
                        receivedInput;

                    m_remoteInputAge =
                        0.0;
                }
                else
                {
                    m_remoteInputAge +=
                        frameTime;
                }

                if (m_remoteInputAge >=
                    RemoteInputTimeout)
                {
                    m_latestRemotePlayerInput =
                        NetworkPlayerInput{};
                }
            }
            else
            {
                m_remoteInputAge =
                    0.0;
            }

            const bool isNetworkHost =
                m_applicationState ==
                ApplicationState::HostGame;

            const bool isConnectedHost =
                isNetworkHost &&
                m_networkSession.IsConnected();

            const bool isSimulationRunning =
                m_applicationState ==
                ApplicationState::LocalGame ||
                (
                    isNetworkHost &&
                    networkGameplayRunning
                    );

            bool shouldSendWorldSnapshot =
                false;

            if (isSimulationRunning)
            {
                GameSession::PlayerCommands
                    playerCommands{};

                if (isNetworkHost)
                {
                    playerCommands =
                        BuildHostPlayerCommands();
                }
                else
                {
                    playerCommands =
                        BuildLocalPlayerCommands();
                }

                accumulatedTime +=
                    frameTime;

                while (accumulatedTime >=
                    fixedDeltaTime)
                {
                    FixedUpdate(
                        playerCommands,
                        fixedDeltaTime
                    );

                    accumulatedTime -=
                        fixedDeltaTime;

                    if (isNetworkHost)
                    {
                        ++m_serverTick;

                        if (isConnectedHost &&
                            m_latestRemotePlayerInput.
                            inputSequence != 0)
                        {
                            m_lastProcessedRemoteInputSequence =
                                m_latestRemotePlayerInput.
                                inputSequence;
                        }

                        if (isConnectedHost &&
                            m_serverTick %
                            SnapshotTickInterval ==
                            0)
                        {
                            shouldSendWorldSnapshot =
                                true;
                        }
                    }
                }
            }
            else
            {
                // Prevent inactive states from accumulating
                // simulation time.
                accumulatedTime = 0.0;
            }

            if (isConnectedHost &&
                shouldSendWorldSnapshot)
            {
                m_networkSession.QueueWorldSnapshot(
                    BuildWorldSnapshot()
                );
            }

            // Send data queued during the current frame.
            m_networkSession.FlushOutgoing();

            switch (m_applicationState)
            {
            case ApplicationState::MainMenu:
            {
                m_graphics.BeginFrame(
                    0.03f,
                    0.03f,
                    0.06f
                );

                m_mainMenu.Render(
                    m_quadRenderer,
                    m_textRenderer,
                    m_window,
                    m_aspectRatio
                );

                break;
            }

            case ApplicationState::LocalGame:
            {
                m_graphics.BeginFrame(
                    0.02f,
                    0.04f,
                    0.08f
                );

                RenderGameplay();

                break;
            }

            case ApplicationState::Paused:
            {
                m_graphics.BeginFrame(
                    0.02f,
                    0.04f,
                    0.08f
                );

                // Draw the frozen game world.
                RenderGameplay();

                // Draw the pause interface over the game.
                m_pauseMenu.Render(
                    m_textRenderer,
                    m_window
                );

                break;
            }

            case ApplicationState::HostGame:
            {
                m_graphics.BeginFrame(
                    0.02f,
                    0.04f,
                    0.08f
                );

                RenderGameplay();

                break;
            }

            case ApplicationState::JoinGame:
            {
                const bool canRenderRemoteWorld =
                    m_hasReceivedWorldSnapshot &&
                    (
                        m_networkSession.IsConnected() ||
                        m_networkGamePhase ==
                        NetworkGamePhase::
                        ConnectionRecovery
                        );

                if (canRenderRemoteWorld)
                {
                    m_graphics.BeginFrame(
                        0.02f,
                        0.04f,
                        0.08f
                    );

                    RenderGameplay();
                }
                else
                {
                    m_graphics.BeginFrame(
                        0.05f,
                        0.03f,
                        0.05f
                    );

                    RenderPlaceholder();
                }

                break;
            }

            case ApplicationState::Settings:
            {
                m_graphics.BeginFrame(
                    0.025f,
                    0.035f,
                    0.06f
                );

                m_settingsMenu.Render(
                    m_quadRenderer,
                    m_textRenderer,
                    m_window,
                    m_settings,
                    m_aspectRatio
                );

                break;
            }
            }

            const bool isConnectionRecovery =
                m_networkGamePhase ==
                NetworkGamePhase::
                ConnectionRecovery;

            const bool isHandshakingHost =
                m_networkGamePhase ==
                NetworkGamePhase::
                HandshakingHost;

            const bool isHandshakingClient =
                m_networkGamePhase ==
                NetworkGamePhase::
                HandshakingClient;

            const bool isSynchronizingHost =
                m_networkGamePhase ==
                NetworkGamePhase::
                SynchronizingHost;

            const bool isSynchronizingClient =
                m_networkGamePhase ==
                NetworkGamePhase::
                SynchronizingClient;

            const bool isNetworkOverlayVisible =
                (
                    m_applicationState ==
                    ApplicationState::HostGame ||
                    m_applicationState ==
                    ApplicationState::JoinGame
                    ) &&
                (
                    isConnectionRecovery ||
                    isHandshakingHost ||
                    isHandshakingClient ||
                    isSynchronizingHost ||
                    isSynchronizingClient
                    );

            if (isNetworkOverlayVisible)
            {
                const bool canContinueSolo =
                    isConnectionRecovery &&
                    m_applicationState ==
                    ApplicationState::JoinGame &&
                    m_hasMigrationState;

                const wchar_t* title =
                    L"CONNECTION LOST";

                const wchar_t* message =
                    L"Restoring the game session...";

                const bool showTimer =
                    isConnectionRecovery;

                if (isHandshakingHost)
                {
                    title =
                        L"PLAYER CONNECTED";

                    message =
                        L"Verifying session identity...";
                }
                else if (isHandshakingClient)
                {
                    title =
                        L"CONNECTING";

                    message =
                        L"Joining the game session...";
                }
                else if (isSynchronizingHost)
                {
                    title =
                        L"PLAYER CONNECTED";

                    message =
                        L"Sending current game state...";
                }
                else if (isSynchronizingClient)
                {
                    title =
                        L"CONNECTING";

                    message =
                        L"Synchronizing game state...";
                }

                m_connectionRecoveryOverlay.Render(
                    m_textRenderer,
                    m_window,
                    m_connectionRecoveryRemaining,
                    showTimer,
                    canContinueSolo,
                    title,
                    message,
                    m_networkSession.
                    GetStatusMessage()
                );
            }

            m_graphics.EndFrame(
                m_settings.verticalSync
            );

            if (isSimulationRunning)
            {
                m_frameStatistics.Update(
                    frameTime
                );
            }

            m_keyboard.EndFrame();
            m_mouse.EndFrame();
        }

        return 0;
    }

    void Application::FixedUpdate(
        const GameSession::PlayerCommands&
        playerCommands,
        double deltaTime
    )
    {
        m_gameSession.Update(
            playerCommands,
            deltaTime
        );
    }

    std::size_t Application::GetCameraPlayerIndex()
        const noexcept
    {
        if (m_applicationState ==
            ApplicationState::HostGame ||
            m_applicationState ==
            ApplicationState::JoinGame)
        {
            return
                m_localNetworkPlayerIndex;
        }

        return 0;
    }
}