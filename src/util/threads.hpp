#pragma once

namespace util
{
  enum class THREAD_TYPE : uint8_t
  {
    DELAY,
    SCAN_STRIPPED_FOR_PUBLIC_RELEASE,
    CREATE_PIPE
    //INJECTOR
  };

  // security threads are located in security.cpp
  struct Threads
  {
    void Create( THREAD_TYPE tt, size_t delay_time = 1000, ModuleStreamInformation msi = {} );

    std::atomic<bool> m_finished = ATOMIC_VAR_INIT( false );
    std::atomic<bool> m_ready = ATOMIC_VAR_INIT( false );
  };

  inline auto threads = std::make_unique<Threads>();
}