#pragma once

#include "Platform/Windows/WindowsCommon.h"
#include "Input/Keyboard.h"
#include <string>

namespace Echo
{
    class Window final
    {
    public:
        Window(
            Keyboard& keyboard,
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

        // Returns true once for every detected resize.
        bool ConsumeResize(
            unsigned int& width,
            unsigned int& height
        ) noexcept;

    private:
        Keyboard& m_keyboard;

        LRESULT HandleMessage(
            HWND window,
            UINT message,
            WPARAM wParam,
            LPARAM lParam
        );

        static LRESULT CALLBACK WindowProcedure(
            HWND window,
            UINT message,
            WPARAM wParam,
            LPARAM lParam
        );

        HINSTANCE m_instance = nullptr;
        HWND m_handle = nullptr;

        unsigned int m_clientWidth = 0;
        unsigned int m_clientHeight = 0;

        bool m_resizePending = false;

        const wchar_t* m_className =
            L"EchoGameWindowClass";
    };
}