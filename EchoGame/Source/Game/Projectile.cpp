#include "Game/Projectile.h"

#include <cmath>

namespace
{
    constexpr float
        ProjectileMovementSpeed =
        2.5f;
}

namespace Echo
{
    Projectile::Projectile(
        EntityId entityId,
        float positionX,
        float positionY,
        float directionX,
        float directionY
    ) noexcept
        : m_entityId(
            entityId
        )
    {
        const float rotation =
            std::atan2(
                directionY,
                directionX
            );

        SetNetworkState(
            positionX,
            positionY,
            rotation
        );
    }

    void Projectile::Update(
        double deltaTime
    ) noexcept
    {
        const float fixedDeltaTime =
            static_cast<float>(
                deltaTime
                );

        m_positionX +=
            m_velocityX *
            fixedDeltaTime;

        m_positionY +=
            m_velocityY *
            fixedDeltaTime;

        m_remainingLifetime -=
            fixedDeltaTime;
    }

    void Projectile::SetNetworkState(
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

        m_velocityX =
            std::cos(
                rotation
            ) *
            ProjectileMovementSpeed;

        m_velocityY =
            std::sin(
                rotation
            ) *
            ProjectileMovementSpeed;
    }

    void Projectile::SetRemainingLifetime(
        float remainingLifetime
    ) noexcept
    {
        m_remainingLifetime =
            remainingLifetime > 0.0f
            ? remainingLifetime
            : 0.0f;
    }

    EntityId Projectile::GetEntityId()
        const noexcept
    {
        return m_entityId;
    }

    float Projectile::GetRemainingLifetime()
        const noexcept
    {
        return m_remainingLifetime;
    }

    bool Projectile::IsAlive() const noexcept
    {
        return
            m_remainingLifetime >
            0.0f;
    }

    float Projectile::GetPositionX()
        const noexcept
    {
        return m_positionX;
    }

    float Projectile::GetPositionY()
        const noexcept
    {
        return m_positionY;
    }

    float Projectile::GetRotation()
        const noexcept
    {
        return m_rotation;
    }

    float Projectile::GetWidth()
        const noexcept
    {
        return m_width;
    }

    float Projectile::GetHeight()
        const noexcept
    {
        return m_height;
    }
}