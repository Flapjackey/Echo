#include "Game/GameSession.h"

#include "Game/World/Levels/TestArena.h"

#include <algorithm>
#include <utility>

namespace Echo
{
    void GameSession::Reset()
    {
        m_level =
            CreateTestArena();

        for (std::size_t playerIndex = 0;
            playerIndex < PlayerCount;
            ++playerIndex)
        {
            const PlayerSpawnPoint& spawn =
                m_level.GetPlayerSpawn(
                    playerIndex
                );

            m_players[playerIndex] =
                Player(
                    spawn.positionX,
                    spawn.positionY
                );
        }

        m_projectiles.clear();

        m_fireCooldowns.fill(
            0.0f
        );
    }

    void GameSession::Update(
        const PlayerCommands& playerCommands,
        double deltaTime
    )
    {
        for (std::size_t playerIndex = 0;
            playerIndex < PlayerCount;
            ++playerIndex)
        {
            m_players[playerIndex].Update(
                playerCommands[playerIndex],
                deltaTime
            );
        }

        const float fixedDeltaTime =
            static_cast<float>(
                deltaTime
                );

        for (float& fireCooldown :
            m_fireCooldowns)
        {
            if (fireCooldown > 0.0f)
            {
                fireCooldown -=
                    fixedDeltaTime;
            }
        }

        for (std::size_t playerIndex = 0;
            playerIndex < PlayerCount;
            ++playerIndex)
        {
            if (playerCommands[playerIndex].fire)
            {
                TryFireProjectile(
                    playerIndex
                );
            }
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

    void GameSession::SetPlayerNetworkState(
        std::size_t playerIndex,
        float positionX,
        float positionY,
        float rotation
    ) noexcept
    {
        if (playerIndex >= PlayerCount)
        {
            return;
        }

        m_players[playerIndex].SetNetworkState(
            positionX,
            positionY,
            rotation
        );
    }

    const Player& GameSession::GetPlayer(
        std::size_t playerIndex
    ) const noexcept
    {
        if (playerIndex >= PlayerCount)
        {
            return m_players[0];
        }

        return m_players[playerIndex];
    }

    const std::array<
        Player,
        GameSession::PlayerCount
    >& GameSession::GetPlayers() const noexcept
    {
        return m_players;
    }

    const Level&
        GameSession::GetLevel() const noexcept
    {
        return m_level;
    }

    const std::vector<Projectile>&
        GameSession::GetProjectiles() const noexcept
    {
        return m_projectiles;
    }

    void GameSession::SetProjectiles(
        std::vector<Projectile> projectiles
    )
    {
        m_projectiles =
            std::move(
                projectiles
            );
    }

    void GameSession::TryFireProjectile(
        std::size_t playerIndex
    )
    {
        if (playerIndex >= PlayerCount)
        {
            return;
        }

        float& fireCooldown =
            m_fireCooldowns[playerIndex];

        if (fireCooldown > 0.0f)
        {
            return;
        }

        constexpr float FireInterval =
            0.15f;

        constexpr float SpawnDistance =
            0.32f;

        const Player& player =
            m_players[playerIndex];

        const float forwardX =
            player.GetForwardX();

        const float forwardY =
            player.GetForwardY();

        const float spawnX =
            player.GetPositionX() +
            forwardX *
            SpawnDistance;

        const float spawnY =
            player.GetPositionY() +
            forwardY *
            SpawnDistance;

        m_projectiles.emplace_back(
            spawnX,
            spawnY,
            forwardX,
            forwardY
        );

        fireCooldown =
            FireInterval;
    }
}