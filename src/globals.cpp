#include <globals.hpp>
#include <nexus/Nexus.h>

HMODULE self_module = nullptr;
AddonDefinition addon_def{};
AddonAPI *api = nullptr;
Mumble::Data *mumble_link;
NexusLinkData *nexus_link;
char addon_name[] = "Chat Shorts";
HWND game_handle = nullptr;
std::map<int, std::string> maps;
