#pragma once

#include <cstddef>

namespace Echo
{
    enum class Key : std::size_t
    {
        W,
        A,
        S,
        D,

        Up,
        Down,
        Enter,
        Escape,

        Count
    };
}