#include "Network/NetworkSession.h"

#include <WinSock2.h>

#include <string>

namespace
{
    SOCKET ToNativeSocket(
        std::uintptr_t socketHandle
    ) noexcept
    {
        return static_cast<SOCKET>(
            socketHandle
            );
    }

    std::uintptr_t FromNativeSocket(
        SOCKET socketHandle
    ) noexcept
    {
        return static_cast<std::uintptr_t>(
            socketHandle
            );
    }

    bool SetSocketNonBlocking(
        SOCKET socketHandle
    ) noexcept
    {
        u_long nonBlockingMode = 1;

        return ioctlsocket(
            socketHandle,
            FIONBIO,
            &nonBlockingMode
        ) != SOCKET_ERROR;
    }

    bool IsConnectionInProgress(
        int errorCode
    ) noexcept
    {
        return
            errorCode == WSAEWOULDBLOCK ||
            errorCode == WSAEINPROGRESS ||
            errorCode == WSAEALREADY;
    }
}

namespace Echo
{
    NetworkSession::~NetworkSession()
        noexcept
    {
        CloseSockets();
    }

    void NetworkSession::StartHost(
        std::uint16_t port
    )
    {
        Stop();

        m_mode =
            NetworkSessionMode::Host;

        const SOCKET listenSocket =
            socket(
                AF_INET,
                SOCK_STREAM,
                IPPROTO_TCP
            );

        if (listenSocket ==
            INVALID_SOCKET)
        {
            SetError(
                L"Failed to create host socket.",
                WSAGetLastError()
            );

            return;
        }

        m_listenSocket =
            FromNativeSocket(
                listenSocket
            );

        if (!SetSocketNonBlocking(
            listenSocket))
        {
            SetError(
                L"Failed to make host socket "
                L"non-blocking.",
                WSAGetLastError()
            );

            return;
        }

        sockaddr_in address{};

        address.sin_family =
            AF_INET;

        address.sin_addr.s_addr =
            htonl(
                INADDR_LOOPBACK
            );

        address.sin_port =
            htons(port);

        const int bindResult =
            bind(
                listenSocket,
                reinterpret_cast<
                const sockaddr*
                >(&address),
                sizeof(address)
            );

        if (bindResult ==
            SOCKET_ERROR)
        {
            SetError(
                L"Failed to bind host socket.",
                WSAGetLastError()
            );

            return;
        }

        const int listenResult =
            listen(
                listenSocket,
                SOMAXCONN
            );

        if (listenResult ==
            SOCKET_ERROR)
        {
            SetError(
                L"Failed to listen for clients.",
                WSAGetLastError()
            );

            return;
        }

        m_status =
            NetworkSessionStatus::Listening;

        m_statusMessage =
            L"Waiting for client on "
            L"127.0.0.1:" +
            std::to_wstring(port);
    }

    void NetworkSession::StartClient(
        std::uint16_t port
    )
    {
        Stop();

        m_mode =
            NetworkSessionMode::Client;

        const SOCKET connectionSocket =
            socket(
                AF_INET,
                SOCK_STREAM,
                IPPROTO_TCP
            );

        if (connectionSocket ==
            INVALID_SOCKET)
        {
            SetError(
                L"Failed to create client socket.",
                WSAGetLastError()
            );

            return;
        }

        m_connectionSocket =
            FromNativeSocket(
                connectionSocket
            );

        if (!SetSocketNonBlocking(
            connectionSocket))
        {
            SetError(
                L"Failed to make client socket "
                L"non-blocking.",
                WSAGetLastError()
            );

            return;
        }

        sockaddr_in address{};

        address.sin_family =
            AF_INET;

        address.sin_addr.s_addr =
            htonl(
                INADDR_LOOPBACK
            );

        address.sin_port =
            htons(port);

        const int connectionResult =
            connect(
                connectionSocket,
                reinterpret_cast<
                const sockaddr*
                >(&address),
                sizeof(address)
            );

        if (connectionResult == 0)
        {
            m_status =
                NetworkSessionStatus::Connected;

            m_statusMessage =
                L"Connected to host";

            return;
        }

        const int errorCode =
            WSAGetLastError();

        if (!IsConnectionInProgress(
            errorCode))
        {
            SetError(
                L"Failed to connect to host.",
                errorCode
            );

            return;
        }

        m_status =
            NetworkSessionStatus::Connecting;

        m_statusMessage =
            L"Connecting to "
            L"127.0.0.1:" +
            std::to_wstring(port);
    }

    void NetworkSession::Update()
    {
        switch (m_mode)
        {
        case NetworkSessionMode::None:
        {
            break;
        }

        case NetworkSessionMode::Host:
        {
            UpdateHost();
            break;
        }

        case NetworkSessionMode::Client:
        {
            UpdateClient();
            break;
        }
        }
    }

    void NetworkSession::QueuePlayerInput(
        const NetworkPlayerInput& input
    ) noexcept
    {
        if (!IsConnected())
        {
            return;
        }

        const NetworkPacket packet =
            CreatePlayerInputPacket(
                input,
                m_nextSequence
            );

        ++m_nextSequence;

        QueuePacket(
            packet
        );
    }

    void NetworkSession::QueueWorldSnapshot(
        const NetworkWorldSnapshot& snapshot
    ) noexcept
    {
        if (!IsConnected())
        {
            return;
        }

        const NetworkPacket packet =
            CreateWorldSnapshotPacket(
                snapshot,
                m_nextSequence
            );

        ++m_nextSequence;

        QueuePacket(
            packet
        );
    }

    bool NetworkSession::TryConsumePlayerInput(
        NetworkPlayerInput& input
    ) noexcept
    {
        if (!m_hasReceivedPlayerInput)
        {
            return false;
        }

        input =
            m_latestReceivedPlayerInput;

        m_hasReceivedPlayerInput =
            false;

        return true;
    }

    bool NetworkSession::TryConsumeWorldSnapshot(
        NetworkWorldSnapshot& snapshot
    ) noexcept
    {
        if (!m_hasReceivedWorldSnapshot)
        {
            return false;
        }

        snapshot =
            m_latestReceivedWorldSnapshot;

        m_hasReceivedWorldSnapshot =
            false;

        return true;
    }

    void NetworkSession::QueuePacket(
        const NetworkPacket& packet
    ) noexcept
    {
        // Nothing from the current packet was sent.
        // Replace it with the newest packet.
        if (!m_hasPendingSendPacket ||
            m_pendingSendOffset == 0)
        {
            m_pendingSendPacket =
                packet;

            m_pendingSendOffset = 0;

            m_hasPendingSendPacket = true;

            return;
        }

        // Part of the current packet was already sent.
        // Preserve it and queue only the newest packet.
        m_queuedSendPacket =
            packet;

        m_hasQueuedSendPacket =
            true;
    }

    bool NetworkSession::IsConnected()
        const noexcept
    {
        return
            m_status ==
            NetworkSessionStatus::Connected;
    }

    void NetworkSession::Stop()
    {
        CloseSockets();
        ResetTransferState();

        m_mode =
            NetworkSessionMode::None;

        m_status =
            NetworkSessionStatus::
            Disconnected;

        m_statusMessage =
            L"Disconnected";
    }

    NetworkSessionMode
        NetworkSession::GetMode()
        const noexcept
    {
        return m_mode;
    }

    NetworkSessionStatus
        NetworkSession::GetStatus()
        const noexcept
    {
        return m_status;
    }

    const std::wstring&
        NetworkSession::GetStatusMessage()
        const noexcept
    {
        return m_statusMessage;
    }

    void NetworkSession::UpdateHost()
    {
        if (m_status ==
            NetworkSessionStatus::Listening)
        {
            const SOCKET listenSocket =
                ToNativeSocket(
                    m_listenSocket
                );

            const SOCKET acceptedSocket =
                accept(
                    listenSocket,
                    nullptr,
                    nullptr
                );

            if (acceptedSocket ==
                INVALID_SOCKET)
            {
                const int errorCode =
                    WSAGetLastError();

                if (errorCode ==
                    WSAEWOULDBLOCK)
                {
                    return;
                }

                SetError(
                    L"Failed to accept client.",
                    errorCode
                );

                return;
            }

            if (!SetSocketNonBlocking(
                acceptedSocket))
            {
                const int errorCode =
                    WSAGetLastError();

                closesocket(
                    acceptedSocket
                );

                SetError(
                    L"Failed to make accepted "
                    L"socket non-blocking.",
                    errorCode
                );

                return;
            }

            m_connectionSocket =
                FromNativeSocket(
                    acceptedSocket
                );

            closesocket(
                listenSocket
            );

            m_listenSocket =
                InvalidSocket;

            m_status =
                NetworkSessionStatus::Connected;

            m_statusMessage =
                L"Client connected";
        }

        if (m_status ==
            NetworkSessionStatus::Connected)
        {
            UpdateConnectedState();
        }
    }

    void NetworkSession::UpdateClient()
    {
        if (m_status ==
            NetworkSessionStatus::Connecting)
        {
            const SOCKET connectionSocket =
                ToNativeSocket(
                    m_connectionSocket
                );

            fd_set writableSockets{};
            FD_ZERO(
                &writableSockets
            );
            FD_SET(
                connectionSocket,
                &writableSockets
            );

            fd_set errorSockets{};
            FD_ZERO(
                &errorSockets
            );
            FD_SET(
                connectionSocket,
                &errorSockets
            );

            timeval timeout{};

            timeout.tv_sec = 0;
            timeout.tv_usec = 0;

            const int selectResult =
                select(
                    0,
                    nullptr,
                    &writableSockets,
                    &errorSockets,
                    &timeout
                );

            if (selectResult ==
                SOCKET_ERROR)
            {
                SetError(
                    L"Failed while checking "
                    L"connection state.",
                    WSAGetLastError()
                );

                return;
            }

            if (selectResult == 0)
            {
                return;
            }

            int socketError = 0;
            int socketErrorSize =
                sizeof(socketError);

            const int optionResult =
                getsockopt(
                    connectionSocket,
                    SOL_SOCKET,
                    SO_ERROR,
                    reinterpret_cast<char*>(
                        &socketError
                        ),
                    &socketErrorSize
                );

            if (optionResult ==
                SOCKET_ERROR)
            {
                SetError(
                    L"Failed to read connection "
                    L"result.",
                    WSAGetLastError()
                );

                return;
            }

            if (socketError != 0)
            {
                SetError(
                    L"Failed to connect to host.",
                    socketError
                );

                return;
            }

            m_status =
                NetworkSessionStatus::Connected;

            m_statusMessage =
                L"Connected to host";
        }

        if (m_status ==
            NetworkSessionStatus::Connected)
        {
            UpdateConnectedState();
        }
    }

    void NetworkSession::UpdateConnectedState()
    {
        FlushOutgoingData();

        if (!IsConnected())
        {
            return;
        }

        ReceiveIncomingData();
    }

    void NetworkSession::FlushOutgoingData()
    {
        if (!m_hasPendingSendPacket)
        {
            return;
        }

        const SOCKET connectionSocket =
            ToNativeSocket(
                m_connectionSocket
            );

        while (m_hasPendingSendPacket)
        {
            const char* packetBytes =
                reinterpret_cast<const char*>(
                    &m_pendingSendPacket
                    );

            const std::size_t packetSize =
                sizeof(
                    m_pendingSendPacket
                    );

            const std::size_t remainingSize =
                packetSize -
                m_pendingSendOffset;

            const int sendResult =
                send(
                    connectionSocket,
                    packetBytes +
                    m_pendingSendOffset,
                    static_cast<int>(
                        remainingSize
                        ),
                    0
                );

            if (sendResult ==
                SOCKET_ERROR)
            {
                const int errorCode =
                    WSAGetLastError();

                if (errorCode ==
                    WSAEWOULDBLOCK)
                {
                    return;
                }

                SetError(
                    L"Failed to send network packet.",
                    errorCode
                );

                return;
            }

            if (sendResult <= 0)
            {
                return;
            }

            m_pendingSendOffset +=
                static_cast<std::size_t>(
                    sendResult
                    );

            if (m_pendingSendOffset <
                packetSize)
            {
                return;
            }

            m_pendingSendOffset = 0;

            if (m_hasQueuedSendPacket)
            {
                m_pendingSendPacket =
                    m_queuedSendPacket;

                m_hasQueuedSendPacket =
                    false;

                continue;
            }

            m_hasPendingSendPacket =
                false;
        }
    }

    void NetworkSession::ReceiveIncomingData()
    {
        const SOCKET connectionSocket =
            ToNativeSocket(
                m_connectionSocket
            );

        while (IsConnected())
        {
            char* packetBytes =
                reinterpret_cast<char*>(
                    &m_receivePacket
                    );

            const std::size_t packetSize =
                sizeof(
                    m_receivePacket
                    );

            const std::size_t remainingSize =
                packetSize -
                m_receiveOffset;

            const int receiveResult =
                recv(
                    connectionSocket,
                    packetBytes +
                    m_receiveOffset,
                    static_cast<int>(
                        remainingSize
                        ),
                    0
                );

            if (receiveResult == 0)
            {
                CloseSockets();
                ResetTransferState();

                m_status =
                    NetworkSessionStatus::
                    Disconnected;

                m_statusMessage =
                    L"Other process disconnected";

                return;
            }

            if (receiveResult ==
                SOCKET_ERROR)
            {
                const int errorCode =
                    WSAGetLastError();

                if (errorCode ==
                    WSAEWOULDBLOCK)
                {
                    return;
                }

                SetError(
                    L"Failed to receive network data.",
                    errorCode
                );

                return;
            }

            m_receiveOffset +=
                static_cast<std::size_t>(
                    receiveResult
                    );

            if (m_receiveOffset <
                packetSize)
            {
                return;
            }

            if (!HasValidNetworkHeader(
                m_receivePacket
            ))
            {
                SetError(
                    L"Received an invalid packet.",
                    0
                );

                return;
            }

            switch (m_receivePacket.type)
            {
            case NetworkPacketType::PlayerInput:
            {
                NetworkPlayerInput input{};

                if (!DecodePlayerInputPacket(
                    m_receivePacket,
                    input
                ))
                {
                    SetError(
                        L"Failed to decode "
                        L"player input.",
                        0
                    );

                    return;
                }

                m_latestReceivedPlayerInput =
                    input;

                m_hasReceivedPlayerInput =
                    true;

                break;
            }

            case NetworkPacketType::WorldSnapshot:
            {
                NetworkWorldSnapshot snapshot{};

                if (!DecodeWorldSnapshotPacket(
                    m_receivePacket,
                    snapshot
                ))
                {
                    SetError(
                        L"Failed to decode "
                        L"world snapshot.",
                        0
                    );

                    return;
                }

                m_latestReceivedWorldSnapshot =
                    snapshot;

                m_hasReceivedWorldSnapshot =
                    true;

                break;
            }

            default:
            {
                SetError(
                    L"Received an unknown "
                    L"packet type.",
                    0
                );

                return;
            }
            }

            m_receivePacket =
                NetworkPacket{};

            m_receiveOffset = 0;
        }
    }

    void NetworkSession::ResetTransferState()
        noexcept
    {
        m_pendingSendPacket =
            NetworkPacket{};

        m_pendingSendOffset = 0;
        m_hasPendingSendPacket = false;

        m_queuedSendPacket =
            NetworkPacket{};

        m_hasQueuedSendPacket = false;

        m_receivePacket =
            NetworkPacket{};

        m_receiveOffset = 0;

        m_latestReceivedPlayerInput =
            NetworkPlayerInput{};

        m_hasReceivedPlayerInput = false;

        m_latestReceivedWorldSnapshot =
            NetworkWorldSnapshot{};

        m_hasReceivedWorldSnapshot = false;

        m_nextSequence = 1;
    }

    void NetworkSession::CloseSockets()
        noexcept
    {
        if (m_connectionSocket !=
            InvalidSocket)
        {
            closesocket(
                ToNativeSocket(
                    m_connectionSocket
                )
            );

            m_connectionSocket =
                InvalidSocket;
        }

        if (m_listenSocket !=
            InvalidSocket)
        {
            closesocket(
                ToNativeSocket(
                    m_listenSocket
                )
            );

            m_listenSocket =
                InvalidSocket;
        }
    }

    void NetworkSession::SetError(
        const wchar_t* operation,
        int errorCode
    )
    {
        CloseSockets();
        ResetTransferState();

        m_status =
            NetworkSessionStatus::Error;

        m_statusMessage =
            std::wstring(operation) +
            L" Error code: " +
            std::to_wstring(
                errorCode
            );
    }
}