#include "logger.h"
#include "sdk.h"
#include <iostream>
#include <thread>
#include <optional>
#include <string_view>
#include "json_loader.h"
#include "request_handler.h
#include "app.h"
#include "boost_includes.h"
#include "ticker.h"


using namespace std::literals;
using namespace logger;
using namespace ticker;

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

struct Args {
	std::optional<std::chrono::milliseconds> tick_period_ms;
	std::string cfg_file;
	std::string root_file;
	std::optional<bool> is_random_positions;
};

[[nodiscard]] std::optional<Args> ParseCommandLine(int argc, const char* const argv[]) {
	namespace po = boost::program_options;

	po::options_description desc{ "All options"s };
	Args args;
	desc.add_options()
		// Добавляем опцию --help и её короткую версию - h
		("help,h", "Show help")
		//Задаёт период автоматического обновления игрового состояния в миллисекундах
		("tick-period,t", po::value<int>()->notifier([&](const int& v) { args.tick_period_ms = std::chrono::milliseconds{ v }; }), "set tick period")
		//Путь к конфигурационному JSON
		("config-file,c", po::value(&args.cfg_file)->value_name("file"s), "set config file path")
		//Путь к каталогу со статическими файлами игры
		("www-root,w", po::value(&args.root_file)->value_name("dir"s), "set static files root")
		//Включает режим, при котором пёс игрока появляется в случайной точке случайно выбранной дороги карты
		("randomize-spawn-points", po::value<bool>()->notifier([&](const bool& v) { args.is_random_positions = v; })->value_name("points"s), "spawn dogs at random positions");
	
	// variables_map хранит значения опций после разбора
	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if (vm.contains("help"s)) {
		// Если был указан параметр --help, то выводим справку и возвращаем nullopt
		std::cout << desc;
		return std::nullopt;
	}

	// Проверяем наличие опций src и dst
	if (!vm.contains("config-file"s)) {
		throw std::runtime_error("Сonfig files have not been specified"s);
	}
	if (!vm.contains("www-root"s)) {
		throw std::runtime_error("Destination static files root path is not specified"s);
	}
	return args;
}

int main(int argc, const char* argv[]) {
	InitBoostLogFilter();

	try {

		if (auto args = ParseCommandLine(argc, argv)) {
			// 1. Загружаем карту из файла и построить модель игры
			std::shared_ptr<model::Game> game = std::make_shared<model::Game>(json_loader::LoadGame(args->cfg_file));

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
			app::JoinGameUseCase join_game_use_case(game, player_tokens, players, (args->is_random_positions.has_value())? *(args->is_random_positions) : false);
			
		
			app::Application application(game, join_game_use_case, player_tokens);
			http_handler::ApiHandler api_handler(application);

			// Настраиваем вызов метода Application::Tick
			std::shared_ptr<Ticker> ticker;
			
			if (args->tick_period_ms.has_value()) {
				ticker = std::make_shared<Ticker>(api_strand, *args->tick_period_ms,
					[&application](std::chrono::milliseconds delta) { application.Tick(delta); }
				);
				
			}
			if (ticker) {
				ticker->Start();
			}

			// 4. Создаём обработчик HTTP-запросов и связываем его с моделью игры
			// Создаём обработчик запросов в куче, управляемый shared_ptr
			auto handler = std::make_shared<http_handler::RequestHandler>(
				args->root_file, api_strand, api_handler, args->tick_period_ms.has_value());
			
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
	}
	catch (const std::exception& ex) {
		BOOST_LOG_TRIVIAL(fatal) << logging::add_value(data, CreateJsonExc(EXIT_FAILURE, ex.what()))
			<< logging::add_value(message, key_server_exited);

		return EXIT_FAILURE;
	}

	
}
