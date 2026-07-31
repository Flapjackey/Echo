#pragma once

#include <cstddef>

namespace Echo
{
    enum class ApplicationState
    {
        MainMenu,
        LocalGame,
        HostGame,
        JoinGame,
        Settings
    };

    enum class MainMenuItem : std::size_t
    {
        LocalGame,
        HostGame,
        JoinGame,
        Settings,
        Exit,

        Count
    };
}