#include <Message.hpp>
#include <globals.hpp>

bool converted = true;

void from_json(const nlohmann::json &j, Message &message)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    try {
        std::wstring wide_short_message = j.at("short_message").get<std::wstring>();
        std::string narrow_short_message = converter.to_bytes(wide_short_message);
        message.short_message = narrow_short_message;
    } catch (std::exception &e) {
        converted = false;
        j["short_message"].get_to(message.short_message);
    }

    try {
        std::wstring wide_message = j.at("message").get<std::wstring>();
        std::string narrow_message = converter.to_bytes(wide_message);
        message.message = narrow_message;
    } catch (std::exception &e) {
        converted = false;
        j["message"].get_to(message.message);
    }

    if (j.contains("squad_broadcast")) {
        message.squad_broadcast = j.at("squad_broadcast").get<bool>();
    } else {
        message.squad_broadcast = false;
    }
}

void to_json(nlohmann::json &j, const Message &message)
{
    j["short_message"] = message.short_message;
    j["message"] = message.message;
    j["squad_broadcast"] = message.squad_broadcast;
}
