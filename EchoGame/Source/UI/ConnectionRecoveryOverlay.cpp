#include "UI/ConnectionRecoveryOverlay.h"

#include "Graphics/TextRenderer.h"
#include "Input/Keyboard.h"
#include "Input/Mouse.h"
#include "Platform/Windows/Window.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

namespace
{
    D2D1_RECT_F GetContinueButtonRectangle(
        float clientWidth,
        float clientHeight
    ) noexcept
    {
        const float centerX =
            clientWidth * 0.5f;

        const float centerY =
            clientHeight * 0.67f;

        const float halfWidth =
            std::min(
                280.0f,
                clientWidth * 0.36f
            );

        const float halfHeight =
            34.0f;

        return D2D1_RECT_F
        {
            centerX - halfWidth,
            centerY - halfHeight,
            centerX + halfWidth,
            centerY + halfHeight
        };
    }
}

namespace Echo
{
    ConnectionRecoveryAction
        ConnectionRecoveryOverlay::Update(
            const Keyboard& keyboard,
            const Mouse& mouse,
            const Window& window,
            bool canContinueSolo
        ) noexcept
    {
        m_buttonHovered =
            canContinueSolo &&
            IsMouseOverButton(
                mouse,
                window
            );

        if (!canContinueSolo)
        {
            return
                ConnectionRecoveryAction::None;
        }

        const bool keyboardActivated =
            keyboard.WasPressed(
                Key::Enter
            );

        const bool mouseActivated =
            mouse.WasLeftButtonPressed() &&
            m_buttonHovered;

        if (keyboardActivated ||
            mouseActivated)
        {
            return
                ConnectionRecoveryAction::
                ContinueSolo;
        }

        return
            ConnectionRecoveryAction::None;
    }

    void ConnectionRecoveryOverlay::Render(
        TextRenderer& textRenderer,
        const Window& window,
        double remainingSeconds,
        bool showTimer,
        bool canContinueSolo,
        std::wstring_view title,
        std::wstring_view message,
        std::wstring_view networkStatus
    ) const
    {
        const float clientWidth =
            static_cast<float>(
                window.GetClientWidth()
                );

        const float clientHeight =
            static_cast<float>(
                window.GetClientHeight()
                );

        if (clientWidth <= 0.0f ||
            clientHeight <= 0.0f)
        {
            return;
        }

        const int displayedSeconds =
            static_cast<int>(
                std::ceil(
                    std::max(
                        remainingSeconds,
                        0.0
                    )
                )
                );

        std::wostringstream timerText;

        timerText
            << L"00:"
            << std::setw(2)
            << std::setfill(L'0')
            << displayedSeconds;

        textRenderer.Begin();

        // Darken the frozen game world.
        textRenderer.FillRectangle(
            D2D1_RECT_F
            {
                0.0f,
                0.0f,
                clientWidth,
                clientHeight
            },
            0.01f,
            0.015f,
            0.03f,
            0.78f
        );

        textRenderer.Draw(
            title,
            D2D1_RECT_F
            {
                0.0f,
                clientHeight * 0.16f,
                clientWidth,
                clientHeight * 0.30f
            },
            TextStyle::Title
        );

        textRenderer.Draw(
            message,
            D2D1_RECT_F
            {
                0.0f,
                clientHeight * 0.32f,
                clientWidth,
                clientHeight * 0.42f
            },
            TextStyle::MenuItem
        );

        if (showTimer)
        {
            textRenderer.Draw(
                timerText.str(),
                D2D1_RECT_F
                {
                    0.0f,
                    clientHeight * 0.42f,
                    clientWidth,
                    clientHeight * 0.54f
                },
                TextStyle::Title
            );
        }

        textRenderer.Draw(
            networkStatus,
            D2D1_RECT_F
            {
                0.0f,
                clientHeight * 0.54f,
                clientWidth,
                clientHeight * 0.62f
            },
            TextStyle::Hint
        );

        if (canContinueSolo)
        {
            const D2D1_RECT_F buttonRectangle =
                GetContinueButtonRectangle(
                    clientWidth,
                    clientHeight
                );

            textRenderer.FillRectangle(
                buttonRectangle,
                m_buttonHovered
                ? 0.08f
                : 0.04f,
                m_buttonHovered
                ? 0.32f
                : 0.12f,
                m_buttonHovered
                ? 0.42f
                : 0.20f,
                0.92f
            );

            textRenderer.Draw(
                L"Continue Solo",
                buttonRectangle,
                TextStyle::MenuItem,
                m_buttonHovered
            );

            textRenderer.Draw(
                L"Enter or Click - Continue Solo",
                D2D1_RECT_F
                {
                    0.0f,
                    clientHeight - 55.0f,
                    clientWidth,
                    clientHeight - 10.0f
                },
                TextStyle::Hint
            );
        }
        else
        {
            textRenderer.Draw(
                L"Waiting for the other player...",
                D2D1_RECT_F
                {
                    0.0f,
                    clientHeight * 0.66f,
                    clientWidth,
                    clientHeight * 0.76f
                },
                TextStyle::Hint
            );
        }

        textRenderer.End();
    }

    void ConnectionRecoveryOverlay::Reset()
        noexcept
    {
        m_buttonHovered =
            false;
    }

    bool ConnectionRecoveryOverlay::
        IsMouseOverButton(
            const Mouse& mouse,
            const Window& window
        ) const noexcept
    {
        if (!mouse.IsInsideWindow())
        {
            return false;
        }

        const float clientWidth =
            static_cast<float>(
                window.GetClientWidth()
                );

        const float clientHeight =
            static_cast<float>(
                window.GetClientHeight()
                );

        if (clientWidth <= 0.0f ||
            clientHeight <= 0.0f)
        {
            return false;
        }

        const D2D1_RECT_F buttonRectangle =
            GetContinueButtonRectangle(
                clientWidth,
                clientHeight
            );

        const float mouseX =
            static_cast<float>(
                mouse.GetX()
                );

        const float mouseY =
            static_cast<float>(
                mouse.GetY()
                );

        return
            mouseX >= buttonRectangle.left &&
            mouseX <= buttonRectangle.right &&
            mouseY >= buttonRectangle.top &&
            mouseY <= buttonRectangle.bottom;
    }
}