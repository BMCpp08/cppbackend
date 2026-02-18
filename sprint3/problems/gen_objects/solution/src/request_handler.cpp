#include "request_handler.h"

namespace http_handler {

	StringResponse MakeStringResponse(http::status status, std::string_view body, unsigned http_version,
		bool keep_alive,
		std::string_view content_type) {

		StringResponse response(status, http_version);
		response.set(http::field::content_type, content_type.data());
		response.set(http::field::cache_control, "no-cache"sv);
		response.body() = body;
		response.content_length(response.body().size());
		response.keep_alive(keep_alive);

		return response;
	}


	std::string GetContentType(const StringRequest& req) {
		auto content_type_it = req.base().find(boost::beast::http::field::content_type);
		std::string content_type = "null"s;

		if (content_type_it != req.base().end()) {
			content_type = content_type_it->value();
		}
		return content_type;
	}

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

	json::array ApiHandler::LoadLootTypes(const model::Game::MapPtr map) const {
		json::array loot_types;

		for (auto& loots : map->GetLoots()) {
			loot_types.emplace_back(
		

				json::object{{key_name, loots.second->GetName()},
							{extra_data::key_file, loots.second->GetFilePath()},
							{extra_data::key_type, loots.second->GetType()},
							{extra_data::key_rotation, loots.second->GetRotation()},
							{extra_data::key_color, loots.second->GetColor()},
							{extra_data::key_scale, loots.second->GetScale()},
				});
		}

		return loot_types;
	}

	bool ApiHandler::IsApiRequest(StringRequest req) {
		std::string_view uri(req.target().data(), req.target().size());
		return uri.find(api, 0) == 0;
	}

	StringResponse ApiHandler::HandlerApiHandler(const StringRequest& req) const {

		std::string_view uri(req.target().data(), req.target().size());
		auto content_type = GetContentType(req);
		StringResponse resp;

		//Вход в игру
		if (!uri.compare(0, api_post_join.size(), api_post_join)) {

			if (req.method() != http::verb::post && req.method() != http::verb::get && req.method() != http::verb::head) {

				resp = MakeStringResponse(http::status::method_not_allowed, post_method_error, req.version(), req.keep_alive(), ContentType::APP_JSON);
				resp.set(http::field::allow, REQ_POST);
				return resp;
			}
			else if (content_type == ContentType::APP_JSON) {//Для входа в игру 
				try {
					auto body_str = req.body();
					auto json_obj = json::parse(body_str).as_object();

					auto user_name = json_obj.at("userName").as_string().c_str();
					std::string map_id = json_obj.at("mapId").as_string().c_str();

					auto res_join = app_.JoinGame(map_id, user_name);

					json::object obj;
					obj[key_auth_token] = *(res_join.GetPlayerTokens());
					obj[key_player_id] = *(res_join.GetPlayerId());

					resp = MakeStringResponse(http::status::ok, json::serialize(obj), req.version(), req.keep_alive(), ContentType::APP_JSON);
				}
				catch (app::GameError<app::JoinGameErrorReason> err) {

					if (err.GetErrorReason() == app::INVALIDE_NAME) {//несуществующий id карты
						resp = MakeStringResponse(http::status::bad_request, json_invalid_name, req.version(), req.keep_alive(), ContentType::APP_JSON);
					}
					else if (err.GetErrorReason() == app::INVALIDE_MAP) {//пустое имя игрока,
						resp = MakeStringResponse(http::status::not_found, json_not_found, req.version(), req.keep_alive(), ContentType::APP_JSON);
					}
				}
				catch (...) {//Если при парсинге JSON или получении его свойств произошла ошибка
					resp = MakeStringResponse(http::status::bad_request, json_parse_error, req.version(), req.keep_alive(), ContentType::APP_JSON);
				}

			}
		}
		else if (!uri.compare(0, api_get_map.size(), api_get_map)) {//полчение списка карт
			std::string_view  map_name = uri.substr(api_get_map.size());
			auto maps = app_.GetMaps();

			if (auto content_type = req[boost::beast::http::field::content_type]; !content_type.empty()) {
				resp = MakeStringResponse(http::status::bad_request, bad_request, req.version(), req.keep_alive(), ContentType::APP_JSON);
				return resp;
			}
			else if (map_name.empty()) {
				json::array arr;
				for (auto& map : maps) {
					json::object obj;
					obj[key_id] = *(map->GetId());
					obj[key_name] = map->GetName();
					arr.push_back(obj);
				}
				resp = MakeStringResponse(http::status::ok, json::serialize(arr), req.version(), req.keep_alive(), ContentType::APP_JSON);
			}
			else if (auto map = app_.FindMap(model::Map::Id(std::string(map_name).substr(1))); map != nullptr) {
				json::object obj;
				obj[key_id] = *(map->GetId());
				obj[key_name] = map->GetName();
				obj[key_roads] = LoadRoadsToJson(map);
				obj[key_buildings] = LoadBuildingsToJson(map);
				obj[key_offices] = LoadOfficesToJson(map);
				obj[extra_data::key_loot_types] = LoadLootTypes(map);

				resp = MakeStringResponse(http::status::ok, json::serialize(obj), req.version(), req.keep_alive(), ContentType::APP_JSON);
			}
			else {
				resp = MakeStringResponse(http::status::not_found, json_not_found, req.version(), req.keep_alive(), ContentType::APP_JSON);
			}
		}
		else if (!uri.compare(0, api_get_players.size(), api_get_players)) {//Получение списка игроков

			std::string_view uri(req.target().data(), req.target().size());
			auto content_type = GetContentType(req);

			if (req.method() != http::verb::get && req.method() != http::verb::head) {
				resp = MakeStringResponse(http::status::method_not_allowed, invalid_method_error, req.version(), req.keep_alive(), ContentType::APP_JSON);
				resp.set(http::field::allow, "GET, HEAD"sv);
				return resp;

			}
			else if (content_type == ContentType::APP_JSON) {//Для входа в игру 

				try {
					auto authorization = req[http::field::authorization];

					resp = MakeStringResponse(http::status::ok,
						app_.GetPlayers(authorization),
						req.version(), req.keep_alive(),
						ContentType::APP_JSON);
				}
				catch (app::GameError<app::AuthorizationGameErrorReason> err) {

					if (err.GetErrorReason() == app::AuthorizationGameErrorReason::AUTHORIZATION_HEADER_MISSING) {
						resp = MakeStringResponse(http::status::unauthorized, authorization_method_missing, req.version(), req.keep_alive(), ContentType::APP_JSON);
					}
					else if (err.GetErrorReason() == app::AuthorizationGameErrorReason::AUTHORIZATION_TOKEN_NOT_FOUND) {
						resp = MakeStringResponse(http::status::unauthorized, token_not_found, req.version(), req.keep_alive(), ContentType::APP_JSON);
					}

				}
				catch (const std::exception& exc) {
					throw std::runtime_error(std::string("Error HandlerApiHandler: ") + exc.what());
				}
			}
		}
		else if (!uri.compare(0, api_get_game_state.size(), api_get_game_state)) {//Запрос игрового состояния

			auto content_type = GetContentType(req);

			if (req.method() != http::verb::get && req.method() != http::verb::head) {
				resp = MakeStringResponse(http::status::method_not_allowed, invalid_method_error, req.version(), req.keep_alive(), ContentType::APP_JSON);
				resp.set(http::field::allow, "GET, HEAD"sv);
				return resp;
			}
			else {
				try {
					auto authorization = req[http::field::authorization];

					resp = MakeStringResponse(http::status::ok,
						app_.GetGameState(authorization),
						req.version(),
						req.keep_alive(),
						ContentType::APP_JSON);
				}
				catch (app::GameError<app::AuthorizationGameErrorReason> err) {

					if (err.GetErrorReason() == app::AuthorizationGameErrorReason::AUTHORIZATION_TOKEN_NOT_FOUND) {

						resp = MakeStringResponse(http::status::unauthorized, token_not_found, req.version(), req.keep_alive(), ContentType::APP_JSON);
					}
					else if (err.GetErrorReason() == app::AuthorizationGameErrorReason::AUTHORIZATION_HEADER_REQ) {
						resp = MakeStringResponse(http::status::unauthorized, authorization_header_req, req.version(), req.keep_alive(), ContentType::APP_JSON);
					}
				}
				catch (const std::exception& exc) {
					throw std::runtime_error(std::string("Error HandlerApiHandler: ") + exc.what());
				}
			}

		}
		else if (!uri.compare(0, api_game_player_action.size(), api_game_player_action)) {//Управление действиями своего персонажа

			auto content_type = GetContentType(req);
			if (req.method() != http::verb::post) {
				resp = MakeStringResponse(http::status::method_not_allowed, invalid_method_error, req.version(), req.keep_alive(), ContentType::APP_JSON);
				resp.set(http::field::allow, REQ_POST);
				return resp;
			}
			else if (content_type == ContentType::APP_JSON) {
				try {
					auto authorization = req[http::field::authorization];

					resp = MakeStringResponse(http::status::ok,
						app_.SetPlayerAction(authorization, req.body()),
						req.version(),
						req.keep_alive(),
						ContentType::APP_JSON);
				}
				catch (app::GameError<app::AuthorizationGameErrorReason> err) {

					if (err.GetErrorReason() == app::AuthorizationGameErrorReason::AUTHORIZATION_TOKEN_NOT_FOUND) {

						resp = MakeStringResponse(http::status::unauthorized, token_not_found, req.version(), req.keep_alive(), ContentType::APP_JSON);
					}
					else if (err.GetErrorReason() == app::AuthorizationGameErrorReason::AUTHORIZATION_HEADER_REQ) {
						resp = MakeStringResponse(http::status::unauthorized, authorization_header_req, req.version(), req.keep_alive(), ContentType::APP_JSON);
					}
				}
				catch (...) {//Если при парсинге JSON или получении его свойств произошла ошибка
					resp = MakeStringResponse(http::status::bad_request, failed_parse_action, req.version(), req.keep_alive(), ContentType::APP_JSON);
				}
			}
			else {
				resp = MakeStringResponse(http::status::bad_request, invalid_content_type, req.version(), req.keep_alive(), ContentType::APP_JSON);
			}
		}
		else if (!uri.compare(0, api_game_tick.size(), api_game_tick)) {

			if (api_ignore_list_.count(api_game_tick) && api_ignore_list_.at(api_game_tick)) {
				resp = MakeStringResponse(http::status::bad_request, bad_request_invalid_endpoint, req.version(), req.keep_alive(), ContentType::APP_JSON);
			}
			else if (req.method() != http::verb::post) {
				resp = MakeStringResponse(http::status::method_not_allowed, invalid_method_error, req.version(), req.keep_alive(), ContentType::APP_JSON);
				resp.set(http::field::allow, REQ_POST);
			}
			else {
				try {

					resp = MakeStringResponse(http::status::ok,
						app_.SetTimeDelta(req.body()),
						req.version(),
						req.keep_alive(),
						ContentType::APP_JSON);
				}
				catch (...) {
					resp = MakeStringResponse(http::status::bad_request, invalid_tick_req, req.version(), req.keep_alive(), ContentType::APP_JSON);
				}
			}
		}
		else {
			resp = MakeStringResponse(http::status::bad_request, bad_request, req.version(), req.keep_alive(), ContentType::APP_JSON);
		}
		return resp;
	}

	void ApiHandler::AddApiIgnore(std::string_view api, bool is_ignore) {
		api_ignore_list_[api] = is_ignore;
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
