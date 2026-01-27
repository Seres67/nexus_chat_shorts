#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#include <Key.hpp>
#include <Message.hpp>
#include <map>
#include <mumble/Mumble.h>
#include <nexus/Nexus.h>
#include <nlohmann/json.hpp>
#include <string>

// handle to self hmodule
extern HMODULE self_module;

// addon definition
extern AddonDefinition addon_def;

// addon api
extern AddonAPI *api;

extern Mumble::Data *mumble_link;
extern NexusLinkData *nexus_link;

extern char addon_name[];

extern HWND game_handle;

extern std::map<int, std::string> maps;

#endif // GLOBALS_HPP
