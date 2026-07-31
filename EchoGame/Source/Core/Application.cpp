#include "Core/Application.h"

#include <algorithm>
#include <iomanip>
#include <sstream>

namespace
{
    constexpr unsigned int ClientWidth =
        1280;

    constexpr unsigned int ClientHeight =
        720;
}

namespace Echo
{
    Application::Application()
        : m_window(
            m_keyboard,
            ClientWidth,
            ClientHeight,
            L"EchoGame"
        ),
        m_graphics(
            m_window.GetHandle(),
            ClientWidth,
            ClientHeight
        ),
        m_quadRenderer(
            m_graphics
        )
    {
    }

    int Application::Run()
    {

        constexpr double fixedDeltaTime =
            1.0 / 60.0;

        constexpr double maximumFrameTime =
            0.25;

        double accumulatedTime = 0.0;

        m_clock.Reset();

        while (m_window.ProcessMessages())
        {
            unsigned int resizedWidth = 0;
            unsigned int resizedHeight = 0;

            if (m_window.ConsumeResize(
                resizedWidth,
                resizedHeight))
            {
                m_graphics.Resize(
                    resizedWidth,
                    resizedHeight
                );

                m_aspectRatio =
                    static_cast<float>(resizedWidth) /
                    static_cast<float>(resizedHeight);
            }

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

            m_graphics.BeginFrame(
                0.02f,
                0.04f,
                0.08f
            );

            m_quadRenderer.Draw(
                m_quadPositionX,
                m_quadPositionY,
                0.4f,
                0.4f,
                m_quadRotation,
                m_aspectRatio
            );

            m_graphics.EndFrame();

            UpdateStatistics(frameTime);

            // Temporary protection from using an entire CPU core.
            // Later vertical synchronization will replace this.
        }

        return 0;
    }

    void Application::FixedUpdate(
        double deltaTime
    )
    {
        float directionX = 0.0f;
        float directionY = 0.0f;

        if (m_keyboard.IsDown(Key::A))
        {
            directionX -= 1.0f;
        }

        if (m_keyboard.IsDown(Key::D))
        {
            directionX += 1.0f;
        }

        if (m_keyboard.IsDown(Key::W))
        {
            directionY += 1.0f;
        }

        if (m_keyboard.IsDown(Key::S))
        {
            directionY -= 1.0f;
        }

        // Prevent faster diagonal movement.
        if (directionX != 0.0f &&
            directionY != 0.0f)
        {
            constexpr float diagonalScale =
                0.70710678f;

            directionX *= diagonalScale;
            directionY *= diagonalScale;
        }

        constexpr float movementSpeed =
            1.0f;

        const float fixedDeltaTime =
            static_cast<float>(deltaTime);

        m_quadPositionX +=
            directionX *
            movementSpeed *
            fixedDeltaTime;

        m_quadPositionY +=
            directionY *
            movementSpeed *
            fixedDeltaTime;
    }

    void Application::Update(double deltaTime)
    {
        m_quadRotation +=
            static_cast<float>(deltaTime);
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