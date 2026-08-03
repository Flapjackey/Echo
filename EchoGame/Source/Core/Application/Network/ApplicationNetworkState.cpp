#include "Core/Application.h"

namespace Echo
{
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

        m_connectionHelloQueued =
            false;

        m_sessionWelcomeQueued =
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

        m_latestMigrationState.players =
        {};

        m_latestMigrationState.
            projectiles.clear();

        m_latestMigrationState.
            nextProjectileEntityId =
            1;

        m_latestMigrationState.
            fireCooldowns =
        {};

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
}