#include "UI/MainMenu.h"

#include "Graphics/QuadRenderer.h"
#include "Graphics/TextRenderer.h"
#include "Input/Keyboard.h"
#include "Input/Mouse.h"
#include "Platform/Windows/Window.h"

namespace
{
    constexpr const wchar_t*
        MainMenuLabels[]
    {
        L"Local Game",
        L"Host Game",
        L"Join Game",
        L"Settings",
        L"Exit"
    };

    constexpr std::size_t MainMenuItemCount =
        sizeof(MainMenuLabels) /
        sizeof(MainMenuLabels[0]);

    static_assert(
        MainMenuItemCount ==
        static_cast<std::size_t>(
            Echo::MainMenuItem::Count
            )
        );

    constexpr float FirstItemPositionY =
        0.56f;

    constexpr float ItemSpacing =
        0.28f;

    constexpr float NormalWidth =
        0.90f;

    constexpr float SelectedWidth =
        1.25f;

    constexpr float NormalHeight =
        0.12f;

    constexpr float SelectedHeight =
        0.18f;

    constexpr float HitboxWidth =
        1.25f;

    constexpr float HitboxHeight =
        0.22f;

    float GetItemPositionY(
        std::size_t index
    ) noexcept
    {
        return
            FirstItemPositionY -
            static_cast<float>(index) *
            ItemSpacing;
    }
}

namespace Echo
{
    MainMenuAction MainMenu::Update(
        const Keyboard& keyboard,
        const Mouse& mouse,
        const Window& window,
        float aspectRatio
    ) noexcept
    {
        constexpr std::size_t itemCount =
            static_cast<std::size_t>(
                MainMenuItem::Count
                );

        std::size_t selectedIndex =
            static_cast<std::size_t>(
                m_selectedItem
                );

        if (keyboard.WasPressed(Key::Up) ||
            keyboard.WasPressed(Key::W))
        {
            selectedIndex =
                (selectedIndex +
                    itemCount -
                    1) %
                itemCount;

            m_selectedItem =
                static_cast<MainMenuItem>(
                    selectedIndex
                    );
        }

        if (keyboard.WasPressed(Key::Down) ||
            keyboard.WasPressed(Key::S))
        {
            selectedIndex =
                (selectedIndex + 1) %
                itemCount;

            m_selectedItem =
                static_cast<MainMenuItem>(
                    selectedIndex
                    );
        }

        MainMenuItem hoveredItem{};

        // A stationary cursor does not override
        // keyboard navigation.
        if (mouse.WasMoved() &&
            TryGetHoveredItem(
                mouse,
                window,
                aspectRatio,
                hoveredItem
            ))
        {
            m_selectedItem =
                hoveredItem;
        }

        bool activateSelectedItem =
            keyboard.WasPressed(
                Key::Enter
            );

        if (mouse.WasLeftButtonPressed() &&
            TryGetHoveredItem(
                mouse,
                window,
                aspectRatio,
                hoveredItem
            ))
        {
            m_selectedItem =
                hoveredItem;

            activateSelectedItem =
                true;
        }

        if (!activateSelectedItem)
        {
            return MainMenuAction::None;
        }

        switch (m_selectedItem)
        {
        case MainMenuItem::LocalGame:
        {
            return
                MainMenuAction::StartLocalGame;
        }

        case MainMenuItem::HostGame:
        {
            return MainMenuAction::HostGame;
        }

        case MainMenuItem::JoinGame:
        {
            return MainMenuAction::JoinGame;
        }

        case MainMenuItem::Settings:
        {
            return
                MainMenuAction::OpenSettings;
        }

        case MainMenuItem::Exit:
        {
            return MainMenuAction::Exit;
        }

        case MainMenuItem::Count:
        {
            break;
        }
        }

        return MainMenuAction::None;
    }

    void MainMenu::Render(
        QuadRenderer& quadRenderer,
        TextRenderer& textRenderer,
        const Window& window,
        float aspectRatio
    ) const
    {
        const float clientWidth =
            static_cast<float>(
                window.GetClientWidth()
                );

        const float clientHeight =
            static_cast<float>(
                window.GetClientHeight()
                );

        if (clientWidth <= 0.0f ||
            clientHeight <= 0.0f)
        {
            return;
        }

        constexpr std::size_t itemCount =
            static_cast<std::size_t>(
                MainMenuItem::Count
                );

        // Draw menu item backgrounds with Direct3D.
        for (std::size_t index = 0;
            index < itemCount;
            ++index)
        {
            const bool isSelected =
                index ==
                static_cast<std::size_t>(
                    m_selectedItem
                    );

            const float positionY =
                GetItemPositionY(index);

            const float width =
                isSelected
                ? SelectedWidth
                : NormalWidth;

            const float height =
                isSelected
                ? SelectedHeight
                : NormalHeight;

            quadRenderer.Draw(
                0.0f,
                positionY,
                width,
                height,
                0.0f,
                aspectRatio
            );
        }

        // Draw menu text with DirectWrite.
        textRenderer.Begin();

        textRenderer.Draw(
            L"ECHO",
            D2D1_RECT_F
            {
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
                    m_selectedItem
                    );

            const float positionY =
                GetItemPositionY(index);

            // Convert world Y to window pixels.
            const float centerY =
                (1.0f - positionY) *
                0.5f *
                clientHeight;

            textRenderer.Draw(
                MainMenuLabels[index],
                D2D1_RECT_F
                {
                    0.0f,
                    centerY - 30.0f,
                    clientWidth,
                    centerY + 30.0f
                },
                TextStyle::MenuItem,
                isSelected
            );
        }

        textRenderer.Draw(
            L"W/S, Arrows or Mouse - Select | "
            L"Enter or Click - Confirm",
            D2D1_RECT_F
            {
                0.0f,
                clientHeight - 55.0f,
                clientWidth,
                clientHeight - 10.0f
            },
            TextStyle::Hint
        );

        textRenderer.End();
    }

    void MainMenu::Reset() noexcept
    {
        m_selectedItem =
            MainMenuItem::LocalGame;
    }

    bool MainMenu::TryGetHoveredItem(
        const Mouse& mouse,
        const Window& window,
        float aspectRatio,
        MainMenuItem& item
    ) const noexcept
    {
        if (!mouse.IsInsideWindow() ||
            aspectRatio <= 0.0f)
        {
            return false;
        }

        const float clientWidth =
            static_cast<float>(
                window.GetClientWidth()
                );

        const float clientHeight =
            static_cast<float>(
                window.GetClientHeight()
                );

        if (clientWidth <= 0.0f ||
            clientHeight <= 0.0f)
        {
            return false;
        }

        const float normalizedMouseX =
            2.0f *
            static_cast<float>(
                mouse.GetX()
                ) /
            clientWidth -
            1.0f;

        const float normalizedMouseY =
            1.0f -
            2.0f *
            static_cast<float>(
                mouse.GetY()
                ) /
            clientHeight;

        // QuadRenderer divides world X by aspectRatio.
        // Reverse that operation for hit testing.
        const float mouseWorldX =
            normalizedMouseX *
            aspectRatio;

        const float mouseWorldY =
            normalizedMouseY;

        constexpr std::size_t itemCount =
            static_cast<std::size_t>(
                MainMenuItem::Count
                );

        const float halfHitboxWidth =
            HitboxWidth *
            0.5f;

        const float halfHitboxHeight =
            HitboxHeight *
            0.5f;

        for (std::size_t index = 0;
            index < itemCount;
            ++index)
        {
            const float itemPositionY =
                GetItemPositionY(index);

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
                item =
                    static_cast<MainMenuItem>(
                        index
                        );

                return true;
            }
        }

        return false;
    }
}