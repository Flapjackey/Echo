#pragma once

#include <cstddef>

namespace Echo
{
    class Camera2D;
    class GameSession;
    class Mouse;
    class Window;

    class GameplayCameraController final
    {
    public:
        void Reset(
            Camera2D& camera,
            const GameSession& gameSession,
            std::size_t playerIndex
        ) const noexcept;

        void Update(
            Camera2D& camera,
            const GameSession& gameSession,
            std::size_t playerIndex,
            const Mouse& mouse,
            const Window& window,
            float aspectRatio,
            double deltaTime
        ) const noexcept;
    };
}