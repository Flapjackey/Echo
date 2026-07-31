#pragma once

#include "Core/Clock.h"
#include "Graphics/GraphicsDevice.h"
#include "Graphics/QuadRenderer.h"
#include "Platform/Windows/Window.h"
#include "Input/Keyboard.h"

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

        Keyboard m_keyboard;
        Window m_window;
        GraphicsDevice m_graphics;
        QuadRenderer m_quadRenderer;
        Clock m_clock;

        double m_statisticsTimer = 0.0;
        unsigned int m_frameCount = 0;

        float m_quadPositionX = 0.0f;
        float m_quadPositionY = 0.0f;
        float m_quadRotation = 0.0f;

        float m_aspectRatio =
            16.0f / 9.0f;
    };
}