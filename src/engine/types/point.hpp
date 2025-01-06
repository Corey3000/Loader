#pragma once

struct Point
{
  Point() = default;

  Point( const Point& point )
  {
    x = point.x;
    y = point.y;
  }

  Point( Point&& point )
  {
    x = point.x;
    y = point.y;

    point.x = 0.f;
    point.y = 0.f;
  }

  Point& operator=( const Point& point )
  {
    x = point.x;
    y = point.y;

    return *this;
  }

  Point& operator=( Point&& point )
  {
    x = point.x;
    y = point.y;

    point.x = 0.f;
    point.y = 0.f;

    return *this;
  }

  template< typename T >
  Point( T x, T y )
  {
    this->x = (float) x;
    this->y = (float) y;
  }

  Point operator+( const Point& rhs ) const;
  Point operator-( const Point& rhs ) const;

  Point& operator+=( const Point& rhs );
  Point& operator-=( const Point& rhs );

  bool operator>( const Point& rhs ) const;
  bool operator>=( const Point& rhs ) const;

  bool operator<( const Point& rhs ) const;
  bool operator<=( const Point& rhs ) const;

  bool operator==( const Point& rhs ) const;

public:
  float x = 0.f;
  float y = 0.f;
};

inline Point Point::operator+( const Point& rhs ) const
{
  Point point( x, y );
  point.x += rhs.x;
  point.y += rhs.y;

  return point;
}

inline Point Point::operator-( const Point& rhs ) const
{
  Point point( x, y );
  point.x -= rhs.x;
  point.y -= rhs.y;

  return point;
}

inline Point& Point::operator+=( const Point& rhs )
{
  *this = *this + rhs;
  return *this;
}

inline Point& Point::operator-=( const Point& rhs )
{
  *this = *this - rhs;
  return *this;
}

inline bool Point::operator>( const Point& rhs ) const
{
  return (x > rhs.x && y > rhs.y);
}

inline bool Point::operator>=( const Point& rhs ) const
{
  return (x >= rhs.x && y >= rhs.y);
}

inline bool Point::operator<( const Point& rhs ) const
{
  return (x < rhs.x&& y < rhs.y);
}

inline bool Point::operator<=( const Point& rhs ) const
{
  return (x <= rhs.x && y <= rhs.y);
}

inline bool Point::operator==( const Point& rhs ) const
{
  return (x == rhs.x && y == rhs.y);
}