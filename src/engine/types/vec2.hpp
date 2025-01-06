#pragma once

namespace engine
{
  struct vec2
  {
    vec2() = default;

    vec2( float n )
    {
      this->x = n;
      this->y = n;
    }

    vec2( float x, float y )
    {
      this->x = x;
      this->y = y;
    }

    void set_value( float x, float y )
    {
      this->x = x;
      this->y = y;
    }

    float x, y;
  };
}