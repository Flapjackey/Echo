#include "Graphics/GraphicsDevice.h"

#include <stdexcept>

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
            DXGI_FORMAT_R8G8B8A8_UNORM;

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

        CreateRenderTarget(width, height);
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
}