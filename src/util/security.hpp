#pragma once

namespace util
{
  enum struct SECURITY_FLAGS : uint8_t
  {
    NONE,
    SILENT_FLAG,
    STOP_EXECUTION
  };

  enum struct SECURITY_ROUTINE : uint8_t
  {
    WAITING,
    START_ROUTINE,
    FINISHED_ROUTINE
  };

  class Security
  {
  public:
    bool CreateThreads();
    bool IsSigned( LPCWSTR pwszSourceFile );
    bool InKnownDLLs( std::wstring dll_name );

    HANDLE watch_thread = NULL;
    HANDLE security_thread = NULL; 

    SECURITY_FLAGS m_global_flag = SECURITY_FLAGS::NONE;
    SECURITY_FLAGS m_process_flag = SECURITY_FLAGS::NONE;

    std::atomic<double> m_routine_last_ran = ATOMIC_VAR_INIT( 0.0f );

    std::atomic<int> m_detection_routine = ATOMIC_VAR_INIT( 0 );
    std::atomic<int> m_threads_initialized = ATOMIC_VAR_INIT( 0 );

    std::atomic<SECURITY_ROUTINE> watch_routine = ATOMIC_VAR_INIT( SECURITY_ROUTINE::WAITING );
    std::atomic<SECURITY_ROUTINE> security_routine = ATOMIC_VAR_INIT( SECURITY_ROUTINE::WAITING );

    std::mt19937_64 m_seed;

  private:
    void GetKnownDLLs();
    SECURITY_FLAGS RanAsAdmin();
    std::vector<std::wstring> m_known_dlls{};
  };

  inline auto security = std::make_unique<Security>();
}