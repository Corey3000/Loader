#pragma once

namespace util
{
  struct Credentials
  {
    Credentials();
    void Save( std::string_view username, std::string_view password);
    bool Load( std::string& username, std::string& password );

    std::filesystem::path m_dir_path;
    std::filesystem::path m_path;
  };

  inline auto credentials = std::make_unique<Credentials>();
}