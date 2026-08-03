#include "Core/Application.h"

namespace Echo
{
    void Application::RenderFrame()
    {
        switch (m_applicationState)
        {
        case ApplicationState::MainMenu:
        {
            m_graphics.BeginFrame(
                0.03f,
                0.03f,
                0.06f
            );

            m_mainMenu.Render(
                m_quadRenderer,
                m_textRenderer,
                m_window,
                m_aspectRatio
            );

            break;
        }

        case ApplicationState::LocalGame:
        {
            m_graphics.BeginFrame(
                0.02f,
                0.04f,
                0.08f
            );

            RenderGameplay();

            break;
        }

        case ApplicationState::Paused:
        {
            m_graphics.BeginFrame(
                0.02f,
                0.04f,
                0.08f
            );

            // Draw the frozen game world.
            RenderGameplay();

            // Draw the pause interface over the game.
            m_pauseMenu.Render(
                m_textRenderer,
                m_window
            );

            break;
        }

        case ApplicationState::HostGame:
        {
            m_graphics.BeginFrame(
                0.02f,
                0.04f,
                0.08f
            );

            RenderGameplay();

            break;
        }

        case ApplicationState::JoinGame:
        {
            const bool canRenderRemoteWorld =
                m_hasReceivedWorldSnapshot &&
                (
                    m_networkSession.IsConnected() ||
                    m_networkGamePhase ==
                    NetworkGamePhase::
                    ConnectionRecovery
                    );

            if (canRenderRemoteWorld)
            {
                m_graphics.BeginFrame(
                    0.02f,
                    0.04f,
                    0.08f
                );

                RenderGameplay();
            }
            else
            {
                m_graphics.BeginFrame(
                    0.05f,
                    0.03f,
                    0.05f
                );

                RenderPlaceholder();
            }

            break;
        }

        case ApplicationState::Settings:
        {
            m_graphics.BeginFrame(
                0.025f,
                0.035f,
                0.06f
            );

            m_settingsMenu.Render(
                m_quadRenderer,
                m_textRenderer,
                m_window,
                m_settings,
                m_aspectRatio
            );

            break;
        }
        }

        RenderNetworkOverlay();

        m_graphics.EndFrame(
            m_settings.verticalSync
        );
    }

    void Application::RenderNetworkOverlay()
    {
        const bool isConnectionRecovery =
            m_networkGamePhase ==
            NetworkGamePhase::
            ConnectionRecovery;

        const bool isHandshakingHost =
            m_networkGamePhase ==
            NetworkGamePhase::
            HandshakingHost;

        const bool isHandshakingClient =
            m_networkGamePhase ==
            NetworkGamePhase::
            HandshakingClient;

        const bool isSynchronizingHost =
            m_networkGamePhase ==
            NetworkGamePhase::
            SynchronizingHost;

        const bool isSynchronizingClient =
            m_networkGamePhase ==
            NetworkGamePhase::
            SynchronizingClient;

        const bool isNetworkOverlayVisible =
            (
                m_applicationState ==
                ApplicationState::HostGame ||
                m_applicationState ==
                ApplicationState::JoinGame
                ) &&
            (
                isConnectionRecovery ||
                isHandshakingHost ||
                isHandshakingClient ||
                isSynchronizingHost ||
                isSynchronizingClient
                );

        if (!isNetworkOverlayVisible)
        {
            return;
        }

        const bool canContinueSolo =
            isConnectionRecovery &&
            m_applicationState ==
            ApplicationState::JoinGame &&
            m_hasMigrationState;

        const wchar_t* title =
            L"CONNECTION LOST";

        const wchar_t* message =
            L"Restoring the game session...";

        const bool showTimer =
            isConnectionRecovery;

        if (isHandshakingHost)
        {
            title =
                L"PLAYER CONNECTED";

            message =
                L"Verifying session identity...";
        }
        else if (isHandshakingClient)
        {
            title =
                L"CONNECTING";

            message =
                L"Joining the game session...";
        }
        else if (isSynchronizingHost)
        {
            title =
                L"PLAYER CONNECTED";

            message =
                L"Sending current game state...";
        }
        else if (isSynchronizingClient)
        {
            title =
                L"CONNECTING";

            message =
                L"Synchronizing game state...";
        }

        m_connectionRecoveryOverlay.Render(
            m_textRenderer,
            m_window,
            m_connectionRecoveryRemaining,
            showTimer,
            canContinueSolo,
            title,
            message,
            m_networkSession.
            GetStatusMessage()
        );
    }
}