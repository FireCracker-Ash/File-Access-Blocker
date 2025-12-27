#include "hook.h"

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hinstDLL);
        hModule = hinstDLL;
        AttachHook();
        break;
    case DLL_PROCESS_DETACH:
        DetachHook();
        break;
    default:
        break;
    }
    return TRUE;
}