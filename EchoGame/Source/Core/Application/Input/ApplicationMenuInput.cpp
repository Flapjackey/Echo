#include "Core/Application.h"

#include "UI/ConnectionRecoveryOverlay.h"
#include "UI/MainMenu.h"
#include "UI/PauseMenu.h"
#include "UI/SettingsMenu.h"

#include <cstddef>
#include <cstdint>

namespace
{
    constexpr std::size_t
        InitialHostPlayerIndex =
        0;

    constexpr std::size_t
        InitialClientPlayerIndex =
        1;

    constexpr std::uint16_t
        LocalNetworkPort =
        27015;
}

namespace Echo
{
    void Application::HandleApplicationInput()
    {
        switch (m_applicationState)
        {
        case ApplicationState::MainMenu:
        {
            const MainMenuAction action =
                m_mainMenu.Update(
                    m_keyboard,
                    m_mouse,
                    m_window,
                    m_aspectRatio
                );

            switch (action)
            {
            case MainMenuAction::None:
            {
                break;
            }

            case MainMenuAction::StartLocalGame:
            {
                m_gameSession.Reset();

                m_gameplayCameraController.Reset(
                    m_gameplayCamera,
                    m_gameSession,
                    GetCameraPlayerIndex()
                );

                EnterState(
                    ApplicationState::LocalGame
                );

                break;
            }

            case MainMenuAction::HostGame:
            {
                m_gameSession.Reset();

                ResetNetworkGameState();

                SetNetworkPlayerOwnership(
                    InitialHostPlayerIndex,
                    InitialClientPlayerIndex
                );

                m_gameplayCameraController.Reset(
                    m_gameplayCamera,
                    m_gameSession,
                    GetCameraPlayerIndex()
                );

                // Create a new identity for this
                // authoritative game session.
                m_sessionId =
                    GenerateRuntimeIdentifier();

                m_remotePlayerId =
                    InvalidPlayerId;

                m_hostPlayerId =
                    m_localPlayerId;

                m_hostEpoch =
                    1;

                m_networkSession.SetLocalIdentity(
                    m_sessionId,
                    m_localPlayerId,
                    m_hostPlayerId,
                    m_hostEpoch
                );

                m_connectionHelloQueued =
                    false;

                m_sessionWelcomeQueued =
                    false;

                m_networkSession.StartHost(
                    LocalNetworkPort
                );

                m_networkGamePhase =
                    NetworkGamePhase::Running;

                EnterState(
                    ApplicationState::HostGame
                );

                break;
            }

            case MainMenuAction::JoinGame:
            {
                m_gameSession.Reset();

                ResetNetworkGameState();

                SetNetworkPlayerOwnership(
                    InitialClientPlayerIndex,
                    InitialHostPlayerIndex
                );

                m_gameplayCameraController.Reset(
                    m_gameplayCamera,
                    m_gameSession,
                    GetCameraPlayerIndex()
                );

                // A new client does not know the session
                // identity until SessionWelcome arrives.
                m_sessionId =
                    InvalidSessionId;

                m_remotePlayerId =
                    InvalidPlayerId;

                m_hostPlayerId =
                    InvalidPlayerId;

                m_hostEpoch =
                    0;

                // The process already has its permanent
                // runtime PlayerId, generated in the
                // Application constructor.
                m_networkSession.SetLocalIdentity(
                    m_sessionId,
                    m_localPlayerId,
                    m_hostPlayerId,
                    m_hostEpoch
                );

                // Normal input must not be sent before
                // the handshake and checkpoint complete.
                m_playerInputSendAccumulator =
                    0.0;

                m_connectionHelloQueued =
                    false;

                m_sessionWelcomeQueued =
                    false;

                m_networkGamePhase =
                    NetworkGamePhase::
                    HandshakingClient;

                m_networkSession.StartClient(
                    LocalNetworkPort
                );

                EnterState(
                    ApplicationState::JoinGame
                );

                break;
            }

            case MainMenuAction::OpenSettings:
            {
                m_settingsReturnState =
                    ApplicationState::MainMenu;

                EnterState(
                    ApplicationState::Settings
                );

                break;
            }

            case MainMenuAction::Exit:
            {
                m_exitRequested = true;
                break;
            }
            }

            break;
        }

        case ApplicationState::LocalGame:
        {
            if (m_keyboard.WasPressed(
                Key::Escape))
            {
                m_pauseMenu.Reset();

                EnterState(
                    ApplicationState::Paused
                );
            }

            break;
        }

        case ApplicationState::Paused:
        {
            const PauseMenuAction action =
                m_pauseMenu.Update(
                    m_keyboard,
                    m_mouse,
                    m_window
                );

            switch (action)
            {
            case PauseMenuAction::None:
            {
                break;
            }

            case PauseMenuAction::Resume:
            {
                EnterState(
                    ApplicationState::LocalGame
                );

                break;
            }

            case PauseMenuAction::OpenSettings:
            {
                m_settingsReturnState =
                    ApplicationState::Paused;

                EnterState(
                    ApplicationState::Settings
                );

                break;
            }

            case PauseMenuAction::ReturnToMainMenu:
            {
                m_mainMenu.Reset();

                EnterState(
                    ApplicationState::MainMenu
                );

                break;
            }

            case PauseMenuAction::Exit:
            {
                m_exitRequested = true;
                break;
            }
            }

            break;
        }

        case ApplicationState::HostGame:
        case ApplicationState::JoinGame:
        {
            if (m_networkGamePhase ==
                NetworkGamePhase::
                ConnectionRecovery)
            {
                const bool canContinueSolo =
                    m_applicationState ==
                    ApplicationState::JoinGame &&
                    m_hasMigrationState;

                const ConnectionRecoveryAction action =
                    m_connectionRecoveryOverlay.Update(
                        m_keyboard,
                        m_mouse,
                        m_window,
                        canContinueSolo
                    );

                if (action ==
                    ConnectionRecoveryAction::
                    ContinueSolo)
                {
                    PromoteClientToHost();
                    break;
                }
            }

            if (m_keyboard.WasPressed(
                Key::Escape))
            {
                m_networkSession.Stop();

                ResetNetworkGameState();

                m_mainMenu.Reset();

                EnterState(
                    ApplicationState::MainMenu
                );
            }

            break;
        }

        case ApplicationState::Settings:
        {
            const SettingsMenuAction action =
                m_settingsMenu.Update(
                    m_keyboard,
                    m_mouse,
                    m_window,
                    m_aspectRatio
                );

            switch (action)
            {
            case SettingsMenuAction::None:
            {
                break;
            }

            case SettingsMenuAction::ToggleFullscreen:
            {
                const bool requestedFullscreen =
                    !m_settings.fullscreen;

                m_window.SetFullscreen(
                    requestedFullscreen
                );

                m_settings.fullscreen =
                    m_window.IsFullscreen();

                break;
            }

            case SettingsMenuAction::ToggleVerticalSync:
            {
                m_settings.verticalSync =
                    !m_settings.verticalSync;

                break;
            }

            case SettingsMenuAction::ToggleDebugOverlay:
            {
                m_settings.debugOverlay =
                    !m_settings.debugOverlay;

                break;
            }

            case SettingsMenuAction::Back:
            {
                EnterState(
                    m_settingsReturnState
                );

                break;
            }
            }

            break;
        }
        }
    }

}