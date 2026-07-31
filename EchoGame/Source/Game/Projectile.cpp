#include "Game/Projectile.h"

#include <cmath>

namespace Echo
{
    Projectile::Projectile(
        float positionX,
        float positionY,
        float directionX,
        float directionY
    ) noexcept
        : m_positionX(positionX),
        m_positionY(positionY)
    {
        constexpr float movementSpeed =
            2.5f;

        m_velocityX =
            directionX * movementSpeed;

        m_velocityY =
            directionY * movementSpeed;

        m_rotation =
            std::atan2(
                directionY,
                directionX
            );
    }

    void Projectile::Update(
        double deltaTime
    ) noexcept
    {
        const float fixedDeltaTime =
            static_cast<float>(deltaTime);

        m_positionX +=
            m_velocityX *
            fixedDeltaTime;

        m_positionY +=
            m_velocityY *
            fixedDeltaTime;

        m_remainingLifetime -=
            fixedDeltaTime;
    }

    bool Projectile::IsAlive() const noexcept
    {
        return m_remainingLifetime > 0.0f;
    }

    float Projectile::GetPositionX() const noexcept
    {
        return m_positionX;
    }

    float Projectile::GetPositionY() const noexcept
    {
        return m_positionY;
    }

    float Projectile::GetRotation() const noexcept
    {
        return m_rotation;
    }

    float Projectile::GetWidth() const noexcept
    {
        return m_width;
    }

    float Projectile::GetHeight() const noexcept
    {
        return m_height;
    }
}