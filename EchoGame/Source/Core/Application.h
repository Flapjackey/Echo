#pragma once

#include "Core/Clock.h"
#include "Game/GameSession.h"
#include "Game/PlayerCommand.h"
#include "Graphics/GraphicsDevice.h"
#include "Graphics/QuadRenderer.h"
#include "Input/Keyboard.h"
#include "Input/Mouse.h"
#include "Platform/Windows/Window.h"
#include "Core/ApplicationState.h"

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
            BuildLocalPlayerCommand() const noexcept;

        void HandleApplicationInput();
        void HandleMainMenuInput();

        void EnterState(
            ApplicationState state
        );

        void RenderGameplay();
        void RenderMainMenu();
        void RenderPlaceholder();

        void UpdateMenuTitle();

        void UpdateStatistics(
            double deltaTime
        );

        Keyboard m_keyboard;
        Mouse m_mouse;

        Window m_window;

        GraphicsDevice m_graphics;
        QuadRenderer m_quadRenderer;

        GameSession m_gameSession;

        Clock m_clock;

        ApplicationState m_applicationState =
            ApplicationState::MainMenu;

        MainMenuItem m_selectedMenuItem =
            MainMenuItem::LocalGame;

        bool m_exitRequested = false;

        double m_statisticsTimer = 0.0;
        unsigned int m_frameCount = 0;

        float m_aspectRatio =
            16.0f / 9.0f;
    };
}