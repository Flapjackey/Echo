#include "Graphics/Camera2D.h"

#include <algorithm>
#include <cmath>

namespace Echo
{
    void Camera2D::SnapTo(
        float positionX,
        float positionY
    ) noexcept
    {
        m_positionX =
            positionX;

        m_positionY =
            positionY;

        m_initialized =
            true;
    }

    void Camera2D::Update(
        float targetX,
        float targetY,
        double deltaTime
    ) noexcept
    {
        if (!m_initialized)
        {
            SnapTo(
                targetX,
                targetY
            );

            return;
        }

        const float safeDeltaTime =
            static_cast<float>(
                std::clamp(
                    deltaTime,
                    0.0,
                    0.25
                )
                );

        // Frame-rate-independent exponential
        // smoothing.
        const float interpolationAmount =
            1.0f -
            std::exp(
                -m_followSharpness *
                safeDeltaTime
            );

        m_positionX +=
            (
                targetX -
                m_positionX
                ) *
            interpolationAmount;

        m_positionY +=
            (
                targetY -
                m_positionY
                ) *
            interpolationAmount;
    }

    float Camera2D::GetPositionX()
        const noexcept
    {
        return m_positionX;
    }

    float Camera2D::GetPositionY()
        const noexcept
    {
        return m_positionY;
    }
}