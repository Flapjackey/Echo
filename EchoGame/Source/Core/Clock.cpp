#include "Core/Clock.h"

namespace Echo
{
    Clock::Clock() noexcept
    {
        Reset();
    }

    void Clock::Reset() noexcept
    {
        m_previousTime = ClockType::now();
    }

    double Clock::Tick() noexcept
    {
        const ClockType::time_point currentTime =
            ClockType::now();

        const std::chrono::duration<double> elapsedTime =
            currentTime - m_previousTime;

        m_previousTime = currentTime;

        return elapsedTime.count();
    }
}