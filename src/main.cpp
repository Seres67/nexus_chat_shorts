#include <clipboard.hpp>
#include <globals.hpp>
#include <gui.hpp>
#include <imgui/imgui.h>
#include <nexus/Nexus.h>
#include <settings.hpp>

void addon_load(AddonAPI *api_p);
void addon_unload();
void addon_render();
void addon_options();
UINT wnd_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

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
    addon_def.Version.Minor = 1;
    addon_def.Version.Build = 0;
    addon_def.Version.Revision = 1;
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
    api->Renderer.Register(ERenderType_Render, addon_render);
    api->Renderer.Register(ERenderType_OptionsRender, addon_options);
    api->WndProc.Register(wnd_proc);

    Settings::settings_path = api->Paths.GetAddonDirectory("chat_shorts\\settings.json");
    if (std::filesystem::exists(Settings::settings_path)) {
        Settings::load(Settings::settings_path);
    }
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
    std::int64_t l_param;
    l_param = down ? 0 : 1; // transition state
    l_param = l_param << 1;
    l_param += down ? 0 : 1; // previous key state
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

bool tmp_open = true;
void addon_render()
{
    ImGui::SetNextWindowPos(ImVec2(300, 400), ImGuiCond_FirstUseEver);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;
    if (Settings::lock_position) {
        flags |= ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    }
    if (tmp_open && ImGui::Begin("Chat Shorts##ChatShortsMainWindow", &tmp_open, flags)) {
        if (ImGui::BeginTable("Messages##", 3)) {
            for (const auto &[short_message, message] : chat_messages) {
                ImGui::TableNextRow();
                ImGui::PushID(short_message.c_str());
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%s", short_message.c_str());
                if (ImGui::IsItemHovered()) {
                    auto size = ImGui::CalcTextSize(message.c_str(), nullptr, false, 500);
                    size.x += 20;
                    ImGui::SetNextWindowSize({size.x, -FLT_MIN});
                    ImGui::BeginTooltip();
                    ImGui::TextWrapped("%s", message.c_str());
                    ImGui::EndTooltip();
                }
                ImGui::TableSetColumnIndex(1);
                if (ImGui::Button("Copy##ChatShortsCopyMessageButton")) {
                    copy_to_clipboard(game_handle, message);
                }
                ImGui::TableSetColumnIndex(2);
                if (ImGui::Button("Send##ChatShortsSendMessageButton")) {
                    std::thread(
                        [message]()
                        {
                            using namespace std::chrono_literals;

                            std::optional<std::string> old = save_and_copy_to_clipboard(game_handle, message);

                            SendMessage(game_handle, WM_KEYDOWN, VK_RETURN, get_l_param(VK_RETURN, true));
                            SendMessage(game_handle, WM_KEYUP, VK_RETURN, get_l_param(VK_RETURN, false));
                            std::this_thread::sleep_for(25ms);

                            INPUT select_text[1] = {};
                            ZeroMemory(select_text, sizeof(select_text));
                            select_text[0].type = INPUT_KEYBOARD;
                            select_text[0].ki.wVk = VK_CONTROL;
                            UINT u_sent = SendInput(ARRAYSIZE(select_text), select_text, sizeof(INPUT));
                            if (u_sent != ARRAYSIZE(select_text)) {
                                api->Log(ELogLevel_DEBUG, addon_name, "SendInput failed");
                            }
                            SendMessage(game_handle, WM_KEYDOWN, 'V', get_l_param('V', true));
                            SendMessage(game_handle, WM_KEYUP, 'V', get_l_param('V', false));
                            std::this_thread::sleep_for(25ms);

                            ZeroMemory(select_text, sizeof(select_text));
                            select_text[0].type = INPUT_KEYBOARD;
                            select_text[0].ki.wVk = VK_CONTROL;
                            select_text[0].ki.dwFlags = KEYEVENTF_KEYUP;
                            u_sent = SendInput(ARRAYSIZE(select_text), select_text, sizeof(INPUT));
                            if (u_sent != ARRAYSIZE(select_text)) {
                                api->Log(ELogLevel_DEBUG, addon_name, "SendInput failed");
                            }
                            std::this_thread::sleep_for(25ms);

                            SendMessage(game_handle, WM_KEYDOWN, VK_RETURN, get_l_param(VK_RETURN, true));
                            SendMessage(game_handle, WM_KEYUP, VK_RETURN, get_l_param(VK_RETURN, false));
                            std::this_thread::sleep_for(25ms);
                            if (old.has_value())
                                copy_to_clipboard(game_handle, old.value());
                        })
                        .detach();
                }
                ImGui::PopID();
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
