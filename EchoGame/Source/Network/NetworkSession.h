#pragma once

#include "Network/NetworkProtocol.h"

#include <cstddef>
#include <cstdint>
#include <string>

namespace Echo
{
    enum class NetworkSessionMode
    {
        None,
        Host,
        Client
    };

    enum class NetworkSessionStatus
    {
        Disconnected,
        Listening,
        Connecting,
        Connected,
        Error
    };

    class NetworkSession final
    {
    public:
        NetworkSession() = default;

        ~NetworkSession() noexcept;

        NetworkSession(
            const NetworkSession&
        ) = delete;

        NetworkSession& operator=(
            const NetworkSession&
            ) = delete;

        NetworkSession(
            NetworkSession&&
        ) = delete;

        NetworkSession& operator=(
            NetworkSession&&
            ) = delete;

        void StartHost(
            std::uint16_t port
        );

        void StartClient(
            std::uint16_t port
        );

        void Update();

        void Stop();

        void QueuePlayerInput(
            const NetworkPlayerInput& input
        ) noexcept;

        void QueueWorldSnapshot(
            const NetworkWorldSnapshot& snapshot
        ) noexcept;

        bool TryConsumePlayerInput(
            NetworkPlayerInput& input
        ) noexcept;

        bool TryConsumeWorldSnapshot(
            NetworkWorldSnapshot& snapshot
        ) noexcept;

        bool IsConnected() const noexcept;

        NetworkSessionMode
            GetMode() const noexcept;

        NetworkSessionStatus
            GetStatus() const noexcept;

        const std::wstring&
            GetStatusMessage()
            const noexcept;

    private:
        using SocketHandle =
            std::uintptr_t;

        static constexpr SocketHandle
            InvalidSocket =
            static_cast<SocketHandle>(
                -1
                );

        void UpdateHost();
        void UpdateClient();
        void UpdateConnectedState();

        void QueuePacket(
            const NetworkPacket& packet
        ) noexcept;

        void FlushOutgoingData();
        void ReceiveIncomingData();

        void ResetTransferState() noexcept;

        void CloseSockets() noexcept;

        void SetError(
            const wchar_t* operation,
            int errorCode
        );

        NetworkSessionMode m_mode =
            NetworkSessionMode::None;

        NetworkSessionStatus m_status =
            NetworkSessionStatus::
            Disconnected;

        SocketHandle m_listenSocket =
            InvalidSocket;

        SocketHandle m_connectionSocket =
            InvalidSocket;

        std::wstring m_statusMessage =
            L"Disconnected";

        NetworkPacket
            m_pendingSendPacket{};

        std::size_t m_pendingSendOffset = 0;

        bool m_hasPendingSendPacket = false;

        NetworkPacket
            m_queuedSendPacket{};

        bool m_hasQueuedSendPacket = false;

        NetworkPacket
            m_receivePacket{};

        std::size_t m_receiveOffset = 0;

        NetworkPlayerInput
            m_latestReceivedPlayerInput{};

        bool m_hasReceivedPlayerInput = false;

        NetworkWorldSnapshot
            m_latestReceivedWorldSnapshot{};

        bool m_hasReceivedWorldSnapshot = false;

        std::uint32_t m_nextSequence = 1;
    };
}