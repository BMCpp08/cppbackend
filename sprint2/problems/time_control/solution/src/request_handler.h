#pragma once
// boost.beast будет использовать std::string_view вместо boost::string_view
#define BOOST_BEAST_USE_STD_STRING_VIEW
#include "http_server.h"
#include "app.h"
#include <boost/json.hpp>
#include "logger.h"

#include <filesystem>
#include <cassert>
#include <boost/beast.hpp>
#include <unordered_map>
#include <variant>
#include <chrono>

// boost.beast будет использовать std::string_view вместо boost::string_view
#define BOOST_BEAST_USE_STD_STRING_VIEW


namespace http_handler {
	namespace beast = boost::beast;
	namespace http = beast::http;
	namespace json = boost::json;
	using namespace std::literals;
	using namespace model::model_details;
	namespace fs = std::filesystem;
	namespace sys = boost::system;
	namespace net = boost::asio;
	using namespace logger;
	namespace ip = boost::asio::ip;


	// Запрос, тело которого представлено в виде строки
	using StringRequest = http::request<http::string_body>;

	using StringResponse = http::response<http::string_body>;
	using FileResponse = http::response<http::file_body>;
	using EmptyResponse = http::response<http::empty_body>;

	const auto json_not_found = R"({"code": "mapNotFound", "message" : "Map not found"})";
	const auto json_invalid_name = R"({"code": "invalidArgument", "message": "Invalid name"})";

	const auto bad_request = R"({"code": "badRequest", "message" : "Bad request"})";
	const auto json_parse_error = R"({"code": "invalidArgument", "message": "Join game request parse error"})";
	const auto post_method_error = R"({"code": "invalidMethod", "message": "Only POST method is expected"})";
	const auto invalid_method_error = R"({"code": "invalidMethod", "message": "Invalid method"})";
	const auto failed_parse_action = R"({"code": "invalidArgument", "message": "Failed to parse action"})";
	const auto invalid_content_type = R"({"code": "invalidArgument", "message": "Invalid content type"})";
	const auto invalid_tick_req = R"({"code": "invalidArgument", "message": "Failed to parse tick request JSON"})";
	
	const auto authorization_method_missing = R"({"code": "invalidToken", "message": "Authorization header is missing"})";
	const auto token_not_found = R"({"code": "unknownToken", "message": "Player token has not been found"})";
	const auto authorization_header_req = R"({ "code": "invalidToken", "message" : "Authorization header is required"})";

	/*API*/
	const std::string_view api = "/api"sv;

	const std::string_view api_get_map = "/api/v1/maps"sv; //полчение списка карт
	const std::string_view api_post_join = "/api/v1/game/join"sv; //Для входа в игру 
	const std::string_view api_get_players = "/api/v1/game/players"sv; //Получение списка игроков
	const std::string_view api_get_game_state = "/api/v1/game/state"sv; //Запрос игрового состояния
	const std::string_view api_game_player_action = "/api/v1/game/player/action"sv; //Управление действиями своего персонажа
	const std::string_view api_game_tick = "/api/v1/game/tick"sv; //Установить время

	const std::string_view req_get = "GET"sv;
	const std::string_view req_head = "HEAD"sv;


	struct ContentType {
		ContentType() = delete;
		constexpr static std::string_view TEXT_HTML = "text/html"sv;
		constexpr static std::string_view TEXT_CSS = "text/css"sv;
		constexpr static std::string_view TEXT_PLAIN = "text/plain"sv;
		constexpr static std::string_view TEXT_JAVASCRIPT = "text/javascript"sv;

		constexpr static std::string_view APP_JSON = "application/json"sv;
		constexpr static std::string_view APP_XML = "application/xml"sv;
		constexpr static std::string_view APP_OCTET = "application/octet-stream"sv;

		constexpr static std::string_view IMAGE_PNG = "image/png"sv;
		constexpr static std::string_view IMAGE_JPEG = "image/jpeg"sv;
		constexpr static std::string_view IMAGE_GIF = "image/gif"sv;
		constexpr static std::string_view IMAGE_BMP = "image/bmp"sv;
		constexpr static std::string_view IMAGE_M_ICON = "image/vnd.microsoft.icon"sv;
		constexpr static std::string_view IMAGE_TIFF = "image/tiff"sv;
		constexpr static std::string_view IMAGE_SVG_XML = "image/svg+xml"sv;

		constexpr static std::string_view AUDIO_MPEG = "audio/mpeg"sv;
	};

	static std::unordered_map<std::string_view, std::string_view> file_ext{ {"html"sv, ContentType::TEXT_HTML},
																{"htm"sv, ContentType::TEXT_HTML},
																{"css"sv, ContentType::TEXT_CSS},
																{"plain"sv, ContentType::TEXT_PLAIN},
																{"javascript"sv, ContentType::TEXT_JAVASCRIPT},
																{"json"sv, ContentType::APP_JSON},
																{"xml"sv, ContentType::APP_XML},
																{"png"sv, ContentType::IMAGE_PNG},
																{"jpeg"sv, ContentType::IMAGE_JPEG},
																{"jpe"sv, ContentType::IMAGE_JPEG},
																{"jpg"sv, ContentType::IMAGE_JPEG},
																{"gif"sv, ContentType::IMAGE_GIF},
																{"bmp"sv, ContentType::IMAGE_BMP},
																{"vnd.microsoft.icon"sv, ContentType::IMAGE_M_ICON},
																{"tiff"sv, ContentType::IMAGE_TIFF},
																{"tif"sv, ContentType::IMAGE_TIFF},
																{"svgz"sv, ContentType::IMAGE_SVG_XML},
																{"svg"sv, ContentType::IMAGE_SVG_XML},
																{"mpeg"sv, ContentType::AUDIO_MPEG} };

	// Возвращает true, если каталог p содержится внутри base_path.
	bool IsSubPath(fs::path path, fs::path base);

	fs::path GetAbsPath(fs::path base_path, std::string_view rel_path);

	std::string UrlEncoded(const std::string_view& uri);

	std::string GetExtType(fs::path path);

	std::string_view GetMimeType(std::unordered_map<std::string_view, std::string_view>& file_ext, std::string_view ext);

	inline std::string GetContentType(const StringRequest& req) {
		auto content_type_it = req.base().find(boost::beast::http::field::content_type);
		std::string content_type = "null"s;

		if (content_type_it != req.base().end()) {
			content_type = content_type_it->value();
		}
		return content_type;
	}

	inline StringResponse MakeStringResponse(http::status status, std::string_view body, unsigned http_version,
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

	class ApiHandler {
	private:
		json::array LoadRoadsToJson(const model::Game::MapPtr map) const;

		json::array LoadBuildingsToJson(const model::Game::MapPtr map) const;

		json::array LoadOfficesToJson(const model::Game::MapPtr map) const;
	public:
		ApiHandler(app::Application& app)
			:app_(app) {
		}

		bool IsApiRequest(StringRequest req) {
			std::string_view uri(req.target().data(), req.target().size());
			return uri.find(api, 0) == 0;
		}

		StringResponse HandlerApiHandler(const StringRequest req) const {

			std::string_view uri(req.target().data(), req.target().size());
			auto content_type = GetContentType(req);
			StringResponse resp;
			
			//Вход в игру
			if (!uri.compare(0, api_post_join.size(), api_post_join)) {

				if (req.method() != http::verb::post && req.method() != http::verb::get && req.method() != http::verb::head) {

					resp = MakeStringResponse(http::status::method_not_allowed, post_method_error, req.version(), req.keep_alive(), ContentType::APP_JSON);
					resp.set(http::field::allow, "POST"sv);
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

						auto dogs = app_.AuthorizationGame(authorization);

						json::object obj;
						for (const auto& dog : dogs) {
							obj[std::to_string(dog.second->GetId())] = json::value{ {"name"s,dog.second->GetName()} };
						}

						resp = MakeStringResponse(http::status::ok, json::serialize(obj), req.version(), req.keep_alive(), ContentType::APP_JSON);
					}
					catch (app::GameError<app::AuthorizationGameErrorReason> err) {
						
						if (err.GetErrorReason() == app::AuthorizationGameErrorReason::AUTHORIZATION_HEADER_MISSING) {
							resp = MakeStringResponse(http::status::unauthorized, authorization_method_missing, req.version(), req.keep_alive(), ContentType::APP_JSON);
						}
						else if (err.GetErrorReason() == app::AuthorizationGameErrorReason::AUTHORIZATION_TOKEN_NOT_FOUND) {
							resp = MakeStringResponse(http::status::unauthorized, token_not_found, req.version(), req.keep_alive(), ContentType::APP_JSON);
						}

					}
					catch (...) {//Если при парсинге JSON или получении его свойств произошла ошибка
						
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
					catch (...) {//Если при парсинге JSON или получении его свойств произошла ошибка
						
					}
				}

			}
			else if (!uri.compare(0, api_game_player_action.size(), api_game_player_action)) {//Управление действиями своего персонажа

				auto content_type = GetContentType(req);
				if (req.method() != http::verb::post) {
					resp = MakeStringResponse(http::status::method_not_allowed, invalid_method_error, req.version(), req.keep_alive(), ContentType::APP_JSON);
					resp.set(http::field::allow, "POST"sv);
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

				auto content_type = GetContentType(req);

				if (req.method() != http::verb::post) {
					resp = MakeStringResponse(http::status::method_not_allowed, invalid_method_error, req.version(), req.keep_alive(), ContentType::APP_JSON);
					resp.set(http::field::allow, "POST"sv);
					return resp;
				}
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
				

			} else {
				resp = MakeStringResponse(http::status::bad_request, bad_request, req.version(), req.keep_alive(), ContentType::APP_JSON);
			}
			return resp;
		}


	private:
		app::Application& app_;

	};
	

	class RequestHandler : public std::enable_shared_from_this<RequestHandler> {

		using FileRequestResult = std::variant<EmptyResponse, StringResponse, FileResponse>;
	public:
		using Strand = net::strand<net::io_context::executor_type>;

		explicit RequestHandler(fs::path root, Strand api_strand, ApiHandler& api_handler)
			: root_{ std::move(root) }
			, api_strand_{ api_strand }
			, api_handler_{ api_handler } {
		}

		RequestHandler(const RequestHandler&) = delete;
		RequestHandler& operator=(const RequestHandler&) = delete;

		template <typename Body, typename Allocator, typename Send>
		void operator()(boost::asio::ip::tcp::endpoint, http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
			auto version = req.version();
			auto keep_alive = req.keep_alive();
			std::string_view uri(req.target().data(), req.target().size());

			try {
				if (api_handler_.IsApiRequest(req)) {

					auto handle = [self = shared_from_this(), send,
						req = std::forward<decltype(req)>(req), version, keep_alive] {
						try {
							assert(self->api_strand_.running_in_this_thread());
							return send(self->api_handler_.HandlerApiHandler(req));
						}
						catch (...) {
							send(self->ReportServerError(version, keep_alive));
						}
						};
					return net::dispatch(api_strand_, handle);
				}
				return std::visit(
					[&send](auto&& result) {
						send(std::forward<decltype(result)>(result));
					},
					HandleFileRequest(req));

			}
			catch (...) {
				send(ReportServerError(version, keep_alive));
			}

		}
	private:

		fs::path root_;
		Strand api_strand_;
		ApiHandler api_handler_;

	private:
		StringResponse ReportServerError(unsigned version, bool keep_alive) {

			StringResponse response(http::status::internal_server_error, version);
			response.set(http::field::cache_control, "no-cache"sv);
			response.body() = "Error";
			response.content_length(response.body().size());
			response.keep_alive(keep_alive);

			return response;
		}

		FileResponse MakeFileResponse(http::status status, fs::path abs_path, unsigned http_version,
			bool keep_alive,
			std::string_view content_type) const;

		FileRequestResult HandleFileRequest(const StringRequest& req) const {

			std::string_view uri(req.target().data(), req.target().size());
			auto url_encoded = UrlEncoded(uri);

			if (!url_encoded.empty()) {

				fs::path abs_path = GetAbsPath(root_.string(), url_encoded);

				if (!IsSubPath(abs_path, root_.string())) {
					return MakeStringResponse(http::status::not_found, bad_request, req.version(), req.keep_alive(), ContentType::TEXT_PLAIN);
				}
				else {

					auto ext = GetExtType(uri);
					std::string_view mime_type;


					if (!uri.empty() && uri.back() != '/') {
						mime_type = GetMimeType(file_ext, ext);
					}
					else {
						mime_type = ContentType::TEXT_HTML;
						abs_path = root_.string() + "/index.html"s;
					}

					if (fs::exists(abs_path)) {
						return MakeFileResponse(http::status::ok, abs_path, req.version(), req.keep_alive(), mime_type);
					}
					else {
						return MakeStringResponse(http::status::not_found, json_not_found, req.version(), req.keep_alive(), ContentType::TEXT_PLAIN);
					}
				}
			}
			else {
				return MakeStringResponse(http::status::not_found, json_not_found, req.version(), req.keep_alive(), ContentType::TEXT_PLAIN);
			}
		}
	};

	namespace server_logging {
		template<class SomeRequestHandler>
		class LoggingRequestHandler {
			static void LogRequest(const auto& req, const auto& endpoint) {

				std::string msg{ key_request_received };

				boost::json::value custom_data{ {key_ip,endpoint.address().to_string() },
												{key_uri,req.target()},
												{key_method,req.method_string()} };

				BOOST_LOG_TRIVIAL(info) << logging::add_value(data, custom_data)
					<< logging::add_value(message, msg);

			}

			static void LogResponse(const auto& resp, long long response_time) {

				std::string msg{ key_response_sent };
				auto content_type_it = resp.base().find(boost::beast::http::field::content_type);
				std::string content_type = "null"s;

				if (content_type_it != resp.base().end()) {
					content_type = content_type_it->value();
				}

				boost::json::value custom_data{ {key_response_time, std::to_string(response_time) },
												{key_code, resp.result_int()},
												{key_content_type, content_type} };

				BOOST_LOG_TRIVIAL(info) << logging::add_value(data, custom_data)
					<< logging::add_value(message, msg);
			}

		public:

			template <typename Body, typename Allocator, typename Send>
			void operator()(boost::asio::ip::tcp::endpoint endpoint, http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {

				LogRequest(req, endpoint);

				auto s__ = [endpoint, req, send, this](auto&& resp) {
					auto start_time = std::chrono::steady_clock::now();
					LogResponse(std::forward<decltype(resp)>(resp), std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time).count());
					send(resp);

					};

				decorated_(endpoint, std::move(req), std::move(s__));
			}

			explicit LoggingRequestHandler(SomeRequestHandler& handler)
				: decorated_(handler) {
			}


		private:
			SomeRequestHandler& decorated_;
			long long response_time_;
		};


	}
}  // namespace http_handler
