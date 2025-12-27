#pragma once
#include "define.h"
#include <Shlwapi.h>
#include <vector>
#include <string>
#include <fstream>

#pragma comment(lib,"shlwapi.lib")

#define CONFIG_NAME L"blocklist.txt"

using std::wstring;
using std::string;
using std::vector;
using std::ifstream;
using std::getline;

#pragma data_seg(".blocker")
WCHAR list[1024][MAX_PATH] = {};
bool isOn = false;
#pragma data_seg()
#pragma comment(linker,"/section:.blocker,rws")

HMODULE hModule;
WCHAR listPath[MAX_PATH];

wstring s2ws(const string& s) {
    wstring ws;
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, NULL, 0);
    ws.resize(len - 1);
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &ws[0], len);
    return ws;
}

bool InList(const wstring& path) {
    for (int i = 0; i < 1024;i++) {
        if (list[i][0] == L'\0') {
            break;
        }
        if (!_wcsicmp(list[i], path.c_str())) {
            return true;
        }
    }
    return false;
}

void Load() {
    GetModuleFileNameW(hModule, listPath, MAX_PATH);
    PathRemoveFileSpecW(listPath);
    PathCombineW(listPath, listPath, CONFIG_NAME);

    ZeroMemory(list, sizeof(list));
   
    ifstream file(listPath, std::ios::binary);

    string line;
    for (int i = 0; i < 1024 && getline(file, line); i++) {
        if (line.back() == '\r') {
            line.pop_back();
        }
        wstring fullPath = L"\\??\\" + s2ws(line);

        wcscpy_s(list[i], MAX_PATH, fullPath.c_str());
    }
}

DWORD WINAPI MonitorFileChange(LPVOID lpParameter) {
    WCHAR dir[MAX_PATH];
    wcscpy_s(dir, MAX_PATH, listPath);
    PathRemoveFileSpecW(dir);

    HANDLE hDir = CreateFileW(dir, FILE_LIST_DIRECTORY, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
    if (hDir == INVALID_HANDLE_VALUE) {
        MessageBoxW(NULL, L"INVALID_HANDLE_VALUE", L"", MB_OK);
        return 1;
    }

    BYTE buffer[1024];
    DWORD bytes;

    while (true) {
        if (ReadDirectoryChangesW(hDir, buffer, sizeof(buffer), FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE, &bytes, NULL, NULL)) {
            PFILE_NOTIFY_INFORMATION info = (PFILE_NOTIFY_INFORMATION)buffer;
            
            while(true){
                wstring changeFile(info->FileName, info->FileNameLength / sizeof(WCHAR));

                if (!_wcsicmp(changeFile.c_str(), CONFIG_NAME)) {
                    isOn = false;
                    Load();
                    isOn = true;
                }
                if (!info->NextEntryOffset) {
                    break;
                }
                info = (PFILE_NOTIFY_INFORMATION)((PBYTE)info + info->NextEntryOffset);
            }
        }
    }

    CloseHandle(hDir);
    return 0;
}