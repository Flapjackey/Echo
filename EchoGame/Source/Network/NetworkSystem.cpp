#include "Network/NetworkSystem.h"

#include <WinSock2.h>

#include <stdexcept>
#include <string>

#pragma comment(lib, "Ws2_32.lib")

namespace Echo
{
    NetworkSystem::NetworkSystem()
    {
        WSADATA socketData{};

        const int startupResult =
            WSAStartup(
                MAKEWORD(2, 2),
                &socketData
            );

        if (startupResult != 0)
        {
            throw std::runtime_error(
                "Failed to initialize Winsock. "
                "Error code: " +
                std::to_string(
                    startupResult
                )
            );
        }

        const bool supportsVersionTwoTwo =
            LOBYTE(socketData.wVersion) == 2 &&
            HIBYTE(socketData.wVersion) == 2;

        if (!supportsVersionTwoTwo)
        {
            WSACleanup();

            throw std::runtime_error(
                "Winsock 2.2 is not supported."
            );
        }

        m_initialized = true;
    }

    NetworkSystem::~NetworkSystem() noexcept
    {
        if (m_initialized)
        {
            WSACleanup();
        }
    }
}