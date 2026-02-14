#pragma once
#include "http_server.h"
#include "model.h"
#include <boost/json.hpp>

#include <filesystem>
#include <cassert>
#include <boost/beast.hpp>
#include <unordered_map>

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

	// Запрос, тело которого представлено в виде строки
	using StringRequest = http::request<http::string_body>;
	// Ответ, тело которого представлено в виде строки
	using StringResponse = http::response<http::string_body>;

	using FileResponse = http::response<http::file_body>;

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
	inline bool IsSubPath(fs::path path, fs::path base) {
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

	inline fs::path GetAbsPath(fs::path base_path, std::string_view rel_path) {
		return fs::weakly_canonical(base_path / rel_path.substr(1));

	}

	inline std::string UrlEncoded(const std::string_view& uri) {
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

	inline std::string GetExtType(fs::path path) {
		std::string ext;
		path = fs::weakly_canonical(path);
		std::string p = path.extension().string();

		size_t dot_pos = path.extension().string().find_last_of('.');

		if (dot_pos != std::string::npos) {
			ext = p.substr(dot_pos + 1);
		}

		return ext;
	}

	inline std::string_view GetMimeType(std::unordered_map<std::string_view, std::string_view>& file_ext, std::string_view ext) {
		std::string_view mime_type;
		if (file_ext.find(ext) != file_ext.end()) {
			mime_type = file_ext[ext];
		}
		else {
			mime_type = ContentType::APP_OCTET;
		}
		return mime_type;
	}

	class RequestHandler {
	public:
		explicit RequestHandler(model::Game& game,const std::filesystem::path& base_path)
			: game_{ game }
			, base_path_(base_path) {
		}

		RequestHandler(const RequestHandler&) = delete;
		RequestHandler& operator=(const RequestHandler&) = delete;


		template <typename Body, typename Allocator, typename Send>
		void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
			std::string_view uri(req.target().data(), req.target().size());


			if (uri.compare(0, api.size(), api) != 0) {

				//2
				auto url_encoded = UrlEncoded(uri);
				if (url_encoded.empty()) {
					return;
				}

				//3
				fs::path abs_path = GetAbsPath(base_path_.string(), url_encoded);

				//4
				if (!IsSubPath(abs_path, base_path_.string())) {
					send(MakeStringResponse(http::status::not_found, bad_request, req.version(), req.keep_alive(), ContentType::TEXT_PLAIN));
					return;
				}
				//5
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
					send(MakeFileResponse(http::status::ok, abs_path, req.version(), req.keep_alive(), mime_type));
				}
				else {
					send(MakeStringResponse(http::status::not_found, json_not_found, req.version(), req.keep_alive(), ContentType::TEXT_PLAIN));
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
						send(MakeStringResponse(http::status::ok, json::serialize(arr), req.version(), req.keep_alive(), ContentType::APP_JSON));
					}
					else if (auto map = game_.FindMap(model::Map::Id(std::string(map_name).substr(1))); map != nullptr) {
						json::object obj;
						obj[key_id] = *(map->GetId());
						obj[key_name] = map->GetName();

						AddRoadsToJson(obj, map);
						AddBuildingsToJson(obj, map);
						AddOfficesToJson(obj, map);

						send(MakeStringResponse(http::status::ok, json::serialize(obj), req.version(), req.keep_alive(), ContentType::APP_JSON));
					}
					else {
						send(MakeStringResponse(http::status::not_found, json_not_found, req.version(), req.keep_alive(), ContentType::APP_JSON));
					}
				}
				else {
					send(MakeStringResponse(http::status::bad_request, bad_request, req.version(), req.keep_alive(), ContentType::APP_JSON));
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

}  // namespace http_handler
