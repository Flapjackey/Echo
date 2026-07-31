#include "Graphics/QuadRenderer.h"

#include <d3dcompiler.h>

#include <cstdint>
#include <stdexcept>
#include <string>

#pragma comment(lib, "d3dcompiler.lib")

namespace
{
    using Microsoft::WRL::ComPtr;

    struct Vertex
    {
        float position[2];
        float color[3];
    };

    struct alignas(16) TransformBuffer
    {
        float position[2];
        float size[2];

        float rotation;
        float aspectRatio;
        float padding[2];
    };

    static_assert(
        sizeof(TransformBuffer) % 16 == 0,
        "Constant buffer size must be a multiple of 16 bytes."
        );

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

    ComPtr<ID3DBlob> CompileShader(
        const wchar_t* filePath,
        const char* entryPoint,
        const char* target
    )
    {
        unsigned int compileFlags =
            D3DCOMPILE_ENABLE_STRICTNESS;

#ifdef _DEBUG
        compileFlags |=
            D3DCOMPILE_DEBUG |
            D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

        ComPtr<ID3DBlob> shaderCode;
        ComPtr<ID3DBlob> errorMessages;

        const HRESULT result =
            D3DCompileFromFile(
                filePath,
                nullptr,
                D3D_COMPILE_STANDARD_FILE_INCLUDE,
                entryPoint,
                target,
                compileFlags,
                0,
                shaderCode.GetAddressOf(),
                errorMessages.GetAddressOf()
            );

        if (FAILED(result))
        {
            std::string message =
                "Failed to compile shader.";

            if (errorMessages != nullptr)
            {
                message += "\n";

                message.append(
                    static_cast<const char*>(
                        errorMessages->GetBufferPointer()
                        ),
                    errorMessages->GetBufferSize()
                );
            }

            throw std::runtime_error(message);
        }

        return shaderCode;
    }
}

namespace Echo
{
    QuadRenderer::QuadRenderer(
        GraphicsDevice& graphics
    )
    {
        D3D11_BUFFER_DESC
            constantBufferDescription{};

        constantBufferDescription.ByteWidth =
            sizeof(TransformBuffer);

        constantBufferDescription.Usage =
            D3D11_USAGE_DYNAMIC;

        constantBufferDescription.BindFlags =
            D3D11_BIND_CONSTANT_BUFFER;

        constantBufferDescription.CPUAccessFlags =
            D3D11_CPU_ACCESS_WRITE;

        ThrowIfFailed(
            device->CreateBuffer(
                &constantBufferDescription,
                nullptr,
                m_constantBuffer.GetAddressOf()
            ),
            "Failed to create transform constant buffer."
        );

        ID3D11Device* device =
            graphics.GetDevice();

        m_context =
            graphics.GetContext();

        if (device == nullptr ||
            m_context == nullptr)
        {
            throw std::runtime_error(
                "Invalid Direct3D device or context."
            );
        }

        const ComPtr<ID3DBlob> vertexShaderCode =
            CompileShader(
                L"Assets/Shaders/BasicColor.hlsl",
                "VSMain",
                "vs_5_0"
            );

        const ComPtr<ID3DBlob> pixelShaderCode =
            CompileShader(
                L"Assets/Shaders/BasicColor.hlsl",
                "PSMain",
                "ps_5_0"
            );

        ThrowIfFailed(
            device->CreateVertexShader(
                vertexShaderCode->GetBufferPointer(),
                vertexShaderCode->GetBufferSize(),
                nullptr,
                m_vertexShader.GetAddressOf()
            ),
            "Failed to create vertex shader."
        );

        ThrowIfFailed(
            device->CreatePixelShader(
                pixelShaderCode->GetBufferPointer(),
                pixelShaderCode->GetBufferSize(),
                nullptr,
                m_pixelShader.GetAddressOf()
            ),
            "Failed to create pixel shader."
        );

        const D3D11_INPUT_ELEMENT_DESC
            inputElements[]
        {
            {
                "POSITION",
                0,
                DXGI_FORMAT_R32G32_FLOAT,
                0,
                0,
                D3D11_INPUT_PER_VERTEX_DATA,
                0
            },
            {
                "COLOR",
                0,
                DXGI_FORMAT_R32G32B32_FLOAT,
                0,
                sizeof(float) * 2,
                D3D11_INPUT_PER_VERTEX_DATA,
                0
            }
        };

        ThrowIfFailed(
            device->CreateInputLayout(
                inputElements,
                2,
                vertexShaderCode->GetBufferPointer(),
                vertexShaderCode->GetBufferSize(),
                m_inputLayout.GetAddressOf()
            ),
            "Failed to create input layout."
        );

        const Vertex vertices[]
        {
            // Top-left.
            {
                { -0.5f, 0.5f },
                { 1.0f, 0.1f, 0.1f }
            },

            // Top-right.
            {
                { 0.5f, 0.5f },
                { 0.1f, 1.0f, 0.1f }
            },

            // Bottom-right.
            {
                { 0.5f, -0.5f },
                { 0.1f, 0.3f, 1.0f }
            },

            // Bottom-left.
            {
                { -0.5f, -0.5f },
                { 1.0f, 0.1f, 1.0f }
            }
        };

        D3D11_BUFFER_DESC vertexBufferDescription{};

        vertexBufferDescription.ByteWidth =
            static_cast<unsigned int>(
                sizeof(vertices)
                );

        vertexBufferDescription.Usage =
            D3D11_USAGE_IMMUTABLE;

        vertexBufferDescription.BindFlags =
            D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA vertexInitialData{};

        vertexInitialData.pSysMem =
            vertices;

        ThrowIfFailed(
            device->CreateBuffer(
                &vertexBufferDescription,
                &vertexInitialData,
                m_vertexBuffer.GetAddressOf()
            ),
            "Failed to create quad vertex buffer."
        );

        const std::uint16_t indices[]
        {
            0, 1, 2,
            0, 2, 3
        };

        D3D11_BUFFER_DESC indexBufferDescription{};

        indexBufferDescription.ByteWidth =
            static_cast<unsigned int>(
                sizeof(indices)
                );

        indexBufferDescription.Usage =
            D3D11_USAGE_IMMUTABLE;

        indexBufferDescription.BindFlags =
            D3D11_BIND_INDEX_BUFFER;

        D3D11_SUBRESOURCE_DATA indexInitialData{};

        indexInitialData.pSysMem =
            indices;

        ThrowIfFailed(
            device->CreateBuffer(
                &indexBufferDescription,
                &indexInitialData,
                m_indexBuffer.GetAddressOf()
            ),
            "Failed to create quad index buffer."
        );
    }

    void QuadRenderer::Draw(
        float positionX,
        float positionY,
        float width,
        float height,
        float rotation,
        float aspectRatio
    )
    {
        if (aspectRatio <= 0.0f)
        {
            return;
        }

        D3D11_MAPPED_SUBRESOURCE
            mappedResource{};

        ThrowIfFailed(
            m_context->Map(
                m_constantBuffer.Get(),
                0,
                D3D11_MAP_WRITE_DISCARD,
                0,
                &mappedResource
            ),
            "Failed to update transform constant buffer."
        );

        auto* transform =
            static_cast<TransformBuffer*>(
                mappedResource.pData
                );

        transform->position[0] = positionX;
        transform->position[1] = positionY;

        transform->size[0] = width;
        transform->size[1] = height;

        transform->rotation = rotation;
        transform->aspectRatio = aspectRatio;

        transform->padding[0] = 0.0f;
        transform->padding[1] = 0.0f;

        m_context->Unmap(
            m_constantBuffer.Get(),
            0
        );

        const unsigned int stride =
            sizeof(Vertex);

        const unsigned int offset = 0;

        ID3D11Buffer* vertexBuffer =
            m_vertexBuffer.Get();

        ID3D11Buffer* constantBuffer =
            m_constantBuffer.Get();

        m_context->IASetInputLayout(
            m_inputLayout.Get()
        );

        m_context->IASetVertexBuffers(
            0,
            1,
            &vertexBuffer,
            &stride,
            &offset
        );

        m_context->IASetIndexBuffer(
            m_indexBuffer.Get(),
            DXGI_FORMAT_R16_UINT,
            0
        );

        m_context->IASetPrimitiveTopology(
            D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
        );

        m_context->VSSetShader(
            m_vertexShader.Get(),
            nullptr,
            0
        );

        m_context->VSSetConstantBuffers(
            0,
            1,
            &constantBuffer
        );

        m_context->PSSetShader(
            m_pixelShader.Get(),
            nullptr,
            0
        );

        m_context->DrawIndexed(
            6,
            0,
            0
        );
    }
}