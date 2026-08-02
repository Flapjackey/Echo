#pragma once

namespace Echo
{
    enum class NetworkGamePhase
    {
        Offline,
        Running,

        HandshakingHost,
        HandshakingClient,

        SynchronizingHost,
        SynchronizingClient,

        ConnectionRecovery
    };
}