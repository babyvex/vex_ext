#pragma once

#include <cstdint>

#if defined(_M_ARM64EC)
#error "Build native ARM64 or x64; ARM64EC is not supported."
#elif !defined(_M_X64) && !defined(__x86_64__) && !defined(_M_ARM64) && !defined(__aarch64__)
#error "mJNI supports x64 and ARM64 only."
#endif

static_assert(sizeof(void*) == 8, "mJNI requires 64-bit pointers.");

namespace mJNI {
    enum class Architecture { unknown, x64, arm64 };

    constexpr Architecture architectureFromMachine(std::uint16_t machine) {
        switch (machine) {
        case 0x8664: return Architecture::x64;   // IMAGE_FILE_MACHINE_AMD64
        case 0xAA64: return Architecture::arm64; // IMAGE_FILE_MACHINE_ARM64
        default: return Architecture::unknown;
        }
    }

    constexpr const char* architectureName(Architecture architecture) {
        switch (architecture) {
        case Architecture::x64: return "x64";
        case Architecture::arm64: return "ARM64";
        default: return "unsupported";
        }
    }
}
