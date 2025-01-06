#pragma once
#include <optional>

class API
{
public:
  API();
  ~API();

  bool SendRequest( nlohmann::json payload );
  std::string& GetExtrasPayload();
  size_t GetExtrasKey();
  _NODISCARD bool IsGroupAvailable(int group) { return group <= group_level; }
  _NODISCARD int HighestGroupAvailable() { return group_level; }
  _NODISCARD bool FillPipe( std::string& pipe_name, std::string& pipe_key, std::string& pipe_message );
private:
  bool FetchEndpoint();
  void ReEncryptKeys();
  _NODISCARD bool HandleResponse( const std::string_view response );

  std::string Encrypt( const std::string str );
  std::string Decrypt( const std::string_view str );

  std::string random_string{};
  std::string endpoint{};
  std::string endpoint_key{};
  std::string extras{};
  std::string extras_key{};
  std::string extras_payload{};
  int group_level = 0;
};

inline std::optional<API> api;