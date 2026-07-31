#include "Core/Application.h"

#include <chrono>
#include <thread>

namespace Echo
{
    Application::Application()
        : m_window(
            1280,
            720,
            L"EchoGame")
    {
    }

    int Application::Run()
    {
        using namespace std::chrono_literals;

        while (m_window.ProcessMessages())
        {
            // Позже здесь будет:
            //
            // 1. Расчёт времени кадра.
            // 2. Чтение управления.
            // 3. Обновление игрового мира.
            // 4. Отрисовка кадра.

            // Пока немного разгружаем процессор.
            std::this_thread::sleep_for(1ms);
        }

        return 0;
    }
}