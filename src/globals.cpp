#include "globals.hpp"

HMODULE self_module = nullptr;
AddonDefinition addon_def{};
AddonAPI *api = nullptr;
Mumble::Data *mumble_link;
NexusLinkData *nexus_link;
char addon_name[] = "Chat Shorts";
HWND game_handle = nullptr;
std::map<int, std::vector<Message>> chat_messages;
std::map<int, std::string> maps;

void from_json(const nlohmann::json &j, Message &message)
{
    j["short_message"].get_to(message.short_message);
    j["message"].get_to(message.message);
}

void to_json(nlohmann::json &j, const Message &message)
{
    j["short_message"] = message.short_message;
    j["message"] = message.message;
}
