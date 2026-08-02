#include "Core/Application.h"

#include "UI/PauseMenu.h"
#include "UI/SettingsMenu.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <utility>
#include <vector>

namespace
{
    constexpr unsigned int ClientWidth =
        1280;

    constexpr unsigned int ClientHeight =
        720;

    constexpr std::size_t
        InitialHostPlayerIndex =
        0;

    constexpr std::size_t
        InitialClientPlayerIndex =
        1;

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

    constexpr double
        RemotePlayerInterpolationDuration =
        1.0 / 30.0;

    constexpr float Pi =
        3.14159265358979323846f;

    constexpr float TwoPi =
        Pi * 2.0f;

    float InterpolateFloat(
        float start,
        float target,
        float amount
    ) noexcept
    {
        return
            start +
            (target - start) *
            amount;
    }

    float InterpolateAngle(
        float start,
        float target,
        float amount
    ) noexcept
    {
        float difference =
            target -
            start;

        while (difference > Pi)
        {
            difference -=
                TwoPi;
        }

        while (difference < -Pi)
        {
            difference +=
                TwoPi;
        }

        return
            start +
            difference *
            amount;
    }
}

namespace Echo
{
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

                if (isSynchronizingHost)
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
                UpdateStatistics(
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

    void Application::SetNetworkPlayerOwnership(
        std::size_t localPlayerIndex,
        std::size_t remotePlayerIndex
    ) noexcept
    {
        static_assert(
            GameSession::PlayerCount ==
            NetworkPlayerCount,
            "Game and network player counts differ."
            );

        if (localPlayerIndex >=
            NetworkPlayerCount ||
            remotePlayerIndex >=
            NetworkPlayerCount ||
            localPlayerIndex ==
            remotePlayerIndex)
        {
            return;
        }

        m_localNetworkPlayerIndex =
            localPlayerIndex;

        m_remoteNetworkPlayerIndex =
            remotePlayerIndex;
    }

    void Application::HandleNetworkConnected()
    {
        switch (m_applicationState)
        {
        case ApplicationState::HostGame:
        {
            BeginHostSynchronization();
            break;
        }

        case ApplicationState::JoinGame:
        {
            BeginClientSynchronization();
            break;
        }

        default:
        {
            break;
        }
        }
    }

    void Application::BeginHostSynchronization()
    {
        m_networkGamePhase =
            NetworkGamePhase::
            SynchronizingHost;

        m_checkpointQueued =
            false;

        m_checkpointAppliedQueued =
            false;

        m_resumeQueued =
            false;

        m_latestRemotePlayerInput =
            NetworkPlayerInput{};

        m_remoteInputAge =
            0.0;

        m_connectionRecoveryRemaining =
            0.0;

        m_connectionRecoveryOverlay.Reset();
    }

    void Application::BeginClientSynchronization()
    {
        m_networkGamePhase =
            NetworkGamePhase::
            SynchronizingClient;

        m_checkpointQueued =
            false;

        m_checkpointAppliedQueued =
            false;

        m_resumeQueued =
            false;

        m_playerInputSendAccumulator =
            0.0;

        m_connectionRecoveryRemaining =
            0.0;

        m_connectionRecoveryOverlay.Reset();
    }

    void Application::UpdateNetworkSynchronization()
    {
        if (!m_networkSession.IsConnected())
        {
            return;
        }

        if (m_networkGamePhase ==
            NetworkGamePhase::
            SynchronizingHost)
        {
            if (!m_checkpointQueued)
            {
                if (!m_networkSession.
                    IsOutgoingIdle())
                {
                    return;
                }

                m_networkSession.QueueWorldSnapshot(
                    BuildWorldSnapshot()
                );

                m_checkpointQueued =
                    true;

                return;
            }

            if (!m_resumeQueued)
            {
                if (!m_networkSession.
                    TryConsumeCheckpointApplied())
                {
                    return;
                }

                m_networkSession.QueueResumeGame();

                m_resumeQueued =
                    true;

                return;
            }

            // ResumeGame has been completely handed
            // to the TCP stream. Later packets will
            // therefore arrive after it.
            if (!m_networkSession.
                IsOutgoingIdle())
            {
                return;
            }

            m_networkGamePhase =
                NetworkGamePhase::Running;

            m_checkpointQueued =
                false;

            m_resumeQueued =
                false;

            m_connectionRecoveryOverlay.Reset();

            return;
        }

        if (m_networkGamePhase !=
            NetworkGamePhase::
            SynchronizingClient)
        {
            return;
        }

        if (!m_checkpointAppliedQueued)
        {
            NetworkWorldSnapshot checkpoint{};

            if (!m_networkSession.
                TryConsumeWorldSnapshot(
                    checkpoint
                ))
            {
                return;
            }

            ApplyWorldSnapshot(
                checkpoint
            );

            m_networkSession.
                QueueCheckpointApplied();

            m_checkpointAppliedQueued =
                true;

            return;
        }

        if (!m_networkSession.
            TryConsumeResumeGame())
        {
            return;
        }

        m_networkGamePhase =
            NetworkGamePhase::Running;

        m_checkpointAppliedQueued =
            false;

        // Send normal input immediately after resume.
        m_playerInputSendAccumulator =
            PlayerInputSendInterval;

        m_connectionRecoveryOverlay.Reset();
    }

    void Application::HandleNetworkConnectionLost()
    {
        if (m_applicationState !=
            ApplicationState::HostGame &&
            m_applicationState !=
            ApplicationState::JoinGame)
        {
            return;
        }

        BeginConnectionRecovery();
    }

    void Application::BeginConnectionRecovery()
    {
        if (m_networkGamePhase ==
            NetworkGamePhase::
            ConnectionRecovery)
        {
            return;
        }

        m_networkGamePhase =
            NetworkGamePhase::
            ConnectionRecovery;

        m_connectionRecoveryRemaining =
            ConnectionRecoveryDuration;

        // Allow the client to make its first
        // reconnect attempt immediately.
        m_reconnectAttemptAccumulator =
            ReconnectAttemptInterval;

        m_checkpointQueued =
            false;

        m_checkpointAppliedQueued =
            false;

        m_resumeQueued =
            false;

        m_latestRemotePlayerInput =
            NetworkPlayerInput{};

        m_remoteInputAge =
            0.0;

        m_playerInputSendAccumulator =
            0.0;

        m_connectionRecoveryOverlay.Reset();

        if (m_applicationState ==
            ApplicationState::HostGame)
        {
            RestartHostListener();
        }
        else
        {
            m_networkSession.Stop();
        }
    }

    void Application::UpdateConnectionRecovery(
        double deltaTime
    )
    {
        if (m_networkGamePhase !=
            NetworkGamePhase::
            ConnectionRecovery)
        {
            return;
        }

        m_connectionRecoveryRemaining =
            std::max(
                0.0,
                m_connectionRecoveryRemaining -
                deltaTime
            );

        if (m_networkSession.IsConnected())
        {
            return;
        }

        m_reconnectAttemptAccumulator +=
            deltaTime;

        const NetworkSessionStatus status =
            m_networkSession.GetStatus();

        const bool canRestartAttempt =
            status !=
            NetworkSessionStatus::
            Connecting &&
            status !=
            NetworkSessionStatus::
            Connected;

        if (m_reconnectAttemptAccumulator >=
            ReconnectAttemptInterval &&
            canRestartAttempt)
        {
            m_reconnectAttemptAccumulator =
                0.0;

            if (m_applicationState ==
                ApplicationState::HostGame)
            {
                RestartHostListener();
            }
            else if (m_applicationState ==
                ApplicationState::JoinGame)
            {
                m_networkSession.StartClient(
                    LocalNetworkPort
                );
            }
        }

        if (m_applicationState !=
            ApplicationState::JoinGame ||
            m_connectionRecoveryRemaining >
            0.0)
        {
            return;
        }

        if (m_hasMigrationState)
        {
            PromoteClientToHost();
            return;
        }

        m_networkSession.Stop();

        ResetNetworkGameState();

        m_mainMenu.Reset();

        EnterState(
            ApplicationState::MainMenu
        );
    }

    void Application::PromoteClientToHost()
    {
        if (!m_hasMigrationState)
        {
            return;
        }

        // Preserve the latest authoritative backup
        // before resetting client-side network state.
        const GameMigrationState migrationState =
            m_latestMigrationState;

        const std::uint32_t restoredServerTick =
            m_latestReceivedServerTick;

        m_networkSession.Stop();

        ResetNetworkGameState();

        m_gameSession.RestoreMigrationState(
            migrationState
        );

        m_serverTick =
            restoredServerTick;

        // Player ownership is intentionally preserved.
        // The former client continues controlling
        // the same player after becoming the host.
        m_latestRemotePlayerInput =
            NetworkPlayerInput{};

        m_networkSession.StartHost(
            LocalNetworkPort
        );

        m_networkGamePhase =
            NetworkGamePhase::Running;

        m_connectionRecoveryRemaining =
            0.0;

        m_reconnectAttemptAccumulator =
            0.0;

        m_checkpointQueued =
            false;

        m_checkpointAppliedQueued =
            false;

        m_resumeQueued =
            false;

        m_connectionRecoveryOverlay.Reset();

        EnterState(
            ApplicationState::HostGame
        );
    }

    void Application::RestartHostListener()
    {
        m_latestRemotePlayerInput =
            NetworkPlayerInput{};

        m_remoteInputAge =
            0.0;

        m_lastProcessedRemoteInputSequence =
            0;

        m_networkSession.StartHost(
            LocalNetworkPort
        );
    }

    void Application::HandleApplicationInput()
    {
        switch (m_applicationState)
        {
        case ApplicationState::MainMenu:
        {
            const MainMenuAction action =
                m_mainMenu.Update(
                    m_keyboard,
                    m_mouse,
                    m_window,
                    m_aspectRatio
                );

            switch (action)
            {
            case MainMenuAction::None:
            {
                break;
            }

            case MainMenuAction::StartLocalGame:
            {
                m_gameSession.Reset();

                EnterState(
                    ApplicationState::LocalGame
                );

                break;
            }

            case MainMenuAction::HostGame:
            {
                m_gameSession.Reset();

                ResetNetworkGameState();

                SetNetworkPlayerOwnership(
                    InitialHostPlayerIndex,
                    InitialClientPlayerIndex
                );

                m_networkSession.StartHost(
                    LocalNetworkPort
                );

                m_networkGamePhase =
                    NetworkGamePhase::Running;

                EnterState(
                    ApplicationState::HostGame
                );

                break;
            }

            case MainMenuAction::JoinGame:
            {
                m_gameSession.Reset();

                ResetNetworkGameState();

                SetNetworkPlayerOwnership(
                    InitialClientPlayerIndex,
                    InitialHostPlayerIndex
                );

                // Send the first command immediately
                // after the connection is established.
                m_playerInputSendAccumulator =
                    PlayerInputSendInterval;

                m_networkSession.StartClient(
                    LocalNetworkPort
                );

                m_networkGamePhase =
                    NetworkGamePhase::Running;

                EnterState(
                    ApplicationState::JoinGame
                );

                break;
            }

            case MainMenuAction::OpenSettings:
            {
                m_settingsReturnState =
                    ApplicationState::MainMenu;

                EnterState(
                    ApplicationState::Settings
                );

                break;
            }

            case MainMenuAction::Exit:
            {
                m_exitRequested = true;
                break;
            }
            }

            break;
        }

        case ApplicationState::LocalGame:
        {
            if (m_keyboard.WasPressed(
                Key::Escape))
            {
                m_pauseMenu.Reset();

                EnterState(
                    ApplicationState::Paused
                );
            }

            break;
        }

        case ApplicationState::Paused:
        {
            const PauseMenuAction action =
                m_pauseMenu.Update(
                    m_keyboard,
                    m_mouse,
                    m_window
                );

            switch (action)
            {
            case PauseMenuAction::None:
            {
                break;
            }

            case PauseMenuAction::Resume:
            {
                EnterState(
                    ApplicationState::LocalGame
                );

                break;
            }

            case PauseMenuAction::OpenSettings:
            {
                m_settingsReturnState =
                    ApplicationState::Paused;

                EnterState(
                    ApplicationState::Settings
                );

                break;
            }

            case PauseMenuAction::ReturnToMainMenu:
            {
                m_mainMenu.Reset();

                EnterState(
                    ApplicationState::MainMenu
                );

                break;
            }

            case PauseMenuAction::Exit:
            {
                m_exitRequested = true;
                break;
            }
            }

            break;
        }

        case ApplicationState::HostGame:
        case ApplicationState::JoinGame:
        {
            if (m_keyboard.WasPressed(
                Key::Escape))
            {

                if (m_networkGamePhase ==
                    NetworkGamePhase::
                    ConnectionRecovery)
                {
                    const bool canContinueSolo =
                        m_applicationState ==
                        ApplicationState::JoinGame &&
                        m_hasMigrationState;

                    const ConnectionRecoveryAction action =
                        m_connectionRecoveryOverlay.Update(
                            m_keyboard,
                            m_mouse,
                            m_window,
                            canContinueSolo
                        );

                    if (action ==
                        ConnectionRecoveryAction::
                        ContinueSolo)
                    {
                        PromoteClientToHost();
                        break;
                    }
                }

                m_networkSession.Stop();

                ResetNetworkGameState();

                m_mainMenu.Reset();

                EnterState(
                    ApplicationState::MainMenu
                );
            }

            break;
        }

        case ApplicationState::Settings:
        {
            const SettingsMenuAction action =
                m_settingsMenu.Update(
                    m_keyboard,
                    m_mouse,
                    m_window,
                    m_aspectRatio
                );

            switch (action)
            {
            case SettingsMenuAction::None:
            {
                break;
            }

            case SettingsMenuAction::ToggleFullscreen:
            {
                const bool requestedFullscreen =
                    !m_settings.fullscreen;

                m_window.SetFullscreen(
                    requestedFullscreen
                );

                m_settings.fullscreen =
                    m_window.IsFullscreen();

                break;
            }

            case SettingsMenuAction::ToggleVerticalSync:
            {
                m_settings.verticalSync =
                    !m_settings.verticalSync;

                break;
            }

            case SettingsMenuAction::ToggleDebugOverlay:
            {
                m_settings.debugOverlay =
                    !m_settings.debugOverlay;

                break;
            }

            case SettingsMenuAction::Back:
            {
                EnterState(
                    m_settingsReturnState
                );

                break;
            }
            }

            break;
        }
        }
    }

    void Application::EnterState(
        ApplicationState state
    )
    {
        m_applicationState =
            state;

        m_statisticsTimer = 0.0;
        m_frameCount = 0;

        UpdateMenuTitle();
    }

    void Application::RenderGameplay()
    {
        const Level& level =
            m_gameSession.GetLevel();

        for (const LevelBlock& block :
            level.GetBlocks())
        {
            m_quadRenderer.Draw(
                block.positionX,
                block.positionY,
                block.width,
                block.height,
                block.rotation,
                m_aspectRatio
            );
        }

        for (const Player& player :
            m_gameSession.GetPlayers())
        {
            m_quadRenderer.Draw(
                player.GetPositionX(),
                player.GetPositionY(),
                player.GetWidth(),
                player.GetHeight(),
                player.GetRotation(),
                m_aspectRatio
            );
        }

        for (const Projectile& projectile :
            m_gameSession.GetProjectiles())
        {
            m_quadRenderer.Draw(
                projectile.GetPositionX(),
                projectile.GetPositionY(),
                projectile.GetWidth(),
                projectile.GetHeight(),
                projectile.GetRotation(),
                m_aspectRatio
            );
        }

        if (m_settings.debugOverlay)
        {
            RenderDebugOverlay();
        }
    }

    void Application::RenderDebugOverlay()
    {
        const float clientWidth =
            static_cast<float>(
                m_window.GetClientWidth()
                );

        if (clientWidth <= 0.0f)
        {
            return;
        }

        const Player& playerOne =
            m_gameSession.GetPlayer(0);

        const Player& playerTwo =
            m_gameSession.GetPlayer(1);

        std::wostringstream debugText;

        debugText
            << std::fixed
            << std::setprecision(1)
            << L"FPS: "
            << m_framesPerSecond
            << L" | Frame: "
            << m_frameTimeMilliseconds
            << L" ms"
            << L" | P1: "
            << std::setprecision(2)
            << playerOne.GetPositionX()
            << L", "
            << playerOne.GetPositionY()
            << L" | P2: "
            << playerTwo.GetPositionX()
            << L", "
            << playerTwo.GetPositionY()
            << L" | Projectiles: "
            << m_gameSession
            .GetProjectiles()
            .size()
            << L" | Local: P"
            << (
                m_localNetworkPlayerIndex +
                1
                )
            << L" | Remote: P"
            << (
                m_remoteNetworkPlayerIndex +
                1
                )
            << L" | Backup: "
            << (
                m_hasMigrationState
                ? L"Ready"
                : L"Waiting"
                )
            << L" | Net: "
            << m_networkSession.
            GetStatusMessage();

        m_textRenderer.Begin();

        m_textRenderer.Draw(
            debugText.str(),
            D2D1_RECT_F{
                0.0f,
                8.0f,
                clientWidth,
                48.0f
            },
            TextStyle::Hint
        );

        m_textRenderer.End();
    }

    void Application::RenderPlaceholder()
    {
        m_quadRenderer.Draw(
            0.0f,
            0.0f,
            0.80f,
            0.30f,
            0.0f,
            m_aspectRatio
        );

        const float clientWidth =
            static_cast<float>(
                m_window.GetClientWidth()
                );

        const float clientHeight =
            static_cast<float>(
                m_window.GetClientHeight()
                );

        const wchar_t* screenTitle =
            L"Placeholder";

        switch (m_applicationState)
        {
        case ApplicationState::HostGame:
            screenTitle = L"Host Game";
            break;

        case ApplicationState::JoinGame:
            screenTitle = L"Join Game";
            break;

        default:
            break;
        }

        m_textRenderer.Begin();

        m_textRenderer.Draw(
            screenTitle,
            D2D1_RECT_F{
                0.0f,
                clientHeight * 0.30f,
                clientWidth,
                clientHeight * 0.46f
            },
            TextStyle::Title
        );

        m_textRenderer.Draw(
            m_networkSession.GetStatusMessage(),
            D2D1_RECT_F{
                0.0f,
                clientHeight * 0.46f,
                clientWidth,
                clientHeight * 0.58f
            },
            TextStyle::MenuItem
        );

        m_textRenderer.Draw(
            L"Escape - Back",
            D2D1_RECT_F{
                0.0f,
                clientHeight - 55.0f,
                clientWidth,
                clientHeight - 10.0f
            },
            TextStyle::Hint
        );

        m_textRenderer.End();
    }

    void Application::UpdateMenuTitle()
    {
        switch (m_applicationState)
        {
        case ApplicationState::MainMenu:
        {
            m_window.SetTitle(
                L"Echo | Main Menu"
            );

            break;
        }

        case ApplicationState::LocalGame:
        {
            m_window.SetTitle(
                L"Echo | Local Game | Escape: Pause"
            );

            break;
        }

        case ApplicationState::Paused:
        {
            m_window.SetTitle(
                L"Echo | Paused | Escape: Resume"
            );

            break;
        }

        case ApplicationState::HostGame:
        {
            m_window.SetTitle(
                L"Echo | Host Game | Escape: Back"
            );

            break;
        }

        case ApplicationState::JoinGame:
        {
            m_window.SetTitle(
                L"Echo | Join Game | Escape: Back"
            );

            break;
        }

        case ApplicationState::Settings:
        {
            m_window.SetTitle(
                L"Echo | Settings | Escape: Back"
            );

            break;
        }
        }
    }

    GameSession::PlayerCommands
        Application::BuildLocalPlayerCommands()
        const noexcept
    {
        GameSession::PlayerCommands
            commands{};

        commands[0] =
            BuildPlayerCommand(
                0,
                BuildLocalNetworkInput()
            );

        return commands;
    }

    GameSession::PlayerCommands
        Application::BuildHostPlayerCommands()
        const noexcept
    {
        GameSession::PlayerCommands
            commands{};

        commands[
            m_localNetworkPlayerIndex
        ] =
            BuildPlayerCommand(
                m_localNetworkPlayerIndex,
                BuildLocalNetworkInput()
            );

            commands[
                m_remoteNetworkPlayerIndex
            ] =
                BuildPlayerCommand(
                    m_remoteNetworkPlayerIndex,
                    m_latestRemotePlayerInput
                );

                return commands;
    }

    NetworkPlayerInput
        Application::BuildLocalNetworkInput()
        const noexcept
    {
        NetworkPlayerInput input{};

        if (m_keyboard.IsDown(Key::A))
        {
            input.movementX -=
                1.0f;
        }

        if (m_keyboard.IsDown(Key::D))
        {
            input.movementX +=
                1.0f;
        }

        if (m_keyboard.IsDown(Key::W))
        {
            input.movementY +=
                1.0f;
        }

        if (m_keyboard.IsDown(Key::S))
        {
            input.movementY -=
                1.0f;
        }

        if (!m_mouse.IsInsideWindow())
        {
            return input;
        }

        input.fire =
            m_mouse.IsLeftButtonDown();

        const float clientWidth =
            static_cast<float>(
                m_window.GetClientWidth()
                );

        const float clientHeight =
            static_cast<float>(
                m_window.GetClientHeight()
                );

        if (clientWidth <= 0.0f ||
            clientHeight <= 0.0f)
        {
            return input;
        }

        const float normalizedMouseX =
            2.0f *
            static_cast<float>(
                m_mouse.GetX()
                ) /
            clientWidth -
            1.0f;

        const float normalizedMouseY =
            1.0f -
            2.0f *
            static_cast<float>(
                m_mouse.GetY()
                ) /
            clientHeight;

        input.aimTargetX =
            normalizedMouseX *
            m_aspectRatio;

        input.aimTargetY =
            normalizedMouseY;

        input.hasAimTarget =
            true;

        return input;
    }

    PlayerCommand Application::BuildPlayerCommand(
        std::size_t playerIndex,
        const NetworkPlayerInput& input
    ) const noexcept
    {
        PlayerCommand command{};

        command.movementX =
            input.movementX;

        command.movementY =
            input.movementY;

        command.fire =
            input.fire;

        const Player& player =
            m_gameSession.GetPlayer(
                playerIndex
            );

        // Preserve the current aim direction when
        // no valid mouse target is available.
        command.aimX =
            player.GetForwardX();

        command.aimY =
            player.GetForwardY();

        if (!input.hasAimTarget)
        {
            return command;
        }

        const float directionX =
            input.aimTargetX -
            player.GetPositionX();

        const float directionY =
            input.aimTargetY -
            player.GetPositionY();

        const float directionLengthSquared =
            directionX * directionX +
            directionY * directionY;

        constexpr float DirectionEpsilon =
            0.000001f;

        if (directionLengthSquared <=
            DirectionEpsilon)
        {
            return command;
        }

        const float inverseLength =
            1.0f /
            std::sqrt(
                directionLengthSquared
            );

        command.aimX =
            directionX *
            inverseLength;

        command.aimY =
            directionY *
            inverseLength;

        return command;
    }

    NetworkWorldSnapshot
        Application::BuildWorldSnapshot()
        const noexcept
    {
        static_assert(
            GameSession::PlayerCount ==
            NetworkPlayerCount,
            "Game and network player counts differ."
            );

        NetworkWorldSnapshot snapshot{};

        snapshot.serverTick =
            m_serverTick;

        snapshot.lastProcessedInputSequence =
            m_lastProcessedRemoteInputSequence;

        snapshot.assignedPlayerIndex =
            static_cast<std::uint32_t>(
                m_remoteNetworkPlayerIndex
                );

        snapshot.nextProjectileEntityId =
            m_gameSession.
            GetNextProjectileEntityId();

        snapshot.fireCooldowns =
            m_gameSession.GetFireCooldowns();

        for (std::size_t playerIndex = 0;
            playerIndex < NetworkPlayerCount;
            ++playerIndex)
        {
            const Player& player =
                m_gameSession.GetPlayer(
                    playerIndex
                );

            NetworkPlayerState&
                playerState =
                snapshot.players[playerIndex];

            playerState.positionX =
                player.GetPositionX();

            playerState.positionY =
                player.GetPositionY();

            playerState.rotation =
                player.GetRotation();
        }

        const std::vector<Projectile>& projectiles =
            m_gameSession.GetProjectiles();

        const std::size_t projectileCount =
            std::min(
                projectiles.size(),
                NetworkMaxProjectileCount
            );

        snapshot.projectileCount =
            static_cast<std::uint32_t>(
                projectileCount
                );

        for (std::size_t projectileIndex = 0;
            projectileIndex < projectileCount;
            ++projectileIndex)
        {
            const Projectile& projectile =
                projectiles[projectileIndex];

            NetworkProjectileState&
                projectileState =
                snapshot.projectiles[
                    projectileIndex
                ];

            projectileState.entityId =
                projectile.GetEntityId();

            projectileState.positionX =
                projectile.GetPositionX();

            projectileState.positionY =
                projectile.GetPositionY();

            projectileState.rotation =
                projectile.GetRotation();

            projectileState.remainingLifetime =
                projectile.GetRemainingLifetime();
        }

        return snapshot;
    }

    GameMigrationState
        Application::BuildMigrationState(
            const NetworkWorldSnapshot& snapshot
        ) const
    {
        static_assert(
            GameSession::PlayerCount ==
            GameMigrationPlayerCount,
            "Game and migration player counts differ."
            );

        static_assert(
            NetworkPlayerCount ==
            GameMigrationPlayerCount,
            "Network and migration player counts differ."
            );

        GameMigrationState migrationState{};

        for (std::size_t playerIndex = 0;
            playerIndex <
            GameMigrationPlayerCount;
            ++playerIndex)
        {
            const NetworkPlayerState&
                networkPlayerState =
                snapshot.players[
                    playerIndex
                ];

            GameMigrationPlayerState&
                migrationPlayerState =
                migrationState.players[
                    playerIndex
                ];

            migrationPlayerState.positionX =
                networkPlayerState.positionX;

            migrationPlayerState.positionY =
                networkPlayerState.positionY;

            migrationPlayerState.rotation =
                networkPlayerState.rotation;
        }

        migrationState.nextProjectileEntityId =
            snapshot.nextProjectileEntityId;

        migrationState.fireCooldowns =
            snapshot.fireCooldowns;

        const std::size_t projectileCount =
            std::min(
                static_cast<std::size_t>(
                    snapshot.projectileCount
                    ),
                NetworkMaxProjectileCount
            );

        migrationState.projectiles.reserve(
            projectileCount
        );

        for (std::size_t projectileIndex = 0;
            projectileIndex < projectileCount;
            ++projectileIndex)
        {
            const NetworkProjectileState&
                networkProjectileState =
                snapshot.projectiles[
                    projectileIndex
                ];

            GameMigrationProjectileState
                migrationProjectileState{};

            migrationProjectileState.entityId =
                networkProjectileState.entityId;

            migrationProjectileState.positionX =
                networkProjectileState.positionX;

            migrationProjectileState.positionY =
                networkProjectileState.positionY;

            migrationProjectileState.rotation =
                networkProjectileState.rotation;

            migrationProjectileState.
                remainingLifetime =
                networkProjectileState.
                remainingLifetime;

            migrationState.projectiles.push_back(
                migrationProjectileState
            );
        }

        return migrationState;
    }

    void Application::ApplyWorldSnapshot(
        const NetworkWorldSnapshot& snapshot
    )
    {
        static_assert(
            GameSession::PlayerCount ==
            NetworkPlayerCount,
            "Game and network player counts differ."
            );

        m_latestReceivedServerTick =
            snapshot.serverTick;

        m_lastAcknowledgedInputSequence =
            snapshot.lastProcessedInputSequence;

        const std::size_t assignedPlayerIndex =
            static_cast<std::size_t>(
                snapshot.assignedPlayerIndex
                );

        const std::size_t remotePlayerIndex =
            (
                assignedPlayerIndex +
                1
                ) %
            NetworkPlayerCount;

        SetNetworkPlayerOwnership(
            assignedPlayerIndex,
            remotePlayerIndex
        );

        m_latestMigrationState =
            BuildMigrationState(
                snapshot
            );

        m_hasMigrationState =
            true;

        if (!m_hasReceivedWorldSnapshot)
        {
            m_remoteInterpolationStart =
                snapshot;

            m_remoteInterpolationTarget =
                snapshot;

            m_remoteInterpolationElapsed =
                RemotePlayerInterpolationDuration;

            for (std::size_t playerIndex = 0;
                playerIndex < NetworkPlayerCount;
                ++playerIndex)
            {
                const NetworkPlayerState&
                    playerState =
                    snapshot.players[
                        playerIndex
                    ];

                m_gameSession.SetPlayerNetworkState(
                    playerIndex,
                    playerState.positionX,
                    playerState.positionY,
                    playerState.rotation
                );
            }

            m_hasReceivedWorldSnapshot =
                true;
        }
        else
        {
            // Begin interpolation from the positions
            // currently displayed on the client.
            for (std::size_t playerIndex = 0;
                playerIndex < NetworkPlayerCount;
                ++playerIndex)
            {
                const Player& player =
                    m_gameSession.GetPlayer(
                        playerIndex
                    );

                NetworkPlayerState&
                    startState =
                    m_remoteInterpolationStart
                    .players[playerIndex];

                startState.positionX =
                    player.GetPositionX();

                startState.positionY =
                    player.GetPositionY();

                startState.rotation =
                    player.GetRotation();
            }

            m_remoteInterpolationTarget =
                snapshot;

            m_remoteInterpolationElapsed =
                0.0;
        }

        const std::size_t projectileCount =
            std::min(
                static_cast<std::size_t>(
                    snapshot.projectileCount
                    ),
                NetworkMaxProjectileCount
            );

        const std::vector<Projectile>&
            currentProjectiles =
            m_gameSession.GetProjectiles();

        std::vector<Projectile>
            synchronizedProjectiles;

        synchronizedProjectiles.reserve(
            projectileCount
        );

        for (std::size_t projectileIndex = 0;
            projectileIndex < projectileCount;
            ++projectileIndex)
        {
            const NetworkProjectileState&
                projectileState =
                snapshot.projectiles[
                    projectileIndex
                ];

            const auto existingProjectile =
                std::find_if(
                    currentProjectiles.begin(),
                    currentProjectiles.end(),
                    [
                        entityId =
                            projectileState.entityId
                    ](
                        const Projectile& projectile
                        )
                    {
                        return
                            projectile.GetEntityId() ==
                            entityId;
                    }
                            );

            if (existingProjectile !=
                currentProjectiles.end())
            {
                synchronizedProjectiles.push_back(
                    *existingProjectile
                );

                synchronizedProjectiles.back().
                    SetNetworkState(
                        projectileState.positionX,
                        projectileState.positionY,
                        projectileState.rotation
                    );

                synchronizedProjectiles.back().
                    SetRemainingLifetime(
                        projectileState.remainingLifetime
                    );

                continue;
            }

            const float directionX =
                std::cos(
                    projectileState.rotation
                );

            const float directionY =
                std::sin(
                    projectileState.rotation
                );

            synchronizedProjectiles.emplace_back(
                projectileState.entityId,
                projectileState.positionX,
                projectileState.positionY,
                directionX,
                directionY
            );

            synchronizedProjectiles.back().
                SetRemainingLifetime(
                    projectileState.remainingLifetime
                );
        }

        m_gameSession.SetProjectiles(
            std::move(
                synchronizedProjectiles
            )
        );
    }

    void Application::UpdateRemoteWorldPresentation(
        double deltaTime
    ) noexcept
    {
        if (!m_hasReceivedWorldSnapshot)
        {
            return;
        }

        if (deltaTime > 0.0)
        {
            m_remoteInterpolationElapsed +=
                deltaTime;
        }

        const double rawAmount =
            m_remoteInterpolationElapsed /
            RemotePlayerInterpolationDuration;

        const float interpolationAmount =
            static_cast<float>(
                std::clamp(
                    rawAmount,
                    0.0,
                    1.0
                )
                );

        for (std::size_t playerIndex = 0;
            playerIndex < NetworkPlayerCount;
            ++playerIndex)
        {
            const NetworkPlayerState&
                startState =
                m_remoteInterpolationStart
                .players[playerIndex];

            const NetworkPlayerState&
                targetState =
                m_remoteInterpolationTarget
                .players[playerIndex];

            const float positionX =
                InterpolateFloat(
                    startState.positionX,
                    targetState.positionX,
                    interpolationAmount
                );

            const float positionY =
                InterpolateFloat(
                    startState.positionY,
                    targetState.positionY,
                    interpolationAmount
                );

            const float rotation =
                InterpolateAngle(
                    startState.rotation,
                    targetState.rotation,
                    interpolationAmount
                );

            m_gameSession.SetPlayerNetworkState(
                playerIndex,
                positionX,
                positionY,
                rotation
            );
        }

        // Projectiles have constant velocity in the
        // current prototype, so advance their visual
        // copies between authoritative snapshots.
        m_gameSession.AdvanceProjectiles(
            deltaTime
        );
    }

    void Application::ResetNetworkGameState()
        noexcept
    {
        m_networkGamePhase =
            NetworkGamePhase::Offline;

        m_connectionRecoveryRemaining =
            0.0;

        m_reconnectAttemptAccumulator =
            0.0;

        m_checkpointQueued =
            false;

        m_checkpointAppliedQueued =
            false;

        m_resumeQueued =
            false;

        m_connectionRecoveryOverlay.Reset();

        m_latestRemotePlayerInput =
            NetworkPlayerInput{};

        m_playerInputSendAccumulator =
            0.0;

        m_remoteInputAge =
            0.0;

        m_nextLocalInputSequence =
            1;

        m_clientTick =
            0;

        m_serverTick =
            0;

        m_lastProcessedRemoteInputSequence =
            0;

        m_lastAcknowledgedInputSequence =
            0;

        m_latestReceivedServerTick =
            0;

        m_latestMigrationState.players = {};

        m_latestMigrationState.
            projectiles.clear();

        m_latestMigrationState.
            nextProjectileEntityId =
            1;

        m_latestMigrationState.
            fireCooldowns = {};

        m_hasMigrationState =
            false;

        m_remoteInterpolationStart =
            NetworkWorldSnapshot{};

        m_remoteInterpolationTarget =
            NetworkWorldSnapshot{};

        m_remoteInterpolationElapsed =
            0.0;

        m_hasReceivedWorldSnapshot =
            false;
    }

    void Application::UpdateStatistics(
        double deltaTime
    )
    {
        m_statisticsTimer +=
            deltaTime;

        ++m_frameCount;

        if (m_statisticsTimer < 1.0)
        {
            return;
        }

        m_framesPerSecond =
            static_cast<double>(
                m_frameCount
                ) /
            m_statisticsTimer;

        m_frameTimeMilliseconds =
            m_framesPerSecond > 0.0
            ? 1000.0 /
            m_framesPerSecond
            : 0.0;

        m_statisticsTimer = 0.0;
        m_frameCount = 0;
    }
}