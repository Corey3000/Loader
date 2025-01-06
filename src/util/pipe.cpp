#include <core.hpp>
#include <engine.hpp>

#include <Themida/ThemidaSDK.h>

#include "pipe.hpp"

namespace util
{
  void PipeThread( HANDLE hpipe, std::string message )
  {
    VM_DOLPHIN_RED_START

    unsigned long bytes_written = 0;

    WriteFile( hpipe, message.c_str(), message.length(), &bytes_written, NULL);
    FlushFileBuffers( hpipe );
    DisconnectNamedPipe( hpipe );
    CloseHandle( hpipe );

    pipe->m_finished.store( true );

    VM_DOLPHIN_RED_END
  }

  void Pipe::Initialize( std::string_view pipe_name, std::string_view pipe_key, std::string message )
  {
    VM_DOLPHIN_RED_START

    m_finished.store( false );

    m_pipe = CreateNamedPipeA( std::format( "\\\\.\\pipe\\{}", pipe_name ).c_str(),
      PIPE_ACCESS_OUTBOUND,
      PIPE_TYPE_BYTE | PIPE_WAIT | PIPE_REJECT_REMOTE_CLIENTS,
      1,
      0,
      0,
      0,
      NULL );

    if( m_pipe == INVALID_HANDLE_VALUE )
      return;

    m_connected = ConnectNamedPipe( m_pipe, NULL ) ? true : (GetLastError() == ERROR_PIPE_CONNECTED);

    if( m_connected )
    {
      std::thread t1( PipeThread, m_pipe, message );
      t1.detach();
    }

    VM_DOLPHIN_RED_END
  }
}