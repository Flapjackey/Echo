#include "Core/Application.h"

#include <cmath>

namespace
{
    constexpr double PlayerInputSendInterval =
        1.0 / 60.0;

    constexpr double RemoteInputTimeout =
        0.25;
}

namespace Echo
{
    bool Application::UpdateNetworkConnectionFrame(
        double frameTime
    )
    {
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

        return
            m_networkGamePhase ==
            NetworkGamePhase::Running;
    }

    void Application::UpdateNetworkGameplayFrame(
        double frameTime,
        bool networkGameplayRunning
    )
    {
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

            NetworkWorldSnapshot
                receivedSnapshot{};

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
    }
}