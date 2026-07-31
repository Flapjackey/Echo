#pragma once

#include "Game/Player.h"
#include "Game/PlayerCommand.h"
#include "Game/Projectile.h"

#include <vector>

namespace Echo
{
    class GameSession final
    {
    public:
        GameSession() = default;

        void Reset();

        void Update(
            const PlayerCommand& playerCommand,
            double deltaTime
        );

        const Player& GetPlayer() const noexcept;

        const std::vector<Projectile>&
            GetProjectiles() const noexcept;

    private:
        void TryFireProjectile();

        Player m_player;

        std::vector<Projectile>
            m_projectiles;

        float m_fireCooldown = 0.0f;
    };
}