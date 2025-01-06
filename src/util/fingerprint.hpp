#pragma once

namespace util
{
  struct FingerprintStorage
  {
    std::wstring m_subkey;
    std::wstring m_key;
    std::wstring m_value;
    std::wstring m_raw;
    size_t m_quadpart;
    size_t m_low_date;
    size_t m_high_date;
    size_t m_timestamp;
  };

  class Fingerprint
  {
  public:
    void Initialize();
    _NODISCARD nlohmann::json Dump();

    bool m_not_enough_fingerprints = false;
  private:
    _NODISCARD bool GetFingerprint( HKEY hkey, const std::wstring subkey, const std::wstring value );
    _NODISCARD std::wstring GetString( HKEY key, const std::wstring subkey, const std::wstring value );

    void CollectSTRIPPED_FOR_PUBLIC_RELEASEData();
    void CollectFingerprints();

    std::vector<FingerprintStorage> m_fingerprints;
    std::vector<std::string> m_STRIPPED_FOR_PUBLIC_RELEASE_data;
  };

  inline auto fingerprint = std::make_unique<Fingerprint>();
}

#ifndef FINGERPRINT_PAIR
#define FINGERPRINT_PAIR( v ) std::string(#v).erase(0,3), v
#define FINGERPRINT_PAIR_K( k, v ) std::string(#k).erase(0,3), v
#endif