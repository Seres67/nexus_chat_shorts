#include <clipboard.hpp>
#include <cwchar>
#include <format>
#include <globals.hpp>
#include <nexus/Nexus.h>
#include <windows.h>

bool copy_to_clipboard(HWND game_handle, const std::wstring &message)
{
    if (!OpenClipboard(game_handle)) {
        return false;
    }
    EmptyClipboard();
    api->Log(ELogLevel_DEBUG, addon_name, std::format("allocating {} bytes", message.length()).c_str());

    HGLOBAL h_mem = GlobalAlloc(GMEM_MOVEABLE, sizeof(WCHAR) * (message.length() + 1));
    auto *w_char_arr = (WCHAR *)GlobalLock(h_mem);
    wcscpy(w_char_arr, message.c_str());
    SetClipboardData(CF_UNICODETEXT, h_mem);
    CloseClipboard();
    return true;
}
