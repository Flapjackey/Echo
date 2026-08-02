#pragma once

#include "Core/EntityId.h"

namespace Echo
{
    class Projectile final
    {
    public:
        Projectile(
            EntityId entityId,
            float positionX,
            float positionY,
            float directionX,
            float directionY
        ) noexcept;

        void Update(
            double deltaTime
        ) noexcept;

        void SetNetworkState(
            float positionX,
            float positionY,
            float rotation
        ) noexcept;

        void SetRemainingLifetime(
            float remainingLifetime
        ) noexcept;

        EntityId GetEntityId()
            const noexcept;

        float GetRemainingLifetime()
            const noexcept;

        bool IsAlive() const noexcept;

        float GetPositionX() const noexcept;
        float GetPositionY() const noexcept;

        float GetRotation() const noexcept;
        float GetWidth() const noexcept;
        float GetHeight() const noexcept;

    private:
        EntityId m_entityId =
            InvalidEntityId;

        float m_positionX = 0.0f;
        float m_positionY = 0.0f;

        float m_velocityX = 0.0f;
        float m_velocityY = 0.0f;

        float m_rotation = 0.0f;

        float m_remainingLifetime = 2.0f;

        float m_width = 0.18f;
        float m_height = 0.06f;
    };
}