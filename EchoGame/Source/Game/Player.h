#pragma once

#include "Input/Keyboard.h"

namespace Echo
{
    class Player final
    {
    public:
        Player() = default;

        void UpdateMovement(
            const Keyboard& keyboard,
            double deltaTime
        ) noexcept;

        void AimAt(
            float worldX,
            float worldY
        ) noexcept;

        float GetPositionX() const noexcept;
        float GetPositionY() const noexcept;

        float GetRotation() const noexcept;

        float GetForwardX() const noexcept;
        float GetForwardY() const noexcept;

        float GetWidth() const noexcept;
        float GetHeight() const noexcept;

    private:
        float m_positionX = 0.0f;
        float m_positionY = 0.0f;

        float m_rotation = 0.0f;

        float m_movementSpeed = 1.0f;

        float m_width = 0.50f;
        float m_height = 0.22f;
    };
}