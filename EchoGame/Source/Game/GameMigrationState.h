#pragma once

#include "Core/EntityId.h"

#include <array>
#include <cstddef>
#include <vector>

namespace Echo
{
    inline constexpr std::size_t
        GameMigrationPlayerCount = 2;

    struct GameMigrationPlayerState final
    {
        float positionX = 0.0f;
        float positionY = 0.0f;
        float rotation = 0.0f;
    };

    struct GameMigrationProjectileState final
    {
        EntityId entityId =
            InvalidEntityId;

        float positionX = 0.0f;
        float positionY = 0.0f;
        float rotation = 0.0f;

        float remainingLifetime = 0.0f;
    };

    struct GameMigrationState final
    {
        std::array<
            GameMigrationPlayerState,
            GameMigrationPlayerCount
        > players{};

        std::vector<
            GameMigrationProjectileState
        > projectiles;

        EntityId nextProjectileEntityId =
            1;

        std::array<
            float,
            GameMigrationPlayerCount
        > fireCooldowns{};
    };
}