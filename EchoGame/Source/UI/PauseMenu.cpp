#include "UI/PauseMenu.h"

#include "Graphics/TextRenderer.h"
#include "Input/Keyboard.h"
#include "Input/Mouse.h"
#include "Platform/Windows/Window.h"

#include <algorithm>

namespace
{
    constexpr const wchar_t*
        PauseMenuLabels[]
    {
        L"Resume",
        L"Settings",
        L"Return to Main Menu",
        L"Exit"
    };

    constexpr std::size_t PauseMenuItemCount =
        sizeof(PauseMenuLabels) /
        sizeof(PauseMenuLabels[0]);

    static_assert(
        PauseMenuItemCount ==
        static_cast<std::size_t>(
            Echo::PauseMenuItem::Count
            )
        );

    D2D1_RECT_F GetPauseMenuItemRectangle(
        std::size_t index,
        float clientWidth,
        float clientHeight
    ) noexcept
    {
        const float centerX =
            clientWidth * 0.5f;

        const float halfWidth =
            std::min(
                270.0f,
                clientWidth * 0.38f
            );

        const float firstCenterY =
            clientHeight * 0.34f;

        const float itemSpacing =
            clientHeight * 0.11f;

        const float centerY =
            firstCenterY +
            static_cast<float>(index) *
            itemSpacing;

        const float halfHeight =
            std::max(
                24.0f,
                clientHeight * 0.035f
            );

        return D2D1_RECT_F
        {
            centerX - halfWidth,
            centerY - halfHeight,
            centerX + halfWidth,
            centerY + halfHeight
        };
    }
}

namespace Echo
{
    PauseMenuAction PauseMenu::Update(
        const Keyboard& keyboard,
        const Mouse& mouse,
        const Window& window
    ) noexcept
    {
        // Escape resumes the game immediately.
        if (keyboard.WasPressed(
            Key::Escape))
        {
            return PauseMenuAction::Resume;
        }

        constexpr std::size_t itemCount =
            static_cast<std::size_t>(
                PauseMenuItem::Count
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
                static_cast<PauseMenuItem>(
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
                static_cast<PauseMenuItem>(
                    selectedIndex
                    );
        }

        PauseMenuItem hoveredItem{};

        // A stationary cursor does not override
        // keyboard navigation.
        if (mouse.WasMoved() &&
            TryGetHoveredItem(
                mouse,
                window,
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
            return PauseMenuAction::None;
        }

        switch (m_selectedItem)
        {
        case PauseMenuItem::Resume:
        {
            return PauseMenuAction::Resume;
        }

        case PauseMenuItem::Settings:
        {
            return
                PauseMenuAction::OpenSettings;
        }

        case PauseMenuItem::ReturnToMainMenu:
        {
            return PauseMenuAction::
                ReturnToMainMenu;
        }

        case PauseMenuItem::Exit:
        {
            return PauseMenuAction::Exit;
        }

        case PauseMenuItem::Count:
        {
            break;
        }
        }

        return PauseMenuAction::None;
    }

    void PauseMenu::Render(
        TextRenderer& textRenderer,
        const Window& window
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

        textRenderer.Begin();

        textRenderer.Draw(
            L"PAUSED",
            D2D1_RECT_F
            {
                0.0f,
                clientHeight * 0.08f,
                clientWidth,
                clientHeight * 0.23f
            },
            TextStyle::Title
        );

        constexpr std::size_t itemCount =
            static_cast<std::size_t>(
                PauseMenuItem::Count
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

            textRenderer.Draw(
                PauseMenuLabels[index],
                GetPauseMenuItemRectangle(
                    index,
                    clientWidth,
                    clientHeight
                ),
                TextStyle::MenuItem,
                isSelected
            );
        }

        textRenderer.Draw(
            L"W/S, Arrows or Mouse - Select | "
            L"Enter or Click - Confirm | "
            L"Escape - Resume",
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

    void PauseMenu::Reset() noexcept
    {
        m_selectedItem =
            PauseMenuItem::Resume;
    }

    bool PauseMenu::TryGetHoveredItem(
        const Mouse& mouse,
        const Window& window,
        PauseMenuItem& item
    ) const noexcept
    {
        if (!mouse.IsInsideWindow())
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

        const float mouseX =
            static_cast<float>(
                mouse.GetX()
                );

        const float mouseY =
            static_cast<float>(
                mouse.GetY()
                );

        constexpr std::size_t itemCount =
            static_cast<std::size_t>(
                PauseMenuItem::Count
                );

        for (std::size_t index = 0;
            index < itemCount;
            ++index)
        {
            const D2D1_RECT_F rectangle =
                GetPauseMenuItemRectangle(
                    index,
                    clientWidth,
                    clientHeight
                );

            const bool insideX =
                mouseX >= rectangle.left &&
                mouseX <= rectangle.right;

            const bool insideY =
                mouseY >= rectangle.top &&
                mouseY <= rectangle.bottom;

            if (insideX && insideY)
            {
                item =
                    static_cast<PauseMenuItem>(
                        index
                        );

                return true;
            }
        }

        return false;
    }
}