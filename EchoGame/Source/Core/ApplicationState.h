#pragma once

#include <cstddef>

namespace Echo
{
    enum class ApplicationState
    {
        MainMenu,
        LocalGame,
        Paused,
        HostGame,
        JoinGame,
        Settings
    };
}