#include "Core/Application.h"

#include "UI/PauseMenu.h"
#include "UI/SettingsMenu.h"

#include <algorithm>
#include <cmath>
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
        ),
        m_textRenderer(
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

        UpdateMenuTitle();

        while (!m_exitRequested &&
            m_window.ProcessMessages())
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

            HandleApplicationInput();

            if (m_applicationState ==
                ApplicationState::LocalGame)
            {
                const GameSession::PlayerCommands
                    playerCommands =
                    BuildLocalPlayerCommands();

                accumulatedTime +=
                    frameTime;

                while (accumulatedTime >=
                    fixedDeltaTime)
                {
                    FixedUpdate(
                        playerCommands,
                        fixedDeltaTime
                    );

                    accumulatedTime -=
                        fixedDeltaTime;
                }
            }
            else
            {
                // Prevent the menu time from accumulating and
                // producing many updates when the game starts.
                accumulatedTime = 0.0;
            }

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
            case ApplicationState::JoinGame:
            {
                m_graphics.BeginFrame(
                    0.05f,
                    0.03f,
                    0.05f
                );

                RenderPlaceholder();

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

            m_graphics.EndFrame(
                m_settings.verticalSync
            );

            if (m_applicationState ==
                ApplicationState::LocalGame)
            {
                UpdateStatistics(frameTime);
            }

            m_keyboard.EndFrame();
            m_mouse.EndFrame();
        }

        return 0;
    }

    void Application::FixedUpdate(
        const GameSession::PlayerCommands&
        playerCommands,
        double deltaTime
    )
    {
        m_gameSession.Update(
            playerCommands,
            deltaTime
        );
    }

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

                EnterState(
                    ApplicationState::LocalGame
                );

                break;
            }

            case MainMenuAction::HostGame:
            {
                EnterState(
                    ApplicationState::HostGame
                );

                break;
            }

            case MainMenuAction::JoinGame:
            {
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
            if (m_keyboard.WasPressed(
                Key::Escape))
            {
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

    void Application::EnterState(
        ApplicationState state
    )
    {
        m_applicationState =
            state;

        m_statisticsTimer = 0.0;
        m_frameCount = 0;

        UpdateMenuTitle();
    }

    void Application::RenderGameplay()
    {
        const Level& level =
            m_gameSession.GetLevel();

        for (const LevelBlock& block :
            level.GetBlocks())
        {
            m_quadRenderer.Draw(
                block.positionX,
                block.positionY,
                block.width,
                block.height,
                block.rotation,
                m_aspectRatio
            );
        }

        for (const Player& player :
            m_gameSession.GetPlayers())
        {
            m_quadRenderer.Draw(
                player.GetPositionX(),
                player.GetPositionY(),
                player.GetWidth(),
                player.GetHeight(),
                player.GetRotation(),
                m_aspectRatio
            );
        }

        for (const Projectile& projectile :
            m_gameSession.GetProjectiles())
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

        if (m_settings.debugOverlay)
        {
            RenderDebugOverlay();
        }
    }

    void Application::RenderDebugOverlay()
    {
        const float clientWidth =
            static_cast<float>(
                m_window.GetClientWidth()
                );

        if (clientWidth <= 0.0f)
        {
            return;
        }

        const Player& playerOne =
            m_gameSession.GetPlayer(0);

        const Player& playerTwo =
            m_gameSession.GetPlayer(1);

        std::wostringstream debugText;

        debugText
            << std::fixed
            << std::setprecision(1)
            << L"FPS: "
            << m_framesPerSecond
            << L" | Frame: "
            << m_frameTimeMilliseconds
            << L" ms"
            << L" | P1: "
            << std::setprecision(2)
            << playerOne.GetPositionX()
            << L", "
            << playerOne.GetPositionY()
            << L" | P2: "
            << playerTwo.GetPositionX()
            << L", "
            << playerTwo.GetPositionY()
            << L" | Projectiles: "
            << m_gameSession
            .GetProjectiles()
            .size();

        m_textRenderer.Begin();

        m_textRenderer.Draw(
            debugText.str(),
            D2D1_RECT_F{
                0.0f,
                8.0f,
                clientWidth,
                48.0f
            },
            TextStyle::Hint
        );

        m_textRenderer.End();
    }

    void Application::RenderPlaceholder()
    {
        m_quadRenderer.Draw(
            0.0f,
            0.0f,
            0.80f,
            0.30f,
            0.0f,
            m_aspectRatio
        );

        const float clientWidth =
            static_cast<float>(
                m_window.GetClientWidth()
                );

        const float clientHeight =
            static_cast<float>(
                m_window.GetClientHeight()
                );

        const wchar_t* screenTitle =
            L"Placeholder";

        switch (m_applicationState)
        {
        case ApplicationState::HostGame:
            screenTitle = L"Host Game";
            break;

        case ApplicationState::JoinGame:
            screenTitle = L"Join Game";
            break;

        default:
            break;
        }

        m_textRenderer.Begin();

        m_textRenderer.Draw(
            screenTitle,
            D2D1_RECT_F{
                0.0f,
                clientHeight * 0.30f,
                clientWidth,
                clientHeight * 0.46f
            },
            TextStyle::Title
        );

        m_textRenderer.Draw(
            L"This screen will be implemented later",
            D2D1_RECT_F{
                0.0f,
                clientHeight * 0.46f,
                clientWidth,
                clientHeight * 0.58f
            },
            TextStyle::MenuItem
        );

        m_textRenderer.Draw(
            L"Escape - Back",
            D2D1_RECT_F{
                0.0f,
                clientHeight - 55.0f,
                clientWidth,
                clientHeight - 10.0f
            },
            TextStyle::Hint
        );

        m_textRenderer.End();
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
                L"Echo | Host Game placeholder | Escape: Back"
            );

            break;
        }

        case ApplicationState::JoinGame:
        {
            m_window.SetTitle(
                L"Echo | Join Game placeholder | Escape: Back"
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

    GameSession::PlayerCommands
        Application::BuildLocalPlayerCommands()
        const noexcept
    {
        GameSession::PlayerCommands
            playerCommands{};

        PlayerCommand& playerOneCommand =
            playerCommands[0];

        if (m_keyboard.IsDown(Key::A))
        {
            playerOneCommand.movementX -=
                1.0f;
        }

        if (m_keyboard.IsDown(Key::D))
        {
            playerOneCommand.movementX +=
                1.0f;
        }

        if (m_keyboard.IsDown(Key::W))
        {
            playerOneCommand.movementY +=
                1.0f;
        }

        if (m_keyboard.IsDown(Key::S))
        {
            playerOneCommand.movementY -=
                1.0f;
        }

        const Player& playerOne =
            m_gameSession.GetPlayer(0);

        // Preserve the current aim direction when the
        // cursor is outside the client area.
        playerOneCommand.aimX =
            playerOne.GetForwardX();

        playerOneCommand.aimY =
            playerOne.GetForwardY();

        if (!m_mouse.IsInsideWindow())
        {
            return playerCommands;
        }

        playerOneCommand.fire =
            m_mouse.IsLeftButtonDown();

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
            return playerCommands;
        }

        const float normalizedMouseX =
            2.0f *
            static_cast<float>(
                m_mouse.GetX()
                ) /
            clientWidth -
            1.0f;

        const float normalizedMouseY =
            1.0f -
            2.0f *
            static_cast<float>(
                m_mouse.GetY()
                ) /
            clientHeight;

        const float mouseWorldX =
            normalizedMouseX *
            m_aspectRatio;

        const float mouseWorldY =
            normalizedMouseY;

        const float directionX =
            mouseWorldX -
            playerOne.GetPositionX();

        const float directionY =
            mouseWorldY -
            playerOne.GetPositionY();

        const float directionLengthSquared =
            directionX * directionX +
            directionY * directionY;

        constexpr float DirectionEpsilon =
            0.000001f;

        if (directionLengthSquared <=
            DirectionEpsilon)
        {
            return playerCommands;
        }

        const float inverseLength =
            1.0f /
            std::sqrt(
                directionLengthSquared
            );

        playerOneCommand.aimX =
            directionX *
            inverseLength;

        playerOneCommand.aimY =
            directionY *
            inverseLength;

        return playerCommands;
    }

    void Application::UpdateStatistics(
        double deltaTime
    )
    {
        m_statisticsTimer +=
            deltaTime;

        ++m_frameCount;

        if (m_statisticsTimer < 1.0)
        {
            return;
        }

        m_framesPerSecond =
            static_cast<double>(
                m_frameCount
                ) /
            m_statisticsTimer;

        m_frameTimeMilliseconds =
            m_framesPerSecond > 0.0
            ? 1000.0 /
            m_framesPerSecond
            : 0.0;

        m_statisticsTimer = 0.0;
        m_frameCount = 0;
    }
}