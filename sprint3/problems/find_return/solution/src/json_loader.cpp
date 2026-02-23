#include "json_loader.h"
#include <iostream>
#include <cassert>
#include "boost_includes.h"
#include <cmath>

namespace json_loader {
	using namespace std::literals;
	using namespace model;
	using namespace model::model_details;
	using namespace boost_aliases;

	static Point CreatePoint(const auto& object, const std::string& x, const std::string& y) {
		return Point{ Coord(object.at(x).as_int64()), Coord(object.at(y).as_int64()) };
	}

	static Building CreateBuilding(const auto& object) {
		Size size{ object.at(key_w).as_int64(),object.at(key_h).as_int64() };
		Point point = std::move(CreatePoint(object, key_x, key_y));
		return Building{ { point ,size } };
	}

	static Office CreateOffice(const auto& object) {
		Office::Id id{ object.at(key_id).as_string().c_str() };
		Point point = std::move(CreatePoint(object, key_x, key_y));
		Offset offset{ object.at(key_offset_x).as_int64(), object.at(key_offset_y).as_int64() };
		return Office{ id, point, offset };
	}

	static Road CreateRoad(const auto& object) {
		if (object.if_contains(key_x1)) {
			return Road{ Road::HORIZONTAL,
				CreatePoint(object,key_x0,key_y0),
				Coord(object.at(key_x1).as_int64())
			};

		}
		else if (object.if_contains(key_y1)) {
			return Road{ Road::VERTICAL,
				Point(object.at(key_x0).as_int64(), object.at(key_y0).as_int64()),
				Coord(object.at(key_y1).as_int64()) };
		}
	}

	model::Game LoadGame(const std::filesystem::path& json_path) {
		model::Game game;
		std::ifstream fjson(json_path);

		assert(fjson.is_open());

		std::stringstream buffer;
		buffer << fjson.rdbuf();

		const auto value = json::parse(buffer.str());
		
		//Скорость персонажей
		double default_dog_speed = 1.;
		double dog_speed = default_dog_speed;
		if (value.as_object().contains(key_default_dog_speed)) {
			default_dog_speed = value.as_object().at(key_default_dog_speed).as_double();
		}

		//Вместимость рюкзаков на всех картах 
		int def_bag_capacity = 3;
		int bag_capacity = def_bag_capacity;
		if (value.as_object().contains(key_def_bag_capacity)) {
			def_bag_capacity = value.as_object().at(key_def_bag_capacity).as_int64();
		}

		if (auto f_maps = value.as_object().find(key_maps); f_maps != value.as_object().end()) {
			const auto& maps = value.as_object().at(key_maps);

			const auto& arr = maps.as_array();
			for (const auto& json_map : arr) {

					Map::Id id{ json_map.as_object().at(key_id).as_string().c_str() };

					if (json_map.as_object().contains(key_dog_speed)) {
						dog_speed = json_map.as_object().at(key_dog_speed).as_double();
					}
					else {
						dog_speed = default_dog_speed;
					}

					if (json_map.as_object().contains(key_bag_capacity)) {
						bag_capacity = json_map.as_object().at(key_bag_capacity).as_int64();
					}
					else {
						bag_capacity = def_bag_capacity;
					}

					Map map(id, json_map.as_object().at(key_name).as_string().c_str(), dog_speed, bag_capacity);


					if (json_map.as_object().if_contains(key_roads)) {
						auto roads = json_map.as_object().at(key_roads).as_array();

						for (auto& road : roads) {
							map.AddRoad(std::move(CreateRoad(road.as_object())));
						}
					}

					if (json_map.as_object().if_contains(key_buildings)) {
						auto buildings = json_map.as_object().at(key_buildings).as_array();

						for (auto& building : buildings) {
							map.AddBuilding(std::move(CreateBuilding(building.as_object())));
						}
					}

					if (json_map.as_object().if_contains(key_offices)) {
						auto offices = json_map.as_object().at(key_offices).as_array();

						for (auto& office : offices) {
							map.AddOffice(std::move(CreateOffice(office.as_object())));
						}
					}
				
					if (json_map.as_object().contains(extra_data::key_loot_types)) {
						const auto& loot_types = json_map.as_object().at(extra_data::key_loot_types).as_array();
						
						for (auto loot : loot_types) {

							extra_data::LootDescription new_loot;

							if (loot.as_object().contains(key_name)) {
								new_loot.name_ = loot.as_object().at(key_name).as_string().c_str();
							}
							if (loot.as_object().contains(extra_data::key_file)) {
								new_loot.file_path_ = loot.as_object().at(extra_data::key_file).as_string().c_str();
							}
							if (loot.as_object().contains(extra_data::key_type)) {
								new_loot.type_ = loot.as_object().at(extra_data::key_type).as_string().c_str();
							}
							if (loot.as_object().contains(extra_data::key_rotation)) {
								new_loot.rotation_ = static_cast<double>(loot.as_object().at(extra_data::key_rotation).as_int64());
							}
							if (loot.as_object().contains(extra_data::key_color)) {
								new_loot.color_ = loot.as_object().at(extra_data::key_color).as_string().c_str();
							}
							if (loot.as_object().contains(extra_data::key_scale)) {
								new_loot.scale_ = loot.as_object().at(extra_data::key_scale).as_double();
							}
							
							map.AddLootDescription(std::move(new_loot));
						}
					}

					

					game.AddMap(map);
			}
			
			if (value.as_object().contains(key_loot_gen_config)) {
				const auto& generator = value.as_object().at(key_loot_gen_config);
				const auto& period_sec = generator.as_object().at(key_period).as_double();
				const auto& probability = generator.as_object().at(key_probability).as_double();

				auto period_ms = static_cast<loot_gen::LootGenerator::TimeInterval::rep>(std::round(period_sec * 1000));
				loot_gen::LootGenerator::TimeInterval period(period_ms);
				game.AddLootGenerator(loot_gen::LootGenerator{ period, probability });
			}
			
		}
		return game;
	}

}  // namespace json_loader
