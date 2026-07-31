#pragma once

namespace Echo
{
    class Projectile final
    {
    public:
        Projectile(
            float positionX,
            float positionY,
            float directionX,
            float directionY
        ) noexcept;

        void Update(
            double deltaTime
        ) noexcept;

        bool IsAlive() const noexcept;

        float GetPositionX() const noexcept;
        float GetPositionY() const noexcept;

        float GetRotation() const noexcept;

        float GetWidth() const noexcept;
        float GetHeight() const noexcept;

    private:
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