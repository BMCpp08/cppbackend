#include "logger.h"

#include "sdk.h"
//
#include <boost/asio/io_context.hpp>
#include <iostream>
#include <thread>
#include <boost/asio/signal_set.hpp>

#include "json_loader.h"
#include "request_handler.h"

#include <string_view>
#include "app.h"

using namespace std::literals;
namespace net = boost::asio;
namespace http = boost::beast::http;
namespace sys = boost::system;
using namespace logger;

namespace {
	// Запускает функцию fn на n потоках, включая текущий
	template <typename Fn>
	void RunWorkers(unsigned n, const Fn& fn) {
		n = std::max(1u, n);
		std::vector<std::jthread> workers;
		workers.reserve(n - 1);
		// Запускаем n-1 рабочих потоков, выполняющих функцию fn
		while (--n) {
			workers.emplace_back(fn);
		}
		fn();
	}

}  // namespace


int main(int argc, const char* argv[]) {
	InitBoostLogFilter();

	if (argc != 3) {
		std::cerr << "Usage: game_server <game-config-json>"sv << std::endl;
		return EXIT_FAILURE;
	}
	try {
		// 1. Загружаем карту из файла и построить модель игры
		//model::Game game = json_loader::LoadGame(argv[1]);
		std::shared_ptr<model::Game> game = std::make_shared<model::Game>(json_loader::LoadGame(argv[1]));

		// 2. Инициализируем io_context
		const unsigned num_threads = std::thread::hardware_concurrency();
		net::io_context ioc(num_threads);
		// strand для выполнения запросов к API
		auto api_strand = net::make_strand(ioc);

		// 3. Добавляем асинхронный обработчик сигналов SIGINT и SIGTERM
		net::signal_set signals(ioc, SIGINT, SIGTERM);
		signals.async_wait([&ioc](const sys::error_code& ec, [[maybe_unused]] int signal_number) {
			if (!ec) {
				ioc.stop();

				BOOST_LOG_TRIVIAL(info) << logging::add_value(data, CreateJsonExc(0))
					<< logging::add_value(message, key_server_exited);
			}
			else {
				BOOST_LOG_TRIVIAL(fatal) << logging::add_value(data, CreateJsonExc(ec.value(), ec.what()))
					<< logging::add_value(message, key_server_exited);
			}

			});



		//Создание уровня Application
		std::shared_ptr<app::Players> players = std::make_shared<app::Players>();
		std::shared_ptr<app::PlayerTokens> player_tokens = std::make_shared<app::PlayerTokens>();
		app::JoinGameUseCase join_game_use_case(game, player_tokens, players);
		app::Application application(game, join_game_use_case, player_tokens);
		http_handler::ApiHandler api_handler(application);



		// 4. Создаём обработчик HTTP-запросов и связываем его с моделью игры
		// Создаём обработчик запросов в куче, управляемый shared_ptr
		/*http_handler::RequestHandler handler{ game, argv[2] };*/
		auto handler = std::make_shared<http_handler::RequestHandler>(
			argv[2], api_strand, api_handler);

		auto lambda = [handler](auto&& endpoint, auto&& req, auto&& send) {
			// Обработка запроса
			(*handler)(std::forward<decltype(endpoint)>(endpoint),
				std::forward<decltype(req)>(req),
				std::forward<decltype(send)>(send));

			};
		// Оборачиваем его в логирующий декоратор
		http_handler::server_logging::LoggingRequestHandler logging_handler{
			lambda };




		// 5. Запустить обработчик HTTP-запросов, делегируя их обработчику запросов
		const auto address = net::ip::make_address("0.0.0.0");
		constexpr net::ip::port_type port = 8080;

		http_server::ServeHttp(ioc, { address, port }, logging_handler);


		// Эта надпись сообщает тестам о том, что сервер запущен и готов обрабатывать запросы
		boost::json::value custom_data{ {"port"s,port},{"address"s, "0.0.0.0"s} };
		std::string msg{ "server started"s };

		BOOST_LOG_TRIVIAL(info) << logging::add_value(data, custom_data)
			<< logging::add_value(message, msg);


		// 6. Запускаем обработку асинхронных операций
		RunWorkers(std::max(1u, num_threads), [&ioc] {
			ioc.run();
			});
	}
	catch (const std::exception& ex) {

		BOOST_LOG_TRIVIAL(fatal) << logging::add_value(data, CreateJsonExc(EXIT_FAILURE, ex.what()))
			<< logging::add_value(message, key_server_exited);

		return EXIT_FAILURE;
	}
}
