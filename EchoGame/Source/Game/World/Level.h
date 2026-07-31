#pragma once

#include <array>
#include <cstddef>
#include <vector>

namespace Echo
{
    struct LevelBlock final
    {
        float positionX = 0.0f;
        float positionY = 0.0f;

        float width = 1.0f;
        float height = 1.0f;

        float rotation = 0.0f;

        bool blocksMovement = true;
    };

    struct PlayerSpawnPoint final
    {
        float positionX = 0.0f;
        float positionY = 0.0f;
    };

    class Level final
    {
    public:
        static constexpr std::size_t
            PlayerSpawnCount = 2;

        void AddBlock(
            float positionX,
            float positionY,
            float width,
            float height,
            float rotation = 0.0f,
            bool blocksMovement = true
        );

        void SetPlayerSpawn(
            std::size_t playerIndex,
            float positionX,
            float positionY
        ) noexcept;

        const std::vector<LevelBlock>&
            GetBlocks() const noexcept;

        const PlayerSpawnPoint&
            GetPlayerSpawn(
                std::size_t playerIndex
            ) const noexcept;

    private:
        std::vector<LevelBlock> m_blocks;

        std::array<
            PlayerSpawnPoint,
            PlayerSpawnCount
        > m_playerSpawns{};
    };
}