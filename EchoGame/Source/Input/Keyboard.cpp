#include "Input/Keyboard.h"

namespace Echo
{
    bool Keyboard::IsDown(
        Key key
    ) const noexcept
    {
        const std::size_t index =
            static_cast<std::size_t>(key);

        return m_keyStates[index];
    }

    bool Keyboard::WasPressed(
        Key key
    ) const noexcept
    {
        const std::size_t index =
            static_cast<std::size_t>(key);

        return m_pressedStates[index];
    }

    void Keyboard::EndFrame() noexcept
    {
        m_pressedStates.fill(false);
    }

    void Keyboard::SetKeyState(
        Key key,
        bool isDown
    ) noexcept
    {
        const std::size_t index =
            static_cast<std::size_t>(key);

        if (isDown &&
            !m_keyStates[index])
        {
            m_pressedStates[index] =
                true;
        }

        m_keyStates[index] =
            isDown;
    }

    void Keyboard::Reset() noexcept
    {
        m_keyStates.fill(false);
        m_pressedStates.fill(false);
    }
}