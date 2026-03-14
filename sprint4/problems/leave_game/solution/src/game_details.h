#pragma once
#include <iostream>
#include <string>
#include <optional>

namespace game_details {
    using namespace std::literals;
    using Score = unsigned;
    using LootType = unsigned;

    struct LootDescription {
        std::string name_;
        std::string file_path_;
        std::string type_;
        std::optional<int> rotation_;
        std::optional<std::string> color_;
        double scale_;
        Score value_;
    };   
}
