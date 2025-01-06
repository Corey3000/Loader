#pragma once

namespace engine
{
  struct rect
  {
    rect() = default;

    rect( float n )
    {
      *this = rect( n, n, n, n );
    }

    rect( const rect& rect )
    {
      start = rect.start;
      end = rect.end;
    }

    rect( Point start, Point end ) :
      start( vec2(start.x, start.y) ),
      end( vec2( end.x, end.y ) )
    {}

    rect( Point xy ) :
      start( vec2( xy.x, xy.y ) ),
      end( 1.f, 1.f )
    {}

    rect( rect&& rect )
    {
      start = std::move( rect.start );
      end = std::move( rect.end );
    }

    rect& operator=( const rect& rect )
    {
      start = rect.start;
      end = rect.end;

      return *this;
    }

    rect& operator=( rect&& rect )
    {
      start = std::move( rect.start );
      end = std::move( rect.end );

      return *this;
    }

    rect( vec2 start, vec2 end ) :
      start( start ),
      end( end )
    {
    }

    rect( vec2 xy ) :
      start( xy ),
      end( 1.f, 1.f )
    {
    }

    template< typename T >
    rect( T x, T y, T w, T h )
    {
      start = vec2( x, y );
      end = vec2( x + w, y + h );
    }

    void  put_x( float x ) { auto width = w; start.x = x; w = width; }
    void  put_y( float y ) { auto height = h; start.y = y; h = height; }
    void  put_w( float w ) { end.x = start.x + w; }
    void  put_h( float h ) { end.y = start.y + h; }
    float get_x() const { return start.x; }
    float get_y() const { return start.y; }
    float get_w() const { return std::max( 0.f, end.x - start.x ); }
    float get_h() const { return std::max( 0.f, end.y - start.y ); }

    _declspec(property(get = get_x, put = put_x)) float x;
    _declspec(property(get = get_y, put = put_y)) float y;
    _declspec(property(get = get_w, put = put_w)) float w;
    _declspec(property(get = get_h, put = put_h)) float h;

    void set_pos( float px = 0.0f, float py = 0.0f )
    {
      if( px != x )
      {
        x = px;
      }

      if( py != y )
      {
        y = py;
      }
    }

    void set_pos( const vec2& point )
    {
      set_pos( point.x, point.y );
    }

    void set_size( float pw = 0.0f, float ph = 0.0f )
    {
      if( pw != w )
      {
        w = pw;
      }

      if( ph != h )
      {
        h = ph;
      }
    }

    void set_size( const vec2& size )
    {
      set_size( size.x, size.y );
    }

  public:
    vec2 start;
    vec2 end;
  };
}