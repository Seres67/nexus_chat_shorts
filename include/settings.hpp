#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <Key.hpp>
#include <mutex>
#include <nlohmann/json.hpp>

void from_json(const nlohmann::json &j, Key &key);
void to_json(nlohmann::json &j, const Key &key);

namespace Settings
{

void load(const std::filesystem::path &path);
void save(const std::filesystem::path &path);

extern nlohmann::json json_settings;
extern std::filesystem::path settings_path;
extern std::mutex mutex;

extern bool lock_position;
extern short visibility;
extern int number_columns;

extern const char *LOCK_POSITION;
extern const char *CHAT_MESSAGES;
extern const char *VISIBILITY;
extern const char *NUMBER_COLUMNS;
} // namespace Settings

#endif // SETTINGS_HPP
