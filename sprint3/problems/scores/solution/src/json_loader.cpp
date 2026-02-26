#include "json_loader.h"
#include <iostream>
#include <cassert>
#include "boost_includes.h"
#include <cmath>

namespace json_loader {
	using namespace std::literals;
	using namespace model;
	using namespace model_details;
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
		if (auto it = value.as_object().if_contains(key_default_dog_speed); it) {
			default_dog_speed = it->as_double();
		}

		//Вместимость рюкзаков на всех картах 
		int def_bag_capacity = 3;
		int bag_capacity = def_bag_capacity;
		if (auto it = value.as_object().if_contains(key_def_bag_capacity); it) {
			def_bag_capacity = it->as_int64();
		}

		if (auto f_maps = value.as_object().find(key_maps); f_maps != value.as_object().end()) {
			const auto& maps = value.as_object().at(key_maps);

			const auto& arr = maps.as_array();
			for (const auto& json_map : arr) {

					Map::Id id{ json_map.as_object().at(key_id).as_string().c_str() };

					if (auto it = json_map.as_object().if_contains(key_dog_speed); it) {
						dog_speed = it->as_double();
					} else {
						dog_speed = default_dog_speed;
					}

					if (auto it = json_map.as_object().if_contains(key_bag_capacity); it) {
						bag_capacity = it->as_int64();
					} else {
						bag_capacity = def_bag_capacity;
					}

					Map map(id, json_map.as_object().at(key_name).as_string().c_str(), dog_speed, bag_capacity);


					if (auto it = json_map.as_object().if_contains(key_roads); it) {
						auto roads = it->as_array();

						for (auto& road : roads) {
							map.AddRoad(std::move(CreateRoad(road.as_object())));
						}
					}

					if (auto it = json_map.as_object().if_contains(key_buildings); it) {
						auto buildings = it->as_array();

						for (auto& building : buildings) {
							map.AddBuilding(std::move(CreateBuilding(building.as_object())));
						}
					}

					if (auto it = json_map.as_object().if_contains(key_offices); it) {
						auto offices = it->as_array();

						for (auto& office : offices) {
							map.AddOffice(std::move(CreateOffice(office.as_object())));
						}
					}
				
					if (auto it = json_map.as_object().if_contains(key_loot_types); it) {
						const auto& loot_types = it->as_array();
						
						for (auto loot : loot_types) {

							extra_data::LootDescription new_loot;

							if (auto it = loot.as_object().if_contains(key_name); it) {
								new_loot.name_ = it->as_string().c_str();
							}
							if (auto it = loot.as_object().if_contains(key_file); it) {
								new_loot.file_path_ = it->as_string().c_str();
							}
							if (auto it = loot.as_object().if_contains(key_type); it) {
								new_loot.type_ = it->as_string().c_str();
							}
							if (auto it = loot.as_object().if_contains(key_rotation); it) {
								new_loot.rotation_ = it->as_int64();
							}
							if (auto it = loot.as_object().if_contains(key_color); it) {
								new_loot.color_ = it->as_string().c_str();
							}
							if (auto it = loot.as_object().if_contains(key_scale); it) {
								new_loot.scale_ = it->as_double();
							}
							if (auto it = loot.as_object().if_contains(key_value); it) {
								new_loot.value_ = it->as_int64();
							}
							map.AddLootDescription(std::move(new_loot));
						}
					}
					game.AddMap(map);
			}
			
			if (auto it = value.as_object().if_contains(key_loot_gen_config); it) {
				const auto& generator = *it;
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
