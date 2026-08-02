#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace Echo
{
    constexpr std::size_t
        NetworkPlayerCount = 2;

    constexpr std::size_t
        NetworkMaxProjectileCount = 32;

    struct NetworkPlayerInput final
    {
        float movementX = 0.0f;
        float movementY = 0.0f;

        float aimTargetX = 0.0f;
        float aimTargetY = 0.0f;

        bool hasAimTarget = false;
        bool fire = false;
    };

    struct NetworkPlayerState final
    {
        float positionX = 0.0f;
        float positionY = 0.0f;
        float rotation = 0.0f;
    };

    struct NetworkProjectileState final
    {
        float positionX = 0.0f;
        float positionY = 0.0f;
        float rotation = 0.0f;
    };

    struct NetworkWorldSnapshot final
    {
        std::array<
            NetworkPlayerState,
            NetworkPlayerCount
        > players{};

        std::uint32_t projectileCount = 0;

        std::array<
            NetworkProjectileState,
            NetworkMaxProjectileCount
        > projectiles{};
    };

    enum class NetworkPacketType :
        std::uint16_t
    {
        PlayerInput = 1,
        WorldSnapshot = 2
    };

    struct NetworkPacket final
    {
        static constexpr std::uint32_t
            ExpectedMagic = 0x4543484F;

        static constexpr std::uint16_t
            ExpectedVersion = 3;

        std::uint32_t magic =
            ExpectedMagic;

        std::uint16_t version =
            ExpectedVersion;

        NetworkPacketType type =
            NetworkPacketType::PlayerInput;

        std::uint32_t sequence = 0;

        // Player input payload.
        float movementX = 0.0f;
        float movementY = 0.0f;

        float aimTargetX = 0.0f;
        float aimTargetY = 0.0f;

        std::uint8_t hasAimTarget = 0;
        std::uint8_t fire = 0;

        std::uint16_t reserved = 0;

        // World snapshot payload.
        std::array<
            NetworkPlayerState,
            NetworkPlayerCount
        > players{};

        std::uint32_t projectileCount = 0;

        std::array<
            NetworkProjectileState,
            NetworkMaxProjectileCount
        > projectiles{};
    };

    static_assert(
        sizeof(NetworkPacket) == 444,
        "Unexpected NetworkPacket size."
        );

    static_assert(
        std::is_trivially_copyable_v<
        NetworkPacket
        >,
        "Network packet must be trivially copyable."
        );

    inline bool HasValidNetworkHeader(
        const NetworkPacket& packet
    ) noexcept
    {
        return
            packet.magic ==
            NetworkPacket::ExpectedMagic &&
            packet.version ==
            NetworkPacket::ExpectedVersion;
    }

    inline NetworkPacket
        CreatePlayerInputPacket(
            const NetworkPlayerInput& input,
            std::uint32_t sequence
        ) noexcept
    {
        NetworkPacket packet{};

        packet.type =
            NetworkPacketType::PlayerInput;

        packet.sequence =
            sequence;

        packet.movementX =
            input.movementX;

        packet.movementY =
            input.movementY;

        packet.aimTargetX =
            input.aimTargetX;

        packet.aimTargetY =
            input.aimTargetY;

        packet.hasAimTarget =
            input.hasAimTarget ? 1u : 0u;

        packet.fire =
            input.fire ? 1u : 0u;

        return packet;
    }

    inline bool DecodePlayerInputPacket(
        const NetworkPacket& packet,
        NetworkPlayerInput& input
    ) noexcept
    {
        if (!HasValidNetworkHeader(packet) ||
            packet.type !=
            NetworkPacketType::PlayerInput)
        {
            return false;
        }

        input.movementX =
            packet.movementX;

        input.movementY =
            packet.movementY;

        input.aimTargetX =
            packet.aimTargetX;

        input.aimTargetY =
            packet.aimTargetY;

        input.hasAimTarget =
            packet.hasAimTarget != 0;

        input.fire =
            packet.fire != 0;

        return true;
    }

    inline NetworkPacket
        CreateWorldSnapshotPacket(
            const NetworkWorldSnapshot& snapshot,
            std::uint32_t sequence
        ) noexcept
    {
        NetworkPacket packet{};

        packet.type =
            NetworkPacketType::WorldSnapshot;

        packet.sequence =
            sequence;

        packet.players =
            snapshot.players;

        packet.projectileCount =
            snapshot.projectileCount;

        packet.projectiles =
            snapshot.projectiles;

        return packet;
    }

    inline bool DecodeWorldSnapshotPacket(
        const NetworkPacket& packet,
        NetworkWorldSnapshot& snapshot
    ) noexcept
    {
        if (!HasValidNetworkHeader(packet) ||
            packet.type !=
            NetworkPacketType::WorldSnapshot)
        {
            return false;
        }

        if (packet.projectileCount >
            NetworkMaxProjectileCount)
        {
            return false;
        }

        snapshot.players =
            packet.players;

        snapshot.projectileCount =
            packet.projectileCount;

        snapshot.projectiles =
            packet.projectiles;

        return true;
    }
}