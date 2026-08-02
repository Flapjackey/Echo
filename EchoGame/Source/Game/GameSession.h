#pragma once

#include "Game/Player.h"
#include "Game/PlayerCommand.h"
#include "Game/Projectile.h"
#include "Game/World/Level.h"

#include <array>
#include <cstddef>
#include <vector>

namespace Echo
{
    class GameSession final
    {
    public:
        static constexpr std::size_t
            PlayerCount = 2;

        using PlayerCommands =
            std::array<
            PlayerCommand,
            PlayerCount
            >;

        GameSession() = default;

        void Reset();

        void Update(
            const PlayerCommands& playerCommands,
            double deltaTime
        );

        void SetPlayerNetworkState(
            std::size_t playerIndex,
            float positionX,
            float positionY,
            float rotation
        ) noexcept;

        const Player& GetPlayer(
            std::size_t playerIndex
        ) const noexcept;

        const std::array<
            Player,
            PlayerCount
        >& GetPlayers() const noexcept;

        const Level&
            GetLevel() const noexcept;

        const std::vector<Projectile>&
            GetProjectiles() const noexcept;

        void SetProjectiles(
            std::vector<Projectile> projectiles
        );

    private:
        void TryFireProjectile(
            std::size_t playerIndex
        );

        Level m_level;

        std::array<
            Player,
            PlayerCount
        > m_players{};

        std::vector<Projectile>
            m_projectiles;

        std::array<
            float,
            PlayerCount
        > m_fireCooldowns{};
    };
}