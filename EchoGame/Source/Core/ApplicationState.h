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

    enum class MainMenuItem : std::size_t
    {
        LocalGame,
        HostGame,
        JoinGame,
        Settings,
        Exit,

        Count
    };

    enum class SettingsMenuItem : std::size_t
    {
        Fullscreen,
        VerticalSync,
        DebugOverlay,
        Back,

        Count
    };
}