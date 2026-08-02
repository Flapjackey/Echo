#pragma once

#include "Game/World/Level.h"

namespace Echo
{
    struct ResolvedWorldPosition final
    {
        float positionX = 0.0f;
        float positionY = 0.0f;
    };

    ResolvedWorldPosition
        MoveCircleAgainstLevel(
            const Level& level,
            float startX,
            float startY,
            float movementX,
            float movementY,
            float radius
        ) noexcept;

    bool MovingCircleHitsLevel(
        const Level& level,
        float startX,
        float startY,
        float endX,
        float endY,
        float radius
    ) noexcept;
}