#pragma once

#include "Core/Clock.h"
#include "Game/Player.h"
#include "Game/Projectile.h"
#include "Graphics/GraphicsDevice.h"
#include "Graphics/QuadRenderer.h"
#include "Input/Keyboard.h"
#include "Input/Mouse.h"
#include "Platform/Windows/Window.h"

#include <vector>

namespace Echo
{
    class Application final
    {
    public:
        Application();

        int Run();

    private:
        void FixedUpdate(double deltaTime);
        void Update(double deltaTime);
        void UpdateStatistics(double deltaTime);

        void TryFireProjectile();

        Keyboard m_keyboard;
        Mouse m_mouse;
        Window m_window;
        GraphicsDevice m_graphics;
        QuadRenderer m_quadRenderer;
        Player m_player;

        std::vector<Projectile> m_projectiles;

        Clock m_clock;

        double m_statisticsTimer = 0.0;
        unsigned int m_frameCount = 0;

        float m_fireCooldown = 0.0f;

        float m_aspectRatio =
            16.0f / 9.0f;
    };
}