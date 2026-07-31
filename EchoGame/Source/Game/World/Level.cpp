#include "Game/World/Level.h"

namespace Echo
{
    void Level::AddBlock(
        float positionX,
        float positionY,
        float width,
        float height,
        float rotation,
        bool blocksMovement
    )
    {
        LevelBlock block{};

        block.positionX =
            positionX;

        block.positionY =
            positionY;

        block.width =
            width;

        block.height =
            height;

        block.rotation =
            rotation;

        block.blocksMovement =
            blocksMovement;

        m_blocks.push_back(
            block
        );
    }

    void Level::SetPlayerSpawn(
        std::size_t playerIndex,
        float positionX,
        float positionY
    ) noexcept
    {
        if (playerIndex >=
            m_playerSpawns.size())
        {
            return;
        }

        m_playerSpawns[playerIndex] =
            PlayerSpawnPoint
        {
            positionX,
            positionY
        };
    }

    const std::vector<LevelBlock>&
        Level::GetBlocks() const noexcept
    {
        return m_blocks;
    }

    const PlayerSpawnPoint&
        Level::GetPlayerSpawn(
            std::size_t playerIndex
        ) const noexcept
    {
        if (playerIndex >=
            m_playerSpawns.size())
        {
            return m_playerSpawns[0];
        }

        return
            m_playerSpawns[playerIndex];
    }
}