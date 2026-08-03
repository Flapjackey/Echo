#include "Core/Application.h"

#include <algorithm>

namespace
{
    constexpr double MaximumFrameTime =
        0.25;
}

namespace Echo
{
    double Application::BeginApplicationFrame()
    {
        unsigned int resizedWidth =
            0;

        unsigned int resizedHeight =
            0;

        if (m_window.ConsumeResize(
            resizedWidth,
            resizedHeight
        ))
        {
            m_graphics.Resize(
                resizedWidth,
                resizedHeight
            );

            m_aspectRatio =
                static_cast<float>(
                    resizedWidth
                    ) /
                static_cast<float>(
                    resizedHeight
                    );
        }

        // Limit unusually large time jumps.
        // This can happen after pausing
        // in the debugger.
        return std::min(
            m_clock.Tick(),
            MaximumFrameTime
        );
    }

    void Application::EndApplicationFrame(
        double frameTime,
        bool isSimulationRunning
    )
    {
        if (isSimulationRunning)
        {
            m_frameStatistics.Update(
                frameTime
            );
        }

        m_keyboard.EndFrame();
        m_mouse.EndFrame();
    }
}