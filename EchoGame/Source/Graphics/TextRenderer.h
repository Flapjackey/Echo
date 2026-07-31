#pragma once

#include "Graphics/GraphicsDevice.h"

#include <dwrite.h>
#include <string_view>
#include <wrl/client.h>

namespace Echo
{
    enum class TextStyle
    {
        Title,
        MenuItem,
        Hint
    };

    class TextRenderer final
    {
    public:
        explicit TextRenderer(
            GraphicsDevice& graphics
        );

        TextRenderer(
            const TextRenderer&
        ) = delete;

        TextRenderer& operator=(
            const TextRenderer&
            ) = delete;

        void Begin();

        void Draw(
            std::wstring_view text,
            const D2D1_RECT_F& rectangle,
            TextStyle style,
            bool highlighted = false
        );

        void End();

    private:
        IDWriteTextFormat* GetFormat(
            TextStyle style
        ) const noexcept;

        ID2D1DeviceContext* m_context =
            nullptr;

        Microsoft::WRL::ComPtr<IDWriteFactory>
            m_writeFactory;

        Microsoft::WRL::ComPtr<IDWriteTextFormat>
            m_titleFormat;

        Microsoft::WRL::ComPtr<IDWriteTextFormat>
            m_menuItemFormat;

        Microsoft::WRL::ComPtr<IDWriteTextFormat>
            m_hintFormat;

        Microsoft::WRL::ComPtr<
            ID2D1SolidColorBrush>
            m_brush;
    };
}