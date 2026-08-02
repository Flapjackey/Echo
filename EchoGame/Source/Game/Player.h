#pragma once

#include "Game/PlayerCommand.h"

namespace Echo
{
    class Player final
    {
    public:
        Player() = default;

        Player(
            float positionX,
            float positionY
        ) noexcept;

        void Update(
            const PlayerCommand& command,
            double deltaTime
        ) noexcept;

        void SetNetworkState(
            float positionX,
            float positionY,
            float rotation
        ) noexcept;

        float GetPositionX() const noexcept;
        float GetPositionY() const noexcept;

        float GetRotation() const noexcept;

        float GetForwardX() const noexcept;
        float GetForwardY() const noexcept;

        float GetWidth() const noexcept;
        float GetHeight() const noexcept;

        float GetCollisionRadius()
            const noexcept;

    private:
        float m_positionX = 0.0f;
        float m_positionY = 0.0f;

        float m_rotation = 0.0f;

        float m_movementSpeed = 1.0f;

        float m_width = 0.50f;
        float m_height = 0.22f;

        float m_collisionRadius =
            0.16f;
    };
}