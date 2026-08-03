#include "Core/Application.h"

#include <random>
#include <cstdint>

namespace
{
    constexpr unsigned int ClientWidth =
        1280;

    constexpr unsigned int ClientHeight =
        720;
}

namespace Echo
{
    std::uint64_t
        Application::GenerateRuntimeIdentifier()
    {
        static std::random_device entropy;

        static std::mt19937_64 generator(
            (
                static_cast<std::uint64_t>(
                    entropy()
                    ) <<
                32u
                ) ^
            static_cast<std::uint64_t>(
                entropy()
                )
        );

        std::uint64_t identifier = 0;

        while (identifier == 0)
        {
            identifier =
                generator();
        }

        return identifier;
    }

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
        ),
        m_textRenderer(
            m_graphics
        )
    {
        m_localPlayerId =
            GenerateRuntimeIdentifier();
    }

    int Application::Run()
    {

        constexpr double fixedDeltaTime =
            1.0 / 60.0;

        double accumulatedTime = 0.0;

        m_clock.Reset();

        UpdateMenuTitle();

        while (!m_exitRequested &&
            m_window.ProcessMessages())
        {
            const double frameTime =
                BeginApplicationFrame();

            HandleApplicationInput();

            const bool networkGameplayRunning =
                UpdateNetworkConnectionFrame(
                    frameTime
                );

            UpdateGameplayCameraFrame(
                frameTime,
                networkGameplayRunning
            );

            UpdateNetworkGameplayFrame(
                frameTime,
                networkGameplayRunning
            );

            const bool isSimulationRunning =
                UpdateSimulationFrame(
                    frameTime,
                    fixedDeltaTime,
                    accumulatedTime,
                    networkGameplayRunning
                );

            // Send data queued during the current frame.
            m_networkSession.FlushOutgoing();

            RenderFrame();

            EndApplicationFrame(
                frameTime,
                isSimulationRunning
            );
        }

        return 0;
    }
}