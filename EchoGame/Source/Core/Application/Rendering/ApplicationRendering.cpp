#include "Core/Application.h"

#include <iomanip>
#include <sstream>

namespace Echo
{
    void Application::RenderGameplay()
    {
        const float cameraX =
            m_gameplayCamera.GetPositionX();

        const float cameraY =
            m_gameplayCamera.GetPositionY();

        const Level& level =
            m_gameSession.GetLevel();

        for (const LevelBlock& block :
            level.GetBlocks())
        {
            m_quadRenderer.Draw(
                block.positionX - cameraX,
                block.positionY - cameraY,
                block.width,
                block.height,
                block.rotation,
                m_aspectRatio
            );
        }

        for (const Player& player :
            m_gameSession.GetPlayers())
        {
            m_quadRenderer.Draw(
                player.GetPositionX() - cameraX,
                player.GetPositionY() - cameraY,
                player.GetWidth(),
                player.GetHeight(),
                player.GetRotation(),
                m_aspectRatio
            );
        }

        for (const Projectile& projectile :
            m_gameSession.GetProjectiles())
        {
            m_quadRenderer.Draw(
                projectile.GetPositionX() -
                cameraX,
                projectile.GetPositionY() -
                cameraY,
                projectile.GetWidth(),
                projectile.GetHeight(),
                projectile.GetRotation(),
                m_aspectRatio
            );
        }

        if (m_settings.debugOverlay)
        {
            RenderDebugOverlay();
        }
    }

    void Application::RenderDebugOverlay()
    {
        const float clientWidth =
            static_cast<float>(
                m_window.GetClientWidth()
                );

        if (clientWidth <= 0.0f)
        {
            return;
        }

        const Player& playerOne =
            m_gameSession.GetPlayer(0);

        const Player& playerTwo =
            m_gameSession.GetPlayer(1);

        std::wostringstream debugText;

        debugText
            << std::fixed
            << std::setprecision(1)
            << L"FPS: "
            << m_frameStatistics.
            GetFramesPerSecond()
            << L" | Frame: "
            << m_frameStatistics.
            GetFrameTimeMilliseconds()
            << L" ms"
            << L" | P1: "
            << std::setprecision(2)
            << playerOne.GetPositionX()
            << L", "
            << playerOne.GetPositionY()
            << L" | P2: "
            << playerTwo.GetPositionX()
            << L", "
            << playerTwo.GetPositionY()
            << L" | Projectiles: "
            << m_gameSession
            .GetProjectiles()
            .size()
            << L" | Local: P"
            << (
                m_localNetworkPlayerIndex +
                1
                )
            << L" | Remote: P"
            << (
                m_remoteNetworkPlayerIndex +
                1
                )
            << L" | Backup: "
            << (
                m_hasMigrationState
                ? L"Ready"
                : L"Waiting"
                )
            << L" | Epoch: "
            << m_hostEpoch
            << L" | Net: "
            << m_networkSession.
            GetStatusMessage();

        m_textRenderer.Begin();

        m_textRenderer.Draw(
            debugText.str(),
            D2D1_RECT_F
            {
                0.0f,
                8.0f,
                clientWidth,
                48.0f
            },
            TextStyle::Hint
        );

        m_textRenderer.End();
    }

    void Application::RenderPlaceholder()
    {
        m_quadRenderer.Draw(
            0.0f,
            0.0f,
            0.80f,
            0.30f,
            0.0f,
            m_aspectRatio
        );

        const float clientWidth =
            static_cast<float>(
                m_window.GetClientWidth()
                );

        const float clientHeight =
            static_cast<float>(
                m_window.GetClientHeight()
                );

        const wchar_t* screenTitle =
            L"Placeholder";

        switch (m_applicationState)
        {
        case ApplicationState::HostGame:
        {
            screenTitle =
                L"Host Game";

            break;
        }

        case ApplicationState::JoinGame:
        {
            screenTitle =
                L"Join Game";

            break;
        }

        default:
        {
            break;
        }
        }

        m_textRenderer.Begin();

        m_textRenderer.Draw(
            screenTitle,
            D2D1_RECT_F
            {
                0.0f,
                clientHeight * 0.30f,
                clientWidth,
                clientHeight * 0.46f
            },
            TextStyle::Title
        );

        m_textRenderer.Draw(
            m_networkSession.
            GetStatusMessage(),
            D2D1_RECT_F
            {
                0.0f,
                clientHeight * 0.46f,
                clientWidth,
                clientHeight * 0.58f
            },
            TextStyle::MenuItem
        );

        m_textRenderer.Draw(
            L"Escape - Back",
            D2D1_RECT_F
            {
                0.0f,
                clientHeight - 55.0f,
                clientWidth,
                clientHeight - 10.0f
            },
            TextStyle::Hint
        );

        m_textRenderer.End();
    }
}