#pragma once

namespace Echo
{
    class Camera2D final
    {
    public:
        void SnapTo(
            float positionX,
            float positionY
        ) noexcept;

        void Update(
            float targetX,
            float targetY,
            double deltaTime
        ) noexcept;

        float GetPositionX()
            const noexcept;

        float GetPositionY()
            const noexcept;

    private:
        float m_positionX = 0.0f;
        float m_positionY = 0.0f;

        float m_followSharpness =
            7.0f;

        bool m_initialized =
            false;
    };
}