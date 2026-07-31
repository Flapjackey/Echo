#include "Core/Application.h"

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <thread>

namespace Echo
{
    Application::Application()
        : m_window(
            1280,
            720,
            L"EchoGame"
        )
    {
    }

    int Application::Run()
    {
        using namespace std::chrono_literals;

        constexpr double fixedDeltaTime =
            1.0 / 60.0;

        constexpr double maximumFrameTime =
            0.25;

        double accumulatedTime = 0.0;

        m_clock.Reset();

        while (m_window.ProcessMessages())
        {
            // Limit unusually large time jumps.
            // This can happen after pausing in the debugger.
            const double frameTime = std::min(
                m_clock.Tick(),
                maximumFrameTime
            );

            accumulatedTime += frameTime;

            // Run the game simulation at 60 updates per second.
            while (accumulatedTime >= fixedDeltaTime)
            {
                FixedUpdate(fixedDeltaTime);

                accumulatedTime -= fixedDeltaTime;
            }

            Update(frameTime);
            UpdateStatistics(frameTime);

            // Temporary protection from using an entire CPU core.
            // Later vertical synchronization will replace this.
            std::this_thread::sleep_for(1ms);
        }

        return 0;
    }

    void Application::FixedUpdate(double deltaTime)
    {
        // Player movement, enemies, projectiles and
        // collision detection will be updated here.

        (void)deltaTime;
    }

    void Application::Update(double deltaTime)
    {
        // Camera, visual effects and interface
        // will be updated here.

        (void)deltaTime;
    }

    void Application::UpdateStatistics(double deltaTime)
    {
        m_statisticsTimer += deltaTime;
        ++m_frameCount;

        if (m_statisticsTimer < 1.0)
        {
            return;
        }

        const double framesPerSecond =
            static_cast<double>(m_frameCount) /
            m_statisticsTimer;

        const double frameTimeMilliseconds =
            framesPerSecond > 0.0
            ? 1000.0 / framesPerSecond
            : 0.0;

        std::wostringstream title;

        title
            << std::fixed
            << std::setprecision(1)
            << L"EchoGame | FPS: "
            << framesPerSecond
            << L" | Frame: "
            << frameTimeMilliseconds
            << L" ms";

        m_window.SetTitle(title.str());

        m_statisticsTimer = 0.0;
        m_frameCount = 0;
    }
}