#include "Core/Application.h"

namespace
{
    constexpr double PlayerInputSendInterval =
        1.0 / 60.0;
}

namespace Echo
{
    void Application::HandleNetworkConnected()
    {
        switch (m_applicationState)
        {
        case ApplicationState::HostGame:
        {
            BeginHostHandshake();
            break;
        }

        case ApplicationState::JoinGame:
        {
            BeginClientHandshake();
            break;
        }

        default:
        {
            break;
        }
        }
    }

    void Application::BeginHostHandshake()
    {
        m_networkGamePhase =
            NetworkGamePhase::
            HandshakingHost;

        m_connectionHelloQueued =
            false;

        m_sessionWelcomeQueued =
            false;

        m_latestRemotePlayerInput =
            NetworkPlayerInput{};

        m_connectionRecoveryOverlay.Reset();
    }

    void Application::BeginClientHandshake()
    {
        m_networkGamePhase =
            NetworkGamePhase::
            HandshakingClient;

        m_connectionHelloQueued =
            false;

        m_sessionWelcomeQueued =
            false;

        m_playerInputSendAccumulator =
            0.0;

        m_connectionRecoveryOverlay.Reset();
    }

    void Application::UpdateNetworkHandshake()
    {
        if (!m_networkSession.IsConnected())
        {
            return;
        }

        if (m_networkGamePhase ==
            NetworkGamePhase::
            HandshakingHost)
        {
            if (!m_sessionWelcomeQueued)
            {
                NetworkConnectionHello hello{};

                if (!m_networkSession.
                    TryConsumeConnectionHello(
                        hello
                    ))
                {
                    return;
                }

                if (hello.playerId ==
                    InvalidPlayerId)
                {
                    m_networkSession.Stop();

                    m_networkGamePhase =
                        NetworkGamePhase::Running;

                    RestartHostListener();
                    return;
                }

                const bool knowsAnotherSession =
                    hello.knownSessionId !=
                    InvalidSessionId &&
                    hello.knownSessionId !=
                    m_sessionId;

                if (knowsAnotherSession)
                {
                    m_networkSession.Stop();

                    m_networkGamePhase =
                        NetworkGamePhase::Running;

                    RestartHostListener();
                    return;
                }

                m_remotePlayerId =
                    hello.playerId;

                NetworkSessionWelcome welcome{};

                welcome.sessionId =
                    m_sessionId;

                welcome.hostPlayerId =
                    m_hostPlayerId;

                welcome.hostEpoch =
                    m_hostEpoch;

                welcome.assignedPlayerIndex =
                    static_cast<std::uint32_t>(
                        m_remoteNetworkPlayerIndex
                        );

                welcome.serverTick =
                    m_serverTick;

                m_networkSession.
                    QueueSessionWelcome(
                        welcome
                    );

                m_sessionWelcomeQueued =
                    true;

                return;
            }

            if (!m_networkSession.
                IsOutgoingIdle())
            {
                return;
            }

            m_sessionWelcomeQueued =
                false;

            BeginHostSynchronization();
            return;
        }

        if (m_networkGamePhase !=
            NetworkGamePhase::
            HandshakingClient)
        {
            return;
        }

        if (!m_connectionHelloQueued)
        {
            NetworkConnectionHello hello{};

            hello.knownSessionId =
                m_sessionId;

            hello.playerId =
                m_localPlayerId;

            hello.knownHostEpoch =
                m_hostEpoch;

            m_networkSession.
                QueueConnectionHello(
                    hello
                );

            m_connectionHelloQueued =
                true;

            return;
        }

        NetworkSessionWelcome welcome{};

        if (!m_networkSession.
            TryConsumeSessionWelcome(
                welcome
            ))
        {
            return;
        }

        const bool hasDifferentSession =
            m_sessionId !=
            InvalidSessionId &&
            welcome.sessionId !=
            m_sessionId;

        const bool hostIsOlder =
            m_sessionId ==
            welcome.sessionId &&
            m_hostEpoch != 0 &&
            welcome.hostEpoch <
            m_hostEpoch;

        if (hasDifferentSession ||
            hostIsOlder)
        {
            m_networkSession.Stop();

            BeginConnectionRecovery();
            return;
        }

        m_sessionId =
            welcome.sessionId;

        m_hostPlayerId =
            welcome.hostPlayerId;

        m_remotePlayerId =
            welcome.hostPlayerId;

        m_hostEpoch =
            welcome.hostEpoch;

        m_latestReceivedServerTick =
            welcome.serverTick;

        const std::size_t assignedPlayerIndex =
            static_cast<std::size_t>(
                welcome.assignedPlayerIndex
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

        // Use the identity received from the
        // authoritative host.
        m_networkSession.SetLocalIdentity(
            m_sessionId,
            m_localPlayerId,
            m_hostPlayerId,
            m_hostEpoch
        );

        m_connectionHelloQueued =
            false;

        m_sessionWelcomeQueued =
            false;

        BeginClientSynchronization();
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
}