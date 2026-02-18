#pragma once
#include <iostream>
#include <string>
#include <filesystem>
#include "boost_includes.h"
#include "tagged.h"
#include "model.h"

//"lootTypes": [
//{
//    "name": "key",
//        "file" : "assets/key.obj",
//        "type" : "obj",
//        "rotation" : 90,
//        "color" : "#338844",
//        "scale" : 0.03
//},
//        {
//          "name": "wallet",
//          "file" : "assets/wallet.obj",
//          "type" : "obj",
//          "rotation" : 0,
//          "color" : "#883344",
//          "scale" : 0.01
//        }
//],
namespace extra_data {
    using namespace std::literals;

    const std::string key_file = "file"s;
    const std::string key_type = "type"s;
    const std::string key_rotation = "rotation"s;
    const std::string key_color = "color"s;
    const std::string key_scale = "scale"s;

    class Loot {
    public:
        using Id = util::Tagged<int, Loot>;

        Loot(std::string name, 
            std::filesystem::path file_path, 
            std::string type, 
            double rotation, 
            std::string color, 
            double scale)
            :name_(name)
            , file_path_(file_path)
            , type_(type)
            , rotation_(rotation)
            , color_(color)
            , scale_(scale) {
        
        }


        std::string GetName() const {
            return name_;
        }

        std::filesystem::path GetFilePath() const noexcept {
            return file_path_;
        }

        std::string GetType() const noexcept {
            return type_;
        }

        double GetRotation() const noexcept {
            return rotation_;
        }

        std::string GetColor() const noexcept {
            return color_;
        }

        double GetScale() const noexcept {
            return scale_;
        }

        void SetName(const std::string& name) {
            name_ = name;
        }

        void SetFilePath(const std::filesystem::path& file_path) {
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
        std::filesystem::path file_path_;
        std::string type_;
        double rotation_;
        std::string color_;
        double scale_;
    };

    class LootTypes {
    public:
        using Loots = std::vector<std::shared_ptr<Loot>>;

        void AddLoot(std::string id, Loot loot) {
            loot_types_[id].emplace_back(std::make_shared<Loot>(std::move(loot)));
        }

        Loots GetLootsByMapId(model::Map::Id id) {
        
        }
    private:

        //можно сохранять по map id
        std::unordered_map<std::string, Loots> loot_types_;
    };
}
