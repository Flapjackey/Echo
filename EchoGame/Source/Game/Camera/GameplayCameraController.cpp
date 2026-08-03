#include "Game/Camera/GameplayCameraController.h"

#include "Game/GameSession.h"
#include "Graphics/Camera2D.h"
#include "Input/Mouse.h"
#include "Platform/Windows/Window.h"

#include <cmath>

namespace
{
    constexpr float CameraMouseLeadFactor =
        0.38f;

    constexpr float MaximumCameraLeadDistance =
        0.65f;
}

namespace Echo
{
    void GameplayCameraController::Reset(
        Camera2D& camera,
        const GameSession& gameSession,
        std::size_t playerIndex
    ) const noexcept
    {
        const Player& player =
            gameSession.GetPlayer(
                playerIndex
            );

        camera.SnapTo(
            player.GetPositionX(),
            player.GetPositionY()
        );
    }

    void GameplayCameraController::Update(
        Camera2D& camera,
        const GameSession& gameSession,
        std::size_t playerIndex,
        const Mouse& mouse,
        const Window& window,
        float aspectRatio,
        double deltaTime
    ) const noexcept
    {
        const Player& player =
            gameSession.GetPlayer(
                playerIndex
            );

        float cameraLeadX =
            0.0f;

        float cameraLeadY =
            0.0f;

        if (mouse.IsInsideWindow())
        {
            const float clientWidth =
                static_cast<float>(
                    window.GetClientWidth()
                    );

            const float clientHeight =
                static_cast<float>(
                    window.GetClientHeight()
                    );

            if (clientWidth > 0.0f &&
                clientHeight > 0.0f)
            {
                const float normalizedMouseX =
                    2.0f *
                    static_cast<float>(
                        mouse.GetX()
                        ) /
                    clientWidth -
                    1.0f;

                const float normalizedMouseY =
                    1.0f -
                    2.0f *
                    static_cast<float>(
                        mouse.GetY()
                        ) /
                    clientHeight;

                cameraLeadX =
                    normalizedMouseX *
                    aspectRatio *
                    CameraMouseLeadFactor;

                cameraLeadY =
                    normalizedMouseY *
                    CameraMouseLeadFactor;

                const float leadLengthSquared =
                    cameraLeadX *
                    cameraLeadX +
                    cameraLeadY *
                    cameraLeadY;

                const float maximumLengthSquared =
                    MaximumCameraLeadDistance *
                    MaximumCameraLeadDistance;

                if (leadLengthSquared >
                    maximumLengthSquared)
                {
                    const float inverseLength =
                        1.0f /
                        std::sqrt(
                            leadLengthSquared
                        );

                    const float scale =
                        MaximumCameraLeadDistance *
                        inverseLength;

                    cameraLeadX *=
                        scale;

                    cameraLeadY *=
                        scale;
                }
            }
        }

        const float targetX =
            player.GetPositionX() +
            cameraLeadX;

        const float targetY =
            player.GetPositionY() +
            cameraLeadY;

        camera.Update(
            targetX,
            targetY,
            deltaTime
        );
    }
}