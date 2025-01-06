#include <core.hpp>
#include <Themida/ThemidaSDK.h>

#include "nt_wrapper.hpp"

namespace util
{
  NtWrapper::NtWrapper()
  {
    //VM_DOLPHIN_RED_START
    MessageBoxTimeoutA = reinterpret_cast<fnMessageBoxTimeoutA>(GetProcAddress( GetModuleHandleA( skCrypt( "user32.dll" ) ), skCrypt( "MessageBoxTimeoutA" ) ));

    const auto ntdll = GetModuleHandleA( skCrypt( "ntdll.dll" ) );

    NtSetSystemInformation = reinterpret_cast<fnNtSetSystemInformation>(GetProcAddress( ntdll, skCrypt( "NtSetSystemInformation" ) ));
    NtQuerySystemInformation = reinterpret_cast<fnNtQuerySystemInformation>(GetProcAddress( ntdll, skCrypt( "NtQuerySystemInformation" ) ));
    NtAllocateVirtualMemory = reinterpret_cast<fnNtAllocateVirtualMemory>(GetProcAddress( ntdll, skCrypt( "NtAllocateVirtualMemory" ) ));
    NtWriteVirtualMemory = reinterpret_cast<fnNtWriteVirtualMemory>(GetProcAddress( ntdll, skCrypt( "NtWriteVirtualMemory" ) ));
    NtQueryInformationProcess = reinterpret_cast<fnNtQueryInformationProcess>(GetProcAddress( ntdll, skCrypt( "NtQueryInformationProcess" ) ));
    NtCreateThreadEx = reinterpret_cast<fnNtCreateThreadEx>(GetProcAddress( ntdll, skCrypt( "NtCreateThreadEx" ) ));
    LdrSetDllManifestProber = reinterpret_cast<fnLdrSetDllManifestProber>(GetProcAddress( ntdll, skCrypt( "LdrSetDllManifestProber" ) ));
    NtQueryDirectoryObject = reinterpret_cast<fnNtQueryDirectoryObject>(GetProcAddress( ntdll, skCrypt( "NtQueryDirectoryObject" ) ));
    NtOpenDirectoryObject = reinterpret_cast<fnNtOpenDirectoryObject>(GetProcAddress( ntdll, skCrypt( "NtOpenDirectoryObject" ) ));
    //VM_DOLPHIN_RED_END
  }
}