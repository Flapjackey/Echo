#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>

namespace Echo
{
    class Window final
    {
    public:
        Window(
            int clientWidth,
            int clientHeight,
            const wchar_t* title
        );

        ~Window();

        // Запрещаем случайное копирование окна.
        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        // Обрабатывает сообщения Windows.
        // Возвращает false, когда программу нужно закрыть.
        bool ProcessMessages() noexcept;

        HWND GetHandle() const noexcept;

    private:
        static LRESULT CALLBACK WindowProcedure(
            HWND window,
            UINT message,
            WPARAM wParam,
            LPARAM lParam
        );

        HINSTANCE m_instance = nullptr;
        HWND m_handle = nullptr;

        const wchar_t* m_className =
            L"EchoGameWindowClass";
    };
}