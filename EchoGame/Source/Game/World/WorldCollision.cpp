#include "Game/World/WorldCollision.h"

#include <algorithm>
#include <cmath>

namespace
{
    constexpr float CollisionEpsilon =
        0.000001f;

    constexpr float RotationEpsilon =
        0.0001f;

    struct ExpandedBlockBounds final
    {
        float minimumX = 0.0f;
        float maximumX = 0.0f;

        float minimumY = 0.0f;
        float maximumY = 0.0f;
    };

    bool IsAxisAlignedBlockingBlock(
        const Echo::LevelBlock& block
    ) noexcept
    {
        return
            block.blocksMovement &&
            std::abs(block.rotation) <=
            RotationEpsilon;
    }

    ExpandedBlockBounds GetExpandedBounds(
        const Echo::LevelBlock& block,
        float radius
    ) noexcept
    {
        const float halfWidth =
            block.width * 0.5f;

        const float halfHeight =
            block.height * 0.5f;

        ExpandedBlockBounds bounds{};

        bounds.minimumX =
            block.positionX -
            halfWidth -
            radius;

        bounds.maximumX =
            block.positionX +
            halfWidth +
            radius;

        bounds.minimumY =
            block.positionY -
            halfHeight -
            radius;

        bounds.maximumY =
            block.positionY +
            halfHeight +
            radius;

        return bounds;
    }

    float ResolveHorizontalMovement(
        const Echo::Level& level,
        float startX,
        float positionY,
        float movementX,
        float radius
    ) noexcept
    {
        float targetX =
            startX +
            movementX;

        if (std::abs(movementX) <=
            CollisionEpsilon)
        {
            return targetX;
        }

        for (const Echo::LevelBlock& block :
            level.GetBlocks())
        {
            if (!IsAxisAlignedBlockingBlock(
                block
            ))
            {
                continue;
            }

            const ExpandedBlockBounds bounds =
                GetExpandedBounds(
                    block,
                    radius
                );

            if (positionY <
                bounds.minimumY ||
                positionY >
                bounds.maximumY)
            {
                continue;
            }

            if (movementX > 0.0f)
            {
                const bool crossesLeftSide =
                    startX <=
                    bounds.minimumX &&
                    targetX >
                    bounds.minimumX;

                if (crossesLeftSide)
                {
                    targetX =
                        std::min(
                            targetX,
                            bounds.minimumX
                        );
                }
            }
            else
            {
                const bool crossesRightSide =
                    startX >=
                    bounds.maximumX &&
                    targetX <
                    bounds.maximumX;

                if (crossesRightSide)
                {
                    targetX =
                        std::max(
                            targetX,
                            bounds.maximumX
                        );
                }
            }
        }

        return targetX;
    }

    float ResolveVerticalMovement(
        const Echo::Level& level,
        float positionX,
        float startY,
        float movementY,
        float radius
    ) noexcept
    {
        float targetY =
            startY +
            movementY;

        if (std::abs(movementY) <=
            CollisionEpsilon)
        {
            return targetY;
        }

        for (const Echo::LevelBlock& block :
            level.GetBlocks())
        {
            if (!IsAxisAlignedBlockingBlock(
                block
            ))
            {
                continue;
            }

            const ExpandedBlockBounds bounds =
                GetExpandedBounds(
                    block,
                    radius
                );

            if (positionX <
                bounds.minimumX ||
                positionX >
                bounds.maximumX)
            {
                continue;
            }

            if (movementY > 0.0f)
            {
                const bool crossesBottomSide =
                    startY <=
                    bounds.minimumY &&
                    targetY >
                    bounds.minimumY;

                if (crossesBottomSide)
                {
                    targetY =
                        std::min(
                            targetY,
                            bounds.minimumY
                        );
                }
            }
            else
            {
                const bool crossesTopSide =
                    startY >=
                    bounds.maximumY &&
                    targetY <
                    bounds.maximumY;

                if (crossesTopSide)
                {
                    targetY =
                        std::max(
                            targetY,
                            bounds.maximumY
                        );
                }
            }
        }

        return targetY;
    }

    bool IntersectsSlab(
        float start,
        float movement,
        float minimum,
        float maximum,
        float& minimumTime,
        float& maximumTime
    ) noexcept
    {
        if (std::abs(movement) <=
            CollisionEpsilon)
        {
            return
                start >= minimum &&
                start <= maximum;
        }

        const float inverseMovement =
            1.0f /
            movement;

        float firstTime =
            (minimum - start) *
            inverseMovement;

        float secondTime =
            (maximum - start) *
            inverseMovement;

        if (firstTime > secondTime)
        {
            std::swap(
                firstTime,
                secondTime
            );
        }

        minimumTime =
            std::max(
                minimumTime,
                firstTime
            );

        maximumTime =
            std::min(
                maximumTime,
                secondTime
            );

        return
            minimumTime <=
            maximumTime;
    }

    bool SegmentIntersectsBounds(
        float startX,
        float startY,
        float endX,
        float endY,
        const ExpandedBlockBounds& bounds
    ) noexcept
    {
        const float movementX =
            endX -
            startX;

        const float movementY =
            endY -
            startY;

        float minimumTime =
            0.0f;

        float maximumTime =
            1.0f;

        if (!IntersectsSlab(
            startX,
            movementX,
            bounds.minimumX,
            bounds.maximumX,
            minimumTime,
            maximumTime
        ))
        {
            return false;
        }

        return IntersectsSlab(
            startY,
            movementY,
            bounds.minimumY,
            bounds.maximumY,
            minimumTime,
            maximumTime
        );
    }
}

namespace Echo
{
    ResolvedWorldPosition
        MoveCircleAgainstLevel(
            const Level& level,
            float startX,
            float startY,
            float movementX,
            float movementY,
            float radius
        ) noexcept
    {
        const float safeRadius =
            std::max(
                radius,
                0.0f
            );

        ResolvedWorldPosition result{};

        // Resolve the axes separately so the
        // player can slide along a wall.
        result.positionX =
            ResolveHorizontalMovement(
                level,
                startX,
                startY,
                movementX,
                safeRadius
            );

        result.positionY =
            ResolveVerticalMovement(
                level,
                result.positionX,
                startY,
                movementY,
                safeRadius
            );

        return result;
    }

    bool MovingCircleHitsLevel(
        const Level& level,
        float startX,
        float startY,
        float endX,
        float endY,
        float radius
    ) noexcept
    {
        const float safeRadius =
            std::max(
                radius,
                0.0f
            );

        for (const LevelBlock& block :
            level.GetBlocks())
        {
            if (!IsAxisAlignedBlockingBlock(
                block
            ))
            {
                continue;
            }

            const ExpandedBlockBounds bounds =
                GetExpandedBounds(
                    block,
                    safeRadius
                );

            if (SegmentIntersectsBounds(
                startX,
                startY,
                endX,
                endY,
                bounds
            ))
            {
                return true;
            }
        }

        return false;
    }
}