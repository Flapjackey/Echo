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

    void Keyboard::SetKeyState(
        Key key,
        bool isDown
    ) noexcept
    {
        const std::size_t index =
            static_cast<std::size_t>(key);

        m_keyStates[index] = isDown;
    }

    void Keyboard::Reset() noexcept
    {
        m_keyStates.fill(false);
    }
}