#include <Windows.h>

typedef enum _SHUTDOWN_ACTION
{
    ShutdownNoReboot,
    ShutdownReboot,
    ShutdownPowerOff
} SHUTDOWN_ACTION, *PSHUTDOWN_ACTION;

typedef LONG (__stdcall *PFN_NT_SHUTDOWN_SYSTEM)(DWORD SHUTDOWN_ACTION);

int DisplayError(LPWSTR);
LPWSTR GetFormattedMessage(DWORD, LPWSTR, DWORD, ...);

int CALLBACK WinMain(HINSTANCE, HINSTANCE, LPSTR lpCmdLine, int)
{
    if (lstrcmpA(lpCmdLine, "/reboot"))
    {
        MessageBox(
            NULL,
            L"Usage: ForceReboot /reboot",
            L"ForceReboot",
            MB_ICONINFORMATION | MB_OK);
        return 1;
    }

    HINSTANCE hNtDll = GetModuleHandle(L"ntdll.dll");

    if (hNtDll == NULL)
        return DisplayError(L"GetModuleHandle()");

    PFN_NT_SHUTDOWN_SYSTEM pNtShutdownSystem =
        (PFN_NT_SHUTDOWN_SYSTEM)GetProcAddress(
            hNtDll,
            "NtShutdownSystem");

    if (pNtShutdownSystem == NULL)
        return DisplayError(L"GetProcAddress()");

    HANDLE hToken;

    if (!OpenProcessToken(
        GetCurrentProcess(),
        TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES,
        &hToken))
    {
        return DisplayError(L"OpenProcessToken()");
    }

    TOKEN_PRIVILEGES tkp;

    if (!LookupPrivilegeValue(
        NULL,
        SE_SHUTDOWN_NAME,
        &tkp.Privileges[0].Luid))
    {
        return DisplayError(L"LookupPrivilegeValue()");
    }

    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (!AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, NULL, 0))
        return DisplayError(L"LookupPrivilegeValue()");

    CloseHandle(hToken);

    LONG status = pNtShutdownSystem(ShutdownReboot);

    if (status)
    {
        MessageBox(
            NULL,
            GetFormattedMessage(
                FORMAT_MESSAGE_FROM_STRING,
                L"NtShutdownSystem() returns 0x%1!x!",
                0,
                status),
            L"Error",
            MB_ICONERROR | MB_OK);
    }

    return 0;
}

int DisplayError(LPWSTR pMessage)
{
    LPWSTR pDelim = L":\r\n\r\n";

    LPWSTR pErrorMsg = GetFormattedMessage(
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        GetLastError());

    LPWSTR pText = (LPWSTR)LocalAlloc(
        LPTR, (
        lstrlen(pMessage) + lstrlen(pDelim) +
            lstrlen(pErrorMsg) + 1) * sizeof(TCHAR));

    lstrcpy(pText, pMessage);
    lstrcat(pText, pDelim);
    lstrcat(pText, pErrorMsg);

    MessageBox(NULL, pText, L"Error", MB_ICONERROR | MB_OK);

    LocalFree(pText);
    LocalFree(pErrorMsg);

    return 1;
}

LPWSTR GetFormattedMessage(DWORD dwFlags, LPWSTR pMsg, DWORD dwMsgId, ...)
{
    LPWSTR pBuffer = NULL;

    va_list args = NULL;
    va_start(args, dwMsgId);

    FormatMessage(
        dwFlags | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        pMsg,
        dwMsgId,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
        (LPWSTR)&pBuffer,
        0,
        &args);

    va_end(args);

    return pBuffer;
}
