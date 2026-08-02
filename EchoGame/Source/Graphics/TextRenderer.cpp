#include "Graphics/TextRenderer.h"

#include <d2d1helper.h>

#include <stdexcept>

#pragma comment(lib, "dwrite.lib")

namespace
{
    void ThrowIfFailed(
        HRESULT result,
        const char* message
    )
    {
        if (FAILED(result))
        {
            throw std::runtime_error(
                message
            );
        }
    }

    void ConfigureTextFormat(
        IDWriteTextFormat* format
    )
    {
        ThrowIfFailed(
            format->SetTextAlignment(
                DWRITE_TEXT_ALIGNMENT_CENTER
            ),
            "Failed to set text alignment."
        );

        ThrowIfFailed(
            format->SetParagraphAlignment(
                DWRITE_PARAGRAPH_ALIGNMENT_CENTER
            ),
            "Failed to set paragraph alignment."
        );
    }
}

namespace Echo
{
    TextRenderer::TextRenderer(
        GraphicsDevice& graphics
    )
        : m_context(
            graphics.GetOverlayContext()
        )
    {
        if (m_context == nullptr)
        {
            throw std::runtime_error(
                "Invalid Direct2D context."
            );
        }

        ThrowIfFailed(
            DWriteCreateFactory(
                DWRITE_FACTORY_TYPE_SHARED,
                __uuidof(IDWriteFactory),
                reinterpret_cast<IUnknown**>(
                    m_writeFactory.GetAddressOf()
                    )
            ),
            "Failed to create DirectWrite factory."
        );

        ThrowIfFailed(
            m_writeFactory->CreateTextFormat(
                L"Segoe UI",
                nullptr,
                DWRITE_FONT_WEIGHT_BOLD,
                DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                52.0f,
                L"en-us",
                m_titleFormat.GetAddressOf()
            ),
            "Failed to create title text format."
        );

        ThrowIfFailed(
            m_writeFactory->CreateTextFormat(
                L"Segoe UI",
                nullptr,
                DWRITE_FONT_WEIGHT_SEMI_BOLD,
                DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                30.0f,
                L"en-us",
                m_menuItemFormat.GetAddressOf()
            ),
            "Failed to create menu text format."
        );

        ThrowIfFailed(
            m_writeFactory->CreateTextFormat(
                L"Segoe UI",
                nullptr,
                DWRITE_FONT_WEIGHT_NORMAL,
                DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                18.0f,
                L"en-us",
                m_hintFormat.GetAddressOf()
            ),
            "Failed to create hint text format."
        );

        ConfigureTextFormat(
            m_titleFormat.Get()
        );

        ConfigureTextFormat(
            m_menuItemFormat.Get()
        );

        ConfigureTextFormat(
            m_hintFormat.Get()
        );

        ThrowIfFailed(
            m_context->CreateSolidColorBrush(
                D2D1::ColorF(
                    1.0f,
                    1.0f,
                    1.0f,
                    1.0f
                ),
                m_brush.GetAddressOf()
            ),
            "Failed to create text brush."
        );
    }

    void TextRenderer::Begin()
    {
        m_context->BeginDraw();

        m_context->SetTransform(
            D2D1::Matrix3x2F::Identity()
        );
    }

    void TextRenderer::Draw(
        std::wstring_view text,
        const D2D1_RECT_F& rectangle,
        TextStyle style,
        bool highlighted
    )
    {
        if (text.empty())
        {
            return;
        }

        D2D1_COLOR_F color{};

        if (highlighted)
        {
            color = D2D1::ColorF(
                0.10f,
                0.85f,
                1.00f,
                1.00f
            );
        }
        else if (style == TextStyle::Hint)
        {
            color = D2D1::ColorF(
                0.65f,
                0.68f,
                0.75f,
                1.00f
            );
        }
        else
        {
            color = D2D1::ColorF(
                0.95f,
                0.96f,
                1.00f,
                1.00f
            );
        }

        m_brush->SetColor(
            color
        );

        m_context->DrawTextW(
            text.data(),
            static_cast<UINT32>(
                text.size()
                ),
            GetFormat(style),
            rectangle,
            m_brush.Get(),
            D2D1_DRAW_TEXT_OPTIONS_CLIP
        );
    }

    void TextRenderer::FillRectangle(
        const D2D1_RECT_F& rectangle,
        float red,
        float green,
        float blue,
        float alpha
    )
    {
        m_brush->SetColor(
            D2D1::ColorF(
                red,
                green,
                blue,
                alpha
            )
        );

        m_context->FillRectangle(
            rectangle,
            m_brush.Get()
        );
    }

    void TextRenderer::End()
    {
        ThrowIfFailed(
            m_context->EndDraw(),
            "Failed to finish Direct2D drawing."
        );
    }

    IDWriteTextFormat*
        TextRenderer::GetFormat(
            TextStyle style
        ) const noexcept
    {
        switch (style)
        {
        case TextStyle::Title:
            return m_titleFormat.Get();

        case TextStyle::MenuItem:
            return m_menuItemFormat.Get();

        case TextStyle::Hint:
            return m_hintFormat.Get();
        }

        return m_menuItemFormat.Get();
    }
}