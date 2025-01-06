#include <core.hpp>
#include <engine.hpp>

#include <Themida/ThemidaSDK.h>

namespace util {
std::vector<DWORD> EnumByName(const wchar_t* Name) {
  std::vector<DWORD> Procs;
  HANDLE Toolhelp = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

  PROCESSENTRY32 ProcessEntry;
  ProcessEntry.dwSize = sizeof(ProcessEntry);

  Process32First(Toolhelp, &ProcessEntry);

  do {
    if (!_wcsicmp(ProcessEntry.szExeFile, Name)) {
      Procs.push_back(ProcessEntry.th32ProcessID);
    }
  } while (Process32Next(Toolhelp, &ProcessEntry));

  return Procs;
}

bool ProcessHandler::IsSTRIPPED_FOR_PUBLIC_RELEASEReady() {
  // get STRIPPED_FOR_PUBLIC_RELEASE first
  // VM_DOLPHIN_RED_START

  const auto pids = EnumByName(L"STRIPPED_FOR_PUBLIC_RELEASE.exe");

  if (pids.empty()) {
    return false;
  }

  const std::array<std::wstring, 12> modules_needed = {
      L"STRIPPED_FOR_PUBLIC_RELEASE.dll",           L"STRIPPED_FOR_PUBLIC_RELEASE.dll",  L"STRIPPED_FOR_PUBLIC_RELEASE.dll",
      L"STRIPPED_FOR_PUBLIC_RELEASE.dll",   L"STRIPPED_FOR_PUBLIC_RELEASE.dll",   L"STRIPPED_FOR_PUBLIC_RELEASE.dll",
      L"STRIPPED_FOR_PUBLIC_RELEASE.dll", L"STRIPPED_FOR_PUBLIC_RELEASE.dll", L"STRIPPED_FOR_PUBLIC_RELEASE.dll",
      L"STRIPPED_FOR_PUBLIC_RELEASE.dll",     L"STRIPPED_FOR_PUBLIC_RELEASE.dll",   L"STRIPPED_FOR_PUBLIC_RELEASE.dll"};

  for (auto& candidate : pids) {
    if (m_STRIPPED_FOR_PUBLIC_RELEASE_candidate && m_STRIPPED_FOR_PUBLIC_RELEASE_process != INVALID_HANDLE_VALUE) {
      break;
    }

    HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                                 FALSE, candidate);

    if (process == NULL) {
      continue;
    }

    std::wstring STRIPPED_FOR_PUBLIC_RELEASE_path;
    TCHAR direct_path[MAX_PATH];

    if (GetModuleFileNameEx(process, NULL, direct_path, MAX_PATH) != 0) {
      STRIPPED_FOR_PUBLIC_RELEASE_path = direct_path;
      std::transform(STRIPPED_FOR_PUBLIC_RELEASE_path.begin(), STRIPPED_FOR_PUBLIC_RELEASE_path.end(), STRIPPED_FOR_PUBLIC_RELEASE_path.begin(),
                     [](unsigned char c) { return std::tolower(c); });
    }

    if (STRIPPED_FOR_PUBLIC_RELEASE_path.empty() || !util::security->IsSigned(STRIPPED_FOR_PUBLIC_RELEASE_path.c_str())) {
      CloseHandle(process);
      continue;
    }

    HMODULE modules[1024];
    DWORD needed{};
    int modules_loaded = 0;

    if (EnumProcessModulesEx(process, modules, sizeof(modules), &needed,
                             LIST_MODULES_32BIT)) {
      for (size_t i = 0; i < (needed / sizeof(HMODULE)); i++) {
        TCHAR mod_name[MAX_PATH];
        if (GetModuleFileNameEx(process, modules[i], mod_name,
                                sizeof(mod_name) / sizeof(TCHAR))) {
          auto w_modules = std::wstring(mod_name);
          std::transform(w_modules.begin(), w_modules.end(), w_modules.begin(),
                         [](unsigned char c) { return std::tolower(c); });

          for (auto& m : modules_needed) {
            if (w_modules.find(m) != std::wstring::npos) modules_loaded++;
          }
        }
      }
    }

    if (modules_loaded >= modules_needed.size()) {
      m_STRIPPED_FOR_PUBLIC_RELEASE_candidate = candidate;

      CLIENT_ID Cid;
      OBJECT_ATTRIBUTES ObjectAttributes;
      HANDLE ProcessHandle;

      Cid.UniqueProcess = (HANDLE)candidate;
      Cid.UniqueThread = NULL;

      InitializeObjectAttributes(&ObjectAttributes, NULL, NULL, NULL, NULL);

      NTSTATUS Status = NtOpenProcess(&ProcessHandle, PROCESS_ALL_ACCESS,
                                      &ObjectAttributes, &Cid);

      if (NT_SUCCESS(Status)) {
        m_STRIPPED_FOR_PUBLIC_RELEASE_process = ProcessHandle;
      }

      // its possible that the process may have already been suspended, resume
      // it to resolve any sort of double suspension anomalies
      NtResumeProcess(m_STRIPPED_FOR_PUBLIC_RELEASE_process);
      NtSuspendProcess(m_STRIPPED_FOR_PUBLIC_RELEASE_process);
    }

    CloseHandle(process);
  }

  // VM_DOLPHIN_RED_END

  return m_STRIPPED_FOR_PUBLIC_RELEASE_candidate != 0;
}
}  // namespace util