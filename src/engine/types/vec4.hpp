#pragma once

namespace engine
{
  struct vec4
  {
    vec4() = default;

    vec4( float x, float y, float z, float w )
    {
      this->x = x;
      this->y = y;
      this->z = z;
      this->w = w;
    }

    // Converts z and w as width and height
    vec4 Rect()
    {
      return vec4( x, y, x + z, y + w );
    }

    float x, y, z, w;
  };
}