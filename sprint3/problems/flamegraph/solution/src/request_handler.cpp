#include "request_handler.h"

namespace http_handler {

	void RequestHandler::AddRoadsToJson(json::object& obj, const model::Map* map) {
		json::array roads;

		for (auto& road : map->GetRoads()) {
			roads.emplace_back(
				json::object{ {key_x0, road.GetStart().x},
				{key_y0, road.GetStart().y},
				{road.IsHorizontal() ? key_x1 : key_y1, road.IsHorizontal() ? road.GetEnd().x : road.GetEnd().y} });
		}

		obj[key_roads] = roads;
	}

	void RequestHandler::AddBuildingsToJson(json::object& obj, const model::Map* map) {
		json::array buildings;
		for (auto& building : map->GetBuildings()) {
			buildings.emplace_back(
				json::object{ {key_x,building.GetBounds().position.x}, {key_y, building.GetBounds().position.y},
				{key_w, building.GetBounds().size.width}, {key_h, building.GetBounds().size.height} });
		}

		obj[key_buildings] = buildings;
	}

	void RequestHandler::AddOfficesToJson(json::object& obj, const model::Map* map) {
		json::array offices;
		for (auto& office : map->GetOffices()) {
			offices.emplace_back(json::object{ {key_id, *(office.GetId())},
				{key_x, office.GetPosition().x}, {key_y,  office.GetPosition().y},
				{key_offset_x, office.GetOffset().dx}, {key_offset_y, office.GetOffset().dy}
				});
		}
		obj[key_offices] = offices;
	}
		
		
}  // namespace http_handler
