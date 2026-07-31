#pragma once

namespace Echo
{
    class Window;

    class Mouse final
    {
    public:
        int GetX() const noexcept;
        int GetY() const noexcept;

        bool IsInsideWindow() const noexcept;
        bool IsLeftButtonDown() const noexcept;

        bool WasMoved() const noexcept;

        bool WasLeftButtonPressed()
            const noexcept;

        void EndFrame() noexcept;

    private:
        friend class Window;

        void SetPosition(
            int x,
            int y
        ) noexcept;

        void SetInsideWindow(
            bool isInside
        ) noexcept;

        void SetLeftButtonState(
            bool isDown
        ) noexcept;

        void Reset() noexcept;

        int m_x = 0;
        int m_y = 0;

        bool m_isInsideWindow = false;
        bool m_isLeftButtonDown = false;

        bool m_movedThisFrame = false;

        bool m_leftButtonPressedThisFrame =
            false;
    };
}