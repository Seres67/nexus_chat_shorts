#include <globals.hpp>
#include <nexus/Nexus.h>
#include <nlohmann/json.hpp>
#include <settings.hpp>

std::unique_ptr<SettingsManager<Settings>> settings_manager;

void from_json(const nlohmann::json &j, Settings &settings)
{
    if (j.contains("LockPosition")) {
        j.at("LockPosition").get_to(settings.lock_position);
    }
    if (j.contains("lock_position")) {
        j.at("lock_position").get_to(settings.lock_position);
    }
    if (j.contains("Visibility")) {
        j.at("Visibility").get_to(settings.visibility);
    }
    if (j.contains("visibility")) {
        j.at("visibility").get_to(settings.visibility);
    }
    if (j.contains("NumberColumns")) {
        j.at("NumberColumns").get_to(settings.number_columns);
    }
    if (j.contains("number_columns")) {
        j.at("number_columns").get_to(settings.number_columns);
    }
    if (j.contains("ChatMessages")) {
        j.at("ChatMessages").get_to(settings.chat_messages);
    }
    if (j.contains("chat_messages")) {
        j.at("chat_messages").get_to(settings.chat_messages);
    }
    if (j.contains("delay")) {
        j.at("delay").get_to(settings.delay);
    }
}

void to_json(nlohmann::json &j, const Settings &settings)
{
    j = nlohmann::json{
        {"lock_position", settings.lock_position},
        {"visibility", settings.visibility},
        {"number_columns", settings.number_columns},
        {"chat_messages", settings.chat_messages},
        {"delay", settings.delay},
    };
}
