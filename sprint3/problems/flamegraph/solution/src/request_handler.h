#pragma once
#include "http_server.h"
#include "model.h"
#include <boost/json.hpp>
// boost.beast будет использовать std::string_view вместо boost::string_view
#define BOOST_BEAST_USE_STD_STRING_VIEW

namespace http_handler {
	namespace beast = boost::beast;
	namespace http = beast::http;
	namespace json = boost::json;
	using namespace std::literals;
	using namespace model::model_details;

	// Запрос, тело которого представлено в виде строки
	using StringRequest = http::request<http::string_body>;
	// Ответ, тело которого представлено в виде строки
	using StringResponse = http::response<http::string_body>;
	const auto json_not_found = R"({"code": "mapNotFound", "message" : "Map not found"})";
	const auto bad_request = R"({"code": "badRequest", "message" : "Bad request"})";

	struct ContentType {
		ContentType() = delete;
		constexpr static std::string_view TEXT_HTML = "text/html"sv;
		constexpr static std::string_view APP_JSON = "application/json"sv;
	};

	class RequestHandler {
	public:
		explicit RequestHandler(model::Game& game)
			: game_{ game } {
		}

		RequestHandler(const RequestHandler&) = delete;
		RequestHandler& operator=(const RequestHandler&) = delete;

	
		template <typename Body, typename Allocator, typename Send>
		void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {

			std::string_view prefix = "/api/v1/maps"sv;

			std::string_view uri(req.target().data(), req.target().size());

			if (uri.compare(0, prefix.size(), prefix) == 0) {
				std::string_view  map_name = uri.substr(prefix.size());
				auto maps = game_.GetMaps();

				if (map_name.empty()) {
					json::array arr;

					for (auto& map : maps) {
						json::object obj;
						obj[key_id] = *(map.GetId());
						obj[key_name] = map.GetName();
						arr.push_back(obj);
					}
					send(MakeStringResponse(http::status::ok, json::serialize(arr), req.version(), req.keep_alive()));
				}
				else if (auto map = game_.FindMap(model::Map::Id(std::string(map_name).substr(1))); map != nullptr) {
					json::object obj;
					obj[key_id] = *(map->GetId());
					obj[key_name] = map->GetName();
			
					AddRoadsToJson(obj, map);
					AddBuildingsToJson(obj, map);
					AddOfficesToJson(obj, map);

					send(MakeStringResponse(http::status::ok, json::serialize(obj), req.version(), req.keep_alive()));
				}
				else {
					send(MakeStringResponse(http::status::not_found, json_not_found, req.version(), req.keep_alive()));
				}
			}
			else {
				send(MakeStringResponse(http::status::bad_request, bad_request, req.version(), req.keep_alive()));
			}
		}

	private:
		void AddRoadsToJson(json::object& obj, const model::Map* map);

		void AddBuildingsToJson(json::object& obj, const model::Map* map);

		void AddOfficesToJson(json::object& obj, const model::Map* map);

		template <typename Body>
		StringResponse MakeStringResponse(http::status status, Body&& body, unsigned http_version,
			bool keep_alive,
			std::string_view content_type = ContentType::APP_JSON) {
			StringResponse response(status, http_version);
			response.set(http::field::content_type, content_type.data());
			response.body() = std::forward<Body>(body);
			response.content_length(response.body().size());
			response.keep_alive(keep_alive);
			return response;
		}

	private:
		model::Game& game_;
	};

}  // namespace http_handler
