#pragma once
#include <iostream>
#include <string>
#include <optional>
#include <filesystem>
#include "boost_includes.h"
#include "tagged.h"

namespace extra_data {
    using namespace std::literals;
    const std::string key_loot_types = "lootTypes"s;
    const std::string key_file = "file"s;
    const std::string key_type = "type"s;
    const std::string key_rotation = "rotation"s;
    const std::string key_color = "color"s;
    const std::string key_scale = "scale"s;

    class Loot {
    public:
        Loot() = default;

    /*    Loot(std::string name, 
            std::string file_path,
            std::string type, 
            std::optional<double> rotation = std::nullopt,
            std::optional<std::string> color = std::nullopt,
            double scale)
            : name_(name)
            , file_path_(file_path)
            , type_(type)
            , rotation_(rotation)
            , color_(color)
            , scale_(scale) {
        
        }*/

        std::string GetName() const {
            return name_;
        }

        std::string GetFilePath() const noexcept {
            return file_path_;
        }

        std::string GetType() const noexcept {
            return type_;
        }

        std::optional<double> GetRotation() const noexcept {
            return rotation_;
        }

        std::optional<std::string> GetColor() const noexcept {
            return color_;
        }

        double GetScale() const noexcept {
            return scale_;
        }

        void SetName(const std::string& name) {
            name_ = name;
        }

        void SetFilePath(const std::string& file_path) {
            file_path_ = file_path;
        }

        void SetType(const std::string& type) {
            type_ = type;
        }

        void SetRotation(double rotation) {
            rotation_ = rotation;
        }

        void SetColor(const std::string& color) {
            color_ = color;
        }

        void SetScale(double scale) {
            scale_ = scale;
        }
    private:
        std::string name_;
        std::string file_path_;
        std::string type_;
        std::optional<double> rotation_;
        std::optional<std::string> color_;
        double scale_;
    };   
}
