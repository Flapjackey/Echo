#pragma once

#include "Window.h"

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