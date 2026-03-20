#include <clipboard.hpp>
#include <globals.hpp>
#include <gui.hpp>
#include <imgui/imgui.h>
#include <memory>
#include <mumble/Mumble.h>
#include <nexus/Nexus.h>
#include <settings.hpp>
#include <string>
#include <thread>
#include <win32-http.hpp>

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
    addon_def.Version.Minor = 4;
    addon_def.Version.Build = 0;
    addon_def.Version.Revision = 0;
    addon_def.Author = "Seres67";
    addon_def.Description = "An addon to store and copy/send recurring chat messages";
    addon_def.Load = addon_load;
    addon_def.Unload = addon_unload;
    addon_def.Flags = EAddonFlags_None;
    addon_def.Provider = EUpdateProvider_Direct;
    addon_def.UpdateLink =
        "https://git.seres.eu.org/seres/nexus_chat_shorts/releases/download/latest/libnexus_chat_shorts.dll";

    return &addon_def;
}

void addon_load(AddonAPI *api_p)
{
    api = api_p;

    ImGui::SetCurrentContext(static_cast<ImGuiContext *>(api->ImguiContext));
    ImGui::SetAllocatorFunctions(reinterpret_cast<void *(*)(size_t, void *)>(api->ImguiMalloc),
                                 reinterpret_cast<void (*)(void *, void *)>(api->ImguiFree)); // on imgui 1.80+

    mumble_link = (Mumble::Data *)api->DataLink.Get("DL_MUMBLE_LINK");
    nexus_link = (NexusLinkData *)api->DataLink.Get("DL_NEXUS_LINK");

    std::thread(
        []()
        {
            auto [status, body] = win32_http::get("api.guildwars2.com", "/v2/maps?ids=all", "");
            if (status == 200) {
                for (auto maps_json = nlohmann::json::parse(body); const auto &map : maps_json) {
                    int id = map["id"].get<int>();
                    const std::string name = map["name"].get<std::string>();
                    maps[id] = name;
                }
            } else {
                api->Log(ELogLevel_WARNING, addon_name, "could not fetch API, message list won't have map names");
            }
        })
        .detach();

    auto settings_path = api->Paths.GetAddonDirectory("chat_shorts\\settings.json");
    settings_manager = std::make_unique<SettingsManager<Settings>>(settings_path);

    if (!converted) {
        settings_manager->save();
    }

    // for (auto &[map, messages] : chat_messages)
    //     for (auto &message : messages)
    //         api->Events.Subscribe(std::string("EV_CHAT_SHORTS_" + message.short_message).c_str(),
    //                               [message](void *data) { send_message(message.message); });

    api->Renderer.Register(ERenderType_Render, addon_render);
    api->Renderer.Register(ERenderType_OptionsRender, addon_options);
    api->WndProc.Register(wnd_proc);
    api->Log(ELogLevel_INFO, addon_name, "addon loaded!");
}

void addon_unload()
{
    api->Log(ELogLevel_INFO, addon_name, "unloading addon...");
    api->Renderer.Deregister(addon_render);
    api->Renderer.Deregister(addon_options);
    api->WndProc.Deregister(wnd_proc);
    for (auto &[map, messages] : settings_manager->get(&Settings::chat_messages)) {
        for (auto &message : messages) {
            api->Events.Unsubscribe(std::string("EV_CHAT_SHORTS_" + message.short_message).c_str(), event_handler);
        }
    }

    api->Log(ELogLevel_INFO, addon_name, "addon unloaded!");
    nexus_link = nullptr;
    mumble_link = nullptr;
    api = nullptr;
}

bool display_window()
{
    auto visibility = settings_manager->get(&Settings::visibility);
    if (visibility == 0) {
        return true;
    }
    if (visibility == 1) {
        if (nexus_link->IsGameplay) {
            return true;
        }
    }
    if (visibility == 2) {
        if (!mumble_link->Context.IsInCombat) {
            return true;
        }
    }
    if (visibility == 3) {
        if (mumble_link->Context.IsInCombat) {
            return true;
        }
    }
    if (visibility == 4) {
        return false;
    }
    return false;
}

bool tmp_open = true;
void addon_render()
{
    auto &s = settings_manager->get();

    ImGui::SetNextWindowPos(ImVec2(300, 400), ImGuiCond_FirstUseEver);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;
    if (s.lock_position) {
        flags |= ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    }
    bool content = false;
    for (const auto &[map_id, value] : settings_manager->get(&Settings::chat_messages)) {
        if ((map_id == 0 || map_id == mumble_link->Context.MapID) && !value.empty()) {
            content = true;
            break;
        }
    }
    if (!content && s.lock_position) {
        return;
    }
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    if (tmp_open && display_window() && ImGui::Begin("Chat Shorts##ChatShortsMainWindow", &tmp_open, flags)) {
        if (ImGui::BeginTable("Messages##ChatShortsMessagesList", s.number_columns)) {
            int i = 0;
            ImGui::TableNextRow();
            for (const auto &[map_id, value] : settings_manager->get(&Settings::chat_messages)) {
                for (const auto &[short_message, message, broadcast, key] : value) {
                    if (map_id == 0 || map_id == mumble_link->Context.MapID) {
                        if (i == s.number_columns) {
                            ImGui::TableNextRow();
                            i = 0;
                        }
                        ImGui::TableSetColumnIndex(i);
                        ImGui::PushID(short_message.c_str());
                        if (ImGui::Button(short_message.c_str())) {
                            if (broadcast) {
                                broadcast_message(converter.from_bytes(message));
                            } else {
                                send_message(converter.from_bytes(message));
                            }
                        }
                        if (ImGui::BeginPopupContextItem()) {
                            if (ImGui::Button(("Copy to clipboard##ChatShorts" + message).c_str())) {
                                std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
                                copy_to_clipboard(game_handle, converter.from_bytes(message));
                                ImGui::CloseCurrentPopup();
                            }
                            ImGui::EndPopup();
                        }
                        if (ImGui::IsItemHovered()) {
                            auto size = ImGui::CalcTextSize(message.c_str(), nullptr, false, 500);
                            size.x += 20;
                            ImGui::SetNextWindowSize({size.x, -FLT_MIN});
                            ImGui::BeginTooltip();
                            ImGui::TextWrapped("%s", message.c_str());
                            ImGui::EndTooltip();
                        }
                        ImGui::PopID();
                        ++i;
                    }
                }
            }
            ImGui::EndTable();
        }
        ImGui::End();
    }
}

void addon_options()
{
    render_options();
}

UINT wnd_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (!game_handle) {
        game_handle = hWnd;
    }
    return uMsg;
}
