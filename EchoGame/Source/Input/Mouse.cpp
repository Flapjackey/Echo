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

    bool Mouse::IsInsideWindow()
        const noexcept
    {
        return m_isInsideWindow;
    }

    bool Mouse::IsLeftButtonDown()
        const noexcept
    {
        return m_isLeftButtonDown;
    }

    bool Mouse::WasMoved() const noexcept
    {
        return m_movedThisFrame;
    }

    bool Mouse::WasLeftButtonPressed()
        const noexcept
    {
        return
            m_leftButtonPressedThisFrame;
    }

    void Mouse::EndFrame() noexcept
    {
        m_movedThisFrame = false;

        m_leftButtonPressedThisFrame =
            false;
    }

    void Mouse::SetPosition(
        int x,
        int y
    ) noexcept
    {
        if (x != m_x ||
            y != m_y)
        {
            m_movedThisFrame = true;
        }

        m_x = x;
        m_y = y;
    }

    void Mouse::SetInsideWindow(
        bool isInside
    ) noexcept
    {
        m_isInsideWindow =
            isInside;
    }

    void Mouse::SetLeftButtonState(
        bool isDown
    ) noexcept
    {
        if (isDown &&
            !m_isLeftButtonDown)
        {
            m_leftButtonPressedThisFrame =
                true;
        }

        m_isLeftButtonDown =
            isDown;
    }

    void Mouse::Reset() noexcept
    {
        m_isInsideWindow = false;
        m_isLeftButtonDown = false;

        m_movedThisFrame = false;

        m_leftButtonPressedThisFrame =
            false;
    }
}