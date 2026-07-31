#pragma once

namespace Echo
{
    class NetworkSystem final
    {
    public:
        NetworkSystem();

        ~NetworkSystem() noexcept;

        NetworkSystem(
            const NetworkSystem&
        ) = delete;

        NetworkSystem& operator=(
            const NetworkSystem&
            ) = delete;

        NetworkSystem(
            NetworkSystem&&
        ) = delete;

        NetworkSystem& operator=(
            NetworkSystem&&
            ) = delete;

    private:
        bool m_initialized = false;
    };
}