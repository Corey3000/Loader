#pragma once

namespace engine
{
  struct color
  {
    color() = default;

    color( uint32_t raw )
    {
      m_color = raw;
    }

    color( uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255 )
    {
      m_color = D3DCOLOR_ARGB( a, r, g, b );
    }

    uint32_t d3d() const
    {
      return m_color;
    }

    color blend( color opposite, float fraction )
    {
      return color( (uint8_t)((opposite.r - r) * fraction + r), (uint8_t)((opposite.g - g) * fraction + g), (uint8_t)((opposite.b - b) * fraction + b), (uint8_t)((opposite.a - a) * fraction + a) );
    }

    color Alpha( float alpha )
    {
      a *= alpha;

      return *this;
    }

    color Brightness( float brightness )
    {
      r *= brightness;
      g *= brightness;
      b *= brightness;

      return *this;
    }

    color& operator[]( const int index )
    {
      return reinterpret_cast<color*>(this)[index];
    }

    void    put_a( uint8_t a ) { m_color = (m_color & 0x00FFFFFF) | (a << 24); }
    void    put_r( uint8_t r ) { m_color = (m_color & 0xFF00FFFF) | (r << 16); }
    void    put_g( uint8_t g ) { m_color = (m_color & 0xFFFF00FF) | (g << 8); }
    void    put_b( uint8_t b ) { m_color = (m_color & 0xFFFFFF00) | (b << 0); }
    uint8_t get_a() const { return (uint8_t)((m_color & 0xFF000000) >> 24); }
    uint8_t get_r() const { return (uint8_t)((m_color & 0x00FF0000) >> 16); }
    uint8_t get_g() const { return (uint8_t)((m_color & 0x0000FF00) >> 8); }
    uint8_t get_b() const { return (uint8_t)((m_color & 0x000000FF) >> 0); }

    _declspec(property(get = get_a, put = put_a)) uint8_t a;
    _declspec(property(get = get_r, put = put_r)) uint8_t r;
    _declspec(property(get = get_g, put = put_g)) uint8_t g;
    _declspec(property(get = get_b, put = put_b)) uint8_t b;

    uint32_t m_color = UINT_MAX;
  };
}