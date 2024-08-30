#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#include "nlohmann/json.hpp"
#include <nexus/Nexus.h>
#include <string>
#include <vector>

// handle to self hmodule
extern HMODULE self_module;
// addon definition
extern AddonDefinition addon_def;
// addon api
extern AddonAPI *api;

extern char addon_name[];

extern HWND game_handle;

typedef struct
{
    std::string short_message;
    std::string message;
} Message;

extern std::vector<Message> chat_messages;

void from_json(const nlohmann::json &j, Message &message);
void to_json(nlohmann::json &j, const Message &message);

#endif // GLOBALS_HPP
