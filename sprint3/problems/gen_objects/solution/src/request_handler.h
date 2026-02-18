#pragma once
// boost.beast будет использовать std::string_view вместо boost::string_view
#define BOOST_BEAST_USE_STD_STRING_VIEW
#include "http_server.h"
#include "app.h"
#include "boost_includes.h"
#include "logger.h"
#include <filesystem>
#include <cassert>
#include <unordered_map>
#include <variant>
#include <chrono>

// boost.beast будет использовать std::string_view вместо boost::string_view
#define BOOST_BEAST_USE_STD_STRING_VIEW


namespace http_handler {
	using namespace std::literals;
	using namespace boost_aliases;
	using namespace model::model_details;
	namespace fs = std::filesystem;
	using namespace logger;

	// Запрос, тело которого представлено в виде строки
	using StringRequest = http::request<http::string_body>;

	using StringResponse = http::response<http::string_body>;
	using FileResponse = http::response<http::file_body>;
	using EmptyResponse = http::response<http::empty_body>;

	constexpr auto json_not_found = R"({"code": "mapNotFound", "message" : "Map not found"})";
	constexpr auto json_invalid_name = R"({"code": "invalidArgument", "message": "Invalid name"})";

	constexpr auto bad_request = R"({"code": "badRequest", "message" : "Bad request"})";
	constexpr auto json_parse_error = R"({"code": "invalidArgument", "message": "Join game request parse error"})";
	constexpr auto post_method_error = R"({"code": "invalidMethod", "message": "Only POST method is expected"})";
	constexpr auto invalid_method_error = R"({"code": "invalidMethod", "message": "Invalid method"})";
	constexpr auto failed_parse_action = R"({"code": "invalidArgument", "message": "Failed to parse action"})";
	constexpr auto invalid_content_type = R"({"code": "invalidArgument", "message": "Invalid content type"})";
	constexpr auto invalid_tick_req = R"({"code": "invalidArgument", "message": "Failed to parse tick request JSON"})";
	constexpr auto bad_request_invalid_endpoint = R"({"code": "badRequest", "message": "Invalid endpoint"})";

	constexpr auto authorization_method_missing = R"({"code": "invalidToken", "message": "Authorization header is missing"})";
	constexpr auto token_not_found = R"({"code": "unknownToken", "message": "Player token has not been found"})";
	constexpr auto authorization_header_req = R"({ "code": "invalidToken", "message" : "Authorization header is required"})";

	/*API*/
	const std::string_view api = "/api"sv;
	constexpr std::string_view api_get_map = "/api/v1/maps"sv; //полчение списка карт
	constexpr std::string_view api_post_join = "/api/v1/game/join"sv; //Для входа в игру 
	constexpr std::string_view api_get_players = "/api/v1/game/players"sv; //Получение списка игроков
	constexpr std::string_view api_get_game_state = "/api/v1/game/state"sv; //Запрос игрового состояния
	constexpr std::string_view api_game_player_action = "/api/v1/game/player/action"sv; //Управление действиями своего персонажа
	constexpr std::string_view api_game_tick = "/api/v1/game/tick"sv; //Установить время

	constexpr std::string_view REQ_GET = "GET"sv;
	constexpr std::string_view REQ_HEAD = "HEAD"sv;
	constexpr std::string_view REQ_POST = "POST"sv;

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

	inline std::string GetContentType(const StringRequest& req);

	StringResponse MakeStringResponse(http::status status, std::string_view body, unsigned http_version,
		bool keep_alive,
		std::string_view content_type);

	class ApiHandler {
	private:
		json::array LoadRoadsToJson(const model::Game::MapPtr map) const;

		json::array LoadBuildingsToJson(const model::Game::MapPtr map) const;

		json::array LoadOfficesToJson(const model::Game::MapPtr map) const;
	public:
		ApiHandler(app::Application& app)
			:app_(app) {
		}

		bool IsApiRequest(StringRequest req);

		StringResponse HandlerApiHandler(const StringRequest& req) const;

		void AddApiIgnore(std::string_view api, bool is_ignore);

	private:
		app::Application& app_;
		std::unordered_map<std::string_view, bool> api_ignore_list_;
	};
	

	class RequestHandler : public std::enable_shared_from_this<RequestHandler> {

		using FileRequestResult = std::variant<EmptyResponse, StringResponse, FileResponse>;
	public:
		using Strand = net::strand<net::io_context::executor_type>;

		explicit RequestHandler(fs::path root, Strand api_strand, ApiHandler& api_handler, bool ignore_api_tick)
			: root_{ std::move(root) }
			, api_strand_{ api_strand }
			, api_handler_{ api_handler } {
			api_handler.AddApiIgnore(api_game_tick, ignore_api_tick);
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
		ApiHandler& api_handler_;

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

				auto handler = [endpoint, req, send, this](auto&& resp) {
					auto start_time = std::chrono::steady_clock::now();
					LogResponse(std::forward<decltype(resp)>(resp), std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time).count());
					send(resp);

					};

				decorated_(endpoint, std::move(req), std::move(handler));
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
