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

	FileResponse RequestHandler::MakeFileResponse(http::status status, fs::path abs_path, unsigned http_version,
		bool keep_alive,
		std::string_view content_type) {

		http::file_body::value_type file;
		FileResponse response(status, http_version);
		response.set(http::field::content_type, content_type.data());

		if (sys::error_code ec_; response.body().open(abs_path.string().c_str(), beast::file_mode::read, ec_), ec_) {
			std::cout << "Failed to open file "sv << abs_path.string() << std::endl;
		}

		response.prepare_payload();
		response.content_length(response.body().size());
		response.keep_alive(keep_alive);
		return response;
	}
		
}  // namespace http_handler
