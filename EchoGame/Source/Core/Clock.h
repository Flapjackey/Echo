#pragma once

#include <chrono>

namespace Echo
{
    class Clock final
    {
    public:
        Clock() noexcept;

        void Reset() noexcept;

        // Returns time since the previous Tick() call in seconds.
        double Tick() noexcept;

    private:
        using ClockType = std::chrono::steady_clock;

        ClockType::time_point m_previousTime;
    };
}