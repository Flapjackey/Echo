#pragma once

#include "Graphics/GraphicsDevice.h"

#include <wrl/client.h>

namespace Echo
{
    class QuadRenderer final
    {
    public:
        explicit QuadRenderer(
            GraphicsDevice& graphics
        );

        QuadRenderer(
            const QuadRenderer&
        ) = delete;

        QuadRenderer& operator=(
            const QuadRenderer&
            ) = delete;

        void Draw() noexcept;

    private:
        ID3D11DeviceContext* m_context =
            nullptr;

        Microsoft::WRL::ComPtr<ID3D11Buffer>
            m_vertexBuffer;

        Microsoft::WRL::ComPtr<ID3D11Buffer>
            m_indexBuffer;

        Microsoft::WRL::ComPtr<ID3D11VertexShader>
            m_vertexShader;

        Microsoft::WRL::ComPtr<ID3D11PixelShader>
            m_pixelShader;

        Microsoft::WRL::ComPtr<ID3D11InputLayout>
            m_inputLayout;
    };
}