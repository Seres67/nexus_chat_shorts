#ifndef INCLUDE_INCLUDE_MESSAGE_HPP_
#define INCLUDE_INCLUDE_MESSAGE_HPP_

#include <Key.hpp>
#include <string>

struct Message
{
    std::string short_message;
    std::string message;
    bool squad_broadcast;
    Key key;
};

void from_json(const nlohmann::json &j, Message &message);
void to_json(nlohmann::json &j, const Message &message);

extern bool converted;

#endif // INCLUDE_INCLUDE_MESSAGE_HPP_
