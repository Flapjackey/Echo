#include "Core/Application.h"

#include <algorithm>
#include <cstdint>
#include <limits>

namespace
{
    constexpr std::uint16_t LocalNetworkPort =
        27015;

    constexpr double ConnectionRecoveryDuration =
        30.0;

    constexpr double ReconnectAttemptInterval =
        1.0;
}

namespace Echo
{
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

        if (m_sessionId ==
            InvalidSessionId ||
            m_localPlayerId ==
            InvalidPlayerId ||
            m_hostPlayerId ==
            InvalidPlayerId ||
            m_hostEpoch == 0)
        {
            return;
        }

        const std::uint32_t maximumHostEpoch =
            std::numeric_limits<
            std::uint32_t
            >::max();

        // Wrapping the epoch back to 1 would make
        // the new host look older than previous hosts.
        if (m_hostEpoch ==
            maximumHostEpoch)
        {
            return;
        }

        // Preserve the authoritative world backup.
        const GameMigrationState migrationState =
            m_latestMigrationState;

        const std::uint32_t restoredServerTick =
            m_latestReceivedServerTick;

        // Preserve session identity before
        // ResetNetworkGameState clears runtime data.
        const SessionId preservedSessionId =
            m_sessionId;

        const PlayerId previousHostPlayerId =
            m_hostPlayerId;

        const std::uint32_t promotedHostEpoch =
            m_hostEpoch + 1;

        m_networkSession.Stop();

        ResetNetworkGameState();

        // Restore the same game session.
        m_sessionId =
            preservedSessionId;

        // The former host becomes the remote player.
        m_remotePlayerId =
            previousHostPlayerId;

        // This process is now the authoritative host.
        m_hostPlayerId =
            m_localPlayerId;

        m_hostEpoch =
            promotedHostEpoch;

        m_gameSession.RestoreMigrationState(
            migrationState
        );

        m_serverTick =
            restoredServerTick;

        // Player ownership is intentionally preserved.
        // The former Join remains the owner of the same
        // player after becoming Host.
        m_latestRemotePlayerInput =
            NetworkPlayerInput{};

        m_networkSession.SetLocalIdentity(
            m_sessionId,
            m_localPlayerId,
            m_hostPlayerId,
            m_hostEpoch
        );

        m_networkSession.StartHost(
            LocalNetworkPort
        );

        m_networkGamePhase =
            NetworkGamePhase::Running;

        m_connectionRecoveryRemaining =
            0.0;

        m_reconnectAttemptAccumulator =
            0.0;

        m_connectionHelloQueued =
            false;

        m_sessionWelcomeQueued =
            false;

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
}