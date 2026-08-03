#pragma once

namespace Echo
{
    class FrameStatistics final
    {
    public:
        void Update(
            double deltaTime
        ) noexcept;

        void ResetSampling()
            noexcept;

        double GetFramesPerSecond()
            const noexcept;

        double GetFrameTimeMilliseconds()
            const noexcept;

    private:
        double m_statisticsTimer =
            0.0;

        unsigned int m_frameCount =
            0;

        double m_framesPerSecond =
            0.0;

        double m_frameTimeMilliseconds =
            0.0;
    };
}