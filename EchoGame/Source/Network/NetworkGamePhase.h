#pragma once

namespace Echo
{
    enum class NetworkGamePhase
    {
        Offline,
        Running,
        ConnectionRecovery,
        SynchronizingHost,
        SynchronizingClient
    };
}