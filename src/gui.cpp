#include <globals.hpp>
#include <gui.hpp>
#include <imgui/imgui.h>
#include <settings.hpp>

#include <imgui/misc/cpp/imgui_stdlib.h>
std::string short_message;
char message[199];
std::string edit_short;
std::string edit_message;
int to_edit = -1;
int to_delete = -1;
void render_messages()
{
    if (ImGui::CollapsingHeader("Messages##ChatShortsMessagesCollapse")) {
        for (int i = 0; i < chat_messages.size(); ++i) {
            ImGui::PushID(chat_messages[i].short_message.c_str());
            if (to_edit == i) {
                ImGui::InputText("New Short Name", &edit_short);
                ImGui::InputTextMultiline("New Chat Message", &edit_message);
                if (ImGui::Button("Confirm")) {
                    chat_messages[to_edit].short_message = edit_short;
                    chat_messages[to_edit].message = edit_message;
                    to_edit = -1;
                    edit_short.clear();
                    edit_message.clear();
                    Settings::json_settings[Settings::CHAT_MESSAGES] = chat_messages;
                    Settings::save(Settings::settings_path);
                }
            } else {
                ImGui::TextWrapped("%s:\n%s", chat_messages[i].short_message.c_str(), chat_messages[i].message.c_str());
                if (ImGui::Button("Edit")) {
                    edit_short = chat_messages[i].short_message;
                    edit_message = chat_messages[i].message;
                    to_edit = i;
                }
                ImGui::SameLine();
                if (ImGui::Button("Delete"))
                    to_delete = i;
            }
            ImGui::PopID();
        }
    }
}

void render_options()
{
    if (ImGui::Checkbox("Lock Position##ChatShortsLockPosition", &Settings::lock_position)) {
        Settings::json_settings[Settings::LOCK_POSITION] = Settings::lock_position;
        Settings::save(Settings::settings_path);
    }
    if (ImGui::CollapsingHeader("Add Message##ChatShortsMessagesCollapse")) {
        ImGui::InputText("Short Name##ChatShortsShortNameInput", &short_message);
        ImGui::InputTextMultiline("Chat Message##ChatShortsAddMessageInput", message, 200);
        if (ImGui::Button("Add Message##AddChatShortsMessage")) {
            chat_messages.emplace_back(short_message, message);
            short_message.clear();
            memset(message, 0, 199);
            Settings::json_settings[Settings::CHAT_MESSAGES] = chat_messages;
            Settings::save(Settings::settings_path);
        }
    }
    render_messages();
    if (to_delete != -1) {
        chat_messages.erase(chat_messages.begin() + to_delete);
        Settings::json_settings[Settings::CHAT_MESSAGES] = chat_messages;
        Settings::save(Settings::settings_path);
        to_delete = -1;
    }
}
