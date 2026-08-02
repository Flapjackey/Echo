#include "Game/World/Levels/TestArena.h"

namespace Echo
{
    Level CreateTestArena()
    {
        Level level;

        // Player spawn points.
        level.SetPlayerSpawn(
            0,
            -0.80f,
            0.0f
        );

        level.SetPlayerSpawn(
            1,
            0.80f,
            0.0f
        );

        // Arena boundary: top.
        level.AddBlock(
            0.0f,
            2.00f,
            6.10f,
            0.12f
        );

        // Arena boundary: bottom.
        level.AddBlock(
            0.0f,
            -2.00f,
            6.10f,
            0.12f
        );

        // Arena boundary: left.
        level.AddBlock(
            -3.00f,
            0.0f,
            0.12f,
            4.12f
        );

        // Arena boundary: right.
        level.AddBlock(
            3.00f,
            0.0f,
            0.12f,
            4.12f
        );

        // Upper-left horizontal wall.
        level.AddBlock(
            -1.35f,
            0.80f,
            1.20f,
            0.18f
        );

        // Lower-right horizontal wall.
        level.AddBlock(
            1.30f,
            -0.80f,
            1.20f,
            0.18f
        );

        // Central vertical cover.
        level.AddBlock(
            0.0f,
            0.0f,
            0.18f,
            1.20f
        );

        // Upper-right vertical wall.
        level.AddBlock(
            1.65f,
            0.85f,
            0.18f,
            0.90f
        );

        // Lower-left vertical wall.
        level.AddBlock(
            -1.65f,
            -0.85f,
            0.18f,
            0.90f
        );

        // Decorative Player 1 spawn marker.
        level.AddBlock(
            -0.80f,
            0.0f,
            0.24f,
            0.24f,
            0.785398f,
            false
        );

        // Decorative Player 2 spawn marker.
        level.AddBlock(
            0.80f,
            0.0f,
            0.24f,
            0.24f,
            0.785398f,
            false
        );

        return level;
    }
}