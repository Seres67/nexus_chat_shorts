#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <Key.hpp>
#include <Message.hpp>
#include <SettingsManager.hpp>
#include <nlohmann/json.hpp>

struct Settings
{
    bool lock_position = false;
    short visibility = 0;
    int number_columns = 2;
    std::map<int, std::vector<Message>> chat_messages;
    int delay = 25;
};

// NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Settings, lock_position, visibility, number_columns, chat_messages);

extern std::unique_ptr<SettingsManager<Settings>> settings_manager;

void from_json(const nlohmann::json &j, Settings &settings);
void to_json(nlohmann::json &j, const Settings &settings);

#endif // SETTINGS_HPP
