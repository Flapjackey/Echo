#pragma once

#include "Platform/Windows/Window.h"

namespace Echo
{
    class Application final
    {
    public:
        Application();

        int Run();

    private:
        Window m_window;
    };
}