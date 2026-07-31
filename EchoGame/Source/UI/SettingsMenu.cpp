#include "UI/SettingsMenu.h"

#include "Core/GameSettings.h"
#include "Graphics/QuadRenderer.h"
#include "Graphics/TextRenderer.h"
#include "Input/Keyboard.h"
#include "Input/Mouse.h"
#include "Platform/Windows/Window.h"

namespace
{
    constexpr const wchar_t*
        SettingsMenuLabels[]
    {
        L"Fullscreen",
        L"VSync",
        L"Debug Overlay",
        L"Back"
    };

    constexpr std::size_t SettingsMenuItemCount =
        sizeof(SettingsMenuLabels) /
        sizeof(SettingsMenuLabels[0]);

    static_assert(
        SettingsMenuItemCount ==
        static_cast<std::size_t>(
            Echo::SettingsMenuItem::Count
            )
        );

    constexpr float FirstItemPositionY =
        0.38f;

    constexpr float ItemSpacing =
        0.30f;

    constexpr float NormalWidth =
        1.15f;

    constexpr float SelectedWidth =
        1.50f;

    constexpr float NormalHeight =
        0.12f;

    constexpr float SelectedHeight =
        0.18f;

    constexpr float HitboxWidth =
        1.50f;

    constexpr float HitboxHeight =
        0.23f;

    float GetItemPositionY(
        std::size_t index
    ) noexcept
    {
        return
            FirstItemPositionY -
            static_cast<float>(index) *
            ItemSpacing;
    }

    constexpr const wchar_t* ToOnOff(
        bool enabled
    ) noexcept
    {
        return enabled
            ? L"ON"
            : L"OFF";
    }
}

namespace Echo
{
    SettingsMenuAction SettingsMenu::Update(
        const Keyboard& keyboard,
        const Mouse& mouse,
        const Window& window,
        float aspectRatio
    ) noexcept
    {
        if (keyboard.WasPressed(
            Key::Escape))
        {
            return SettingsMenuAction::Back;
        }

        constexpr std::size_t itemCount =
            static_cast<std::size_t>(
                SettingsMenuItem::Count
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
                static_cast<SettingsMenuItem>(
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
                static_cast<SettingsMenuItem>(
                    selectedIndex
                    );
        }

        SettingsMenuItem hoveredItem{};

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
            return SettingsMenuAction::None;
        }

        switch (m_selectedItem)
        {
        case SettingsMenuItem::Fullscreen:
        {
            return
                SettingsMenuAction::
                ToggleFullscreen;
        }

        case SettingsMenuItem::VerticalSync:
        {
            return
                SettingsMenuAction::
                ToggleVerticalSync;
        }

        case SettingsMenuItem::DebugOverlay:
        {
            return
                SettingsMenuAction::
                ToggleDebugOverlay;
        }

        case SettingsMenuItem::Back:
        {
            return SettingsMenuAction::Back;
        }

        case SettingsMenuItem::Count:
        {
            break;
        }
        }

        return SettingsMenuAction::None;
    }

    void SettingsMenu::Render(
        QuadRenderer& quadRenderer,
        TextRenderer& textRenderer,
        const Window& window,
        const GameSettings& settings,
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
                SettingsMenuItem::Count
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
            L"SETTINGS",
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
            const SettingsMenuItem item =
                static_cast<SettingsMenuItem>(
                    index
                    );

            const bool isSelected =
                item == m_selectedItem;

            const float positionY =
                GetItemPositionY(index);

            const float centerY =
                (1.0f - positionY) *
                0.5f *
                clientHeight;

            if (item ==
                SettingsMenuItem::Back)
            {
                textRenderer.Draw(
                    SettingsMenuLabels[index],
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

                continue;
            }

            const wchar_t* value = L"OFF";

            switch (item)
            {
            case SettingsMenuItem::Fullscreen:
            {
                value = ToOnOff(
                    settings.fullscreen
                );

                break;
            }

            case SettingsMenuItem::VerticalSync:
            {
                value = ToOnOff(
                    settings.verticalSync
                );

                break;
            }

            case SettingsMenuItem::DebugOverlay:
            {
                value = ToOnOff(
                    settings.debugOverlay
                );

                break;
            }

            case SettingsMenuItem::Back:
            case SettingsMenuItem::Count:
            {
                break;
            }
            }

            textRenderer.Draw(
                SettingsMenuLabels[index],
                D2D1_RECT_F
                {
                    clientWidth * 0.12f,
                    centerY - 30.0f,
                    clientWidth * 0.63f,
                    centerY + 30.0f
                },
                TextStyle::MenuItem,
                isSelected
            );

            textRenderer.Draw(
                value,
                D2D1_RECT_F
                {
                    clientWidth * 0.63f,
                    centerY - 30.0f,
                    clientWidth * 0.88f,
                    centerY + 30.0f
                },
                TextStyle::MenuItem,
                isSelected
            );
        }

        textRenderer.Draw(
            L"W/S, Arrows or Mouse - Select | "
            L"Enter or Click - Change | "
            L"Escape - Back",
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

    void SettingsMenu::Reset() noexcept
    {
        m_selectedItem =
            SettingsMenuItem::Fullscreen;
    }

    bool SettingsMenu::TryGetHoveredItem(
        const Mouse& mouse,
        const Window& window,
        float aspectRatio,
        SettingsMenuItem& item
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

        const float mouseWorldX =
            normalizedMouseX *
            aspectRatio;

        const float mouseWorldY =
            normalizedMouseY;

        constexpr std::size_t itemCount =
            static_cast<std::size_t>(
                SettingsMenuItem::Count
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
                    static_cast<SettingsMenuItem>(
                        index
                        );

                return true;
            }
        }

        return false;
    }
}