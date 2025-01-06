#pragma once

namespace util
{
  class Pipe
  {
  public:
    void Initialize( std::string_view pipe_name, std::string_view pipe_key, std::string message );

    std::atomic<bool> m_finished = ATOMIC_VAR_INIT( false );
  private:

    HANDLE m_pipe = INVALID_HANDLE_VALUE;
    std::atomic<bool> m_connected = ATOMIC_VAR_INIT( false );
  };

  inline auto pipe = std::make_unique<Pipe>();

}