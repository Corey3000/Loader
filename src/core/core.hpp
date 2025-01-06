#pragma once

#define NOMINMAX 
#define PHNT_NO_INLINE_INIT_STRING
#define PHNT_VERSION PHNT_THRESHOLD

// clang-format off
#include <phnt_windows.h>
#include <phnt.h>
// clang-format on

#include <windowsx.h>
#include <mutex>
#include <deque>
#include <tchar.h>
#include <WinUser.h>
#include <corecrt_math_defines.h>
#include <math.h>
#include <random>
#include <xmmintrin.h>
#include <pmmintrin.h>
#include <format>
#include <Psapi.h>
#include <WinTrust.h>
#include <TlHelp32.h>
#include <shlwapi.h>
#include <ShlObj_core.h>

#pragma comment(lib, "d3d9.lib")
#include <d3d9.h>
#pragma comment(lib, "d3dx9.lib")
#include <d3dx9.h>

#include <cpr/cpr.h>
#include <skadro-official/skCrypter.h>
#include <nlohmann/json.hpp>

#include <utf/utf8.h>

#include <zlib.h>

#ifdef _DEBUG
#define ASSERT(assertion) if (assertion == false) __debugbreak();
#else
#define ASSERT(assertion) assertion;
#endif

#ifdef _DEBUG
#define LOG(f, ...) OutputDebugStringA( std::format( f, __VA_ARGS__ ).c_str() )
#endif