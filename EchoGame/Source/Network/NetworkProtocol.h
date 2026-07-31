#pragma once

#include <cstdint>
#include <type_traits>

namespace Echo
{
    struct NetworkPlayerInput final
    {
        float movementX = 0.0f;
        float movementY = 0.0f;

        float aimTargetX = 0.0f;
        float aimTargetY = 0.0f;

        bool hasAimTarget = false;
        bool fire = false;
    };

    enum class NetworkPacketType :
        std::uint16_t
    {
        PlayerInput = 1
    };

    struct PlayerInputPacket final
    {
        static constexpr std::uint32_t
            ExpectedMagic = 0x4543484F;

        static constexpr std::uint16_t
            ExpectedVersion = 1;

        std::uint32_t magic =
            ExpectedMagic;

        std::uint16_t version =
            ExpectedVersion;

        NetworkPacketType type =
            NetworkPacketType::PlayerInput;

        std::uint32_t sequence = 0;

        float movementX = 0.0f;
        float movementY = 0.0f;

        float aimTargetX = 0.0f;
        float aimTargetY = 0.0f;

        std::uint8_t hasAimTarget = 0;
        std::uint8_t fire = 0;

        std::uint16_t reserved = 0;
    };

    static_assert(
        sizeof(PlayerInputPacket) == 32,
        "Unexpected PlayerInputPacket size."
        );

    static_assert(
        std::is_trivially_copyable_v<
        PlayerInputPacket
        >,
        "Network packet must be trivially copyable."
        );

    inline PlayerInputPacket
        CreatePlayerInputPacket(
            const NetworkPlayerInput& input,
            std::uint32_t sequence
        ) noexcept
    {
        PlayerInputPacket packet{};

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
        const PlayerInputPacket& packet,
        NetworkPlayerInput& input
    ) noexcept
    {
        if (packet.magic !=
            PlayerInputPacket::ExpectedMagic ||
            packet.version !=
            PlayerInputPacket::ExpectedVersion ||
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
}