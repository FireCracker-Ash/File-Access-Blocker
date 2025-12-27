#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <winternl.h>

#define EXPORT __declspec(dllexport)

#define EVENT_NAME L"FileAccessBlocker_EVENT"

typedef const OBJECT_ATTRIBUTES* PCOBJECT_ATTRIBUTES;

typedef NTSTATUS(NTAPI* pNtOpenFile)(
    PHANDLE FileHandle,
    ACCESS_MASK DesiredAccess,
    PCOBJECT_ATTRIBUTES ObjectAttributes,
    PIO_STATUS_BLOCK IoStatusBlock,
    ULONG ShareAccess,
    ULONG OpenOptions
);

typedef NTSTATUS(NTAPI* pNtCreateFile)(
    PHANDLE FileHandle,
    ACCESS_MASK DesiredAccess,
    PCOBJECT_ATTRIBUTES ObjectAttributes,
    PIO_STATUS_BLOCK IoStatusBlock,
    PLARGE_INTEGER AllocationSize,
    ULONG FileAttributes,
    ULONG ShareAccess,
    ULONG CreateDisposition,
    ULONG CreateOptions,
    PVOID EaBuffer,
    ULONG EaLength
);

HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");

pNtOpenFile NtOpenFile_Real = (pNtOpenFile)GetProcAddress(ntdll, "NtOpenFile");
pNtCreateFile NtCreateFile_Real = (pNtCreateFile)GetProcAddress(ntdll, "NtCreateFile");
