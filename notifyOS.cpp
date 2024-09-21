#include "pch.h"

#include "Windows.h"
#include "winuser.h"
#include "processthreadsapi.h"

#include "notifyOS.h"

// Find main window functions is from https://stackoverflow.com/a/21767578

struct handle_data {
    DWORD process_id;
    HWND window_handle;
};

BOOL is_main_window(HWND handle)
{
    return GetWindow(handle, GW_OWNER) == (HWND)0 && IsWindowVisible(handle);
}

BOOL CALLBACK enum_windows_callback(HWND handle, LPARAM lParam)
{
    handle_data& data = *(handle_data*)lParam;
    DWORD process_id = 0;
    GetWindowThreadProcessId(handle, &process_id);
    if (data.process_id != process_id || !is_main_window(handle))
        return TRUE;
    data.window_handle = handle;
    return FALSE;
}

HWND find_main_window(DWORD process_id)
{
    handle_data data;
    data.process_id = process_id;
    data.window_handle = 0;
    EnumWindows(enum_windows_callback, (LPARAM)&data);
    return data.window_handle;
}

void flashNotifyOS(UINT count) {
    DWORD pid = GetCurrentProcessId();
    HWND window = find_main_window(pid);

    FLASHWINFO param;
    param.cbSize = sizeof(FLASHWINFO);
    param.hwnd = window;
    param.dwFlags = FLASHW_ALL | FLASHW_TIMERNOFG;
    param.uCount = count;
    param.dwTimeout = 0;

    FlashWindowEx(&param);
}

