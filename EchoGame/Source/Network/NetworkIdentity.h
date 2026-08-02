#pragma once

#include <cstdint>

namespace Echo
{
    using SessionId =
        std::uint64_t;

    using PlayerId =
        std::uint64_t;

    inline constexpr SessionId
        InvalidSessionId = 0;

    inline constexpr PlayerId
        InvalidPlayerId = 0;

    struct NetworkConnectionHello final
    {
        SessionId knownSessionId =
            InvalidSessionId;

        PlayerId playerId =
            InvalidPlayerId;

        std::uint32_t knownHostEpoch =
            0;
    };

    struct NetworkSessionWelcome final
    {
        SessionId sessionId =
            InvalidSessionId;

        PlayerId hostPlayerId =
            InvalidPlayerId;

        std::uint32_t hostEpoch =
            0;

        std::uint32_t assignedPlayerIndex =
            0;

        std::uint32_t serverTick =
            0;
    };
}