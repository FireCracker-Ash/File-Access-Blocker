#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <detours/detours.h>
#include <ntstatus.h>
#include "utils.h"

NTSTATUS NTAPI NtOpenFile_Hook(PHANDLE FileHandle, ACCESS_MASK DesiredAccess, PCOBJECT_ATTRIBUTES ObjectAttributes, PIO_STATUS_BLOCK IoStatusBlock, ULONG ShareAccess, ULONG OpenOptions) {

	if (isOn && ObjectAttributes && ObjectAttributes->ObjectName && ObjectAttributes->ObjectName->Buffer) {
		wstring path(ObjectAttributes->ObjectName->Buffer, ObjectAttributes->ObjectName->Length / sizeof(WCHAR));
		if (InList(path)) {
			//OutputDebugStringW((L"Open: " + path).c_str());
			return STATUS_ACCESS_DENIED;
		}
	}
	return NtOpenFile_Real(FileHandle, DesiredAccess, ObjectAttributes, IoStatusBlock, ShareAccess, OpenOptions);
}

NTSTATUS NTAPI NtCreateFile_Hook(PHANDLE FileHandle, ACCESS_MASK DesiredAccess, PCOBJECT_ATTRIBUTES ObjectAttributes, PIO_STATUS_BLOCK IoStatusBlock, PLARGE_INTEGER AllocationSize, ULONG FileAttributes, ULONG ShareAccess, ULONG CreateDisposition, ULONG CreateOptions, PVOID EaBuffer, ULONG EaLength) {
	if (isOn && ObjectAttributes && ObjectAttributes->ObjectName && ObjectAttributes->ObjectName->Buffer) {
		wstring path(ObjectAttributes->ObjectName->Buffer, ObjectAttributes->ObjectName->Length / sizeof(WCHAR));
		if (InList(path)) {
			//OutputDebugStringW((L"Create: " + path).c_str());
			return STATUS_ACCESS_DENIED;
		}
	}
	return NtCreateFile_Real(FileHandle, DesiredAccess, ObjectAttributes, IoStatusBlock, AllocationSize, FileAttributes, ShareAccess, CreateDisposition, CreateOptions, EaBuffer, EaLength);
}

void Wineventproc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD idEventThread, DWORD dwmsEventTime) {
	return;
}

EXTERN_C EXPORT void start() {
	Load();
	isOn = true;

	HWINEVENTHOOK hook = SetWinEventHook(EVENT_MIN, EVENT_MAX, hModule, Wineventproc, 0, 0, WINEVENT_INCONTEXT | WINEVENT_SKIPOWNPROCESS);
	HANDLE hEvent = CreateEventW(NULL, FALSE, FALSE, EVENT_NAME);
	if (!hEvent || !hook) {
		return;
	}

	HANDLE hThread = CreateThread(NULL, 0, MonitorFileChange, NULL, 0, NULL);

	WaitForSingleObject(hEvent, INFINITE);
	
	if (hThread) {
		TerminateThread(hThread, 0);
	}

	UnhookWinEvent(hook);
	CloseHandle(hEvent);
}

EXTERN_C EXPORT void stop() {
	isOn = false;

	HANDLE hEvent = OpenEventW(EVENT_MODIFY_STATE, FALSE, EVENT_NAME);
	if (hEvent) {
		SetEvent(hEvent);
		CloseHandle(hEvent);
	}
}

void AttachHook() {
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((PVOID*)&NtOpenFile_Real, NtOpenFile_Hook);
	DetourAttach((PVOID*)&NtCreateFile_Real, NtCreateFile_Hook);
	DetourTransactionCommit();
}

void DetachHook() {
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourDetach((PVOID*)&NtOpenFile_Real, NtOpenFile_Hook);
	DetourDetach((PVOID*)&NtCreateFile_Real, NtCreateFile_Hook);
	DetourTransactionCommit();
}