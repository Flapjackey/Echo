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
            m_mouse,
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
                m_player.GetPositionX(),
                m_player.GetPositionY(),
                m_player.GetWidth(),
                m_player.GetHeight(),
                m_player.GetRotation(),
                m_aspectRatio
            );

            for (const Projectile& projectile : m_projectiles)
            {
                m_quadRenderer.Draw(
                    projectile.GetPositionX(),
                    projectile.GetPositionY(),
                    projectile.GetWidth(),
                    projectile.GetHeight(),
                    projectile.GetRotation(),
                    m_aspectRatio
                );
            }

            m_graphics.EndFrame();

            UpdateStatistics(frameTime);
        }

        return 0;
    }

    void Application::FixedUpdate(
        double deltaTime
    )
    {
        m_player.UpdateMovement(
            m_keyboard,
            deltaTime
        );

        const float fixedDeltaTime =
            static_cast<float>(deltaTime);

        if (m_fireCooldown > 0.0f)
        {
            m_fireCooldown -=
                fixedDeltaTime;
        }

        if (m_mouse.IsLeftButtonDown())
        {
            TryFireProjectile();
        }

        for (Projectile& projectile : m_projectiles)
        {
            projectile.Update(deltaTime);
        }

        const auto firstExpiredProjectile =
            std::remove_if(
                m_projectiles.begin(),
                m_projectiles.end(),
                [](const Projectile& projectile)
                {
                    return !projectile.IsAlive();
                }
            );

        m_projectiles.erase(
            firstExpiredProjectile,
            m_projectiles.end()
        );
    }

    void Application::TryFireProjectile()
    {
        if (m_fireCooldown > 0.0f)
        {
            return;
        }

        constexpr float fireInterval =
            0.15f;

        const float forwardX =
            m_player.GetForwardX();

        const float forwardY =
            m_player.GetForwardY();

        constexpr float spawnDistance =
            0.32f;

        const float spawnX =
            m_player.GetPositionX() +
            forwardX *
            spawnDistance;

        const float spawnY =
            m_player.GetPositionY() +
            forwardY *
            spawnDistance;

        m_projectiles.emplace_back(
            spawnX,
            spawnY,
            forwardX,
            forwardY
        );

        m_fireCooldown =
            fireInterval;
    }

    void Application::Update(
        double deltaTime
    )
    {
        (void)deltaTime;

        if (!m_mouse.IsInsideWindow())
        {
            return;
        }

        const float clientWidth =
            static_cast<float>(
                m_window.GetClientWidth()
                );

        const float clientHeight =
            static_cast<float>(
                m_window.GetClientHeight()
                );

        if (clientWidth <= 0.0f ||
            clientHeight <= 0.0f)
        {
            return;
        }

        // Convert mouse X from pixels to range [-1, 1].
        const float normalizedMouseX =
            2.0f *
            static_cast<float>(
                m_mouse.GetX()
                ) /
            clientWidth -
            1.0f;

        // Windows Y grows downward, but game Y grows upward.
        const float normalizedMouseY =
            1.0f -
            2.0f *
            static_cast<float>(
                m_mouse.GetY()
                ) /
            clientHeight;

        // The shader divides world X by aspectRatio.
        // Reverse that conversion for the mouse.
        const float mouseWorldX =
            normalizedMouseX *
            m_aspectRatio;

        const float mouseWorldY =
            normalizedMouseY;

        m_player.AimAt(
            mouseWorldX,
            mouseWorldY
        );
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