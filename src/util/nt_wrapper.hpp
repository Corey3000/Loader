#pragma once

#include "nt_defs.hpp"

namespace util
{
  struct NtWrapper
  {
    NtWrapper();

    fnMessageBoxTimeoutA MessageBoxTimeoutA;
    fnNtSetSystemInformation NtSetSystemInformation;
    fnNtQuerySystemInformation NtQuerySystemInformation;
    fnNtAllocateVirtualMemory NtAllocateVirtualMemory;
    fnNtWriteVirtualMemory NtWriteVirtualMemory;
    fnNtQueryInformationProcess NtQueryInformationProcess;
    fnNtCreateThreadEx NtCreateThreadEx;
    fnLdrSetDllManifestProber LdrSetDllManifestProber;
    fnNtQueryDirectoryObject NtQueryDirectoryObject;
    fnNtOpenDirectoryObject NtOpenDirectoryObject;
  };

  inline auto nt = std::make_unique<NtWrapper>();
}