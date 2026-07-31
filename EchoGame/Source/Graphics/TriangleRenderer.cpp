#include "Graphics/TriangleRenderer.h"

#include <d3dcompiler.h>

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
    TriangleRenderer::TriangleRenderer(
        GraphicsDevice& graphics
    )
    {
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
                static_cast<unsigned int>(
                    std::size(inputElements)
                    ),
                vertexShaderCode->GetBufferPointer(),
                vertexShaderCode->GetBufferSize(),
                m_inputLayout.GetAddressOf()
            ),
            "Failed to create input layout."
        );

        const Vertex vertices[]
        {
            {
                { 0.0f, 0.65f },
                { 1.0f, 0.1f, 0.1f }
            },
            {
                { 0.65f, -0.65f },
                { 0.1f, 1.0f, 0.1f }
            },
            {
                { -0.65f, -0.65f },
                { 0.1f, 0.3f, 1.0f }
            }
        };

        D3D11_BUFFER_DESC bufferDescription{};

        bufferDescription.ByteWidth =
            sizeof(vertices);

        bufferDescription.Usage =
            D3D11_USAGE_IMMUTABLE;

        bufferDescription.BindFlags =
            D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA initialData{};

        initialData.pSysMem =
            vertices;

        ThrowIfFailed(
            device->CreateBuffer(
                &bufferDescription,
                &initialData,
                m_vertexBuffer.GetAddressOf()
            ),
            "Failed to create triangle vertex buffer."
        );
    }

    void TriangleRenderer::Draw() noexcept
    {
        const unsigned int stride =
            sizeof(Vertex);

        const unsigned int offset = 0;

        ID3D11Buffer* vertexBuffer =
            m_vertexBuffer.Get();

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

        m_context->IASetPrimitiveTopology(
            D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
        );

        m_context->VSSetShader(
            m_vertexShader.Get(),
            nullptr,
            0
        );

        m_context->PSSetShader(
            m_pixelShader.Get(),
            nullptr,
            0
        );

        m_context->Draw(
            3,
            0
        );
    }
}