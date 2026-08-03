#include "Core/Application.h"

#include <random>
#include <algorithm>
#include <cstdint>

namespace
{
    constexpr unsigned int ClientWidth =
        1280;

    constexpr unsigned int ClientHeight =
        720;

    constexpr std::uint32_t
        SnapshotTickInterval =
        2;
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

            const bool networkGameplayRunning =
                UpdateNetworkConnectionFrame(
                    frameTime
                );

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

            UpdateNetworkGameplayFrame(
                frameTime,
                networkGameplayRunning
            );

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

            RenderFrame();

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