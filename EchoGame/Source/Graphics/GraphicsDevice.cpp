#include "Graphics/GraphicsDevice.h"

#include <d2d1_1helper.h>

#include <stdexcept>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

namespace
{
    void ThrowIfFailed(
        HRESULT result,
        const char* message
    )
    {
        if (FAILED(result))
        {
            throw std::runtime_error(message);
        }
    }
}

namespace Echo
{
    GraphicsDevice::GraphicsDevice(
        HWND windowHandle,
        unsigned int width,
        unsigned int height
    )
    {
        DXGI_SWAP_CHAIN_DESC swapChainDescription{};

        swapChainDescription.BufferDesc.Width =
            width;

        swapChainDescription.BufferDesc.Height =
            height;

        swapChainDescription.BufferDesc.Format =
            DXGI_FORMAT_B8G8R8A8_UNORM;

        swapChainDescription.BufferDesc.RefreshRate.Numerator =
            0;

        swapChainDescription.BufferDesc.RefreshRate.Denominator =
            1;

        swapChainDescription.SampleDesc.Count =
            1;

        swapChainDescription.SampleDesc.Quality =
            0;

        swapChainDescription.BufferUsage =
            DXGI_USAGE_RENDER_TARGET_OUTPUT;

        swapChainDescription.BufferCount =
            2;

        swapChainDescription.OutputWindow =
            windowHandle;

        swapChainDescription.Windowed =
            TRUE;

        swapChainDescription.SwapEffect =
            DXGI_SWAP_EFFECT_DISCARD;

        unsigned int creationFlags =
            D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#ifdef _DEBUG
        creationFlags |=
            D3D11_CREATE_DEVICE_DEBUG;
#endif

        const auto createDevice =
            [&](unsigned int flags)
            {
                return D3D11CreateDeviceAndSwapChain(
                    nullptr,
                    D3D_DRIVER_TYPE_HARDWARE,
                    nullptr,
                    flags,
                    nullptr,
                    0,
                    D3D11_SDK_VERSION,
                    &swapChainDescription,
                    m_swapChain.GetAddressOf(),
                    m_device.GetAddressOf(),
                    nullptr,
                    m_context.GetAddressOf()
                );
            };

        HRESULT result =
            createDevice(creationFlags);

#ifdef _DEBUG
        // The Direct3D debug layer may not be installed.
        // In that case, retry without it.
        if (FAILED(result))
        {
            m_swapChain.Reset();
            m_device.Reset();
            m_context.Reset();

            creationFlags &=
                ~D3D11_CREATE_DEVICE_DEBUG;

            result =
                createDevice(creationFlags);
        }
#endif

        ThrowIfFailed(
            result,
            "Failed to create Direct3D device and swap chain."
        );

        D2D1_FACTORY_OPTIONS
            factoryOptions{};

        ThrowIfFailed(
            D2D1CreateFactory(
                D2D1_FACTORY_TYPE_SINGLE_THREADED,
                __uuidof(ID2D1Factory1),
                &factoryOptions,
                reinterpret_cast<void**>(
                    m_d2dFactory.GetAddressOf()
                    )
            ),
            "Failed to create Direct2D factory."
        );

        Microsoft::WRL::ComPtr<IDXGIDevice>
            dxgiDevice;

        ThrowIfFailed(
            m_device.As(
                &dxgiDevice
            ),
            "Failed to get DXGI device."
        );

        ThrowIfFailed(
            m_d2dFactory->CreateDevice(
                dxgiDevice.Get(),
                m_d2dDevice.GetAddressOf()
            ),
            "Failed to create Direct2D device."
        );

        ThrowIfFailed(
            m_d2dDevice->CreateDeviceContext(
                D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
                m_d2dContext.GetAddressOf()
            ),
            "Failed to create Direct2D device context."
        );

        CreateRenderTarget(
            width,
            height
        );
    }

    void GraphicsDevice::CreateRenderTarget(
        unsigned int width,
        unsigned int height
    )
    {
        Microsoft::WRL::ComPtr<ID3D11Texture2D>
            backBuffer;

        ThrowIfFailed(
            m_swapChain->GetBuffer(
                0,
                IID_PPV_ARGS(
                    backBuffer.GetAddressOf()
                )
            ),
            "Failed to get Direct3D back buffer."
        );

        ThrowIfFailed(
            m_device->CreateRenderTargetView(
                backBuffer.Get(),
                nullptr,
                m_renderTargetView.GetAddressOf()
            ),
            "Failed to create render target view."
        );

        Microsoft::WRL::ComPtr<IDXGISurface>
            backBufferSurface;

        ThrowIfFailed(
            backBuffer.As(
                &backBufferSurface
            ),
            "Failed to get DXGI back buffer surface."
        );

        const D2D1_BITMAP_PROPERTIES1
            bitmapProperties =
            D2D1::BitmapProperties1(
                D2D1_BITMAP_OPTIONS_TARGET |
                D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
                D2D1::PixelFormat(
                    DXGI_FORMAT_B8G8R8A8_UNORM,
                    D2D1_ALPHA_MODE_IGNORE
                ),
                96.0f,
                96.0f
            );

        ThrowIfFailed(
            m_d2dContext->CreateBitmapFromDxgiSurface(
                backBufferSurface.Get(),
                &bitmapProperties,
                m_d2dTargetBitmap.GetAddressOf()
            ),
            "Failed to create Direct2D target bitmap."
        );

        m_d2dContext->SetTarget(
            m_d2dTargetBitmap.Get()
        );

        D3D11_VIEWPORT viewport{};

        viewport.TopLeftX = 0.0f;
        viewport.TopLeftY = 0.0f;
        viewport.Width =
            static_cast<float>(width);

        viewport.Height =
            static_cast<float>(height);

        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;

        m_context->RSSetViewports(
            1,
            &viewport
        );
    }

    void GraphicsDevice::Resize(
        unsigned int width,
        unsigned int height
    )
    {
        if (width == 0 || height == 0)
        {
            return;
        }

        m_d2dContext->SetTarget(
            nullptr
        );

        m_d2dTargetBitmap.Reset();

        // Unbind the old render target from the pipeline.
        m_context->OMSetRenderTargets(
            0,
            nullptr,
            nullptr
        );

        // Release our reference to the old back buffer.
        m_renderTargetView.Reset();

        ThrowIfFailed(
            m_swapChain->ResizeBuffers(
                0,
                width,
                height,
                DXGI_FORMAT_UNKNOWN,
                0
            ),
            "Failed to resize Direct3D swap chain."
        );

        CreateRenderTarget(
            width,
            height
        );
    }

    void GraphicsDevice::BeginFrame(
        float red,
        float green,
        float blue
    ) noexcept
    {
        const float clearColor[4]{
            red,
            green,
            blue,
            1.0f
        };

        ID3D11RenderTargetView*
            renderTarget =
            m_renderTargetView.Get();

        m_context->OMSetRenderTargets(
            1,
            &renderTarget,
            nullptr
        );

        m_context->ClearRenderTargetView(
            m_renderTargetView.Get(),
            clearColor
        );
    }

    void GraphicsDevice::EndFrame()
    {
        ThrowIfFailed(
            m_swapChain->Present(
                1,
                0
            ),
            "Failed to present Direct3D frame."
        );
    }

    ID3D11Device*
        GraphicsDevice::GetDevice() const noexcept
    {
        return m_device.Get();
    }

    ID3D11DeviceContext*
        GraphicsDevice::GetContext() const noexcept
    {
        return m_context.Get();
    }

    ID2D1DeviceContext*
        GraphicsDevice::GetOverlayContext() const noexcept
    {
        return m_d2dContext.Get();
    }
}