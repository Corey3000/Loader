#pragma once

namespace util
{
  static std::string MakeUTF8( const std::wstring_view string )
  {
    std::string utf8;
    utf8::utf32to8( string.begin(), utf8::find_invalid( string.begin(), string.end() ), std::back_inserter( utf8 ) );

    return utf8;
  }
}