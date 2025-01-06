#include "injector.hpp"

#include <core.hpp>
#include <engine.hpp>

#include <Themida/ThemidaSDK.h>

extern "C" {
NTSTATUS InjLoadImage(HANDLE ProcessHandle, PBYTE ImageBuffer);
}

namespace util {
 // STRIPPED_FOR_PUBLIC_RELEASE
}  // namespace util