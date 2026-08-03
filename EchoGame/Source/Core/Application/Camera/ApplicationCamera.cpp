#include "Core/Application.h"

namespace Echo
{
    void Application::UpdateGameplayCameraFrame(
        double frameTime,
        bool networkGameplayRunning
    ) noexcept
    {
        const bool shouldUpdateGameplayCamera =
            m_applicationState ==
            ApplicationState::LocalGame ||
            (
                (
                    m_applicationState ==
                    ApplicationState::HostGame ||
                    m_applicationState ==
                    ApplicationState::JoinGame
                    ) &&
                networkGameplayRunning
                );

        if (!shouldUpdateGameplayCamera)
        {
            return;
        }

        m_gameplayCameraController.Update(
            m_gameplayCamera,
            m_gameSession,
            GetCameraPlayerIndex(),
            m_mouse,
            m_window,
            m_aspectRatio,
            frameTime
        );
    }

    std::size_t Application::GetCameraPlayerIndex()
        const noexcept
    {
        if (m_applicationState ==
            ApplicationState::HostGame ||
            m_applicationState ==
            ApplicationState::JoinGame)
        {
            return
                m_localNetworkPlayerIndex;
        }

        return 0;
    }
}