#pragma once

#include "Platform/Windows/WindowsCommon.h"
#include "Input/Keyboard.h"
#include "Input/Mouse.h"
#include <string>

namespace Echo
{
    class Window final
    {
    public:
        Window(
            Keyboard& keyboard,
            Mouse& mouse,
            int clientWidth,
            int clientHeight,
            const wchar_t* title
        );

        ~Window();

        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        bool ProcessMessages() noexcept;

        HWND GetHandle() const noexcept;

        unsigned int GetClientWidth() const noexcept;
        unsigned int GetClientHeight() const noexcept;

        void SetTitle(
            const std::wstring& title
        ) noexcept;

        void SetFullscreen(
            bool fullscreen
        ) noexcept;

        bool IsFullscreen() const noexcept;

        // Returns true once for every detected resize.
        bool ConsumeResize(
            unsigned int& width,
            unsigned int& height
        ) noexcept;

    private:
        Keyboard& m_keyboard;
        Mouse& m_mouse;

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
        bool m_isFullscreen = false;

        LONG_PTR m_windowedStyle =
            WS_OVERLAPPEDWINDOW;

        WINDOWPLACEMENT m_windowedPlacement{};

        const wchar_t* m_className =
            L"EchoGameWindowClass";
    };
}