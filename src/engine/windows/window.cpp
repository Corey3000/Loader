#include <Themida/ThemidaSDK.h>

#include <core.hpp>
#include <engine.hpp>

// --------READ BEFORE CONTINUING--------
// This is incomplete and needs to be properly recoded.
// This may be the final version due to heavy time constraints
// --------READ BEFORE CONTINUING--------

namespace engine {
constexpr ImGuiWindowFlags flags =
    ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

Window::Window(HINSTANCE instance) : m_hwnd(0), m_params{} {
  m_instance = instance;

  // Grab the monitors size
  RECT desktop;
  const HWND hDesktop = GetDesktopWindow();
  GetWindowRect(hDesktop, &desktop);

  m_monitor_size.x = desktop.right;
  m_monitor_size.y = desktop.bottom;

  m_window_size.set_value(500.0f, 450.0f);

  // Set the window position so that its in the center
  m_window_position.x = (m_monitor_size.x - m_window_size.x) * 0.5f;
  m_window_position.y = (m_monitor_size.y - m_window_size.y) * 0.5f;
}

void Window::Run() {
  m_d3d = Direct3DCreate9(D3D_SDK_VERSION);

  // Create the D3DDevice
  ZeroMemory(&m_params, sizeof(m_params));
  m_params.Windowed = TRUE;
  m_params.SwapEffect = D3DSWAPEFFECT_DISCARD;
  m_params.BackBufferFormat =
      D3DFMT_UNKNOWN;  // Need to use an explicit format with alpha if needing
                       // per-pixel alpha composition.
  m_params.EnableAutoDepthStencil = TRUE;
  m_params.AutoDepthStencilFormat = D3DFMT_D16;
  m_params.PresentationInterval =
      D3DPRESENT_INTERVAL_ONE;  // Present with vsync
  // g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present
  // without vsync, maximum unthrottled framerate

  m_d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_hwnd,
                      D3DCREATE_HARDWARE_VERTEXPROCESSING, &m_params,
                      &m_device);

  MSG msg;

  try {
    api.emplace(API{});
  } catch (APIException& e) {
    api.reset();
#ifdef _DEBUG
    MessageBoxA(NULL, e.what(), NULL, MB_OK | MB_TOPMOST | MB_ICONERROR);
#endif
    return;
  }

  char windows[MAX_PATH + 1];
  GetWindowsDirectoryA(windows, MAX_PATH);

  std::filesystem::path font_path(windows);
  font_path.append(skCryptS("Fonts"));

  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  (void)io;

  // Line 191 of imgui_draw.cpp (void ImGui::StyleColorsDark(ImGuiStyle* dst))
  ImGui::StyleColorsDark();

  ImGui_ImplWin32_Init(m_hwnd);
  ImGui_ImplDX9_Init(m_device);

  // remove the imgui.ini file
  io.IniFilename = NULL;

  // Load fonts
  io.Fonts->AddFontDefault();

  // backup fonts
  if (std::filesystem::exists(font_path)) {
    std::filesystem::path segoe_path(font_path);
    segoe_path.append(skCryptS("segoeui.ttf"));

    std::filesystem::path verdana_path(font_path);
    verdana_path.append(skCryptS("verdana.ttf"));

    if (std::filesystem::exists(segoe_path)) {
      io.Fonts->AddFontFromFileTTF(segoe_path.string().c_str(), 18.0f);
    }

    if (std::filesystem::exists(verdana_path)) {
      io.Fonts->AddFontFromFileTTF(verdana_path.string().c_str(), 18.0f);
    }

    if (io.Fonts->Fonts.size() > 1) io.FontDefault = io.Fonts->Fonts[1];

    io.Fonts->Build();
  }

  if (D3DXCheckTextureRequirements(m_device, NULL, NULL, NULL, 0, NULL,
                                   D3DPOOL_DEFAULT) == S_OK) {
    D3DXCreateTextureFromFileInMemoryEx(
        m_device, &resources::STRIPPED_FOR_PUBLIC_RELEASE, sizeof(resources::STRIPPED_FOR_PUBLIC_RELEASE), NULL, NULL, NULL,
        D3DUSAGE_RENDERTARGET, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT,
        D3DX_DEFAULT, 0, NULL, NULL, &m_STRIPPED_FOR_PUBLIC_RELEASE_texture);
  }

  m_save_creds = util::credentials->Load(m_username, m_password);

  while (MessageLoop(msg)) {
    if (util::security->m_global_flag == util::SECURITY_FLAGS::STOP_EXECUTION)
      break;

    UpdateElapsedTime();

    if (!SetupGUI()) break;

    HandleDevice();

    m_time += m_elapsed_time;
  }

  ImGui_ImplDX9_Shutdown();
  ImGui_ImplWin32_Shutdown();
  ImGui::DestroyContext();

  CleanupDevice();
}

bool Window::MessageLoop(MSG& msg) {
  while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  if (msg.message == WM_CLOSE || msg.message == WM_QUIT) {
    return false;
  }

  return true;
}

void Window::CleanupDevice() {
  if (m_device) {
    m_device->Release();
    m_device = NULL;
  }

  if (m_d3d) {
    m_d3d->Release();
    m_d3d = NULL;
  }

  if (m_STRIPPED_FOR_PUBLIC_RELEASE_texture) {
    m_STRIPPED_FOR_PUBLIC_RELEASE_texture->Release();
    m_STRIPPED_FOR_PUBLIC_RELEASE_texture = NULL;
  }
}

void Window::ResetDevice() {
  ImGui_ImplDX9_InvalidateDeviceObjects();

  // who cares. just let it loop again and eventually itll go through
  if (m_device->Reset(&m_params) == D3DERR_INVALIDCALL) return;

  ImGui_ImplDX9_CreateDeviceObjects();
}

bool Window::SetupGUI() {
  ImGui_ImplDX9_NewFrame();
  ImGui_ImplWin32_NewFrame();

  ImGui::NewFrame();
  {
    ImGui::Begin(skCrypt("Background"), 0,
                 flags | ImGuiWindowFlags_NoBringToFrontOnFocus);
    {
      ImGui::SetWindowPos(ImVec2(0, 0));
      ImGui::SetWindowSize(ImVec2(m_window_size.x, m_window_size.y));

      ImDrawList* draw_list = ImGui::GetWindowDrawList();

      DrawBackgroundAnimations(draw_list);

      // Our "close" button
      ImGui::SameLine(ImGui::GetWindowWidth() - 30);
      // ImGui::PushStyleColor( ImGuiCol_Button, IM_COL32( 255, 255, 255, 255 )
      // );
      if (ImGui::Button(" ", ImVec2(20, 20)) || m_finished_execution) {
        return false;
      }
      // ImGui::PopStyleColor();

      SetupStages(draw_list);
    }
    ImGui::End();

    ImGui::Begin(skCrypt("MainWindow"), 0,
                 flags | ImGuiWindowFlags_NoBackground);
    { CreateStageElements(); }
    ImGui::End();
  }
  ImGui::EndFrame();
  return true;
}

void Window::DrawBackgroundAnimations(ImDrawList* draw_list) {
  // STRIPPED_FOR_PUBLIC_RELEASE
}

void Window::HandleDevice() {
  m_device->SetRenderState(D3DRS_ZENABLE, FALSE);
  m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
  m_device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

  D3DCOLOR clear_col_dx = D3DCOLOR_RGBA(255, 255, 255, 255);
  m_device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx,
                  1.0f, 0);

  if (m_device->BeginScene() >= 0) {
    ImGui::Render();
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
    m_device->EndScene();
  }
  HRESULT result = m_device->Present(NULL, NULL, NULL, NULL);

  // Handle loss of D3D9 device
  if (result == D3DERR_DEVICELOST &&
      m_device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
    ResetDevice();
}

void Window::UpdateElapsedTime() {
  const auto current_time = std::chrono::utc_clock::now();
  static auto previous_time = current_time;

  std::chrono::duration<double> change_in_time = current_time - previous_time;
  previous_time = std::chrono::utc_clock::now();

  m_elapsed_time = change_in_time.count();
}

void Window::SetupStages(ImDrawList* draw_list) {
  // STRIPPED_FOR_PUBLIC_RELEASE

  if (m_paint_stage == PAINT_STAGE::LOGIN) {
    // STRIPPED_FOR_PUBLIC_RELEASE
  } else if (m_paint_stage == PAINT_STAGE::STREAM) {
    // STRIPPED_FOR_PUBLIC_RELEASE
    if (m_STRIPPED_FOR_PUBLIC_RELEASE_texture)
      // STRIPPED_FOR_PUBLIC_RELEASE

    // so we need to setup an "area" for STRIPPED_FOR_PUBLIC_RELEASE/STRIPPED_FOR_PUBLIC_RELEASE/STRIPPED_FOR_PUBLIC_RELEASE, so lets draw divide it by 3, diagonally, and make it look cool.
    // we'll display it like STRIPPED_FOR_PUBLIC_RELEASE, STRIPPED_FOR_PUBLIC_RELEASE, STRIPPED_FOR_PUBLIC_RELEASE, but packages twrhat arent accessible will be GREYED.
    // the area we're working with
    float image_base[4] = { control_base.x + 8, control_base.y + 14, control_base.x + 360 - 8, menu_end.y - 116 };

    if (m_package_selected == -1)
    {
      m_package_selected = api->HighestGroupAvailable();
    }

    if (auto new_section_selected = Section(vec2(engine::window->m_mouse_location.x, engine::window->m_mouse_location.y), image_base); new_section_selected != 0)
    {
      if (api->IsGroupAvailable(new_section_selected))
      {
        m_package_selected = new_section_selected;
      }
    }

    // left side
    draw_list->AddQuadFilled(ImVec2(image_base[0], image_base[1] + 1), // top left
                             ImVec2(image_base[0], image_base[3]), // bottom left
                             ImVec2(image_base[2] * 0.33f, image_base[3]), // bottom right
                             ImVec2(image_base[2] * 0.50f, image_base[1] + 1), // top right
                             m_package_selected == 1 ? IM_COL32(40, 40, 40, 0) : IM_COL32(40, 40, 40, 200));
    
    // middle
    draw_list->AddQuadFilled(ImVec2(image_base[2] * 0.50f, image_base[1] + 1), // top left
                             ImVec2(image_base[2] * 0.33f, image_base[3]), // bottom left
                             ImVec2(image_base[2] * 0.67f, image_base[3]), // bottom right
                             ImVec2(image_base[2] * 0.85f, image_base[1] + 1), // top right
                             m_package_selected == 2 ? IM_COL32(40, 40, 40, 0) : IM_COL32(40, 40, 40, 200));

    // right side
    draw_list->AddQuadFilled(ImVec2(image_base[2] * 0.85f, image_base[1] + 1), // top left
                             ImVec2(image_base[2] * 0.67f, image_base[3]), // bottom left
                             ImVec2(image_base[2], image_base[3]), // bottom right
                             ImVec2(image_base[2], image_base[1] + 1), // top right
                             m_package_selected == 3 ? IM_COL32(40, 40, 40, 0) : IM_COL32(40, 40, 40, 200));

  } else if (m_paint_stage == PAINT_STAGE::PRE_LOAD ||
             m_paint_stage == PAINT_STAGE::LOADING ||
             m_paint_stage == PAINT_STAGE::POST_STREAM ||
             m_paint_stage == PAINT_STAGE::STANDBY) {
    // STRIPPED_FOR_PUBLIC_RELEASE

    auto draw_spinning_arc = [](ImDrawList* draw_list, const ImVec2& center, float radius, float thickness, const ImVec4& color, float angle, float arc_length) -> void
    {
      const int num_segments = 20;
      draw_list->PathArcTo(center, radius, angle, angle + arc_length, num_segments);
      draw_list->PathStroke(ImGui::ColorConvertFloat4ToU32(color), false, thickness);
    };
                           
    constexpr float radius = 15.0f;
    constexpr float thickness = 6.0f;    
    constexpr ImVec4 color(45.0f / 255.0f, 122.0f / 255.0f, 68.0f / 255.0f, 1.0f);

    const ImVec2 text_size = ImGui::CalcTextSize(text.c_str());
    const ImVec2 point = ImVec2((radius * 2.0f) + menu_end.x / 2,
                                ((radius * 2.0f) + menu_end.y / 2) - 15.0f);

    const float angle = m_time * 6.0f;

    draw_list->AddCircle(point, radius, IM_COL32(19, 31, 46, 255), 20, thickness);
    draw_spinning_arc(draw_list, point, radius, thickness, color, angle, M_PI / 3.0f);
    draw_spinning_arc(draw_list, point, radius, thickness, color, angle + M_PI, M_PI / 3.0f);
    draw_list->AddText(ImVec2(point.x - text_size.x / 2, point.y + 30.0f),
                       IM_COL32(255, 255, 255, 255), text.c_str());
  }
}

void Window::CreateStageElements() {
  ImGui::SetWindowPos(
      ImVec2((m_window_size.x / 2) - 180, (m_window_size.y / 2) - 150));
  ImGui::SetWindowSize(ImVec2(360, 300));
  if (m_paint_stage == PAINT_STAGE::LOGIN) {
    // STRIPPED_FOR_PUBLIC_RELEASE

    for (size_t i = 0; i < 9; i++) ImGui::Spacing();

    if (ImGui::Button(skCrypt("LOG IN"), ImVec2(344, 40))) {
      nlohmann::json j;
#ifdef _DEBUG
      // m_username = "STRIPPED_FOR_PUBLIC_RELEASE";
      // m_password = "STRIPPED_FOR_PUBLIC_RELEASE";
#endif

      if (m_username.empty() || m_password.empty()) {
        util::nt->MessageBoxTimeoutA(
            NULL, skCryptS("Username or password is empty").c_str(), NULL,
            MB_TASKMODAL | MB_ICONERROR | MB_OK, 0, 5000);
      } else {
        j[skCryptS("username")] = m_username;
        j[skCryptS("password")] = m_password;
        j[skCryptS("fp")] = util::fingerprint->Dump();

        if (api.has_value() && api.value().SendRequest(j) && m_save_creds) {
          util::credentials->Save(m_username, m_password);
        }
      }
    }
  } else if (m_paint_stage == PAINT_STAGE::STREAM) {
    // STRIPPED_FOR_PUBLIC_RELEASE
  } else if (m_paint_stage == PAINT_STAGE::POST_STREAM) {
    static bool created = false;
    if (!created) {
      std::uniform_int_distribution<size_t> dist(100u, 2000u);
      util::threads->Create(util::THREAD_TYPE::DELAY,
                            1000u + dist(util::security->m_seed));
      created = true;
    }

    if (util::threads->m_finished.load()) m_paint_stage = PAINT_STAGE::PRE_LOAD;
  } else if (m_paint_stage == PAINT_STAGE::PRE_LOAD) {
    static bool created = false;
    if (!created) {
      util::threads->Create(util::THREAD_TYPE::SCAN_STRIPPED_FOR_PUBLIC_RELEASE);
      created = true;
    }

    if (util::threads->m_finished.load()) {
      static bool created2 = false;
      if (!created2) {
        util::threads->Create(util::THREAD_TYPE::DELAY);
        created2 = true;
      }

      if (util::threads->m_finished.load()) {
        // temporarily default to STRIPPED_FOR_PUBLIC_RELEASE
        nlohmann::json j;

        m_package_selected = std::clamp(m_package_selected, 1, api->HighestGroupAvailable());

        switch (m_package_selected)
        {
          case 1:
            j[skCryptS("branch")] = skCryptS("STRIPPED_FOR_PUBLIC_RELEASE");
            break;
          case 2:
            j[skCryptS("branch")] = skCryptS("STRIPPED_FOR_PUBLIC_RELEASE");
            break;
          case 3:
            j[skCryptS("branch")] = skCryptS("STRIPPED_FOR_PUBLIC_RELEASE");
            break;
        }

        static bool sent = false;

        if (api.has_value() && !sent) {
          // get our keys
          if (api.value().SendRequest(j)) {
            // get our module
            if (!api.value().SendRequest(j)) {
              if (util::process_handler->m_STRIPPED_FOR_PUBLIC_RELEASE_process !=
                  INVALID_HANDLE_VALUE) {
                // restore STRIPPED_FOR_PUBLIC_RELEASE state
                NtResumeProcess(util::process_handler->m_STRIPPED_FOR_PUBLIC_RELEASE_process);
              }

              m_finished_execution = true;
            }

            sent = true;
          }
        }
      }
    } else {
      // Process our flag, just throw a message box
      if (util::security->m_process_flag != util::SECURITY_FLAGS::NONE) {
        // throw an error
        util::nt->MessageBoxTimeoutA(
            NULL,
            skCryptS("STRIPPED_FOR_PUBLIC_RELEASE")
                .c_str(),
            NULL, MB_TASKMODAL | MB_ICONERROR | MB_OK, 0, 5000);
        // graceful exit
        m_finished_execution = true;
      }
    }
  } else if (m_paint_stage == PAINT_STAGE::LOADING) {
    if (api.has_value()) {
      //VM_DOLPHIN_RED_START
      static bool ran = false;
      if (!ran) {
        std::vector<char> buffer(api.value().GetExtrasPayload().begin(),
                                 api.value().GetExtrasPayload().end());

        if (buffer.empty()) {
          util::nt->MessageBoxTimeoutA(
              NULL,
              skCryptS("Something went wrong. This could be the "
                       "result of a temporary outage, maintenance, or a "
                       "network connectivity issue.")
                  .c_str(),
              NULL, MB_TASKMODAL | MB_ICONERROR | MB_OK, 0, 5000);
          m_finished_execution = true;
        } else {
          size_t i = 0;
          size_t xor_key = api.value().GetExtrasKey();

          for ([[maybe_unused]] size_t c : buffer) {
            buffer[i] = buffer[i] ^ xor_key;
            i++;
          }

          util::ModuleStreamInformation msi = {
              .buffer = buffer.data(),
              .buffer_size = buffer.size(),
              .process_id = util::process_handler->m_STRIPPED_FOR_PUBLIC_RELEASE_candidate};

          if (api.value().FillPipe(msi.pipe_name, msi.pipe_key,
                                   msi.pipe_message)) {
            util::injector.emplace(util::Injector{msi});
          } else {
            // exit gracefully, resume STRIPPED_FOR_PUBLIC_RELEASE
            if (util::process_handler->m_STRIPPED_FOR_PUBLIC_RELEASE_process != INVALID_HANDLE_VALUE) {
              // restore STRIPPED_FOR_PUBLIC_RELEASE state
              NtResumeProcess(util::process_handler->m_STRIPPED_FOR_PUBLIC_RELEASE_process);
            }

            m_finished_execution = true;
          }

          msi = {};
        }

        ran = true;
      }

      if (util::injector.value().m_finished &&
          util::threads->m_finished.load()) {
        m_finished_execution = true;
      }
      //VM_DOLPHIN_RED_END
    }
  }
}
}  // namespace engine