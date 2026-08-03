#include "Core/Application.h"

namespace Echo
{
    void Application::EnterState(
        ApplicationState state
    )
    {
        m_applicationState =
            state;

        m_frameStatistics.ResetSampling();

        UpdateMenuTitle();
    }

    void Application::UpdateMenuTitle()
    {
        switch (m_applicationState)
        {
        case ApplicationState::MainMenu:
        {
            m_window.SetTitle(
                L"Echo | Main Menu"
            );

            break;
        }

        case ApplicationState::LocalGame:
        {
            m_window.SetTitle(
                L"Echo | Local Game | Escape: Pause"
            );

            break;
        }

        case ApplicationState::Paused:
        {
            m_window.SetTitle(
                L"Echo | Paused | Escape: Resume"
            );

            break;
        }

        case ApplicationState::HostGame:
        {
            m_window.SetTitle(
                L"Echo | Host Game | Escape: Back"
            );

            break;
        }

        case ApplicationState::JoinGame:
        {
            m_window.SetTitle(
                L"Echo | Join Game | Escape: Back"
            );

            break;
        }

        case ApplicationState::Settings:
        {
            m_window.SetTitle(
                L"Echo | Settings | Escape: Back"
            );

            break;
        }
        }
    }
}