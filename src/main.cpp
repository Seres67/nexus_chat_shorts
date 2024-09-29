#include <clipboard.hpp>
#include <cpr/cpr.h>
#include <globals.hpp>
#include <gui.hpp>
#include <imgui/imgui.h>
#include <mumble/Mumble.h>
#include <nexus/Nexus.h>
#include <settings.hpp>

void addon_load(AddonAPI *api_p);
void addon_unload();
void addon_render();
void addon_options();
UINT wnd_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void event_handler(void *payload);

BOOL APIENTRY dll_main(const HMODULE hModule, const DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        self_module = hModule;
        break;
    case DLL_PROCESS_DETACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    default:
        break;
    }
    return TRUE;
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" __declspec(dllexport) AddonDefinition *GetAddonDef()
{
    addon_def.Signature = -912373178;
    addon_def.APIVersion = NEXUS_API_VERSION;
    addon_def.Name = addon_name;
    addon_def.Version.Major = 0;
    addon_def.Version.Minor = 2;
    addon_def.Version.Build = 0;
    addon_def.Version.Revision = 0;
    addon_def.Author = "Seres67";
    addon_def.Description = "An addon to store and copy/send recurring chat messages";
    addon_def.Load = addon_load;
    addon_def.Unload = addon_unload;
    addon_def.Flags = EAddonFlags_None;
    addon_def.Provider = EUpdateProvider_GitHub;
    addon_def.UpdateLink = "https://github.com/Seres67/nexus_chat_shorts";

    return &addon_def;
}

void addon_load(AddonAPI *api_p)
{
    api = api_p;

    ImGui::SetCurrentContext(static_cast<ImGuiContext *>(api->ImguiContext));
    ImGui::SetAllocatorFunctions(static_cast<void *(*)(size_t, void *)>(api->ImguiMalloc),
                                 static_cast<void (*)(void *, void *)>(api->ImguiFree)); // on imgui 1.80+

    mumble_link = (Mumble::Data *)api->DataLink.Get("DL_MUMBLE_LINK");
    nexus_link = (NexusLinkData *)api->DataLink.Get("DL_NEXUS_LINK");
    api->Renderer.Register(ERenderType_Render, addon_render);
    api->Renderer.Register(ERenderType_OptionsRender, addon_options);
    api->WndProc.Register(wnd_proc);

    std::thread(
        []()
        {
            cpr::Response response =
                cpr::Get(cpr::Url{"https://api.guildwars2.com/v2/maps"}, cpr::Parameters{{"ids", "all"}});
            if (response.status_code == 200) {
                for (auto maps_json = nlohmann::json::parse(response.text); const auto &map : maps_json) {
                    int id = map["id"].get<int>();
                    std::string name = map["name"].get<std::string>();
                    maps[id] = name;
                }
            }
        })
        .detach();

    Settings::settings_path = api->Paths.GetAddonDirectory("chat_shorts\\settings.json");
    if (std::filesystem::exists(Settings::settings_path)) {
        Settings::load(Settings::settings_path);
    }
    api->Events.Subscribe("EV_SEND_CHAT_SHORTS_MESSAGE", event_handler);

    api->Log(ELogLevel_INFO, addon_name, "addon loaded!");
}

void addon_unload()
{
    api->Log(ELogLevel_INFO, addon_name, "unloading addon...");
    api->Renderer.Deregister(addon_render);
    api->Renderer.Deregister(addon_options);
    api->WndProc.Deregister(wnd_proc);
    api->Log(ELogLevel_INFO, addon_name, "addon unloaded!");
    api = nullptr;
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

void send_message(const std::string &message)
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

bool display_window()
{
    if (Settings::visibility == 0)
        return true;
    if (Settings::visibility == 1) {
        if (nexus_link->IsGameplay)
            return true;
    }
    if (Settings::visibility == 2) {
        if (!mumble_link->Context.IsInCombat)
            return true;
    }
    if (Settings::visibility == 3) {
        if (mumble_link->Context.IsInCombat)
            return true;
    }
    if (Settings::visibility == 4) {
        return false;
    }
    return false;
}

bool tmp_open = true;
void addon_render()
{
    //    ImGui::ShowDemoWindow();
    ImGui::SetNextWindowPos(ImVec2(300, 400), ImGuiCond_FirstUseEver);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;
    if (Settings::lock_position) {
        flags |= ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    }
    if (tmp_open && display_window() && ImGui::Begin("Chat Shorts##ChatShortsMainWindow", &tmp_open, flags)) {
        if (ImGui::BeginTable("Messages##", 3)) {
            for (const auto &[map_id, value] : chat_messages) {
                for (const auto &[short_message, message] : value) {
                    if (map_id == 0 || map_id == mumble_link->Context.MapID) {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::PushID(short_message.c_str());
                        ImGui::Text("%s", short_message.c_str());
                        if (ImGui::IsItemHovered()) {
                            auto size = ImGui::CalcTextSize(message.c_str(), nullptr, false, 500);
                            size.x += 20;
                            ImGui::SetNextWindowSize({size.x, -FLT_MIN});
                            ImGui::BeginTooltip();
                            ImGui::TextWrapped("%s", message.c_str());
                            ImGui::EndTooltip();
                        }
                        ImGui::TableNextColumn();
                        std::string copy_text = "Copy";
                        auto copy_posX = (ImGui::GetCursorPosX() + ImGui::GetColumnWidth() -
                                          ImGui::CalcTextSize(copy_text.c_str()).x - ImGui::GetScrollX() -
                                          2 * ImGui::GetStyle().ItemSpacing.x);
                        if (copy_posX > ImGui::GetCursorPosX())
                            ImGui::SetCursorPosX(copy_posX);
                        std::string button_text = copy_text + "##ChatShortsCopyMessageButton";
                        if (ImGui::Button(button_text.c_str()))
                            copy_to_clipboard(game_handle, message);
                        ImGui::TableNextColumn();
                        std::string send_text = "Send";
                        auto send_posX = (ImGui::GetCursorPosX() + ImGui::GetColumnWidth() -
                                          ImGui::CalcTextSize(send_text.c_str()).x - ImGui::GetScrollX() -
                                          2 * ImGui::GetStyle().ItemSpacing.x);
                        if (send_posX > ImGui::GetCursorPosX())
                            ImGui::SetCursorPosX(send_posX);
                        std::string button_text2 = send_text + "##ChatShortsSendMessageButton";
                        if (ImGui::Button(button_text2.c_str()))
                            send_message(message);
                        ImGui::PopID();
                    }
                }
            }
            ImGui::EndTable();
        }
        ImGui::End();
    }
}

void addon_options() { render_options(); }

UINT wnd_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (!game_handle)
        game_handle = hWnd;
    return uMsg;
}

void event_handler(void *payload)
{
    std::string event_name = reinterpret_cast<const char *>(payload);
    api->Log(ELogLevel_INFO, addon_name, event_name.c_str());
    //    auto msg = std::ranges::find(chat_messages, event_name, &Message::short_message);
    //    if (msg != chat_messages.end())
    //        send_message(msg->message);
}
