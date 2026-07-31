#include "Platform/Windows/Window.h"

#include <stdexcept>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

namespace Echo
{
    Window::Window(
        int clientWidth,
        int clientHeight,
        const wchar_t* title
    )
    {
        m_instance = GetModuleHandleW(nullptr);

        if (m_instance == nullptr)
        {
            throw std::runtime_error(
                "Failed to get application instance."
            );
        }

        WNDCLASSW windowClass{};

        windowClass.lpfnWndProc = WindowProcedure;
        windowClass.hInstance = m_instance;
        windowClass.lpszClassName = m_className;
        windowClass.hCursor =
            LoadCursorW(nullptr, IDC_ARROW);

        windowClass.hbrBackground =
            static_cast<HBRUSH>(
                GetStockObject(BLACK_BRUSH)
                );

        if (RegisterClassW(&windowClass) == 0)
        {
            throw std::runtime_error(
                "Failed to register window class."
            );
        }

        RECT windowRectangle{
            0,
            0,
            clientWidth,
            clientHeight
        };

        if (!AdjustWindowRect(
            &windowRectangle,
            WS_OVERLAPPEDWINDOW,
            FALSE))
        {
            throw std::runtime_error(
                "Failed to calculate window size."
            );
        }

        const int windowWidth =
            windowRectangle.right -
            windowRectangle.left;

        const int windowHeight =
            windowRectangle.bottom -
            windowRectangle.top;

        m_handle = CreateWindowExW(
            0,
            m_className,
            title,
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            windowWidth,
            windowHeight,
            nullptr,
            nullptr,
            m_instance,
            nullptr
        );

        if (m_handle == nullptr)
        {
            throw std::runtime_error(
                "Failed to create window."
            );
        }

        ShowWindow(m_handle, SW_SHOW);
        UpdateWindow(m_handle);
    }

    Window::~Window()
    {
        if (m_handle != nullptr &&
            IsWindow(m_handle))
        {
            DestroyWindow(m_handle);
        }

        if (m_instance != nullptr)
        {
            UnregisterClassW(
                m_className,
                m_instance
            );
        }
    }

    bool Window::ProcessMessages() noexcept
    {
        MSG message{};

        while (PeekMessageW(
            &message,
            nullptr,
            0,
            0,
            PM_REMOVE))
        {
            if (message.message == WM_QUIT)
            {
                return false;
            }

            TranslateMessage(&message);
            DispatchMessageW(&message);
        }

        return true;
    }

    HWND Window::GetHandle() const noexcept
    {
        return m_handle;
    }

    LRESULT CALLBACK Window::WindowProcedure(
        HWND window,
        UINT message,
        WPARAM wParam,
        LPARAM lParam)
    {
        switch (message)
        {
        case WM_CLOSE:
        {
            DestroyWindow(window);
            return 0;
        }

        case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 0;
        }
        }

        return DefWindowProcW(
            window,
            message,
            wParam,
            lParam
        );
    }
}