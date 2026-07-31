#pragma once

#include <cstddef>

namespace Echo
{
    struct GameSettings;

    class Keyboard;
    class Mouse;
    class QuadRenderer;
    class TextRenderer;
    class Window;

    enum class SettingsMenuItem : std::size_t
    {
        Fullscreen,
        VerticalSync,
        DebugOverlay,
        Back,

        Count
    };

    enum class SettingsMenuAction
    {
        None,
        ToggleFullscreen,
        ToggleVerticalSync,
        ToggleDebugOverlay,
        Back
    };

    class SettingsMenu final
    {
    public:
        SettingsMenu() = default;

        SettingsMenuAction Update(
            const Keyboard& keyboard,
            const Mouse& mouse,
            const Window& window,
            float aspectRatio
        ) noexcept;

        void Render(
            QuadRenderer& quadRenderer,
            TextRenderer& textRenderer,
            const Window& window,
            const GameSettings& settings,
            float aspectRatio
        ) const;

        void Reset() noexcept;

    private:
        bool TryGetHoveredItem(
            const Mouse& mouse,
            const Window& window,
            float aspectRatio,
            SettingsMenuItem& item
        ) const noexcept;

        SettingsMenuItem m_selectedItem =
            SettingsMenuItem::Fullscreen;
    };
}