#pragma once
// boost.beast будет использовать std::string_view вместо boost::string_view
#define BOOST_BEAST_USE_STD_STRING_VIEW
#include "http_server.h"
#include "model.h"
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


	// Запрос, тело которого представлено в виде строки
	using StringRequest = http::request<http::string_body>;
	// Ответ, тело которого представлено в виде строки
	using StringResponse = http::response<http::string_body>;

	using FileResponse = http::response<http::file_body>;
	using Response = std::variant<StringResponse, FileResponse>;


	const auto json_not_found = R"({"code": "mapNotFound", "message" : "Map not found"})";
	const auto bad_request = R"({"code": "badRequest", "message" : "Bad request"})";


	const std::string_view api = "/api/"sv;
	const std::string_view prefix = "/api/v1/maps"sv;


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

	class RequestHandler {
	public:
		explicit RequestHandler(model::Game& game, const std::filesystem::path& base_path)
			: game_{ game }
			, base_path_(base_path) {
		}

		RequestHandler(const RequestHandler&) = delete;
		RequestHandler& operator=(const RequestHandler&) = delete;


		template <typename Body, typename Allocator>
		Response operator()(http::request<Body, http::basic_fields<Allocator>>&& req) {
			std::string_view uri(req.target().data(), req.target().size());
			Response resp;

			if (uri.compare(0, api.size(), api) != 0) {

				auto url_encoded = UrlEncoded(uri);

				if (!url_encoded.empty()) {

					fs::path abs_path = GetAbsPath(base_path_.string(), url_encoded);

					if (!IsSubPath(abs_path, base_path_.string())) {
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
							abs_path = base_path_.string() + "/index.html"s;
						}

						if (fs::exists(abs_path)) {
							return MakeFileResponse(http::status::ok, abs_path, req.version(), req.keep_alive(), mime_type);
						}
						else {
							return MakeStringResponse(http::status::not_found, json_not_found, req.version(), req.keep_alive(), ContentType::TEXT_PLAIN);
						}
					}

				}

			}
			else {
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
						return MakeStringResponse(http::status::ok, json::serialize(arr), req.version(), req.keep_alive(), ContentType::APP_JSON);
					}
					else if (auto map = game_.FindMap(model::Map::Id(std::string(map_name).substr(1))); map != nullptr) {
						json::object obj;
						obj[key_id] = *(map->GetId());
						obj[key_name] = map->GetName();

						AddRoadsToJson(obj, map);
						AddBuildingsToJson(obj, map);
						AddOfficesToJson(obj, map);

						return MakeStringResponse(http::status::ok, json::serialize(obj), req.version(), req.keep_alive(), ContentType::APP_JSON);
					}
					else {
						return MakeStringResponse(http::status::not_found, json_not_found, req.version(), req.keep_alive(), ContentType::APP_JSON);
					}
				}
				else {
					return MakeStringResponse(http::status::bad_request, bad_request, req.version(), req.keep_alive(), ContentType::APP_JSON);

				}
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


		FileResponse MakeFileResponse(http::status status, fs::path abs_path, unsigned http_version,
			bool keep_alive,
			std::string_view content_type);

	private:
		model::Game& game_;
		const std::filesystem::path base_path_;
	};


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

		template <typename Body, typename Allocator, typename Endpoint>
		auto operator () (http::request<Body, http::basic_fields<Allocator>>&& req, Endpoint endpoint) {

			LogRequest(req, endpoint);

			auto start_time = std::chrono::steady_clock::now();
			auto resp = decorated_(std::move(req));
			response_time_ = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time).count();

			if (auto str_response = std::get_if<http_handler::StringResponse>(&resp)) {
				LogResponse(*str_response, response_time_);
			}
			else if (auto file_response = std::get_if<http_handler::FileResponse>(&resp)) {
				LogResponse(*file_response, response_time_);
			}

			return resp;
		}

		LoggingRequestHandler(SomeRequestHandler& handler)
			: decorated_(handler) {
		}


	private:
		SomeRequestHandler& decorated_;
		long long response_time_;
	};

}  // namespace http_handler
