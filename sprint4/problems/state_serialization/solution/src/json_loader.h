#pragma once

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#include "model.h"
#include "game_details.h"

namespace json_loader {

model::Game LoadGame(const std::filesystem::path& json_path);


}  // namespace json_loader
