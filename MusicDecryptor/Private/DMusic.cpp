#include "DMusic.h"

KeyMap::KeyMap()
{
	this->_keymap = std::map<std::u8string, std::vector<uint8_t>>();
}

KeyMap::KeyMap(std::u8string keystr) 
{
    std::u8string key;
    std::vector<uint8_t> value;
    key.reserve(32);
    value.reserve(704);
    enum State { KEY, VALUE } state = KEY;

    for (char8_t ch : keystr) 
    {
        if (ch == u8'$') {
            state = VALUE;
            continue;
        }

        if (ch == u8'\n') 
        {
            if (!key.empty() || !value.empty()) 
            {
                _keymap[key] = value;
            }
            key.clear();
            value.clear();
            state = KEY;
            continue;
        }

        
        if (ch == u8'\r') continue;

        (state == KEY) ? key.push_back(ch) : value.push_back(ch);
    }

    // 处理末尾无\n的数据
    if (!key.empty() || !value.empty()) {
        _keymap[key] = value;
    }
}

KeyMap::KeyMap(std::map<std::u8string, std::u8string> stdmap)
{
    for (auto iter = stdmap.begin(); iter != stdmap.end(); iter++)
    {
        this->_keymap[iter->first] = std::vector<uint8_t>(iter->second.begin(), iter->second.end());
    }
}

std::u8string KeyMap::getStr()
{
    auto str = std::u8string();
    for (auto iter = _keymap.begin(); iter != _keymap.end(); iter++)
    {
        str += iter->first + u8'$';
        str += std::u8string(iter->second.begin(), iter->second.end());
        str += u8"\n";
    }
    return str;
}

std::vector<uint8_t>& KeyMap::operator[](std::u8string& id)
{
    auto [iter, inserted] = _keymap.emplace(id, std::vector<uint8_t>());
    return iter->second;
}


