#ifndef NEXUS_CHAT_SHORTS_CLIPBOARD_HPP
#define NEXUS_CHAT_SHORTS_CLIPBOARD_HPP

#include <string>
#include <windows.h>

bool copy_to_clipboard(HWND game_handle, const std::wstring &message);

#endif // !NEXUS_CHAT_SHORTS_CLIPBOARD_HPP
