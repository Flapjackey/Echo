#pragma once

#include "Platform/Windows/WindowsCommon.h"

#include <d2d1_1.h>
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

        GraphicsDevice(
            const GraphicsDevice&
        ) = delete;

        GraphicsDevice& operator=(
            const GraphicsDevice&
            ) = delete;

        void BeginFrame(
            float red,
            float green,
            float blue
        ) noexcept;

        void EndFrame(
            bool verticalSync
        );

        void Resize(
            unsigned int width,
            unsigned int height
        );

        ID3D11Device*
            GetDevice() const noexcept;

        ID3D11DeviceContext*
            GetContext() const noexcept;

        ID2D1DeviceContext*
            GetOverlayContext() const noexcept;

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

        Microsoft::WRL::ComPtr<
            ID3D11RenderTargetView>
            m_renderTargetView;

        Microsoft::WRL::ComPtr<ID2D1Factory1>
            m_d2dFactory;

        Microsoft::WRL::ComPtr<ID2D1Device>
            m_d2dDevice;

        Microsoft::WRL::ComPtr<
            ID2D1DeviceContext>
            m_d2dContext;

        Microsoft::WRL::ComPtr<ID2D1Bitmap1>
            m_d2dTargetBitmap;
    };
}