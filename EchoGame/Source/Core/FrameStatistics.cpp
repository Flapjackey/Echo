#include "Core/FrameStatistics.h"

namespace Echo
{
    void FrameStatistics::Update(
        double deltaTime
    ) noexcept
    {
        m_statisticsTimer +=
            deltaTime;

        ++m_frameCount;

        if (m_statisticsTimer < 1.0)
        {
            return;
        }

        m_framesPerSecond =
            static_cast<double>(
                m_frameCount
                ) /
            m_statisticsTimer;

        m_frameTimeMilliseconds =
            m_framesPerSecond > 0.0
            ? 1000.0 /
            m_framesPerSecond
            : 0.0;

        m_statisticsTimer =
            0.0;

        m_frameCount =
            0;
    }

    void FrameStatistics::ResetSampling()
        noexcept
    {
        m_statisticsTimer =
            0.0;

        m_frameCount =
            0;
    }

    double FrameStatistics::
        GetFramesPerSecond()
        const noexcept
    {
        return m_framesPerSecond;
    }

    double FrameStatistics::
        GetFrameTimeMilliseconds()
        const noexcept
    {
        return m_frameTimeMilliseconds;
    }
}