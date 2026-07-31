#pragma once

#include <cstddef>

namespace Echo
{
    class Keyboard;
    class Mouse;
    class TextRenderer;
    class Window;

    enum class PauseMenuItem : std::size_t
    {
        Resume,
        Settings,
        ReturnToMainMenu,
        Exit,

        Count
    };

    enum class PauseMenuAction
    {
        None,
        Resume,
        OpenSettings,
        ReturnToMainMenu,
        Exit
    };

    class PauseMenu final
    {
    public:
        PauseMenu() = default;

        PauseMenuAction Update(
            const Keyboard& keyboard,
            const Mouse& mouse,
            const Window& window
        ) noexcept;

        void Render(
            TextRenderer& textRenderer,
            const Window& window
        ) const;

        void Reset() noexcept;

    private:
        bool TryGetHoveredItem(
            const Mouse& mouse,
            const Window& window,
            PauseMenuItem& item
        ) const noexcept;

        PauseMenuItem m_selectedItem =
            PauseMenuItem::Resume;
    };
}