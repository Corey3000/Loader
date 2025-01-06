#include <core.hpp>
#include <engine.hpp>

#include <Themida/ThemidaSDK.h>
#include <KnownFolders.h>

namespace util
{
  Credentials::Credentials()
  {
    VM_DOLPHIN_RED_START
    wchar_t* local_app[MAX_PATH];
    SHGetKnownFolderPath( FOLDERID_RoamingAppData, KF_FLAG_DEFAULT_PATH, NULL, local_app );

    m_dir_path = *local_app;
    m_dir_path.append( skCryptS( "STRIPPED_FOR_PUBLIC_RELEASE" ) );

    if( !std::filesystem::exists( m_dir_path ) )
    {
      std::filesystem::create_directory( m_dir_path );
    }

    // m_dir_path is our directory path, which we use in our injector to pass our JWT token
    // m_path is our credentials path
    m_path = m_dir_path;
    m_path.append( skCryptS( "client" ) );

    // set file modification time
    if( HANDLE64 handle = CreateFile( m_path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL ) )
    {
      FILE_BASIC_INFO b;
      b.CreationTime.QuadPart = 120000000000000000;
      b.LastAccessTime.QuadPart = 120000000000000000;
      b.LastWriteTime.QuadPart = 120000000000000000;
      b.ChangeTime.QuadPart = 120000000000000000;
      b.FileAttributes = GetFileAttributes( m_path.c_str() );

      SetFileInformationByHandle( handle, FileBasicInfo, &b, sizeof( b ) );
      CloseHandle( handle );
    }

    VM_DOLPHIN_RED_END
  }

  void Credentials::Save( std::string_view username, std::string_view password )
  {
    VM_DOLPHIN_RED_START
    FILE* file;
    errno_t err = fopen_s( &file, m_path.string().c_str(), skCrypt( "wb" ) );

    if( err != 0 )
    {
      return;
    }

    STR_ENCRYPT_START

    nlohmann::json j;

    j[skCryptS( "username" )] = username;
    j[skCryptS( "password" )] = password;

    auto config = j.dump();

    std::vector<uint8_t> bytes( config.begin(), config.end() );

    // should probably use HWID
    const std::vector<unsigned char> key = plusaes::key_from_string( &"STRIPPED_FOR_PUBLIC_RELEASE" );

    std::random_device r;
    const std::seed_seq seed{ r() };

    const auto rand = std::bind( std::uniform_int_distribution<>( 0u, UCHAR_MAX ),
      std::mt19937_64( seed ) );

    unsigned char iv[16];
    std::generate_n( iv, 16, rand );

    const unsigned long encrypted_size = plusaes::get_padded_encrypted_size( bytes.size() ) + 16;
    std::vector<unsigned char> encrypted( encrypted_size );

    // add IV to the start of the file
    for( size_t i = 0; i < 16; i++ )
    {
      encrypted[i] = iv[i];
    }

    plusaes::encrypt_cbc( static_cast<unsigned char*>(bytes.data()), bytes.size(), &key[0], key.size(), &iv, &encrypted[16], encrypted.size(), true );

    fwrite( encrypted.data(), sizeof( uint8_t ), encrypted.size(), file );
    fclose( file );

    STR_ENCRYPT_END
    VM_DOLPHIN_RED_END
  }

  bool Credentials::Load( std::string& username, std::string& password )
  {
    FILE* file;
    errno_t err = fopen_s( &file, m_path.string().c_str(), skCrypt( "rb" ) );

    if( err != 0 )
    {
      return false;
    }

    std::vector<uint8_t> bytes;

    // Set our stream's position to the end of the file to get its size
    fseek( file, 0, SEEK_END );

    size_t size = ftell( file );

    if( size == 0 )
    {
      fclose( file );
      return false;
    }

    STR_ENCRYPT_START

    bytes.resize( size );

    // Go back to the start of the file
    rewind( file );

    // Read the encrypted data data into our string
    fread( &bytes[0], sizeof( uint8_t ), size, file );
    fclose( file );

    // Decrypt the read bytes and store it as a string
    // todo: maybe use HWID
    const std::vector<unsigned char> key = plusaes::key_from_string( &"STRIPPED_FOR_PUBLIC_RELEASE" );

    unsigned long padded_size = 0;
    const unsigned long encrypted_size = plusaes::get_padded_encrypted_size( bytes.size() - 16 );
    std::vector<unsigned char> decrypted( encrypted_size );

    // retrieve stored IV
    unsigned char iv[16] = { 0 };

    for( size_t i = 0; i < 16; i++ )
    {
      iv[i] = bytes[i];
    }

    plusaes::decrypt_cbc( &bytes[16], bytes.size() - 16, &key[0], key.size(), &iv, &decrypted[0], decrypted.size(), &padded_size );

    std::string creds( decrypted.begin(), decrypted.end() );

    auto j = nlohmann::json::parse( creds, nullptr, false );

    STR_ENCRYPT_END

    if( j.empty() || j.is_discarded() )
      return false;

    if( j.contains( skCryptS( "username" ) ) && j.contains( skCryptS( "password" ) ) )
    {
      username = j[skCryptS( "username" )].get<std::string>();
      password = j[skCryptS( "password" )].get<std::string>();
      return true;
    }

    return false;
  }
}