#include "request_handler.h"

namespace http_handler {

	json::array ApiHandler::LoadRoadsToJson(const model::Game::MapPtr map) const {
		json::array roads;

		for (auto& road : map->GetRoads()) {
			roads.emplace_back(
				json::object{ {key_x0, road->GetStart().x},
				{key_y0, road->GetStart().y},
				{road->IsHorizontal() ? key_x1 : key_y1, road->IsHorizontal() ? road->GetEnd().x : road->GetEnd().y} });
		}
		return roads;
	}

	json::array ApiHandler::LoadBuildingsToJson(const model::Game::MapPtr map) const {
		json::array buildings;

		for (auto& building : map->GetBuildings()) {
			buildings.emplace_back(
				json::object{ {key_x,building.GetBounds().position.x}, {key_y, building.GetBounds().position.y},
				{key_w, building.GetBounds().size.width}, {key_h, building.GetBounds().size.height} });
		}
		return buildings;
	}

	json::array ApiHandler::LoadOfficesToJson(const model::Game::MapPtr map) const {
		json::array offices;

		for (auto& office : map->GetOffices()) {
			offices.emplace_back(json::object{ {key_id, *(office.GetId())},
				{key_x, office.GetPosition().x}, {key_y,  office.GetPosition().y},
				{key_offset_x, office.GetOffset().dx}, {key_offset_y, office.GetOffset().dy}
				});
		}
		
		return offices;
	}



	FileResponse RequestHandler::MakeFileResponse(http::status status, fs::path abs_path, unsigned http_version,
		bool keep_alive,
		std::string_view content_type) const {

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

	std::string GetExtType(fs::path path) {
		std::string ext;
		path = fs::weakly_canonical(path);
		std::string p = path.extension().string();

		size_t dot_pos = path.extension().string().find_last_of('.');

		if (dot_pos != std::string::npos) {
			ext = p.substr(dot_pos + 1);
		}

		return ext;
	}

	std::string_view GetMimeType(std::unordered_map<std::string_view, std::string_view>& file_ext, std::string_view ext) {
		std::string_view mime_type;
		if (file_ext.find(ext) != file_ext.end()) {
			mime_type = file_ext[ext];
		}
		else {
			mime_type = ContentType::APP_OCTET;
		}
		return mime_type;
	}

	std::string UrlEncoded(const std::string_view& uri) {
		auto uri_size = uri.size();
		std::string res;

		res.reserve(uri_size);

		for (int i = 0; i < uri_size; ++i) {

			if (uri[i] == '+') {
				res += ' ';
			}
			else if (uri[i] == '%') {
				if (i + 2 >= uri_size) {
					return {};
				}

				char ch = static_cast<char>(std::stoi(std::string(uri.substr(i + 1, 2)), nullptr, 16));
				res += ch;
				i += 2;
			}
			else {
				res += uri[i];
			}

		}
		return res;
	}

	// Возвращает true, если каталог p содержится внутри base_path.
	bool IsSubPath(fs::path path, fs::path base) {
		// Приводим оба пути к каноничному виду (без . и ..)
		path = fs::weakly_canonical(path);
		base = fs::weakly_canonical(base);

		// Проверяем, что все компоненты base содержатся внутри path
		for (auto b = base.begin(), p = path.begin(); b != base.end(); ++b, ++p) {
			if (p == path.end() || *p != *b) {
				return false;
			}
		}
		return true;
	}

	fs::path GetAbsPath(fs::path base_path, std::string_view rel_path) {
		return fs::weakly_canonical(base_path / rel_path.substr(1));
	}
}  // namespace http_handler
