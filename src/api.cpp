#include <core.hpp>
#include <engine.hpp>

#include "api.hpp"

#include <Themida/ThemidaSDK.h>

namespace KEYS
{
#ifdef _DEBUG
  // staging env
  auto ENDPOINT_URL = skCrypt("https://router-staging.STRIPPED_FOR_PUBLIC_RELEASE.com/");
#else
  // prod env
  auto ENDPOINT_URL = skCrypt( "https://router.STRIPPED_FOR_PUBLIC_RELEASE.com/" );
#endif
  auto CLIENT_VERSION = skCrypt( "STRIPPED_FOR_PUBLIC_RELEASE" );
  auto USER_AGENT = skCrypt( "STRIPPED_FOR_PUBLIC_RELEASE" );
};

void API::ReEncryptKeys()
{  
  VM_DOLPHIN_RED_START
  KEYS::ENDPOINT_URL.encrypt();
  KEYS::CLIENT_VERSION.encrypt();
  KEYS::USER_AGENT.encrypt();
  VM_DOLPHIN_RED_END
}

API::API()
{
  VM_DOLPHIN_WHITE_START
  random_string = skCryptS( "STRIPPED_FOR_PUBLIC_RELEASE" );

  std::shuffle( random_string.begin(), random_string.end(), util::security->m_seed );

  if( !FetchEndpoint() )
    throw APIException( "" );

  // re-encrypt everything
  ReEncryptKeys();
  VM_DOLPHIN_WHITE_END
}

API::~API()
{
  VM_DOLPHIN_RED_START
  ReEncryptKeys();
  RtlSecureZeroMemory( random_string.data(), random_string.size() );
  RtlSecureZeroMemory( endpoint.data(), endpoint.size() );
  RtlSecureZeroMemory( endpoint_key.data(), endpoint_key.size() );
  RtlSecureZeroMemory( extras.data(), extras.size() );
  RtlSecureZeroMemory( extras_key.data(), extras_key.size() );
  RtlSecureZeroMemory( extras_payload.data(), extras_payload.size() );
  VM_DOLPHIN_RED_END
}

bool API::FetchEndpoint()
{
  try
  {
    VM_DOLPHIN_RED_START
    nlohmann::json j;
    j[skCryptS( "client_version" )] = std::string( KEYS::CLIENT_VERSION );

    cpr::Response r = cpr::Post( cpr::Url{ KEYS::ENDPOINT_URL },
      cpr::UserAgent{ std::string( KEYS::USER_AGENT ) },
      cpr::Payload{
        { skCryptS( "endpoint" ), Encrypt( skCryptS( "fetch" ) ) },
        { skCryptS( "payload" ), Encrypt( j.dump() ) }
      } );

#ifdef _DEBUG
    std::string message = std::format( "{}", r.status_code );
    MessageBoxA( nullptr, message.c_str(), skCryptS( "Info" ).c_str(), MB_OK | MB_TOPMOST | MB_ICONINFORMATION );
#endif
    VM_DOLPHIN_RED_END
    return HandleResponse( r.text );
  }
  catch( std::exception& e )
  {
#ifdef _DEBUG
    LOG( "API::FetchEndpoint() threw an exception ({})\n", e.what() );
#endif

    util::nt->MessageBoxTimeoutA( NULL, skCryptS( "Something went wrong" ).c_str(), NULL, MB_TASKMODAL | MB_ICONERROR | MB_OK, 0, 5000 );
    return false;
  }
}

bool API::HandleResponse( const std::string_view response )
{
  try
  {
    if( !extras.empty() )
    {
      VM_DOLPHIN_RED_START
      // 1kb max payload, this should never hit on any sort of module, only our unsuccessful response
      if( response.size() < 1000 )
      {
        auto dec_resp = Decrypt( response );
        auto j_dec_resp = nlohmann::json::parse( dec_resp, nullptr, false );

        if( !j_dec_resp.empty() && !j_dec_resp.is_discarded() )
        {
          if( !j_dec_resp[skCryptS( "success" )].get<bool>() )
            throw APIException( j_dec_resp[skCryptS( "response" )][skCryptS( "message" )].get<std::string>() );
        }
      }
      else
      {
        extras_payload = response;
      }

      VM_DOLPHIN_RED_END
      return true;
    }
    else
    {
      VM_DOLPHIN_RED_START
      auto dec_resp = Decrypt( response );
      auto j_dec_resp = nlohmann::json::parse( dec_resp );

      if( !j_dec_resp[skCryptS( "success" )].get<bool>() )
        throw APIException( j_dec_resp[skCryptS( "response" )][skCryptS( "message" )].get<std::string>() );

      endpoint = Encrypt( j_dec_resp[skCryptS( "response" )][skCryptS( "endpoint" )].get<std::string>() );
      endpoint_key = Encrypt( j_dec_resp[skCryptS( "response" )][skCryptS( "endpoint_key" )].get<std::string>() );

      if( endpoint.empty() || endpoint_key.empty() )
        throw APIException( skCryptS( "The server returned a response the client could not understand." ) );

      if (auto groups = j_dec_resp[skCryptS("response")][skCryptS("level")].get<std::string>(); !groups.empty())
      {
        if (auto level = std::stoi(groups); level != 0)
          group_level = level;
      }
      
      auto paint_stage = j_dec_resp[skCryptS( "response" )][skCryptS( "paint_stage" )].get<std::string>();
      engine::window->HandlePaintStage( paint_stage );

      if( j_dec_resp[skCryptS( "has_extras" )].get<bool>() )
      {
        extras = Encrypt( j_dec_resp[skCryptS( "response" )][skCryptS( "extras" )].get<std::string>() );
        extras_key = Encrypt( j_dec_resp[skCryptS( "response" )][skCryptS( "extras_key" )].get<std::string>() );
      }
      else
      {
        extras = {};
        extras_key = {};
      }
      VM_DOLPHIN_RED_END
      return true;
    }
    return false;
  }
  catch( APIException& e )
  {
#ifdef _DEBUG
    LOG( "API::HandleResponse threw an APIException ({})\n", e.what() );
#endif

    util::nt->MessageBoxTimeoutA( NULL, e.what(), NULL, MB_TASKMODAL | MB_ICONERROR | MB_OK, 0, 5000 );
    return false;
  }
  catch( std::exception& e )
  {
#ifdef _DEBUG
    LOG( "API::HandleResponse threw an exception ({})\n", e.what() );
#endif

    RtlSecureZeroMemory( extras.data(), extras.size() );
    RtlSecureZeroMemory( endpoint.data(), endpoint.size() );
    RtlSecureZeroMemory( endpoint_key.data(), endpoint_key.size() );

    util::nt->MessageBoxTimeoutA( NULL, skCryptS( "Something went wrong" ).c_str(), NULL, MB_TASKMODAL | MB_ICONERROR | MB_OK, 0, 5000 );
    return false;
  }
}

bool API::SendRequest( nlohmann::json payload )
{
  try
  {
    VM_DOLPHIN_RED_START
    if( util::security->m_global_flag == util::SECURITY_FLAGS::STOP_EXECUTION )
    {
      TerminateProcess( GetCurrentProcess(), EXIT_SUCCESS );
      return false;
    }

    cpr::Response r = cpr::Post( cpr::Url{ KEYS::ENDPOINT_URL },
      cpr::UserAgent{ std::string( KEYS::USER_AGENT ) },
      cpr::Payload{
        { skCryptS( "endpoint" ), extras.empty() ? endpoint : extras }, // endpoint and endpoint_key are stored encrypted
        { skCryptS( "endpoint_key" ), endpoint_key },
        { skCryptS( "payload" ), Encrypt( payload.dump() )}
      } );
    VM_DOLPHIN_RED_END
    return HandleResponse( r.text );
  }
  catch( std::exception e )
  {
#ifdef _DEBUG
    LOG( "API::SendRequest threw an exception ({})\n", e.what() );
#endif

    return false;
  }
}

std::string API::Encrypt( const std::string str )
{
  std::string message{};
  try
  {
    VM_DOLPHIN_RED_START
    message = util::base64::encode( str );

    for( std::size_t i = 0u; i < message.size(); i++ )
    {
      if( i % 2 != 0u )
        continue;

      message.insert( i, std::string( 1u, random_string[std::clamp<std::size_t>( i, 0u, random_string.length() )] ) );
    }

    std::reverse( message.begin(), message.end() );
    VM_DOLPHIN_RED_END
    return message;
  }
  catch( std::exception e )
  {
#ifdef _DEBUG
    LOG( "API::Encrypt threw an exception ({})\n", e.what() );
#endif

    RtlSecureZeroMemory( message.data(), message.size() );

    return {};
  }
}

std::string API::Decrypt( const std::string_view str )
{
  std::string message = std::string( str );
  std::string parsed_body = {};
  try
  {
    VM_DOLPHIN_RED_START
    std::reverse( message.begin(), message.end() );

    for( std::size_t i = 0u; i < message.length(); i++ )
    {
      if( i % 2 == 0u )
        continue;

      parsed_body.append( std::string( 1u, message[i] ) );
    }

    RtlSecureZeroMemory( message.data(), message.size() );

    VM_DOLPHIN_RED_END
    return util::base64::decode( parsed_body );
  }
  catch( std::exception e )
  {
    VM_DOLPHIN_RED_START
#ifdef _DEBUG
    LOG( "API::Decrypt threw an exception ({})\n", e.what() );
#endif

    RtlSecureZeroMemory( message.data(), message.size() );
    RtlSecureZeroMemory( parsed_body.data(), parsed_body.size() );

    VM_DOLPHIN_RED_END
    return {};
  }
}

// We need to process our endpoint since it's passed as a JSON string that contains our pipe name and pipe key.
_NODISCARD bool API::FillPipe( std::string& pipe_name, std::string& pipe_key, std::string& pipe_message )
{
  try
  {
    if( endpoint.empty() || endpoint_key.empty() )
    {
      throw APIException( skCryptS( "Something went wrong (0xC0001000)" ) );
    }

    VM_DOLPHIN_RED_START

    // We usually dont decrypt the endpoint in our API since the server will do that,
    // however, we arent sending this data to the server and are using it ourselves.
    const auto pipe_data = Decrypt( endpoint );

    // We now have a JSON string we need to convert into an object
    const auto j_parsed = nlohmann::json::parse( pipe_data );

    VM_DOLPHIN_RED_END

    if( !j_parsed[skCryptS( "name" )].empty() && !j_parsed[skCryptS( "key" )].empty() && !j_parsed[skCryptS( "data" )].empty() )
    {
      VM_DOLPHIN_RED_START

      // fill our variables
      pipe_name = j_parsed[skCryptS( "name" )].get<std::string>();
      pipe_key = j_parsed[skCryptS( "key" )].get<std::string>();

      auto data = j_parsed[skCryptS( "data" )].get<std::string>();

      std::vector<uint8_t> bytes( data.begin(), data.end() );
      
      const std::vector<unsigned char> key( pipe_key.begin(), pipe_key.end() );
      
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

      pipe_message = util::base64::encode( std::string( encrypted.begin(), encrypted.end() ) );

      VM_DOLPHIN_RED_END

      return true;
    }
    else
    {
      throw APIException( skCryptS( "Something went wrong (0xC0001001)" ) );
    }
  }
  catch( APIException &e )
  {
    VM_DOLPHIN_RED_START
#ifdef _DEBUG
      LOG( "API::FillPipe threw an exception ({})\n", e.what() );
#endif

    RtlSecureZeroMemory( endpoint.data(), endpoint.size() );
    RtlSecureZeroMemory( endpoint_key.data(), endpoint_key.size() );

    util::nt->MessageBoxTimeoutA( NULL, e.what(), NULL, MB_TASKMODAL | MB_ICONERROR | MB_OK, 0, 5000 );

    VM_DOLPHIN_RED_END

    return false;
  }
}

std::string& API::GetExtrasPayload()
{
  return extras_payload;
}

size_t API::GetExtrasKey()
{
  try
  {
    return std::stoi( Decrypt( extras_key ) );
  }
  catch( ... )
  {
    return 0;
  }
}