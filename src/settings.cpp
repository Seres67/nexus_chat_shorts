#include "settings.hpp"
#include "nlohmann/json.hpp"
#include <filesystem>
#include <fstream>
#include <globals.hpp>

#include <nexus/Nexus.h>

using json = nlohmann::json;
namespace Settings
{
const char *LOCK_POSITION = "LockPosition";
const char *CHAT_MESSAGES = "ChatMessages";
const char *VISIBILITY = "Visibility";

json json_settings;
std::mutex mutex;
std::filesystem::path settings_path;

bool lock_position = false;
short visibility = 0;

void load(const std::filesystem::path &path)
{
    json_settings = json::object();
    if (!std::filesystem::exists(path)) {
        return;
    }

    {
        std::lock_guard lock(mutex);
        try {
            if (std::ifstream file(path); file.is_open()) {
                json_settings = json::parse(file);
                file.close();
            }
        } catch (json::parse_error &ex) {
            api->Log(ELogLevel_WARNING, addon_name, "settings.json could not be parsed.");
            api->Log(ELogLevel_WARNING, addon_name, ex.what());
        }
    }
    if (!json_settings[LOCK_POSITION].is_null()) {
        json_settings[LOCK_POSITION].get_to(lock_position);
    }
    if (!json_settings[CHAT_MESSAGES].is_null()) {
        chat_messages = json_settings[CHAT_MESSAGES].get<std::map<int, std::vector<Message>>>();
    }
    if (!json_settings[VISIBILITY].is_null()) {
        json_settings[VISIBILITY].get_to(visibility);
    }
    api->Log(ELogLevel_INFO, addon_name, "settings loaded!");
}

void save(const std::filesystem::path &path)
{
    if (json_settings.is_null()) {
        api->Log(ELogLevel_WARNING, addon_name, "settings.json is null, cannot save.");
        return;
    }
    if (!std::filesystem::exists(path.parent_path())) {
        std::filesystem::create_directories(path.parent_path());
    }
    {
        std::lock_guard lock(mutex);
        if (std::ofstream file(path); file.is_open()) {
            file << json_settings.dump(1, '\t') << std::endl;
            file.close();
        }
        api->Log(ELogLevel_INFO, addon_name, "settings saved!");
    }
}
} // namespace Settings
