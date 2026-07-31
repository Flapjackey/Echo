#pragma once

#include "Platform/Windows/WindowsCommon.h"

#include <string>

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

        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        bool ProcessMessages() noexcept;

        HWND GetHandle() const noexcept;

        void SetTitle(
            const std::wstring& title
        ) noexcept;

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