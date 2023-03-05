#include <WinSock2.h>
#include <Windows.h>
#include "minhook/minhook.h"
#include <iostream>
#pragma comment(lib, "ws2_32.lib")

typedef int(WINAPI* select_t)(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, struct timeval* timeout);
select_t orig_select;


typedef int(WINAPI* WSASend_t)(
    SOCKET                             s,
    LPWSABUF                           lpBuffers,
    DWORD                              dwBufferCount,
    LPDWORD                            lpNumberOfBytesSent,
    DWORD                              dwFlags,
    LPWSAOVERLAPPED                    lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    );
WSASend_t orig_wsasend;



int WINAPI wsasend_hook(
    SOCKET                             s,
    LPWSABUF                           lpBuffers,
    DWORD                              dwBufferCount,
    LPDWORD                            lpNumberOfBytesSent,
    DWORD                              dwFlags,
    LPWSAOVERLAPPED                    lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
) {
    if (GetAsyncKeyState(88)) {
        return 0;
    }

    return orig_wsasend(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpOverlapped, lpCompletionRoutine);
}



int select_hook(
    int             nfds,
    fd_set*         readfds,
    fd_set*         writefds,
    fd_set*         exceptfds,
    struct timeval* timeout
) {
    if (GetAsyncKeyState(90)) {
        readfds->fd_count = 0;
        timeout->tv_usec = 1;
        timeout->tv_sec = 0;
    }
 
    return orig_select(nfds, readfds, writefds, exceptfds, timeout);
}

const char* Hook()
{
    if (MH_Initialize() != MH_OK)
        return "MH_Initialize failed";

    if (MH_CreateHookApi(L"Ws2_32.dll", "select", select_hook, (LPVOID*)&orig_select) != MH_OK)
        return "select hook failed";

    if (MH_CreateHookApi(L"Ws2_32.dll", "WSASend", wsasend_hook, (LPVOID*)&orig_wsasend) != MH_OK)
        return "WSASend hook failed";

    if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK)
        return "MH_EnableHook failed";

    return nullptr;
}

DWORD WINAPI shit(LPVOID lpParam)
{
    const auto hooks = Hook();

    if (hooks)
    {
        MessageBoxA(NULL, hooks, ":(", MB_ICONERROR);
    }
    return 0;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     ) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        CreateThread(NULL, 0, &shit, NULL, 0, NULL);
    }
    return TRUE;
}

