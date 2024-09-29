#ifndef GLOBALS_HPP
#define GLOBALS_HPP

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

typedef struct
{
    std::string short_message;
    std::string message;
} Message;

extern std::map<int, std::vector<Message>> chat_messages;
extern std::map<int, std::string> maps;

void from_json(const nlohmann::json &j, Message &message);
void to_json(nlohmann::json &j, const Message &message);

#endif // GLOBALS_HPP
