#pragma once

namespace Echo
{
    struct PlayerCommand final
    {
        float movementX = 0.0f;
        float movementY = 0.0f;

        float aimX = 1.0f;
        float aimY = 0.0f;

        bool fire = false;
    };
}