#include "globals.hpp"
#include "nexus/Nexus.h"
#include <clipboard.hpp>
#include <optional>

bool copy_to_clipboard(HWND game_handle, const std::string &message)
{
    if (!OpenClipboard(game_handle))
        return false;
    EmptyClipboard();
    HGLOBAL h_mem = GlobalAlloc(GMEM_MOVEABLE, message.size() + 1);
    memcpy(GlobalLock(h_mem), message.c_str(), message.size() + 1);
    SetClipboardData(CF_TEXT, h_mem);
    CloseClipboard();
    return true;
}