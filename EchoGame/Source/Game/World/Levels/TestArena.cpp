#include "Game/World/Levels/TestArena.h"

namespace Echo
{
    Level CreateTestArena()
    {
        Level level;

        // Player spawn points.
        level.SetPlayerSpawn(
            0,
            -1.05f,
            0.0f
        );

        level.SetPlayerSpawn(
            1,
            1.05f,
            0.0f
        );

        // Arena boundary: top.
        level.AddBlock(
            0.0f,
            0.88f,
            3.30f,
            0.10f
        );

        // Arena boundary: bottom.
        level.AddBlock(
            0.0f,
            -0.88f,
            3.30f,
            0.10f
        );

        // Arena boundary: left.
        level.AddBlock(
            -1.68f,
            0.0f,
            0.10f,
            1.85f
        );

        // Arena boundary: right.
        level.AddBlock(
            1.68f,
            0.0f,
            0.10f,
            1.85f
        );

        // Upper test obstacle.
        level.AddBlock(
            -0.65f,
            0.42f,
            0.55f,
            0.14f
        );

        // Lower test obstacle.
        level.AddBlock(
            0.65f,
            -0.42f,
            0.55f,
            0.14f
        );

        // Central vertical cover.
        level.AddBlock(
            0.0f,
            0.0f,
            0.16f,
            0.62f
        );

        // Player 1 spawn marker.
        // It is decorative and will not block movement.
        level.AddBlock(
            -1.05f,
            0.0f,
            0.24f,
            0.24f,
            0.785398f,
            false
        );

        // Player 2 spawn marker.
        level.AddBlock(
            1.05f,
            0.0f,
            0.24f,
            0.24f,
            0.785398f,
            false
        );

        return level;
    }
}