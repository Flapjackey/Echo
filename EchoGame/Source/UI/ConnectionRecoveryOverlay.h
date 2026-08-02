#pragma once

#include <string_view>

namespace Echo
{
    class Keyboard;
    class Mouse;
    class TextRenderer;
    class Window;

    enum class ConnectionRecoveryAction
    {
        None,
        ContinueSolo
    };

    class ConnectionRecoveryOverlay final
    {
    public:
        ConnectionRecoveryOverlay() =
            default;

        ConnectionRecoveryAction Update(
            const Keyboard& keyboard,
            const Mouse& mouse,
            const Window& window,
            bool canContinueSolo
        ) noexcept;

        void Render(
            TextRenderer& textRenderer,
            const Window& window,
            double remainingSeconds,
            bool showTimer,
            bool canContinueSolo,
            std::wstring_view title,
            std::wstring_view message,
            std::wstring_view networkStatus
        ) const;

        void Reset() noexcept;

    private:
        bool IsMouseOverButton(
            const Mouse& mouse,
            const Window& window
        ) const noexcept;

        bool m_buttonHovered =
            false;
    };
}