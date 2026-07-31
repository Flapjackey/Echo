#include "Game/GameSession.h"

#include <algorithm>

namespace Echo
{
    void GameSession::Reset()
    {
        m_player =
            Player{};

        m_projectiles.clear();

        m_fireCooldown =
            0.0f;
    }

    void GameSession::Update(
        const PlayerCommand& playerCommand,
        double deltaTime
    )
    {
        m_player.Update(
            playerCommand,
            deltaTime
        );

        const float fixedDeltaTime =
            static_cast<float>(
                deltaTime
                );

        if (m_fireCooldown > 0.0f)
        {
            m_fireCooldown -=
                fixedDeltaTime;
        }

        if (playerCommand.fire)
        {
            TryFireProjectile();
        }

        for (Projectile& projectile :
            m_projectiles)
        {
            projectile.Update(
                deltaTime
            );
        }

        const auto firstExpiredProjectile =
            std::remove_if(
                m_projectiles.begin(),
                m_projectiles.end(),
                [](
                    const Projectile& projectile
                    )
                {
                    return
                        !projectile.IsAlive();
                }
            );

        m_projectiles.erase(
            firstExpiredProjectile,
            m_projectiles.end()
        );
    }

    const Player&
        GameSession::GetPlayer() const noexcept
    {
        return m_player;
    }

    const std::vector<Projectile>&
        GameSession::GetProjectiles() const noexcept
    {
        return m_projectiles;
    }

    void GameSession::TryFireProjectile()
    {
        if (m_fireCooldown > 0.0f)
        {
            return;
        }

        constexpr float FireInterval =
            0.15f;

        constexpr float SpawnDistance =
            0.32f;

        const float forwardX =
            m_player.GetForwardX();

        const float forwardY =
            m_player.GetForwardY();

        const float spawnX =
            m_player.GetPositionX() +
            forwardX *
            SpawnDistance;

        const float spawnY =
            m_player.GetPositionY() +
            forwardY *
            SpawnDistance;

        m_projectiles.emplace_back(
            spawnX,
            spawnY,
            forwardX,
            forwardY
        );

        m_fireCooldown =
            FireInterval;
    }
}