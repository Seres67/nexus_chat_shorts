#ifndef NEXUS_CHAT_SHORTS_CLIPBOARD_HPP
#define NEXUS_CHAT_SHORTS_CLIPBOARD_HPP

#include <optional>
#include <string>
#include <windows.h>

bool copy_to_clipboard(HWND game_handle, const std::string &message);
std::optional<std::string> save_and_copy_to_clipboard(HWND game_handle, const std::string &message);

#endif // !NEXUS_CHAT_SHORTS_CLIPBOARD_HPP
