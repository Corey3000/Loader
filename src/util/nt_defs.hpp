#pragma once

typedef int(WINAPI* fnMessageBoxTimeoutA)(IN HWND hWnd,
  IN LPCSTR lpText, IN LPCSTR lpCaption,
  IN UINT uType, IN WORD wLanguageId, IN DWORD dwMilliseconds);

typedef BOOL( WINAPI* pDllMain ) (HMODULE, DWORD, PVOID);
typedef NTSTATUS( WINAPI* fnNtSetSystemInformation )(INT, PVOID, ULONG);
typedef NTSTATUS( WINAPI* fnNtQuerySystemInformation )(INT, PVOID, ULONG, PULONG);
typedef NTSTATUS( WINAPI* fnNtAllocateVirtualMemory )(HANDLE, PVOID, ULONG_PTR, PSIZE_T, ULONG, ULONG);
typedef NTSTATUS( WINAPI* fnNtWriteVirtualMemory )(HANDLE, PVOID, PVOID, ULONG, PULONG);
typedef NTSTATUS(WINAPI* fnNtQueryInformationProcess)(HANDLE, ULONG, PVOID,
                                                      ULONG, PULONG);
typedef NTSTATUS( NTAPI* fnNtCreateThreadEx )( OUT PHANDLE hThread, IN ACCESS_MASK DesiredAccess, IN PVOID ObjectAttributes, IN HANDLE ProcessHandle, IN PVOID lpStartAddress, IN PVOID lpParameter, IN ULONG Flags, IN SIZE_T StackZeroBits, IN SIZE_T SizeOfStackCommit, IN SIZE_T SizeOfStackReserve, OUT PVOID lpBytesBuffer);

typedef NTSTATUS( NTAPI* PLDR_MANIFEST_PROBER_ROUTINE )
(
  IN HMODULE DllBase,
  IN PCWSTR FullDllPath,
  OUT PHANDLE ActivationContext
  );

typedef NTSTATUS( NTAPI* PLDR_ACTX_LANGUAGE_ROURINE )
(
  IN HANDLE Unk,
  IN USHORT LangID,
  OUT PHANDLE ActivationContext
  );

typedef void(NTAPI* PLDR_RELEASE_ACT_ROUTINE)
(
  IN HANDLE ActivationContext
  );

typedef VOID( NTAPI* fnLdrSetDllManifestProber )
(
  IN PLDR_MANIFEST_PROBER_ROUTINE ManifestProberRoutine,
  IN PLDR_ACTX_LANGUAGE_ROURINE CreateActCtxLanguageRoutine,
  IN PLDR_RELEASE_ACT_ROUTINE ReleaseActCtxRoutine
  );

typedef NTSTATUS( WINAPI* fnNtQueryDirectoryObject )(
  _In_      HANDLE  DirectoryHandle,
  _Out_opt_ PVOID   Buffer,
  _In_      ULONG   Length,
  _In_      BOOLEAN ReturnSingleEntry,
  _In_      BOOLEAN RestartScan,
  _Inout_   PULONG  Context,
  _Out_opt_ PULONG  ReturnLength
  );

typedef NTSTATUS( WINAPI* fnNtOpenDirectoryObject )(
  _Out_ PHANDLE            DirectoryHandle,
  _In_  ACCESS_MASK        DesiredAccess,
  _In_  PVOID ObjectAttributes
  );
