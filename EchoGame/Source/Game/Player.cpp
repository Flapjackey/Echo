#include "Game/Player.h"

#include <cmath>

namespace
{
    constexpr float DirectionEpsilon =
        0.000001f;
}

namespace Echo
{
    Player::Player(
        float positionX,
        float positionY
    ) noexcept
        : m_positionX(positionX),
        m_positionY(positionY)
    {
    }

    void Player::Update(
        const PlayerCommand& command,
        double deltaTime
    ) noexcept
    {
        float movementX =
            command.movementX;

        float movementY =
            command.movementY;

        const float movementLengthSquared =
            movementX * movementX +
            movementY * movementY;

        // Limit movement length to one.
        // This prevents faster diagonal movement and
        // protects against invalid input commands.
        if (movementLengthSquared > 1.0f)
        {
            const float inverseLength =
                1.0f /
                std::sqrt(
                    movementLengthSquared
                );

            movementX *= inverseLength;
            movementY *= inverseLength;
        }

        const float fixedDeltaTime =
            static_cast<float>(
                deltaTime
                );

        m_positionX +=
            movementX *
            m_movementSpeed *
            fixedDeltaTime;

        m_positionY +=
            movementY *
            m_movementSpeed *
            fixedDeltaTime;

        const float aimLengthSquared =
            command.aimX * command.aimX +
            command.aimY * command.aimY;

        if (aimLengthSquared >
            DirectionEpsilon)
        {
            m_rotation =
                std::atan2(
                    command.aimY,
                    command.aimX
                );
        }
    }

    void Player::SetNetworkState(
        float positionX,
        float positionY,
        float rotation
    ) noexcept
    {
        m_positionX =
            positionX;

        m_positionY =
            positionY;

        m_rotation =
            rotation;
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
        return std::cos(
            m_rotation
        );
    }

    float Player::GetForwardY() const noexcept
    {
        return std::sin(
            m_rotation
        );
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