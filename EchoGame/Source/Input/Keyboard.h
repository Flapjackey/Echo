#pragma once

#include "Input/Key.h"

#include <array>
#include <cstddef>

namespace Echo
{
    class Window;

    class Keyboard final
    {
    public:
        bool IsDown(
            Key key
        ) const noexcept;

        bool WasPressed(
            Key key
        ) const noexcept;

        void EndFrame() noexcept;

    private:
        friend class Window;

        void SetKeyState(
            Key key,
            bool isDown
        ) noexcept;

        void Reset() noexcept;

        static constexpr std::size_t KeyCount =
            static_cast<std::size_t>(
                Key::Count
                );

        std::array<bool, KeyCount>
            m_keyStates{};

        std::array<bool, KeyCount>
            m_pressedStates{};
    };
}