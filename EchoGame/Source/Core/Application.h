#pragma once

#include "Core/ApplicationState.h"
#include "Core/Clock.h"
#include "Core/GameSettings.h"
#include "Game/GameSession.h"
#include "Game/PlayerCommand.h"
#include "Graphics/GraphicsDevice.h"
#include "Graphics/QuadRenderer.h"
#include "Graphics/TextRenderer.h"
#include "Input/Keyboard.h"
#include "Input/Mouse.h"
#include "Network/NetworkSession.h"
#include "Network/NetworkSystem.h"
#include "Platform/Windows/Window.h"
#include "UI/MainMenu.h"
#include "UI/PauseMenu.h"
#include "UI/SettingsMenu.h"

#include <cstddef>

namespace Echo
{
    class Application final
    {
    public:
        Application();

        int Run();

    private:
        void FixedUpdate(
            const GameSession::PlayerCommands&
            playerCommands,
            double deltaTime
        );

        GameSession::PlayerCommands
            BuildLocalPlayerCommands()
            const noexcept;

        GameSession::PlayerCommands
            BuildHostPlayerCommands()
            const noexcept;

        NetworkPlayerInput
            BuildLocalNetworkInput()
            const noexcept;

        PlayerCommand BuildPlayerCommand(
            std::size_t playerIndex,
            const NetworkPlayerInput& input
        ) const noexcept;

        NetworkWorldSnapshot
            BuildWorldSnapshot()
            const noexcept;

        void ApplyWorldSnapshot(
            const NetworkWorldSnapshot& snapshot
        ) noexcept;

        void HandleApplicationInput();

        void EnterState(
            ApplicationState state
        );

        void RenderGameplay();
        void RenderPlaceholder();
        void RenderDebugOverlay();

        void UpdateMenuTitle();

        void UpdateStatistics(
            double deltaTime
        );

        Keyboard m_keyboard;
        Mouse m_mouse;

        NetworkSystem m_networkSystem;
        NetworkSession m_networkSession;

        NetworkPlayerInput
            m_latestRemotePlayerInput{};

        bool m_hasReceivedWorldSnapshot = false;

        Window m_window;

        GraphicsDevice m_graphics;
        QuadRenderer m_quadRenderer;
        TextRenderer m_textRenderer;

        GameSession m_gameSession;

        MainMenu m_mainMenu;
        PauseMenu m_pauseMenu;
        SettingsMenu m_settingsMenu;

        Clock m_clock;

        GameSettings m_settings;

        ApplicationState m_applicationState =
            ApplicationState::MainMenu;

        ApplicationState m_settingsReturnState =
            ApplicationState::MainMenu;

        bool m_exitRequested = false;

        double m_statisticsTimer = 0.0;
        unsigned int m_frameCount = 0;

        double m_framesPerSecond = 0.0;
        double m_frameTimeMilliseconds = 0.0;

        float m_aspectRatio =
            16.0f / 9.0f;
    };
}