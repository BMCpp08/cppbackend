#pragma once
#include <iostream>
#include <string>
#include <optional>

namespace extra_data {
    using namespace std::literals;
    
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
