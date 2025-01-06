#include <core.hpp>
#include <engine.hpp>

#include <Themida/ThemidaSDK.h>

namespace util
{
  void Fingerprint::Initialize()
  {
    // collect STRIPPED_FOR_PUBLIC_RELEASE data
    CollectSTRIPPED_FOR_PUBLIC_RELEASEData();

    // collect fingerprint data
    CollectFingerprints();
  }

  void Fingerprint::CollectFingerprints()
  {
    VM_DOLPHIN_WHITE_START
    STR_ENCRYPT_START

    // This should be routed via the server instead of clientside
    const std::vector<std::tuple<HKEY, std::wstring, std::wstring>> sources = {
      // STRIPPED_FOR_PUBLIC_RELEASE
    };

    size_t fingerprints_retrieved = 0;
    for( const auto& [hkey, k, v] : sources )
    {
      if( GetFingerprint( hkey, k, v ) )
        fingerprints_retrieved++;
    }

#ifdef _DEBUG
    for( const auto& fp : m_fingerprints )
    {
      LOG( "{} {} {} {} {}\n", MakeUTF8( fp.m_subkey ), MakeUTF8( fp.m_key ), MakeUTF8( fp.m_value ), fp.m_quadpart, fp.m_timestamp );
    }
#endif

    // require atleast 2 fingerprints
    m_not_enough_fingerprints = fingerprints_retrieved < 3;

    STR_ENCRYPT_END
    VM_DOLPHIN_WHITE_END
  }

  // collect their STRIPPED_FOR_PUBLIC_RELEASE user information, we can use this for information purposes to help identify malicious actors / bad hwids
  void Fingerprint::CollectSTRIPPED_FOR_PUBLIC_RELEASEData()
  {
    VM_DOLPHIN_WHITE_START
    STR_ENCRYPT_START

    // first, get the STRIPPED_FOR_PUBLIC_RELEASE path from registry
    // maybe in future we will recode Fingerprint::GetFingerprint to avoid duplicate code

    const std::wstring subkey = L"SOFTWARE\\STRIPPED_FOR_PUBLIC_RELEASE\\STRIPPED_FOR_PUBLIC_RELEASE";

    if( HKEY new_key = nullptr; RegOpenKeyEx( HKEY_CURRENT_USER, subkey.c_str(), 0, KEY_QUERY_VALUE, &new_key) == ERROR_SUCCESS )
    {
      const auto kv = GetString( new_key, subkey, L"STRIPPED_FOR_PUBLIC_RELEASE" );

      // no STRIPPED_FOR_PUBLIC_RELEASE path found, server will flag them for this
      if( !kv.empty() )
      {
        // kv will return something like c:/program files (x86)/STRIPPED_FOR_PUBLIC_RELEASE
        // append userdata to that value
        std::filesystem::path userdata_path( kv );
        userdata_path.append( skCryptS( "STRIPPED_FOR_PUBLIC_RELEASE" ) );

        if( std::filesystem::exists( userdata_path ) )
        {
          // normalize our path
          userdata_path = std::filesystem::weakly_canonical( userdata_path );

          // iterate through the entire folder and get all folder names
          for( const auto& it : std::filesystem::directory_iterator( userdata_path ) )
          {
            if( it.path().empty() || !it.path().has_filename() )
              continue;

            if( !std::filesystem::is_directory( it.path() ) )
              continue;

            m_STRIPPED_FOR_PUBLIC_RELEASE_data.emplace_back( it.path().filename().string() );
          }
        }
      }
    }

    STR_ENCRYPT_END
    VM_DOLPHIN_WHITE_END
  }

  _NODISCARD nlohmann::json Fingerprint::Dump()
  {
    VM_DOLPHIN_WHITE_START
    STR_ENCRYPT_START

    // We want a structure like: {"fingerprints":[objects],"STRIPPED_FOR_PUBLIC_RELEASE_data":[strings]}
    // Server will receive: {"fp":{"fingerprints":[],"STRIPPED_FOR_PUBLIC_RELEASE_data":[]},"username":{},"password":{}}
    nlohmann::json fingerprints = nlohmann::json::array();

    for( const auto& fp : m_fingerprints )
    {
      const auto object = nlohmann::json::object(
        {
          { FINGERPRINT_PAIR_K( fp.m_key,     util::sha256::hash256_hex_string( MakeUTF8( fp.m_key ) ) ) },
          { FINGERPRINT_PAIR_K( fp.m_subkey,  util::sha256::hash256_hex_string( MakeUTF8( fp.m_subkey ) ) ) },
          { FINGERPRINT_PAIR_K( fp.m_value,   util::sha256::hash256_hex_string( MakeUTF8( fp.m_value ) ) ) },
          { FINGERPRINT_PAIR_K( fp.m_raw,     MakeUTF8( fp.m_raw ) ) },
          { FINGERPRINT_PAIR( fp.m_quadpart ) },
          { FINGERPRINT_PAIR( fp.m_timestamp ) },
          { FINGERPRINT_PAIR( fp.m_low_date ) },
          { FINGERPRINT_PAIR( fp.m_high_date ) }
        }
      );

      fingerprints.emplace_back( object );
    }

    // dump our STRIPPED_FOR_PUBLIC_RELEASE data into an array
    nlohmann::json STRIPPED_FOR_PUBLIC_RELEASE_data = nlohmann::json::array();

    for( const auto& sd : m_STRIPPED_FOR_PUBLIC_RELEASE_data )
    {
      STRIPPED_FOR_PUBLIC_RELEASE_data.emplace_back( sd );
    }

    nlohmann::json dump;
    dump[skCryptS( "fingerprints" )] = fingerprints;
    dump[skCryptS( "STRIPPED_FOR_PUBLIC_RELEASE_data" )] = STRIPPED_FOR_PUBLIC_RELEASE_data;

    STR_ENCRYPT_END
    VM_DOLPHIN_WHITE_END

    return dump;
  }

  _NODISCARD bool Fingerprint::GetFingerprint( HKEY hkey, const std::wstring subkey, const std::wstring value )
  {
    VM_DOLPHIN_WHITE_START
    
    bool ret = false;
    
    if( HKEY new_key = nullptr; RegOpenKeyEx( hkey, subkey.c_str(), 0, KEY_QUERY_VALUE, &new_key ) == ERROR_SUCCESS )
    {
      // Get key value
      const std::wstring kv = GetString( new_key, subkey, value );

      if( !kv.empty() )
      {
        // Retrieve last write time of the key
        FILETIME filetime;

        if( RegQueryInfoKeyA( new_key, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &filetime ) == ERROR_SUCCESS )
        {
          ULARGE_INTEGER ui = { filetime.dwLowDateTime, filetime.dwHighDateTime };

          FingerprintStorage fp = {
            .m_subkey = subkey,
            .m_key = value,
            .m_value = kv,
            .m_raw = kv,
            .m_quadpart = ui.QuadPart,
            .m_low_date = filetime.dwLowDateTime,
            .m_high_date = filetime.dwHighDateTime,
            .m_timestamp = ui.QuadPart / 10000000 - 11644473600LL
          };

          m_fingerprints.emplace_back( fp );
          ret = true;
        }
      }

      RegCloseKey( new_key );
    }
    
    VM_DOLPHIN_WHITE_END
    
    return ret;
  }

  _NODISCARD std::wstring Fingerprint::GetString( HKEY key, std::wstring subkey, std::wstring value )
  {
    VM_DOLPHIN_WHITE_START

    wchar_t buffer[1024] = {};
    DWORD data_size = sizeof( buffer );
    DWORD type = REG_SZ;

    LSTATUS status = RegQueryValueExW( key, value.c_str(), NULL, &type, reinterpret_cast<LPBYTE>(&buffer), &data_size );

    VM_DOLPHIN_WHITE_END

    return status == ERROR_SUCCESS ? buffer : std::wstring();
  }
}