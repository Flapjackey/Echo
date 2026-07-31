#pragma once

#include <cstddef>

namespace Echo
{
    class Keyboard;
    class Mouse;
    class QuadRenderer;
    class TextRenderer;
    class Window;

    enum class MainMenuItem : std::size_t
    {
        LocalGame,
        HostGame,
        JoinGame,
        Settings,
        Exit,

        Count
    };

    enum class MainMenuAction
    {
        None,
        StartLocalGame,
        HostGame,
        JoinGame,
        OpenSettings,
        Exit
    };

    class MainMenu final
    {
    public:
        MainMenu() = default;

        MainMenuAction Update(
            const Keyboard& keyboard,
            const Mouse& mouse,
            const Window& window,
            float aspectRatio
        ) noexcept;

        void Render(
            QuadRenderer& quadRenderer,
            TextRenderer& textRenderer,
            const Window& window,
            float aspectRatio
        ) const;

        void Reset() noexcept;

    private:
        bool TryGetHoveredItem(
            const Mouse& mouse,
            const Window& window,
            float aspectRatio,
            MainMenuItem& item
        ) const noexcept;

        MainMenuItem m_selectedItem =
            MainMenuItem::LocalGame;
    };
}