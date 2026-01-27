#include <Key.hpp>

void from_json(const nlohmann::json &j, Key &key)
{
    unsigned int vk = j.at("virtual_code").get<unsigned int>();
    key.set_virtual_code(vk);
    unsigned int sc = j.at("scan_code").get<unsigned int>();
    key.set_scan_code(sc);
    bool control = j.at("control").get<bool>();
    key.set_control(control);
    bool alt = j.at("alt").get<bool>();
    key.set_alt(alt);
    bool shift = j.at("shift").get<bool>();
    key.set_shift(shift);
}

void to_json(nlohmann::json &j, const Key &key)
{
    j = nlohmann::json{
        {"virtual_code", key.get_virtual_code()},
        {"scan_code", key.get_scan_code()},
        {"control", key.get_control()},
        {"alt", key.get_alt()},
        {"shift", key.get_shift()},
    };
}
