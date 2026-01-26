#ifndef GUI_HPP
#define GUI_HPP

#include <string>

void render_messages();
void render_options();

void broadcast_message(const std::wstring &message);
void send_message(const std::wstring &message);
void keybind_handler(const char *identifier, bool release);

#endif // GUI_HPP
