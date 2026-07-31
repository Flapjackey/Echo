#pragma once

#include "Graphics/GraphicsDevice.h"

#include <wrl/client.h>

namespace Echo
{
    class TriangleRenderer final
    {
    public:
        explicit TriangleRenderer(
            GraphicsDevice& graphics
        );

        TriangleRenderer(
            const TriangleRenderer&
        ) = delete;

        TriangleRenderer& operator=(
            const TriangleRenderer&
            ) = delete;

        void Draw() noexcept;

    private:
        ID3D11DeviceContext* m_context =
            nullptr;

        Microsoft::WRL::ComPtr<ID3D11Buffer>
            m_vertexBuffer;

        Microsoft::WRL::ComPtr<ID3D11VertexShader>
            m_vertexShader;

        Microsoft::WRL::ComPtr<ID3D11PixelShader>
            m_pixelShader;

        Microsoft::WRL::ComPtr<ID3D11InputLayout>
            m_inputLayout;
    };
}