#include <core.hpp>
#include <engine.hpp>

#include "security.hpp"

#pragma comment(lib, "ntdll.lib")

#define _NTDEF_
#pragma comment(lib, "secur32.lib")

#include <WindowsDefender.h>

#include <Themida/ThemidaSDK.h>

// The precedent behind them relies on:
// a) No interaction outside of our own process
// b) No "privacy-invasive" approaches such as reading files on disk
// c) No direct action mechanism, strictly detection.
namespace util
{
  // STRIPPED_FOR_PUBLIC_RELEASE
}