#include "Core/Application.h"

#include "Game/Player.h"
#include "Game/Projectile.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <utility>
#include <vector>

namespace
{
    constexpr double
        RemotePlayerInterpolationDuration =
        1.0 / 30.0;

    constexpr float Pi =
        3.14159265358979323846f;

    constexpr float TwoPi =
        Pi * 2.0f;

    float InterpolateFloat(
        float start,
        float target,
        float amount
    ) noexcept
    {
        return
            start +
            (target - start) *
            amount;
    }

    float InterpolateAngle(
        float start,
        float target,
        float amount
    ) noexcept
    {
        float difference =
            target -
            start;

        while (difference > Pi)
        {
            difference -=
                TwoPi;
        }

        while (difference < -Pi)
        {
            difference +=
                TwoPi;
        }

        return
            start +
            difference *
            amount;
    }
}

namespace Echo
{
    NetworkWorldSnapshot
        Application::BuildWorldSnapshot()
        const noexcept
    {
        static_assert(
            GameSession::PlayerCount ==
            NetworkPlayerCount,
            "Game and network player counts differ."
            );

        NetworkWorldSnapshot snapshot{};

        snapshot.serverTick =
            m_serverTick;

        snapshot.lastProcessedInputSequence =
            m_lastProcessedRemoteInputSequence;

        snapshot.assignedPlayerIndex =
            static_cast<std::uint32_t>(
                m_remoteNetworkPlayerIndex
                );

        snapshot.nextProjectileEntityId =
            m_gameSession.
            GetNextProjectileEntityId();

        snapshot.fireCooldowns =
            m_gameSession.GetFireCooldowns();

        for (std::size_t playerIndex = 0;
            playerIndex < NetworkPlayerCount;
            ++playerIndex)
        {
            const Player& player =
                m_gameSession.GetPlayer(
                    playerIndex
                );

            NetworkPlayerState&
                playerState =
                snapshot.players[playerIndex];

            playerState.positionX =
                player.GetPositionX();

            playerState.positionY =
                player.GetPositionY();

            playerState.rotation =
                player.GetRotation();
        }

        const std::vector<Projectile>& projectiles =
            m_gameSession.GetProjectiles();

        const std::size_t projectileCount =
            std::min(
                projectiles.size(),
                NetworkMaxProjectileCount
            );

        snapshot.projectileCount =
            static_cast<std::uint32_t>(
                projectileCount
                );

        for (std::size_t projectileIndex = 0;
            projectileIndex < projectileCount;
            ++projectileIndex)
        {
            const Projectile& projectile =
                projectiles[projectileIndex];

            NetworkProjectileState&
                projectileState =
                snapshot.projectiles[
                    projectileIndex
                ];

            projectileState.entityId =
                projectile.GetEntityId();

            projectileState.positionX =
                projectile.GetPositionX();

            projectileState.positionY =
                projectile.GetPositionY();

            projectileState.rotation =
                projectile.GetRotation();

            projectileState.remainingLifetime =
                projectile.GetRemainingLifetime();
        }

        return snapshot;
    }

    GameMigrationState
        Application::BuildMigrationState(
            const NetworkWorldSnapshot& snapshot
        ) const
    {
        static_assert(
            GameSession::PlayerCount ==
            GameMigrationPlayerCount,
            "Game and migration player counts differ."
            );

        static_assert(
            NetworkPlayerCount ==
            GameMigrationPlayerCount,
            "Network and migration player counts differ."
            );

        GameMigrationState migrationState{};

        for (std::size_t playerIndex = 0;
            playerIndex <
            GameMigrationPlayerCount;
            ++playerIndex)
        {
            const NetworkPlayerState&
                networkPlayerState =
                snapshot.players[
                    playerIndex
                ];

            GameMigrationPlayerState&
                migrationPlayerState =
                migrationState.players[
                    playerIndex
                ];

            migrationPlayerState.positionX =
                networkPlayerState.positionX;

            migrationPlayerState.positionY =
                networkPlayerState.positionY;

            migrationPlayerState.rotation =
                networkPlayerState.rotation;
        }

        migrationState.nextProjectileEntityId =
            snapshot.nextProjectileEntityId;

        migrationState.fireCooldowns =
            snapshot.fireCooldowns;

        const std::size_t projectileCount =
            std::min(
                static_cast<std::size_t>(
                    snapshot.projectileCount
                    ),
                NetworkMaxProjectileCount
            );

        migrationState.projectiles.reserve(
            projectileCount
        );

        for (std::size_t projectileIndex = 0;
            projectileIndex < projectileCount;
            ++projectileIndex)
        {
            const NetworkProjectileState&
                networkProjectileState =
                snapshot.projectiles[
                    projectileIndex
                ];

            GameMigrationProjectileState
                migrationProjectileState{};

            migrationProjectileState.entityId =
                networkProjectileState.entityId;

            migrationProjectileState.positionX =
                networkProjectileState.positionX;

            migrationProjectileState.positionY =
                networkProjectileState.positionY;

            migrationProjectileState.rotation =
                networkProjectileState.rotation;

            migrationProjectileState.
                remainingLifetime =
                networkProjectileState.
                remainingLifetime;

            migrationState.projectiles.push_back(
                migrationProjectileState
            );
        }

        return migrationState;
    }

            void Application::ApplyWorldSnapshot(
                const NetworkWorldSnapshot& snapshot
            )
            {
                static_assert(
                    GameSession::PlayerCount ==
                    NetworkPlayerCount,
                    "Game and network player counts differ."
                    );

                m_latestReceivedServerTick =
                    snapshot.serverTick;

                m_lastAcknowledgedInputSequence =
                    snapshot.lastProcessedInputSequence;

                const std::size_t assignedPlayerIndex =
                    static_cast<std::size_t>(
                        snapshot.assignedPlayerIndex
                        );

                const std::size_t remotePlayerIndex =
                    (
                        assignedPlayerIndex +
                        1
                        ) %
                    NetworkPlayerCount;

                SetNetworkPlayerOwnership(
                    assignedPlayerIndex,
                    remotePlayerIndex
                );

                m_latestMigrationState =
                    BuildMigrationState(
                        snapshot
                    );

                m_hasMigrationState =
                    true;

                if (!m_hasReceivedWorldSnapshot)
                {
                    m_remoteInterpolationStart =
                        snapshot;

                    m_remoteInterpolationTarget =
                        snapshot;

                    m_remoteInterpolationElapsed =
                        RemotePlayerInterpolationDuration;

                    for (std::size_t playerIndex = 0;
                        playerIndex < NetworkPlayerCount;
                        ++playerIndex)
                    {
                        const NetworkPlayerState&
                            playerState =
                            snapshot.players[
                                playerIndex
                            ];

                        m_gameSession.SetPlayerNetworkState(
                            playerIndex,
                            playerState.positionX,
                            playerState.positionY,
                            playerState.rotation
                        );
                    }

                    m_gameplayCameraController.Reset(
                        m_gameplayCamera,
                        m_gameSession,
                        GetCameraPlayerIndex()
                    );

                    m_hasReceivedWorldSnapshot =
                        true;
                }
                else
                {
                    // Begin interpolation from the positions
                    // currently displayed on the client.
                    for (std::size_t playerIndex = 0;
                        playerIndex < NetworkPlayerCount;
                        ++playerIndex)
                    {
                        const Player& player =
                            m_gameSession.GetPlayer(
                                playerIndex
                            );

                        NetworkPlayerState&
                            startState =
                            m_remoteInterpolationStart
                            .players[playerIndex];

                        startState.positionX =
                            player.GetPositionX();

                        startState.positionY =
                            player.GetPositionY();

                        startState.rotation =
                            player.GetRotation();
                    }

                    m_remoteInterpolationTarget =
                        snapshot;

                    m_remoteInterpolationElapsed =
                        0.0;
                }

                const std::size_t projectileCount =
                    std::min(
                        static_cast<std::size_t>(
                            snapshot.projectileCount
                            ),
                        NetworkMaxProjectileCount
                    );

                const std::vector<Projectile>&
                    currentProjectiles =
                    m_gameSession.GetProjectiles();

                std::vector<Projectile>
                    synchronizedProjectiles;

                synchronizedProjectiles.reserve(
                    projectileCount
                );

                for (std::size_t projectileIndex = 0;
                    projectileIndex < projectileCount;
                    ++projectileIndex)
                {
                    const NetworkProjectileState&
                        projectileState =
                        snapshot.projectiles[
                            projectileIndex
                        ];

                    const auto existingProjectile =
                        std::find_if(
                            currentProjectiles.begin(),
                            currentProjectiles.end(),
                            [
                                entityId =
                                    projectileState.entityId
                            ](
                                const Projectile& projectile
                                )
                            {
                                return
                                    projectile.GetEntityId() ==
                                    entityId;
                            }
                                    );

                    if (existingProjectile !=
                        currentProjectiles.end())
                    {
                        synchronizedProjectiles.push_back(
                            *existingProjectile
                        );

                        synchronizedProjectiles.back().
                            SetNetworkState(
                                projectileState.positionX,
                                projectileState.positionY,
                                projectileState.rotation
                            );

                        synchronizedProjectiles.back().
                            SetRemainingLifetime(
                                projectileState.remainingLifetime
                            );

                        continue;
                    }

                    const float directionX =
                        std::cos(
                            projectileState.rotation
                        );

                    const float directionY =
                        std::sin(
                            projectileState.rotation
                        );

                    synchronizedProjectiles.emplace_back(
                        projectileState.entityId,
                        projectileState.positionX,
                        projectileState.positionY,
                        directionX,
                        directionY
                    );

                    synchronizedProjectiles.back().
                        SetRemainingLifetime(
                            projectileState.remainingLifetime
                        );
                }

                m_gameSession.SetProjectiles(
                    std::move(
                        synchronizedProjectiles
                    )
                );
            }

            void Application::UpdateRemoteWorldPresentation(
                double deltaTime
            ) noexcept
            {
                if (!m_hasReceivedWorldSnapshot)
                {
                    return;
                }

                if (deltaTime > 0.0)
                {
                    m_remoteInterpolationElapsed +=
                        deltaTime;
                }

                const double rawAmount =
                    m_remoteInterpolationElapsed /
                    RemotePlayerInterpolationDuration;

                const float interpolationAmount =
                    static_cast<float>(
                        std::clamp(
                            rawAmount,
                            0.0,
                            1.0
                        )
                        );

                for (std::size_t playerIndex = 0;
                    playerIndex < NetworkPlayerCount;
                    ++playerIndex)
                {
                    const NetworkPlayerState&
                        startState =
                        m_remoteInterpolationStart
                        .players[playerIndex];

                    const NetworkPlayerState&
                        targetState =
                        m_remoteInterpolationTarget
                        .players[playerIndex];

                    const float positionX =
                        InterpolateFloat(
                            startState.positionX,
                            targetState.positionX,
                            interpolationAmount
                        );

                    const float positionY =
                        InterpolateFloat(
                            startState.positionY,
                            targetState.positionY,
                            interpolationAmount
                        );

                    const float rotation =
                        InterpolateAngle(
                            startState.rotation,
                            targetState.rotation,
                            interpolationAmount
                        );

                    m_gameSession.SetPlayerNetworkState(
                        playerIndex,
                        positionX,
                        positionY,
                        rotation
                    );
                }

                // Projectiles have constant velocity in the
                // current prototype, so advance their visual
                // copies between authoritative snapshots.
                m_gameSession.AdvanceProjectiles(
                    deltaTime
                );
            }
}