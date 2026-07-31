#include "Platform/Windows/Window.h"

#include <stdexcept>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

namespace Echo
{
    namespace
    {
        bool TryMapKey(
            WPARAM virtualKey,
            Key& key
        ) noexcept
        {
            switch (virtualKey)
            {
            case 'W':
                key = Key::W;
                return true;

            case 'A':
                key = Key::A;
                return true;

            case 'S':
                key = Key::S;
                return true;

            case 'D':
                key = Key::D;
                return true;

            case VK_UP:
                key = Key::Up;
                return true;

            case VK_DOWN:
                key = Key::Down;
                return true;

            case VK_RETURN:
                key = Key::Enter;
                return true;

            case VK_ESCAPE:
                key = Key::Escape;
                return true;

            default:
                return false;
            }
        }
    }

    Window::Window(
        Keyboard& keyboard,
        Mouse& mouse,
        int clientWidth,
        int clientHeight,
        const wchar_t* title
    )
        : m_keyboard(keyboard),
        m_mouse(mouse),
        m_clientWidth(
            static_cast<unsigned int>(
                clientWidth
                )
        ),
        m_clientHeight(
            static_cast<unsigned int>(
                clientHeight
                )
        )
    {
        if (clientWidth <= 0 || clientHeight <= 0)
        {
            throw std::invalid_argument(
                "Window size must be greater than zero."
            );
        }

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

            // Pass the current C++ object to WindowProcedure.
            this
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

    unsigned int Window::GetClientWidth() const noexcept
    {
        return m_clientWidth;
    }

    unsigned int Window::GetClientHeight() const noexcept
    {
        return m_clientHeight;
    }

    void Window::SetTitle(
        const std::wstring& title
    ) noexcept
    {
        if (m_handle != nullptr)
        {
            SetWindowTextW(
                m_handle,
                title.c_str()
            );
        }
    }

    void Window::SetFullscreen(
        bool fullscreen
    ) noexcept
    {
        if (m_handle == nullptr ||
            fullscreen == m_isFullscreen)
        {
            return;
        }

        if (fullscreen)
        {
            m_windowedStyle =
                GetWindowLongPtrW(
                    m_handle,
                    GWL_STYLE
                );

            m_windowedPlacement = {};

            m_windowedPlacement.length =
                sizeof(WINDOWPLACEMENT);

            if (!GetWindowPlacement(
                m_handle,
                &m_windowedPlacement))
            {
                return;
            }

            const HMONITOR monitor =
                MonitorFromWindow(
                    m_handle,
                    MONITOR_DEFAULTTONEAREST
                );

            if (monitor == nullptr)
            {
                return;
            }

            MONITORINFO monitorInformation{};

            monitorInformation.cbSize =
                sizeof(MONITORINFO);

            if (!GetMonitorInfoW(
                monitor,
                &monitorInformation))
            {
                return;
            }

            const LONG_PTR fullscreenStyle =
                m_windowedStyle &
                ~static_cast<LONG_PTR>(
                    WS_OVERLAPPEDWINDOW
                    );

            SetWindowLongPtrW(
                m_handle,
                GWL_STYLE,
                fullscreenStyle
            );

            const RECT& monitorRectangle =
                monitorInformation.rcMonitor;

            if (!SetWindowPos(
                m_handle,
                HWND_TOP,
                monitorRectangle.left,
                monitorRectangle.top,
                monitorRectangle.right -
                monitorRectangle.left,
                monitorRectangle.bottom -
                monitorRectangle.top,
                SWP_NOOWNERZORDER |
                SWP_FRAMECHANGED))
            {
                SetWindowLongPtrW(
                    m_handle,
                    GWL_STYLE,
                    m_windowedStyle
                );

                SetWindowPlacement(
                    m_handle,
                    &m_windowedPlacement
                );

                return;
            }

            m_isFullscreen = true;

            return;
        }

        SetWindowLongPtrW(
            m_handle,
            GWL_STYLE,
            m_windowedStyle
        );

        m_windowedPlacement.length =
            sizeof(WINDOWPLACEMENT);

        SetWindowPlacement(
            m_handle,
            &m_windowedPlacement
        );

        SetWindowPos(
            m_handle,
            nullptr,
            0,
            0,
            0,
            0,
            SWP_NOMOVE |
            SWP_NOSIZE |
            SWP_NOZORDER |
            SWP_NOOWNERZORDER |
            SWP_FRAMECHANGED
        );

        m_isFullscreen = false;
    }

    bool Window::IsFullscreen() const noexcept
    {
        return m_isFullscreen;
    }

    bool Window::ConsumeResize(
        unsigned int& width,
        unsigned int& height
    ) noexcept
    {
        if (!m_resizePending)
        {
            return false;
        }

        width = m_clientWidth;
        height = m_clientHeight;

        m_resizePending = false;

        return true;
    }

    LRESULT Window::HandleMessage(
        HWND window,
        UINT message,
        WPARAM wParam,
        LPARAM lParam
    )
    {
        switch (message)
        {

        case WM_KEYDOWN:
        {
            Key key{};

            if (TryMapKey(wParam, key))
            {
                m_keyboard.SetKeyState(
                    key,
                    true
                );

                return 0;
            }

            break;
        }

        case WM_KEYUP:
        {
            Key key{};

            if (TryMapKey(wParam, key))
            {
                m_keyboard.SetKeyState(
                    key,
                    false
                );

                return 0;
            }

            break;
        }

        case WM_KILLFOCUS:
        {
            m_keyboard.Reset();
            m_mouse.Reset();

            return 0;
        }

        case WM_MOUSEMOVE:
        {
            const int mouseX =
                GET_X_LPARAM(lParam);

            const int mouseY =
                GET_Y_LPARAM(lParam);

            m_mouse.SetPosition(
                mouseX,
                mouseY
            );

            if (!m_mouse.IsInsideWindow())
            {
                TRACKMOUSEEVENT trackingEvent{};

                trackingEvent.cbSize =
                    sizeof(TRACKMOUSEEVENT);

                trackingEvent.dwFlags =
                    TME_LEAVE;

                trackingEvent.hwndTrack =
                    window;

                TrackMouseEvent(
                    &trackingEvent
                );

                m_mouse.SetInsideWindow(true);
            }

            return 0;
        }

        case WM_MOUSELEAVE:
        {
            m_mouse.SetInsideWindow(false);

            return 0;
        }

        case WM_LBUTTONDOWN:
        {
            m_mouse.SetLeftButtonState(true);

            SetCapture(window);

            return 0;
        }

        case WM_LBUTTONUP:
        {
            m_mouse.SetLeftButtonState(false);

            ReleaseCapture();

            return 0;
        }

        case WM_SIZE:
        {
            // Do not resize Direct3D buffers to 0x0
            // while the window is minimized.
            if (wParam == SIZE_MINIMIZED)
            {
                return 0;
            }

            const unsigned int width =
                static_cast<unsigned int>(
                    LOWORD(lParam)
                    );

            const unsigned int height =
                static_cast<unsigned int>(
                    HIWORD(lParam)
                    );

            if (width > 0 &&
                height > 0 &&
                (width != m_clientWidth ||
                    height != m_clientHeight))
            {
                m_clientWidth = width;
                m_clientHeight = height;
                m_resizePending = true;
            }

            return 0;
        }

        case WM_CLOSE:
        {
            DestroyWindow(window);
            return 0;
        }

        case WM_DESTROY:
        {
            m_handle = nullptr;

            PostQuitMessage(0);
            return 0;
        }

        case WM_NCDESTROY:
        {
            SetWindowLongPtrW(
                window,
                GWLP_USERDATA,
                0
            );

            return DefWindowProcW(
                window,
                message,
                wParam,
                lParam
            );
        }
        }

        return DefWindowProcW(
            window,
            message,
            wParam,
            lParam
        );
    }

    LRESULT CALLBACK Window::WindowProcedure(
        HWND window,
        UINT message,
        WPARAM wParam,
        LPARAM lParam
    )
    {
        Window* owner =
            reinterpret_cast<Window*>(
                GetWindowLongPtrW(
                    window,
                    GWLP_USERDATA
                )
                );

        if (message == WM_NCCREATE)
        {
            const auto* creationData =
                reinterpret_cast<CREATESTRUCTW*>(
                    lParam
                    );

            owner =
                static_cast<Window*>(
                    creationData->lpCreateParams
                    );

            if (owner != nullptr)
            {
                owner->m_handle = window;

                SetWindowLongPtrW(
                    window,
                    GWLP_USERDATA,
                    reinterpret_cast<LONG_PTR>(
                        owner
                        )
                );
            }
        }

        if (owner != nullptr)
        {
            return owner->HandleMessage(
                window,
                message,
                wParam,
                lParam
            );
        }

        return DefWindowProcW(
            window,
            message,
            wParam,
            lParam
        );
    }
}