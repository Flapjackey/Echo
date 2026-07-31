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
#include "Platform/Windows/Window.h"
#include "UI/PauseMenu.h"

namespace Echo
{
    class Application final
    {
    public:
        Application();

        int Run();

    private:
        void FixedUpdate(
            const PlayerCommand& playerCommand,
            double deltaTime
        );

        PlayerCommand
            BuildLocalPlayerCommand()
            const noexcept;

        void HandleApplicationInput();
        void HandleMainMenuInput();
        void HandleSettingsInput();

        bool TryGetHoveredMainMenuItem(
            MainMenuItem& menuItem
        ) const noexcept;

        bool TryGetHoveredSettingsMenuItem(
            SettingsMenuItem& menuItem
        ) const noexcept;

        void ActivateSelectedSettingsItem();

        void EnterState(
            ApplicationState state
        );

        void RenderGameplay();
        void RenderMainMenu();
        void RenderSettings();
        void RenderPlaceholder();
        void RenderDebugOverlay();

        void UpdateMenuTitle();

        void UpdateStatistics(
            double deltaTime
        );

        Keyboard m_keyboard;
        Mouse m_mouse;

        Window m_window;

        GraphicsDevice m_graphics;
        QuadRenderer m_quadRenderer;
        TextRenderer m_textRenderer;

        GameSession m_gameSession;

        PauseMenu m_pauseMenu;

        Clock m_clock;

        GameSettings m_settings;

        ApplicationState m_applicationState =
            ApplicationState::MainMenu;

        ApplicationState m_settingsReturnState =
            ApplicationState::MainMenu;

        MainMenuItem m_selectedMenuItem =
            MainMenuItem::LocalGame;

        SettingsMenuItem m_selectedSettingsItem =
            SettingsMenuItem::Fullscreen;

        bool m_exitRequested = false;

        double m_statisticsTimer = 0.0;
        unsigned int m_frameCount = 0;

        double m_framesPerSecond = 0.0;
        double m_frameTimeMilliseconds = 0.0;

        float m_aspectRatio =
            16.0f / 9.0f;
    };
}