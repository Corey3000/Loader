#include <core.hpp>
#include <engine.hpp>

#include "engine/imgui/imgui.h"
#include "engine/imgui/imgui_impl_dx9.h"
#include "engine/imgui/imgui_impl_win32.h"

#include <Themida/ThemidaSDK.h>

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previous_instance, LPSTR cmd, int show)
{
  VM_DOLPHIN_WHITE_START

  if (!util::security->CreateThreads())
  {
    util::security->m_global_flag = util::SECURITY_FLAGS::STOP_EXECUTION;
    util::nt->MessageBoxTimeoutA(NULL, skCryptS("Something went wrong during initialization. Please try again.").c_str(), NULL, MB_TASKMODAL | MB_ICONERROR | MB_OK, 0, 5000);
    return 0;
  }

  util::fingerprint->Initialize();

  if (util::fingerprint->m_not_enough_fingerprints)
  {
    util::security->m_global_flag = util::SECURITY_FLAGS::STOP_EXECUTION;
    util::nt->MessageBoxTimeoutA(NULL, skCryptS("Please run the program as administrator and try again.").c_str(), NULL, MB_TASKMODAL | MB_ICONERROR | MB_OK, 0, 5000);
    return 0;
  }

  engine::window.emplace(engine::Window{ instance });

  const auto class_name = L" ";
  const auto window_name = L" ";

  // Setup with window class settings
  WNDCLASS winclass;
  winclass.style = WS_EX_TOPMOST | CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
  winclass.lpfnWndProc = WndProc;
  winclass.cbClsExtra = 0;
  winclass.cbWndExtra = 0;
  winclass.hInstance = engine::window->m_instance;
  winclass.hIcon = 0;
  winclass.hCursor = LoadCursor(nullptr, IDC_ARROW);
  winclass.hbrBackground = NULL;
  winclass.lpszMenuName = window_name;
  winclass.lpszClassName = class_name;

  // Create the actual window
  // Subtract 1 and add 2 for the window border, so when we render we have the desired width and height of the window
  RegisterClass(&winclass);

  engine::window->m_hwnd = CreateWindowW(class_name,
                                         window_name,
                                         WS_POPUPWINDOW,
                                         engine::window->m_window_position.x - 1,
                                         engine::window->m_window_position.y - 1,
                                         engine::window->m_window_size.x + 2,
                                         engine::window->m_window_size.y + 2,
                                         nullptr,
                                         nullptr,
                                         engine::window->m_instance,
                                         nullptr
  );

  char system_path[MAX_PATH];
  GetSystemDirectoryA(system_path, MAX_PATH);

  std::string title = system_path;

  if (std::filesystem::exists(system_path))
  {
    std::vector<std::string> name_candidates{};
    for (const auto& t : std::filesystem::directory_iterator(system_path))
    {
      if (!t.path().has_extension())
        continue;

      if (t.path().extension() == skCryptS(".exe"))
        name_candidates.push_back(t.path().filename().string());
    }

    std::shuffle(name_candidates.begin(), name_candidates.end(), util::security->m_seed);
    title = system_path;
    title.append(skCrypt("\\"));
    title.append(name_candidates[0]);
  }

  SetWindowTextA(engine::window->m_hwnd, title.c_str());
  SetWindowPos(engine::window->m_hwnd, HWND_TOP, engine::window->m_window_position.x, engine::window->m_window_position.y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);

  engine::window->Run();

  VM_DOLPHIN_WHITE_END
    return 0;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
  if (ImGui_ImplWin32_WndProcHandler(hwnd, message, wparam, lparam))
    return true;

  if (message == WM_KILLFOCUS)
  {
    if (engine::window->m_mouse_down)
      engine::window->m_mouse_down = false;

    if (engine::window->m_mouse_down_global)
      engine::window->m_mouse_down_global = false;
  }

  if (message == WM_LBUTTONDOWN)
  {
    if (engine::window->m_mouse_down_global)
      engine::window->m_mouse_down_global = false;

    if (engine::window->m_mouse_down)
    {
      engine::window->m_mouse_down = false;
    }
    else
    {
      engine::window->m_mouse_down_global = true;

      RECT rect;
      GetWindowRect(hwnd, &rect);
      GetCursorPos(&engine::window->m_mouse_location);
      engine::window->m_mouse_location.x = fabsf(engine::window->m_mouse_location.x - rect.left);
      engine::window->m_mouse_location.y = fabsf(engine::window->m_mouse_location.y - rect.top);

      if (engine::window->m_mouse_location.y <= 68 || engine::window->m_mouse_location.x <= 68 ||
          engine::window->m_mouse_location.x >= 428 || engine::window->m_mouse_location.y >= 388)
      {
        engine::window->m_mouse_down = true;
      }
    }
  }
  else if (message == WM_LBUTTONUP)
  {
    if (engine::window->m_mouse_down)
      engine::window->m_mouse_down = false;

    if (engine::window->m_mouse_down_global)
      engine::window->m_mouse_down_global = false;
  }
  else if (message == WM_MOUSEMOVE)
  {
    if (engine::window->m_mouse_down)
    {
      POINT current_pos;
      GetCursorPos(&current_pos);
      MoveWindow(hwnd,
                 current_pos.x - engine::window->m_mouse_location.x,
                 current_pos.y - engine::window->m_mouse_location.y,
                 engine::window->m_window_size.x + 2,
                 engine::window->m_window_size.y + 2,
                 false
      );
    }
  }
  else if (message == WM_CLOSE || message == WM_QUIT || message == WM_DESTROY)
  {
    PostQuitMessage(0);
    return 0;
  }
  else if (message == WM_SYSCOMMAND)
  {
    // Disable ALT application menu
    if ((wparam & 0xfff0) == SC_KEYMENU)
      return 0;
  }

  return(DefWindowProc(hwnd, message, wparam, lparam));
}