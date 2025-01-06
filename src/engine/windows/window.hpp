#pragma once

#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_dx9.h"
#include "../imgui/imgui_impl_win32.h"
#include "../imgui/imgui_stdlib.h"

namespace engine
{
  enum struct PAINT_STAGE : uint8_t
  {
    STANDBY,
    LOGIN,
    STREAM,
    POST_STREAM,
    PRE_LOAD,
    LOADING,
  };

  class Window
  {
  public:
    Window( HINSTANCE instance );

    void Run();
    bool MessageLoop( MSG& msg );

    vec2 m_monitor_size;
    vec2 m_window_size;
    vec2 m_window_position;

    HINSTANCE m_instance;
    std::wstring m_window_name;
    std::wstring m_class_name;
    HWND m_hwnd;

    LPDIRECT3D9 m_d3d{};
    LPDIRECT3DDEVICE9 m_device{};
    D3DPRESENT_PARAMETERS m_params;

    bool m_mouse_down = false;
    bool m_mouse_down_global = false;
    POINT m_mouse_location{};

    void HandlePaintStage( std::string& paint_stage )
    {
      if( paint_stage == skCryptS( "login" ) )
      {
        m_paint_stage = PAINT_STAGE::LOGIN;
      }
      else if( paint_stage == skCryptS( "stream" ) )
      {
        m_paint_stage = PAINT_STAGE::STREAM;
      }
      else if( paint_stage == skCryptS( "load" ) )
      {
        m_paint_stage = PAINT_STAGE::LOADING;
      }
    }

    PAINT_STAGE m_paint_stage = PAINT_STAGE::STANDBY;
  private:
    void CleanupDevice();
    void ResetDevice();
    bool SetupGUI();
    void HandleDevice();
    void UpdateElapsedTime();
    void DrawBackgroundAnimations( ImDrawList* draw_list );

    void SetupStages( ImDrawList* draw_list );
    void CreateStageElements();

    float m_time{};
    float m_elapsed_time{};

    ImFont* m_font;

    std::string m_username;
    std::string m_password;
    bool m_save_creds = false;

    bool m_finished_execution = false;

    LPDIRECT3DTEXTURE9 m_STRIPPED_FOR_PUBLIC_RELEASE_texture{};

    int m_package_selected = -1;

    // plz dont look
    int Section(vec2 p, float image_base[])
    {
      auto InTriangle = [](vec2 p, vec2 a, vec2 b, vec2 c) -> bool
      {
        auto s1 = c.y - a.y;
        auto s2 = c.x - a.x;
        auto s3 = b.y - a.y;
        auto s4 = p.y - a.y;

        auto w1 = (a.x * s1 + s4 * s2 - p.x * s1) / (s3 * s2 - (b.x - a.x) * s1);
        auto w2 = (s4 - w1 * s3) / s1;
        return w1 >= 0 && w2 >= 0 && (w1 + w2) <= 1;
      };

      auto InRectangle = [](vec2 p, rect rect) -> bool
      {
        return (p.x >= rect.x && p.x <= rect.w
                && p.y >= rect.y && p.y <= rect.h);
      };

      rect image = rect(image_base[0], image_base[1], image_base[2], image_base[3]);

      if (InRectangle(p, rect(image)))
      {
        // left side
        if (InRectangle(p, rect(image_base[0], image_base[1] + 1, image_base[2] * 0.33f, image_base[3]))
            || InTriangle(p, vec2(image_base[2] * 0.33f, image_base[3]), vec2(image_base[2] * 0.33f, image_base[1] + 1), vec2(image_base[2] * 0.50f, image_base[1] + 1)))
        {
          return 1;
        }
        else if (InRectangle(p, rect(image_base[2] * 0.85f, image_base[1] + 1, image_base[2], image_base[3]))
                || InTriangle(p, vec2(image_base[2] * 0.85f, image_base[1] + 1), vec2(image_base[2] * 0.85f, image_base[3]), vec2(image_base[2] * 0.67f, image_base[3])))
        {
          return 3;
        }

        // if its in the bounds of the image_base then its gotta be one of these 3
        return 2;
      }

      return 0;
    }
  };

  inline std::optional<Window> window;
}