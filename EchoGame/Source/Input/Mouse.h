#pragma once

namespace Echo
{
    class Window;

    class Mouse final
    {
    public:
        int GetX() const noexcept;
        int GetY() const noexcept;
        bool IsLeftButtonDown() const noexcept;
        bool IsInsideWindow() const noexcept;

    private:
        // Only Window may change mouse state.
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

        int m_x = 0;
        int m_y = 0;

        bool m_isInsideWindow = false;
        bool m_isLeftButtonDown = false;
    };
}