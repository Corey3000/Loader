#pragma once

namespace engine
{
  struct vec3
  {
    vec3() = default;

    vec3( float n )
    {
      this->x = n;
      this->y = n;
      this->z = n;
    }

    vec3( float x, float y, float z = 0.0f )
    {
      this->x = x;
      this->y = y;
      this->z = z;
    }

    float x, y, z;
  };
}