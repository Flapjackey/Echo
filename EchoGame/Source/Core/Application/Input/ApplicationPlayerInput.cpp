#include "Core/Application.h"

#include <cmath>

namespace Echo
{
    GameSession::PlayerCommands
        Application::BuildLocalPlayerCommands()
        const noexcept
    {
        GameSession::PlayerCommands
            commands{};

        commands[0] =
            BuildPlayerCommand(
                0,
                BuildLocalNetworkInput()
            );

        return commands;
    }

    GameSession::PlayerCommands
        Application::BuildHostPlayerCommands()
        const noexcept
    {
        GameSession::PlayerCommands
            commands{};

        commands[
            m_localNetworkPlayerIndex
        ] =
            BuildPlayerCommand(
                m_localNetworkPlayerIndex,
                BuildLocalNetworkInput()
            );

            commands[
                m_remoteNetworkPlayerIndex
            ] =
                BuildPlayerCommand(
                    m_remoteNetworkPlayerIndex,
                    m_latestRemotePlayerInput
                );

                return commands;
    }

    NetworkPlayerInput
        Application::BuildLocalNetworkInput()
        const noexcept
    {
        NetworkPlayerInput input{};

        if (m_keyboard.IsDown(Key::A))
        {
            input.movementX -=
                1.0f;
        }

        if (m_keyboard.IsDown(Key::D))
        {
            input.movementX +=
                1.0f;
        }

        if (m_keyboard.IsDown(Key::W))
        {
            input.movementY +=
                1.0f;
        }

        if (m_keyboard.IsDown(Key::S))
        {
            input.movementY -=
                1.0f;
        }

        if (!m_mouse.IsInsideWindow())
        {
            return input;
        }

        input.fire =
            m_mouse.IsLeftButtonDown();

        const float clientWidth =
            static_cast<float>(
                m_window.GetClientWidth()
                );

        const float clientHeight =
            static_cast<float>(
                m_window.GetClientHeight()
                );

        if (clientWidth <= 0.0f ||
            clientHeight <= 0.0f)
        {
            return input;
        }

        const float normalizedMouseX =
            2.0f *
            static_cast<float>(
                m_mouse.GetX()
                ) /
            clientWidth -
            1.0f;

        const float normalizedMouseY =
            1.0f -
            2.0f *
            static_cast<float>(
                m_mouse.GetY()
                ) /
            clientHeight;

        input.aimTargetX =
            m_gameplayCamera.GetPositionX() +
            normalizedMouseX *
            m_aspectRatio;

        input.aimTargetY =
            m_gameplayCamera.GetPositionY() +
            normalizedMouseY;

        input.hasAimTarget =
            true;

        return input;
    }

    PlayerCommand Application::BuildPlayerCommand(
        std::size_t playerIndex,
        const NetworkPlayerInput& input
    ) const noexcept
    {
        PlayerCommand command{};

        command.movementX =
            input.movementX;

        command.movementY =
            input.movementY;

        command.fire =
            input.fire;

        const Player& player =
            m_gameSession.GetPlayer(
                playerIndex
            );

        // Preserve the current aim direction when
        // no valid mouse target is available.
        command.aimX =
            player.GetForwardX();

        command.aimY =
            player.GetForwardY();

        if (!input.hasAimTarget)
        {
            return command;
        }

        const float directionX =
            input.aimTargetX -
            player.GetPositionX();

        const float directionY =
            input.aimTargetY -
            player.GetPositionY();

        const float directionLengthSquared =
            directionX *
            directionX +
            directionY *
            directionY;

        constexpr float DirectionEpsilon =
            0.000001f;

        if (directionLengthSquared <=
            DirectionEpsilon)
        {
            return command;
        }

        const float inverseLength =
            1.0f /
            std::sqrt(
                directionLengthSquared
            );

        command.aimX =
            directionX *
            inverseLength;

        command.aimY =
            directionY *
            inverseLength;

        return command;
    }
}