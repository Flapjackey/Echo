#pragma once

#include "Core/ApplicationState.h"
#include "Core/Clock.h"
#include "Core/GameSettings.h"
#include "Game/GameSession.h"
#include "Game/PlayerCommand.h"
#include "Graphics/GraphicsDevice.h"
#include "Graphics/QuadRenderer.h"
#include "Graphics/TextRenderer.h"
#include "Input/Keyboard.h"
#include "Input/Mouse.h"
#include "Network/NetworkSession.h"
#include "Network/NetworkGamePhase.h"
#include "Network/NetworkSystem.h"
#include "Platform/Windows/Window.h"
#include "UI/MainMenu.h"
#include "UI/PauseMenu.h"
#include "UI/ConnectionRecoveryOverlay.h"
#include "UI/SettingsMenu.h"

#include <cstddef>
#include <cstdint>

namespace Echo
{
    class Application final
    {
    public:
        Application();

        int Run();

    private:
        void FixedUpdate(
            const GameSession::PlayerCommands&
            playerCommands,
            double deltaTime
        );

        GameSession::PlayerCommands
            BuildLocalPlayerCommands()
            const noexcept;

        void SetNetworkPlayerOwnership(
            std::size_t localPlayerIndex,
            std::size_t remotePlayerIndex
        ) noexcept;

        GameSession::PlayerCommands
            BuildHostPlayerCommands()
            const noexcept;

        NetworkPlayerInput
            BuildLocalNetworkInput()
            const noexcept;

        PlayerCommand BuildPlayerCommand(
            std::size_t playerIndex,
            const NetworkPlayerInput& input
        ) const noexcept;

        NetworkWorldSnapshot
            BuildWorldSnapshot()
            const noexcept;

        GameMigrationState BuildMigrationState(
            const NetworkWorldSnapshot& snapshot
        ) const;

        void ApplyWorldSnapshot(
            const NetworkWorldSnapshot& snapshot
        );

        void UpdateRemoteWorldPresentation(
            double deltaTime
        ) noexcept;

        void ResetNetworkGameState()
            noexcept;

        void HandleNetworkConnected();

        void HandleNetworkConnectionLost();

        void BeginHostSynchronization();

        void BeginClientSynchronization();

        void UpdateNetworkSynchronization();

        void BeginConnectionRecovery();

        void UpdateConnectionRecovery(
            double deltaTime
        );

        void PromoteClientToHost();

        void RestartHostListener();

        void HandleApplicationInput();

        void EnterState(
            ApplicationState state
        );

        void RenderGameplay();
        void RenderPlaceholder();
        void RenderDebugOverlay();

        void UpdateMenuTitle();

        void UpdateStatistics(
            double deltaTime
        );

        Keyboard m_keyboard;
        Mouse m_mouse;

        NetworkSystem m_networkSystem;
        NetworkSession m_networkSession;

        NetworkGamePhase m_networkGamePhase =
            NetworkGamePhase::Offline;

        double m_connectionRecoveryRemaining =
            0.0;

        double m_reconnectAttemptAccumulator =
            0.0;

        bool m_checkpointQueued =
            false;

        bool m_checkpointAppliedQueued =
            false;

        bool m_resumeQueued =
            false;

        std::size_t m_localNetworkPlayerIndex =
            0;

        std::size_t m_remoteNetworkPlayerIndex =
            1;

        NetworkPlayerInput
            m_latestRemotePlayerInput{};

        double m_playerInputSendAccumulator =
            0.0;

        double m_remoteInputAge =
            0.0;

        std::uint32_t m_nextLocalInputSequence =
            1;

        std::uint32_t m_clientTick =
            0;

        std::uint32_t m_serverTick =
            0;

        std::uint32_t
            m_lastProcessedRemoteInputSequence =
            0;

        std::uint32_t
            m_lastAcknowledgedInputSequence =
            0;

        std::uint32_t
            m_latestReceivedServerTick =
            0;

        GameMigrationState
            m_latestMigrationState{};

        bool m_hasMigrationState =
            false;

        NetworkWorldSnapshot
            m_remoteInterpolationStart{};

        NetworkWorldSnapshot
            m_remoteInterpolationTarget{};

        double m_remoteInterpolationElapsed =
            0.0;

        bool m_hasReceivedWorldSnapshot =
            false;

        Window m_window;

        GraphicsDevice m_graphics;
        QuadRenderer m_quadRenderer;
        TextRenderer m_textRenderer;

        GameSession m_gameSession;

        MainMenu m_mainMenu;
        PauseMenu m_pauseMenu;

        ConnectionRecoveryOverlay
            m_connectionRecoveryOverlay;

        SettingsMenu m_settingsMenu;

        Clock m_clock;

        GameSettings m_settings;

        ApplicationState m_applicationState =
            ApplicationState::MainMenu;

        ApplicationState m_settingsReturnState =
            ApplicationState::MainMenu;

        bool m_exitRequested = false;

        double m_statisticsTimer = 0.0;
        unsigned int m_frameCount = 0;

        double m_framesPerSecond = 0.0;
        double m_frameTimeMilliseconds = 0.0;

        float m_aspectRatio =
            16.0f / 9.0f;
    };
}