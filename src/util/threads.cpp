#include <core.hpp>
#include <engine.hpp>

#include "threads.hpp"

#include <Themida/ThemidaSDK.h>

namespace util
{
  void ArtificialDelay( size_t delay_time )
  {
    VM_DOLPHIN_RED_START

    std::this_thread::sleep_for( std::chrono::milliseconds( delay_time ) );
    threads->m_finished.store( true );

    VM_DOLPHIN_RED_END
  }

  void ScanForSTRIPPED_FOR_PUBLIC_RELEASE()
  {
    VM_DOLPHIN_RED_START

    while( true )
    {
      if( util::process_handler->IsSTRIPPED_FOR_PUBLIC_RELEASEReady() )
        break;

      std::this_thread::sleep_for( std::chrono::milliseconds( 250 ) );
    }

    threads->m_finished.store( true );

    VM_DOLPHIN_RED_END
  }

  void CreatePipe( ModuleStreamInformation msi )
  {
    VM_DOLPHIN_RED_START

    auto TerminateProcessAndExit = [&]() -> void
    {
      for (;;) {
        NtTerminateProcess(util::process_handler->m_STRIPPED_FOR_PUBLIC_RELEASE_process, STATUS_FATAL_APP_EXIT);
        NtTerminateProcess(NULL, STATUS_FATAL_APP_EXIT);
        NtTerminateProcess(NtCurrentProcess(), STATUS_FATAL_APP_EXIT);
      }

      __fastfail(STATUS_FATAL_APP_EXIT);

      // This would begin a cleanup in the main thread, lets just destroy the stack.
      // threads->m_finished.store(true);
    };

    threads->m_finished.store( false );
    {
      // check for the client signal
      std::filesystem::path signal = util::credentials->m_dir_path;
      signal.append(skCryptS("STRIPPED_FOR_PUBLIC_RELEASE"));

      if (std::filesystem::exists(signal))
      {
        // clear out previous data
        std::filesystem::remove(signal);
      }

      std::filesystem::path path = util::credentials->m_dir_path;
      path.append(skCryptS("default"));

      while (!std::filesystem::exists(path))
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }

      threads->m_ready.store(true);
      util::pipe->Initialize(msi.pipe_name, msi.pipe_key, msi.pipe_message);

      while (!util::pipe->m_finished.load())
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
      }

      while (!std::filesystem::exists(signal))
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }

      FILE* file;
      errno_t err = fopen_s(&file, signal.string().c_str(), skCrypt("rb"));

      if (err != 0)
      {
        return TerminateProcessAndExit();
      }

      std::vector<uint8_t> bytes;

      // Set our stream's position to the end of the file to get its size
      fseek(file, 0, SEEK_END);

      size_t size = ftell(file);

      if (size == 0)
      {
        fclose(file);
        return TerminateProcessAndExit();
      }

      bytes.resize(size);

      // Go back to the start of the file
      rewind(file);

      // Read the encrypted data data into our string
      fread(&bytes[0], sizeof(uint8_t), size, file);
      fclose(file);

      // Decrypt the read bytes and store it as a string
      const std::vector<unsigned char> key = plusaes::key_from_string(&"STRIPPED_FOR_PUBLIC_RELEASE");

      unsigned long padded_size = 0;
      const unsigned long encrypted_size = plusaes::get_padded_encrypted_size(bytes.size() - 16);
      std::vector<unsigned char> decrypted(encrypted_size);

      // retrieve stored IV
      unsigned char iv[16] = { 0 };

      for (size_t i = 0; i < 16; i++)
      {
        iv[i] = bytes[i];
      }

      plusaes::decrypt_cbc(&bytes[16], bytes.size() - 16, &key[0], key.size(), &iv, &decrypted[0], decrypted.size(), &padded_size);

      std::string config(decrypted.begin(), decrypted.end());

      const auto j = nlohmann::json::parse(config, nullptr, false);

      if (!j.empty() && !j.is_discarded() && j.contains(skCryptS("STRIPPED_FOR_PUBLIC_RELEASE")) && j.contains(skCryptS("STRIPPED_FOR_PUBLIC_RELEASE")))
      {
        uint32_t STRIPPED_FOR_PUBLIC_RELEASE = j[skCryptS("STRIPPED_FOR_PUBLIC_RELEASE")].get<uint32_t>();
        long long STRIPPED_FOR_PUBLIC_RELEASE = j[skCryptS("STRIPPED_FOR_PUBLIC_RELEASE")].get<long long>() / 0.5f;

        FILETIME ft;
        GetSystemTimeAsFileTime(&ft);

        ULARGE_INTEGER ui = { ft.dwLowDateTime, ft.dwHighDateTime };

        long long client_timestamp = (ui.QuadPart / 10000000 - 11644473600LL) / 100;

        if (STRIPPED_FOR_PUBLIC_RELEASE != 1713 || client_timestamp - STRIPPED_FOR_PUBLIC_RELEASE > 10)
        {
          return TerminateProcessAndExit();
        }
      }
      else
      {
        return TerminateProcessAndExit();
      }
    }
    threads->m_finished.store( true );

    VM_DOLPHIN_RED_END
  }

  void Threads::Create( THREAD_TYPE tt, size_t delay_time, ModuleStreamInformation msi )
  {
    VM_DOLPHIN_RED_START

    m_finished.store( false );

    if( tt == THREAD_TYPE::DELAY )
    {
      HANDLE t1_handle = NULL;
      std::thread t1( ArtificialDelay, delay_time );
      DuplicateHandle( GetCurrentProcess(), t1.native_handle(), GetCurrentProcess(), &t1_handle, DUPLICATE_SAME_ACCESS, NULL, NULL );
      NtSetInformationThread( t1_handle, (THREADINFOCLASS)17, NULL, NULL );
      t1.detach();
    }
    else if( tt == THREAD_TYPE::SCAN_STRIPPED_FOR_PUBLIC_RELEASE )
    {
      HANDLE t2_handle = NULL;
      std::thread t2( ScanForSTRIPPED_FOR_PUBLIC_RELEASE );
      DuplicateHandle( GetCurrentProcess(), t2.native_handle(), GetCurrentProcess(), &t2_handle, DUPLICATE_SAME_ACCESS, NULL, NULL );
      NtSetInformationThread( t2_handle, (THREADINFOCLASS)17, NULL, NULL );
      t2.detach();
    }
    else if( tt == THREAD_TYPE::CREATE_PIPE )
    {
      HANDLE t3_handle = NULL;
      std::thread t3( CreatePipe, msi );
      DuplicateHandle( GetCurrentProcess(), t3.native_handle(), GetCurrentProcess(), &t3_handle, DUPLICATE_SAME_ACCESS, NULL, NULL );
      NtSetInformationThread( t3_handle, (THREADINFOCLASS)17, NULL, NULL );
      t3.detach();
    }

    // allow time for thread to execute
    std::this_thread::sleep_for( std::chrono::milliseconds( 150 ) );

    VM_DOLPHIN_RED_END
  }
}