#include "Core/Application.h"

#include <cstdint>

namespace
{
    constexpr std::uint32_t
        SnapshotTickInterval =
        2;
}

namespace Echo
{
    bool Application::UpdateSimulationFrame(
        double frameTime,
        double fixedDeltaTime,
        double& accumulatedTime,
        bool networkGameplayRunning
    )
    {
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
            accumulatedTime =
                0.0;
        }

        if (isConnectedHost &&
            shouldSendWorldSnapshot)
        {
            m_networkSession.QueueWorldSnapshot(
                BuildWorldSnapshot()
            );
        }

        return isSimulationRunning;
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
}