#include <globals.hpp>
#include <gui.hpp>
#include <imgui/imgui.h>
#include <settings.hpp>

#include <imgui/misc/cpp/imgui_stdlib.h>

std::string edit_short;
std::string edit_message;
int edit_map_id = 0;
std::pair to_edit = {-1, -1};
std::pair to_delete = {-1, -1};
bool confirm = false;
void render_messages()
{
    if (ImGui::CollapsingHeader("Messages##ChatShortsMessagesCollapse")) {
        for (auto it = chat_messages.begin(); it != chat_messages.end(); ++it) {
            if (it->second.empty()) {
                it = chat_messages.erase(it);
                Settings::json_settings[Settings::CHAT_MESSAGES] = chat_messages;
                Settings::save(Settings::settings_path);
                if (it == chat_messages.end())
                    break;
            }
            std::string header;
            if (it->first == 0)
                header = "Any Map";
            else {
                header = std::to_string(it->first);
                if (!maps[it->first].empty())
                    header += " - " + maps[it->first];
            }
            if (ImGui::CollapsingHeader(header.c_str())) {
                for (int i = 0; i < it->second.size(); ++i) {
                    auto &short_message = it->second[i].short_message;
                    auto &message = it->second[i].message;
                    ImGui::PushID(short_message.c_str());
                    if (to_edit == std::pair{it->first, i}) {
                        ImGui::InputText("New Short Name", &edit_short);
                        ImGui::InputTextMultiline("New Chat Message", &edit_message);
                        ImGui::InputInt("New Map ID", &edit_map_id);
                        ImGui::SameLine();
                        if (ImGui::Button("Current Map##SetCurrentMapChatShortsMessage")) {
                            if (nexus_link->IsGameplay)
                                edit_map_id = mumble_link->Context.MapID;
                            else
                                edit_map_id = 0;
                        }
                        if (ImGui::Button("Confirm")) {
                            confirm = true;
                            short_message = edit_short;
                            message = edit_message;
                            if (edit_map_id != it->first) {
                                it->second.erase(it->second.begin() + i);
                                chat_messages[edit_map_id].emplace_back(edit_short, edit_message);
                                --i;
                            }
                            to_edit = {-1, -1};
                            edit_map_id = 0;
                            edit_short.clear();
                            edit_message.clear();
                            Settings::json_settings[Settings::CHAT_MESSAGES] = chat_messages;
                            Settings::save(Settings::settings_path);
                        }
                    } else {
                        ImGui::TextWrapped("%s:\n%s", short_message.c_str(), message.c_str());
                        if (ImGui::Button("Edit")) {
                            edit_short = short_message;
                            edit_message = message;
                            edit_map_id = it->first;
                            to_edit = {it->first, i};
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Delete"))
                            to_delete = {it->first, i};
                    }
                    ImGui::PopID();
                }
            }
        }
    }
}

std::string short_message;
char message[199];
int map_id = 0;
static std::vector<std::pair<int, std::string>> visibility_options = {{
    {0, "Always"},
    {1, "During Gameplay"},
    {2, "Out of Combat"},
    {3, "In Combat"},
    {4, "Never"},
}};
void render_options()
{
    if (ImGui::Checkbox("Lock Position##ChatShortsLockPosition", &Settings::lock_position)) {
        Settings::json_settings[Settings::LOCK_POSITION] = Settings::lock_position;
        Settings::save(Settings::settings_path);
    }
    if (ImGui::BeginCombo("Visibility##ChatShortsVisibility",
                          visibility_options[Settings::visibility].second.c_str())) {
        for (const auto &[key, value] : visibility_options) {
            if (ImGui::Selectable(value.c_str(), key == Settings::visibility)) {
                Settings::visibility = key;
                Settings::json_settings[Settings::VISIBILITY] = Settings::visibility;
                Settings::save(Settings::settings_path);
            }
        }
        ImGui::EndCombo();
    }
    if (ImGui::CollapsingHeader("Add Message##ChatShortsMessagesCollapse")) {
        ImGui::InputText("Short Name##ChatShortsShortNameInput", &short_message);
        ImGui::InputTextMultiline("Chat Message##ChatShortsAddMessageInput", message, 200);
        ImGui::InputInt("Map ID##ChatShortsMapID", &map_id);
        ImGui::SameLine();
        if (ImGui::Button("Current Map##SetCurrentMapChatShortsMessage")) {
            if (nexus_link->IsGameplay)
                map_id = mumble_link->Context.MapID;
            else
                map_id = 0;
        }
        if (ImGui::Button("Add Message##AddChatShortsMessage")) {
            chat_messages[map_id].emplace_back(short_message, message);
            short_message.clear();
            memset(message, 0, 199);
            map_id = 0;
            Settings::json_settings[Settings::CHAT_MESSAGES] = chat_messages;
            Settings::save(Settings::settings_path);
        }
    }
    render_messages();
    if (to_delete != std::pair{-1, -1}) {
        chat_messages[to_delete.first].erase(chat_messages[to_delete.first].begin() + to_delete.second);
        Settings::json_settings[Settings::CHAT_MESSAGES] = chat_messages;
        Settings::save(Settings::settings_path);
        to_delete = {-1, -1};
    }
}
