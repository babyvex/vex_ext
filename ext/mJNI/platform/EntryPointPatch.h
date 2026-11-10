#pragma once

#include "Architecture.h"

#include <array>
#include <cstddef>
#include <cstring>
#include <limits>
#include <optional>

namespace mJNI {
    struct EntryPointPatch {
        std::array<std::uint8_t, 5> bytes{};
        std::size_t size = 0;
    };

    // The destination must be HotSpot's handle_wrong_method stub.
    inline std::optional<EntryPointPatch> makeDeoptimizationPatch(
        Architecture architecture, std::uintptr_t entry, std::uintptr_t destination) {
        if (!entry || !destination) return std::nullopt;

        EntryPointPatch patch;
        if (architecture == Architecture::x64) {
            if (entry > std::numeric_limits<std::uintptr_t>::max() - 5) return std::nullopt;
            const auto next = entry + 5;
            const bool forward = destination >= next;
            const auto distance = forward ? destination - next : next - destination;
            const auto limit = forward ? std::uint64_t{0x7FFFFFFF} : std::uint64_t{0x80000000};
            if (distance > limit) return std::nullopt;

            const auto displacement = static_cast<std::uint32_t>(
                forward ? distance : std::uint64_t{0} - distance);
            patch.bytes[0] = 0xE9; // JMP rel32, relative to the next instruction.
            std::memcpy(patch.bytes.data() + 1, &displacement, sizeof(displacement));
            patch.size = 5;
        } else if (architecture == Architecture::arm64) {
            if ((entry & 3) || (destination & 3)) return std::nullopt;
            const bool forward = destination >= entry;
            const auto distance = forward ? destination - entry : entry - destination;
            const auto limit = forward ? std::uint64_t{0x07FFFFFC} : std::uint64_t{0x08000000};

            // HotSpot uses its not-entrant trap when the stub is outside B imm26 range.
            // https://github.com/openjdk/jdk21u/blob/master/src/hotspot/cpu/aarch64/nativeInst_aarch64.cpp
            std::uint32_t instruction = 0xD4BBD5A1; // DCPS1 #0xDEAD
            if (distance <= limit) {
                const auto words = distance / 4;
                const auto immediate = static_cast<std::uint32_t>(
                    forward ? words : std::uint64_t{0} - words) & 0x03FFFFFF;
                instruction = 0x14000000 | immediate; // B imm26, relative to this instruction.
            }
            std::memcpy(patch.bytes.data(), &instruction, sizeof(instruction));
            patch.size = 4;
        } else {
            return std::nullopt;
        }
        return patch;
    }
}
