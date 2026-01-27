#include <clipboard.hpp>
#include <format>
#include <globals.hpp>
#include <gui.hpp>
#include <imgui/imgui.h>
#include <nexus/Nexus.h>
#include <settings.hpp>
#include <thread>
#include <windows.h>

#include <imgui/misc/cpp/imgui_stdlib.h>

std::string edit_short;
std::string edit_message;
int edit_map_id = 0;
bool edit_squad_broadcast;
std::pair to_edit = {-1, -1};
std::pair to_delete = {-1, -1};
bool confirm = false;
void render_messages()
{
    auto &s = settings_manager->get();
    auto &chat_messages = s.chat_messages;

    if (ImGui::CollapsingHeader("Messages##ChatShortsMessagesCollapse")) {
        for (auto it = chat_messages.begin(); it != chat_messages.end(); ++it) {
            if (it->second.empty()) {
                it = chat_messages.erase(it);
                settings_manager->set(&Settings::chat_messages, chat_messages);
                if (it == chat_messages.end()) {
                    break;
                }
            }
            std::string header;
            if (it->first == 0) {
                header = "Any Map";
            } else {
                header = std::to_string(it->first);
                if (!maps[it->first].empty()) {
                    header += " - " + maps[it->first];
                }
            }
            if (ImGui::CollapsingHeader(header.c_str())) {
                for (int i = 0; i < it->second.size(); ++i) {
                    auto &short_message = it->second[i].short_message;
                    auto &message = it->second[i].message;
                    auto &squad_broadcast = it->second[i].squad_broadcast;
                    ImGui::PushID(short_message.c_str());
                    if (to_edit == std::pair{it->first, i}) {
                        ImGui::InputText("New Short Name", &edit_short);
                        ImGui::InputTextMultiline("New Chat Message", &edit_message);
                        ImGui::Checkbox("Squad broadcast?", &edit_squad_broadcast);
                        ImGui::InputInt("New Map ID", &edit_map_id);
                        ImGui::SameLine();
                        if (ImGui::Button("Current Map##SetCurrentMapChatShortsMessage")) {
                            if (nexus_link->IsGameplay) {
                                edit_map_id = mumble_link->Context.MapID;
                            } else {
                                edit_map_id = 0;
                            }
                        }
                        if (ImGui::Button("Confirm")) {
                            confirm = true;
                            short_message = edit_short;
                            message = edit_message;
                            squad_broadcast = edit_squad_broadcast;
                            if (edit_map_id != it->first) {
                                it->second.erase(it->second.begin() + i);
                                chat_messages[edit_map_id].emplace_back(edit_short, edit_message);
                                --i;
                            }
                            to_edit = {-1, -1};
                            edit_map_id = 0;
                            edit_short.clear();
                            edit_message.clear();
                            settings_manager->set(&Settings::chat_messages, chat_messages);
                        }
                    } else {
                        ImGui::TextWrapped("%s:\n%s", short_message.c_str(), message.c_str());
                        if (ImGui::Button("Edit")) {
                            edit_short = short_message;
                            edit_message = message;
                            edit_map_id = it->first;
                            edit_squad_broadcast = squad_broadcast;
                            to_edit = {it->first, i};
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Delete")) {
                            to_delete = {it->first, i};
                        }
                    }
                    ImGui::PopID();
                }
            }
        }
    }
}

LPARAM get_l_param(std::uint32_t key, bool down)
{
    std::int64_t l_param = !down; // transition state
    l_param = l_param << 1;
    l_param += !down; // previous key state
    l_param = l_param << 1;
    l_param += 0; // context code
    l_param = l_param << 1;
    l_param = l_param << 4;
    l_param = l_param << 1;
    l_param = l_param << 8;
    l_param += MapVirtualKeyA(key, MAPVK_VK_TO_VSC);
    l_param = l_param << 16;
    l_param += 1;

    return l_param;
}

void broadcast_message(const std::wstring &message)
{
    std::thread(
        [message]()
        {
            using namespace std::chrono_literals;
            copy_to_clipboard(game_handle, message);
            if (mumble_link->Context.IsTextboxFocused) {
                SendMessage(game_handle, WM_KEYDOWN, VK_ESCAPE, get_l_param(VK_ESCAPE, true));
                SendMessage(game_handle, WM_KEYUP, VK_ESCAPE, get_l_param(VK_ESCAPE, false));
                std::this_thread::sleep_for(25ms);
            }

            SendMessage(game_handle, WM_KEYDOWN, VK_SHIFT, get_l_param(VK_SHIFT, true));
            SendMessage(game_handle, WM_KEYDOWN, VK_RETURN, get_l_param(VK_RETURN, true));
            SendMessage(game_handle, WM_KEYUP, VK_RETURN, get_l_param(VK_RETURN, false));
            SendMessage(game_handle, WM_KEYUP, VK_SHIFT, get_l_param(VK_SHIFT, false));

            INPUT select_text[1] = {};
            ZeroMemory(select_text, sizeof(select_text));
            select_text[0].type = INPUT_KEYBOARD;
            select_text[0].ki.wVk = VK_CONTROL;
            UINT u_sent = SendInput(ARRAYSIZE(select_text), select_text, sizeof(INPUT));
            assert(u_sent == ARRAYSIZE(select_text));
            std::this_thread::sleep_for(25ms);

            SendMessage(game_handle, WM_KEYDOWN, 'V', get_l_param('V', true));
            SendMessage(game_handle, WM_KEYUP, 'V', get_l_param('V', false));
            std::this_thread::sleep_for(25ms);
            ZeroMemory(select_text, sizeof(select_text));
            select_text[0].type = INPUT_KEYBOARD;
            select_text[0].ki.wVk = VK_CONTROL;
            select_text[0].ki.dwFlags = KEYEVENTF_KEYUP;
            u_sent = SendInput(ARRAYSIZE(select_text), select_text, sizeof(INPUT));
            assert(u_sent == ARRAYSIZE(select_text));

            std::this_thread::sleep_for(25ms);
            SendMessage(game_handle, WM_KEYDOWN, VK_RETURN, get_l_param(VK_RETURN, true));
            SendMessage(game_handle, WM_KEYUP, VK_RETURN, get_l_param(VK_RETURN, false));
            std::this_thread::sleep_for(25ms);
        })
        .detach();
}

void send_message(const std::wstring &message)
{
    std::thread(
        [message]()
        {
            using namespace std::chrono_literals;
            bool open_chat = false;
            copy_to_clipboard(game_handle, message);
            if (!mumble_link->Context.IsTextboxFocused) {
                SendMessage(game_handle, WM_KEYDOWN, VK_RETURN, get_l_param(VK_RETURN, true));
                SendMessage(game_handle, WM_KEYUP, VK_RETURN, get_l_param(VK_RETURN, false));
                std::this_thread::sleep_for(25ms);
            } else {
                open_chat = true;
            }
            INPUT select_text[1] = {};
            ZeroMemory(select_text, sizeof(select_text));
            select_text[0].type = INPUT_KEYBOARD;
            select_text[0].ki.wVk = VK_CONTROL;
            UINT u_sent = SendInput(ARRAYSIZE(select_text), select_text, sizeof(INPUT));
            assert(u_sent == ARRAYSIZE(select_text));

            SendMessage(game_handle, WM_KEYDOWN, 'V', get_l_param('V', true));
            SendMessage(game_handle, WM_KEYUP, 'V', get_l_param('V', false));
            std::this_thread::sleep_for(25ms);
            ZeroMemory(select_text, sizeof(select_text));
            select_text[0].type = INPUT_KEYBOARD;
            select_text[0].ki.wVk = VK_CONTROL;
            select_text[0].ki.dwFlags = KEYEVENTF_KEYUP;
            u_sent = SendInput(ARRAYSIZE(select_text), select_text, sizeof(INPUT));
            assert(u_sent == ARRAYSIZE(select_text));

            std::this_thread::sleep_for(25ms);
            SendMessage(game_handle, WM_KEYDOWN, VK_RETURN, get_l_param(VK_RETURN, true));
            SendMessage(game_handle, WM_KEYUP, VK_RETURN, get_l_param(VK_RETURN, false));
            std::this_thread::sleep_for(25ms);
            if (open_chat) {
                SendMessage(game_handle, WM_KEYDOWN, VK_RETURN, get_l_param(VK_RETURN, true));
                SendMessage(game_handle, WM_KEYUP, VK_RETURN, get_l_param(VK_RETURN, false));
                std::this_thread::sleep_for(25ms);
            }
        })
        .detach();
}

void keybind_handler(const char *identifier, bool release)
{
    std::string event_name = reinterpret_cast<const char *>(identifier);
    api->Log(ELogLevel_INFO, addon_name, event_name.c_str());
    auto &chat_messages = settings_manager->get(&Settings::chat_messages);
    auto global_messages = chat_messages[0];
    auto current_messages = chat_messages[(int)mumble_link->Context.MapID];

    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    for (auto &m : global_messages) {
        if (m.short_message == event_name) {
            if (m.squad_broadcast) {
                broadcast_message(converter.from_bytes(m.message));
            } else {
                send_message(converter.from_bytes(m.message));
            }
            return;
        }
    }
    for (auto &m : current_messages) {
        if (m.short_message == event_name) {
            if (m.squad_broadcast) {
                broadcast_message(converter.from_bytes(m.message));
            } else {
                send_message(converter.from_bytes(m.message));
            }
            return;
        }
    }
}

void event_handler(void *data)
{
    std::string event_name = reinterpret_cast<const char *>(data);
    api->Log(ELogLevel_DEBUG, addon_name, std::format("received event {}", event_name).c_str());
    auto &chat_messages = settings_manager->get(&Settings::chat_messages);

    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    for (auto &[map, messages] : chat_messages) {
        for (auto &message : messages) {
            api->Log(ELogLevel_DEBUG, addon_name,
                     std::format("checking against \"EV_CHAT_SHORTS_{}\"", message.short_message).c_str());
            if (message.short_message == std::string("EV_CHAT_SHORTS_" + event_name)) {
                if (message.squad_broadcast) {
                    broadcast_message(converter.from_bytes(message.message));
                } else {
                    send_message(converter.from_bytes(message.message));
                }
            }
        }
    }
}

std::string short_message;
char message[199];
bool squad_broadcast = false;
int map_id = 0;
static std::vector<std::pair<short, std::string>> visibility_options = {{
    {0, "Always"},
    {1, "During Gameplay"},
    {2, "Out of Combat"},
    {3, "In Combat"},
    {4, "Never"},
}};
void render_options()
{
    auto &chat_messages = settings_manager->get(&Settings::chat_messages);

    auto &s = settings_manager->get();
    if (ImGui::Checkbox("Lock Position##ChatShortsLockPosition", &s.lock_position)) {
        settings_manager->save();
    }
    if (ImGui::InputInt("Number of columns##ChatShortsNumberColumns", &s.number_columns)) {
        settings_manager->save();
    }
    if (ImGui::BeginCombo("Visibility##ChatShortsVisibility", visibility_options[s.visibility].second.c_str())) {
        for (const auto &[key, value] : visibility_options) {
            if (ImGui::Selectable(value.c_str(), key == s.visibility)) {
                s.visibility = key;
                settings_manager->save();
            }
        }
        ImGui::EndCombo();
    }
    if (ImGui::CollapsingHeader("Add Message##ChatShortsMessagesCollapse")) {
        ImGui::InputText("Short Name##ChatShortsShortNameInput", &short_message);
        ImGui::InputTextMultiline("Chat Message##ChatShortsAddMessageInput", message, 200);
        ImGui::Checkbox("Squad Broadcast?##ChatShortsSquadBrodcast", &squad_broadcast);
        ImGui::InputInt("Map ID##ChatShortsMapID", &map_id);
        ImGui::SameLine();
        if (ImGui::Button("Current Map##SetCurrentMapChatShortsMessage")) {
            if (nexus_link->IsGameplay) {
                map_id = mumble_link->Context.MapID;
            } else {
                map_id = 0;
            }
        }
        if (ImGui::Button("Add Message##AddChatShortsMessage")) {
            chat_messages[map_id].emplace_back(short_message, message, squad_broadcast);
            // TODO: subscribe to event
            short_message.clear();
            memset(message, 0, 199);
            squad_broadcast = false;
            map_id = 0;
            settings_manager->set(&Settings::chat_messages, chat_messages);
        }
    }
    render_messages();
    if (to_delete != std::pair{-1, -1}) {
        chat_messages[to_delete.first].erase(chat_messages[to_delete.first].begin() + to_delete.second);
        settings_manager->set(&Settings::chat_messages, chat_messages);
        to_delete = {-1, -1};
    }
}
