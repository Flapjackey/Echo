#include "Input/Mouse.h"

namespace Echo
{
    int Mouse::GetX() const noexcept
    {
        return m_x;
    }

    int Mouse::GetY() const noexcept
    {
        return m_y;
    }

    bool Mouse::IsInsideWindow() const noexcept
    {
        return m_isInsideWindow;
    }

    bool Mouse::IsLeftButtonDown() const noexcept
    {
        return m_isLeftButtonDown;
    }

    void Mouse::SetPosition(
        int x,
        int y
    ) noexcept
    {
        m_x = x;
        m_y = y;
    }

    void Mouse::SetInsideWindow(
        bool isInside
    ) noexcept
    {
        m_isInsideWindow = isInside;
    }

    void Mouse::SetLeftButtonState(
        bool isDown
    ) noexcept
    {
        m_isLeftButtonDown = isDown;
    }
}