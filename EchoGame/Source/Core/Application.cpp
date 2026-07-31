#include "Core/Application.h"

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

    constexpr const wchar_t*
        MainMenuLabels[]
    {
        L"Local Game",
        L"Host Game",
        L"Join Game",
        L"Settings",
        L"Exit"
    };

    constexpr float MainMenuFirstItemY =
        0.56f;

    constexpr float MainMenuItemSpacing =
        0.28f;

    constexpr float MainMenuNormalWidth =
        0.90f;

    constexpr float MainMenuSelectedWidth =
        1.25f;

    constexpr float MainMenuNormalHeight =
        0.12f;

    constexpr float MainMenuSelectedHeight =
        0.18f;

    constexpr float MainMenuHitboxWidth =
        1.25f;

    constexpr float MainMenuHitboxHeight =
        0.22f;

    constexpr std::size_t
        MainMenuLabelCount =
        sizeof(MainMenuLabels) /
        sizeof(MainMenuLabels[0]);

    static_assert(
        MainMenuLabelCount ==
        static_cast<std::size_t>(
            Echo::MainMenuItem::Count
            )
        );
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
                const PlayerCommand playerCommand =
                    BuildLocalPlayerCommand();

                accumulatedTime +=
                    frameTime;

                while (accumulatedTime >=
                    fixedDeltaTime)
                {
                    FixedUpdate(
                        playerCommand,
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

                RenderMainMenu();

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

            case ApplicationState::HostGame:
            case ApplicationState::JoinGame:
            case ApplicationState::Settings:
            {
                m_graphics.BeginFrame(
                    0.05f,
                    0.03f,
                    0.05f
                );

                RenderPlaceholder();

                break;
            }
            }

            m_graphics.EndFrame();

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
        const PlayerCommand& playerCommand,
        double deltaTime
    )
    {
        m_gameSession.Update(
            playerCommand,
            deltaTime
        );
    }

    void Application::HandleApplicationInput()
    {
        switch (m_applicationState)
        {
        case ApplicationState::MainMenu:
        {
            HandleMainMenuInput();
            break;
        }

        case ApplicationState::LocalGame:
        {
            if (m_keyboard.WasPressed(
                Key::Escape))
            {
                EnterState(
                    ApplicationState::MainMenu
                );
            }

            break;
        }

        case ApplicationState::HostGame:
        case ApplicationState::JoinGame:
        case ApplicationState::Settings:
        {
            if (m_keyboard.WasPressed(
                Key::Escape))
            {
                EnterState(
                    ApplicationState::MainMenu
                );
            }

            break;
        }
        }
    }

    bool Application::TryGetHoveredMainMenuItem(
        MainMenuItem& menuItem
    ) const noexcept
    {
        if (!m_mouse.IsInsideWindow())
        {
            return false;
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
            return false;
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

        // Convert screen X to the world coordinate
        // system used by QuadRenderer.
        const float mouseWorldX =
            normalizedMouseX *
            m_aspectRatio;

        const float mouseWorldY =
            normalizedMouseY;

        constexpr std::size_t itemCount =
            static_cast<std::size_t>(
                MainMenuItem::Count
                );

        const float halfHitboxWidth =
            MainMenuHitboxWidth *
            0.5f;

        const float halfHitboxHeight =
            MainMenuHitboxHeight *
            0.5f;

        for (std::size_t index = 0;
            index < itemCount;
            ++index)
        {
            const float itemPositionY =
                MainMenuFirstItemY -
                static_cast<float>(index) *
                MainMenuItemSpacing;

            const bool insideX =
                mouseWorldX >=
                -halfHitboxWidth &&
                mouseWorldX <=
                halfHitboxWidth;

            const bool insideY =
                mouseWorldY >=
                itemPositionY -
                halfHitboxHeight &&
                mouseWorldY <=
                itemPositionY +
                halfHitboxHeight;

            if (insideX && insideY)
            {
                menuItem =
                    static_cast<MainMenuItem>(
                        index
                        );

                return true;
            }
        }

        return false;
    }

    void Application::HandleMainMenuInput()
    {
        constexpr std::size_t itemCount =
            static_cast<std::size_t>(
                MainMenuItem::Count
                );

        std::size_t selectedIndex =
            static_cast<std::size_t>(
                m_selectedMenuItem
                );

        bool selectionChanged = false;

        if (m_keyboard.WasPressed(Key::Up) ||
            m_keyboard.WasPressed(Key::W))
        {
            selectedIndex =
                (selectedIndex +
                    itemCount -
                    1) %
                itemCount;

            selectionChanged = true;
        }

        if (m_keyboard.WasPressed(Key::Down) ||
            m_keyboard.WasPressed(Key::S))
        {
            selectedIndex =
                (selectedIndex + 1) %
                itemCount;

            selectionChanged = true;
        }

        if (selectionChanged)
        {
            m_selectedMenuItem =
                static_cast<MainMenuItem>(
                    selectedIndex
                    );

            UpdateMenuTitle();
        }

        MainMenuItem hoveredItem{};

        // Change selection only when the mouse actually
        // moves. A stationary cursor will not block
        // keyboard navigation.
        if (m_mouse.WasMoved() &&
            TryGetHoveredMainMenuItem(
                hoveredItem
            ))
        {
            if (hoveredItem !=
                m_selectedMenuItem)
            {
                m_selectedMenuItem =
                    hoveredItem;

                UpdateMenuTitle();
            }
        }

        bool activateSelectedItem =
            m_keyboard.WasPressed(
                Key::Enter
            );

        if (m_mouse.WasLeftButtonPressed() &&
            TryGetHoveredMainMenuItem(
                hoveredItem
            ))
        {
            m_selectedMenuItem =
                hoveredItem;

            UpdateMenuTitle();

            activateSelectedItem =
                true;
        }

        if (!activateSelectedItem)
        {
            return;
        }

        switch (m_selectedMenuItem)
        {
        case MainMenuItem::LocalGame:
        {
            m_gameSession.Reset();

            EnterState(
                ApplicationState::LocalGame
            );

            break;
        }

        case MainMenuItem::HostGame:
        {
            EnterState(
                ApplicationState::HostGame
            );

            break;
        }

        case MainMenuItem::JoinGame:
        {
            EnterState(
                ApplicationState::JoinGame
            );

            break;
        }

        case MainMenuItem::Settings:
        {
            EnterState(
                ApplicationState::Settings
            );

            break;
        }

        case MainMenuItem::Exit:
        {
            m_exitRequested = true;
            break;
        }

        case MainMenuItem::Count:
        {
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
        const Player& player =
            m_gameSession.GetPlayer();

        m_quadRenderer.Draw(
            player.GetPositionX(),
            player.GetPositionY(),
            player.GetWidth(),
            player.GetHeight(),
            player.GetRotation(),
            m_aspectRatio
        );

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
    }

    void Application::RenderMainMenu()
    {
        constexpr std::size_t itemCount =
            static_cast<std::size_t>(
                MainMenuItem::Count
                );

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

        for (std::size_t index = 0;
            index < itemCount;
            ++index)
        {
            const bool isSelected =
                index ==
                static_cast<std::size_t>(
                    m_selectedMenuItem
                    );

            const float positionY =
                MainMenuFirstItemY -
                static_cast<float>(index) *
                MainMenuItemSpacing;

            const float width =
                isSelected
                ? MainMenuSelectedWidth
                : MainMenuNormalWidth;

            const float height =
                isSelected
                ? MainMenuSelectedHeight
                : MainMenuNormalHeight;

            m_quadRenderer.Draw(
                0.0f,
                positionY,
                width,
                height,
                0.0f,
                m_aspectRatio
            );
        }

        // Direct2D text is drawn after the Direct3D menu bars.
        m_textRenderer.Begin();

        m_textRenderer.Draw(
            L"ECHO",
            D2D1_RECT_F{
                0.0f,
                20.0f,
                clientWidth,
                115.0f
            },
            TextStyle::Title
        );

        for (std::size_t index = 0;
            index < itemCount;
            ++index)
        {
            const bool isSelected =
                index ==
                static_cast<std::size_t>(
                    m_selectedMenuItem
                    );

            const float positionY =
                MainMenuFirstItemY -
                static_cast<float>(index) *
                MainMenuItemSpacing;

            // Convert world Y to pixel Y.
            const float centerY =
                (1.0f - positionY) *
                0.5f *
                clientHeight;

            m_textRenderer.Draw(
                MainMenuLabels[index],
                D2D1_RECT_F{
                    0.0f,
                    centerY - 30.0f,
                    clientWidth,
                    centerY + 30.0f
                },
                TextStyle::MenuItem,
                isSelected
            );
        }

        m_textRenderer.Draw(
            L"W/S or Up/Down - Select | Enter - Confirm",
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

        case ApplicationState::Settings:
            screenTitle = L"Settings";
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
            std::wstring title =
                L"Echo | Main Menu | Selected: ";

            switch (m_selectedMenuItem)
            {
            case MainMenuItem::LocalGame:
                title += L"Local Game";
                break;

            case MainMenuItem::HostGame:
                title += L"Host Game";
                break;

            case MainMenuItem::JoinGame:
                title += L"Join Game";
                break;

            case MainMenuItem::Settings:
                title += L"Settings";
                break;

            case MainMenuItem::Exit:
                title += L"Exit";
                break;

            case MainMenuItem::Count:
                break;
            }

            title +=
                L" | W/S or Up/Down + Enter";

            m_window.SetTitle(title);

            break;
        }

        case ApplicationState::LocalGame:
        {
            m_window.SetTitle(
                L"Echo | Local Game | Escape: Menu"
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
                L"Echo | Settings placeholder | Escape: Back"
            );

            break;
        }
        }
    }

    PlayerCommand
        Application::BuildLocalPlayerCommand()
        const noexcept
    {
        PlayerCommand command{};

        if (m_keyboard.IsDown(Key::A))
        {
            command.movementX -= 1.0f;
        }

        if (m_keyboard.IsDown(Key::D))
        {
            command.movementX += 1.0f;
        }

        if (m_keyboard.IsDown(Key::W))
        {
            command.movementY += 1.0f;
        }

        if (m_keyboard.IsDown(Key::S))
        {
            command.movementY -= 1.0f;
        }

        const Player& player =
            m_gameSession.GetPlayer();

        // Preserve the current aim direction when the
        // cursor is outside the client area.
        command.aimX =
            player.GetForwardX();

        command.aimY =
            player.GetForwardY();

        if (!m_mouse.IsInsideWindow())
        {
            return command;
        }

        command.fire =
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
            return command;
        }

        // Convert pixel coordinates to normalized
        // device coordinates.
        const float normalizedMouseX =
            2.0f *
            static_cast<float>(
                m_mouse.GetX()
                ) /
            clientWidth -
            1.0f;

        // Windows Y grows downward.
        // World Y grows upward.
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
            player.GetPositionX();

        const float directionY =
            mouseWorldY -
            player.GetPositionY();

        const float directionLengthSquared =
            directionX * directionX +
            directionY * directionY;

        constexpr float DirectionEpsilon =
            0.000001f;

        if (directionLengthSquared <=
            DirectionEpsilon)
        {
            return command;
        }

        const float inverseLength =
            1.0f /
            std::sqrt(
                directionLengthSquared
            );

        command.aimX =
            directionX *
            inverseLength;

        command.aimY =
            directionY *
            inverseLength;

        return command;
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