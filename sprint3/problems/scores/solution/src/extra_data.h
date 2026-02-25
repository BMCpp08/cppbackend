#pragma once
#include <iostream>
#include <string>
#include <optional>

namespace extra_data {
    using namespace std::literals;
    const std::string key_loot_types = "lootTypes"s;
    const std::string key_file = "file"s;
    const std::string key_type = "type"s;
    const std::string key_rotation = "rotation"s;
    const std::string key_color = "color"s;
    const std::string key_scale = "scale"s;
    const std::string key_value = "value"s;
    
    struct LootDescription {
        std::string name_;
        std::string file_path_;
        std::string type_;
        std::optional<int> rotation_;
        std::optional<std::string> color_;
        double scale_;
        int value_;
    };   
}
