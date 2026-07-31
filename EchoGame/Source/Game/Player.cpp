#include "Game/Player.h"

#include <cmath>

namespace Echo
{
    void Player::UpdateMovement(
        const Keyboard& keyboard,
        double deltaTime
    ) noexcept
    {
        float directionX = 0.0f;
        float directionY = 0.0f;

        if (keyboard.IsDown(Key::A))
        {
            directionX -= 1.0f;
        }

        if (keyboard.IsDown(Key::D))
        {
            directionX += 1.0f;
        }

        if (keyboard.IsDown(Key::W))
        {
            directionY += 1.0f;
        }

        if (keyboard.IsDown(Key::S))
        {
            directionY -= 1.0f;
        }

        // Prevent faster diagonal movement.
        if (directionX != 0.0f &&
            directionY != 0.0f)
        {
            constexpr float diagonalScale =
                0.70710678f;

            directionX *= diagonalScale;
            directionY *= diagonalScale;
        }

        const float fixedDeltaTime =
            static_cast<float>(deltaTime);

        m_positionX +=
            directionX *
            m_movementSpeed *
            fixedDeltaTime;

        m_positionY +=
            directionY *
            m_movementSpeed *
            fixedDeltaTime;
    }

    void Player::AimAt(
        float worldX,
        float worldY
    ) noexcept
    {
        const float directionX =
            worldX - m_positionX;

        const float directionY =
            worldY - m_positionY;

        const float distanceSquared =
            directionX * directionX +
            directionY * directionY;

        if (distanceSquared < 0.000001f)
        {
            return;
        }

        m_rotation =
            std::atan2(
                directionY,
                directionX
            );
    }

    float Player::GetPositionX() const noexcept
    {
        return m_positionX;
    }

    float Player::GetPositionY() const noexcept
    {
        return m_positionY;
    }

    float Player::GetRotation() const noexcept
    {
        return m_rotation;
    }

    float Player::GetForwardX() const noexcept
    {
        return std::cos(m_rotation);
    }

    float Player::GetForwardY() const noexcept
    {
        return std::sin(m_rotation);
    }

    float Player::GetWidth() const noexcept
    {
        return m_width;
    }

    float Player::GetHeight() const noexcept
    {
        return m_height;
    }
}