#pragma once

namespace util
{
  struct ProcessHandler
  {
    bool IsSTRIPPED_FOR_PUBLIC_RELEASEReady();

    size_t m_STRIPPED_FOR_PUBLIC_RELEASE_candidate = 0;
    HANDLE m_STRIPPED_FOR_PUBLIC_RELEASE_process = INVALID_HANDLE_VALUE;
  };

  inline auto process_handler = std::make_unique<ProcessHandler>();
}