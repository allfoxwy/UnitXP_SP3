#include "pch.h"

#include <string>

using namespace std;

#include "Windows.h"
#include "winuser.h"
#include "processthreadsapi.h"
#include "Mmsystem.h"

#include "utf8_to_utf16.h"
#include "notifyOS.h"

// Find main window functions is from https://stackoverflow.com/a/21767578

struct handle_data {
    DWORD process_id;
    HWND window_handle;
};

static BOOL is_main_window(HWND handle)
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

static HWND find_main_window(DWORD process_id)
{
    handle_data data = {};
    data.process_id = process_id;
    data.window_handle = 0;
    EnumWindows(enum_windows_callback, (LPARAM)&data);
    return data.window_handle;
}

static bool gameInForeground() {
    DWORD pid = GetCurrentProcessId();
    HWND window = find_main_window(pid);

    return GetForegroundWindow() == window;
}

void flashTaskbarIcon() {
    if (gameInForeground() == true) {
        return;
    }

    DWORD pid = GetCurrentProcessId();
    HWND window = find_main_window(pid);

    FLASHWINFO param = {};
    param.cbSize = sizeof(FLASHWINFO);
    param.hwnd = window;
    param.dwFlags = FLASHW_TRAY | FLASHW_TIMERNOFG;
    param.uCount = 0u;
    param.dwTimeout = 0u;

    FlashWindowEx(&param);
}

// Play a system sound
bool playSystemSound(const string soundName) {
    if (gameInForeground() == true) {
        return false;
    }

    if (soundName == u8"SystemAsterisk"
        || soundName == u8"SystemDefault"
        || soundName == u8"SystemExclamation"
        || soundName == u8"SystemExit"
        || soundName == u8"SystemHand"
        || soundName == u8"SystemQuestion"
        || soundName == u8"SystemStart"
        || soundName == u8"SystemWelcome") {

        return PlaySoundW(utf8_to_utf16(soundName).data(), NULL, SND_ALIAS | SND_ASYNC | SND_SENTRY);
    }
    return false;
}
