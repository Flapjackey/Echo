#pragma once

#include "Platform/Windows/WindowsCommon.h"

#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>

namespace Echo
{
    class GraphicsDevice final
    {
    public:
        GraphicsDevice(
            HWND windowHandle,
            unsigned int width,
            unsigned int height
        );

        ~GraphicsDevice() = default;

        GraphicsDevice(const GraphicsDevice&) = delete;
        GraphicsDevice& operator=(const GraphicsDevice&) = delete;

        void BeginFrame(
            float red,
            float green,
            float blue
        ) noexcept;

        void EndFrame();

        void Resize(
            unsigned int width,
            unsigned int height
        );

    private:
        void CreateRenderTarget(
            unsigned int width,
            unsigned int height
        );

        Microsoft::WRL::ComPtr<ID3D11Device>
            m_device;

        Microsoft::WRL::ComPtr<ID3D11DeviceContext>
            m_context;

        Microsoft::WRL::ComPtr<IDXGISwapChain>
            m_swapChain;

        Microsoft::WRL::ComPtr<ID3D11RenderTargetView>
            m_renderTargetView;
    };
}