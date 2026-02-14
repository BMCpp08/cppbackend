#pragma once
#ifdef _WIN32
#include <sdkddkver.h>
#endif
#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>
#include <memory>
#include <cassert>
#include "hotdog.h"
#include "result.h"
#include <mutex>
namespace net = boost::asio;
using Timer = net::steady_timer;

using namespace std::chrono;
using namespace std::literals;
using Timer = net::steady_timer;
// Функция-обработчик операции приготовления хот-дога
using HotDogHandler = std::function<void(Result<HotDog> hot_dog)>;

class Order : public std::enable_shared_from_this<Order> {
public:

	Order(net::io_context& io, const int id, Store& store,
		std::shared_ptr<GasCooker> gas_cooker, std::mutex& store_mutex, HotDogHandler handler)
		: io_{ io }
		, id_{ id }
		, store_{ store }
		, gas_cooker_{ gas_cooker }
		, store_mutex_{store_mutex}
		, handler_{ std::move(handler) } {
	}


	// Запускает асинхронное выполнение заказа
	void Execute() {

		boost::asio::post(io_, [self = shared_from_this(), this] {
			std::lock_guard<std::mutex> lock(store_mutex_);
			ptr_sausage = std::move(store_.GetSausage());
			StartFrySausage();
			});

		boost::asio::post(io_, [self = shared_from_this(), this] {
			std::lock_guard<std::mutex> lock(store_mutex_);
			ptr_bread = std::move(store_.GetBread());
			StartFryBread();
			});
	}

	void StartFrySausage() {

		try {
			boost::asio::post(strand_, [self = shared_from_this(), this] {  // Эта функция будет первой внутри strand
				ptr_sausage->StartFry(*gas_cooker_, net::bind_executor(strand_, [self, this]() {
					StartTimerFrySausage();
					}));
				});
		}
		catch (const std::invalid_argument& e) {
			std::cerr << "Обнаружено invalid_argument: " << e.what() << std::endl;
			// дополнительно можно вывести стек или предпринять меры
		}
		catch (const std::exception& e) {
			std::cerr << "Исключение: " << e.what() << std::endl;
		}
	}

	void StartFryBread() {

		try {

			boost::asio::post(strand_, [self = shared_from_this(), this] {  // Эта функция будет первой внутри strand
				ptr_bread->StartBake(*gas_cooker_, net::bind_executor(strand_, [self, this]() {
					StartTimerFryBread();
					}));
				});
		}
		catch (const std::invalid_argument& e) {
			std::cerr << "Обнаружено invalid_argument: " << e.what() << std::endl;
			// дополнительно можно вывести стек или предпринять меры
		}
		catch (const std::exception& e) {
			std::cerr << "Исключение: " << e.what() << std::endl;
		}
	}

	void StartTimerFrySausage() {
		try {
			boost::asio::post(strand_, [self = shared_from_this(), this] {  // Эта функция будет первой внутри strand

				self->sausage_timer_.expires_after(HotDog::MIN_SAUSAGE_COOK_DURATION);
				self->sausage_timer_.async_wait(
					net::bind_executor(strand_, [self = shared_from_this()](sys::error_code ec) {
						self->OnRoastedSausage(ec);
						})
				);
				});

		}
		catch (const std::invalid_argument& e) {
			std::cerr << "Обнаружено invalid_argument: " << e.what() << std::endl;
			// дополнительно можно вывести стек или предпринять меры
		}
		catch (const std::exception& e) {
			std::cerr << "Исключение: " << e.what() << std::endl;
		}
	}

	void StartTimerFryBread() {

		try {
			boost::asio::post(strand_, [self = shared_from_this(), this]() {
				self->bread_timer_.expires_after(HotDog::MIN_BREAD_COOK_DURATION);

				self->bread_timer_.async_wait(
					net::bind_executor(strand_, [self = shared_from_this()](sys::error_code ec) {
						self->OnRoastedBread(ec);
						})
				);
				});
		}
		catch (const std::invalid_argument& e) {
			std::cerr << "Обнаружено invalid_argument: " << e.what() << std::endl;
			// дополнительно можно вывести стек или предпринять меры
		}
		catch (const std::exception& e) {
			std::cerr << "Исключение: " << e.what() << std::endl;
		}
	}


	void OnRoastedSausage(sys::error_code ec) {

		try {
			if (ec) {
				Result<sys::error_code> result{ std::make_exception_ptr(ec) };
			}
			else {
				ptr_sausage->StopFry();
				sausage_is_cooked = ptr_sausage->IsCooked();
			}
			CheckReadiness(ec);
		}
		catch (const std::invalid_argument& e) {
			std::cerr << "Обнаружено invalid_argument: " << e.what() << std::endl;
			// дополнительно можно вывести стек или предпринять меры
		}
		catch (const std::exception& e) {
			std::cerr << "Исключение: " << e.what() << std::endl;
		}
	}





	void OnRoastedBread(sys::error_code ec) {

		try {
			if (ec) {
				Result<sys::error_code> result{ std::make_exception_ptr(ec) };
			}
			else {
				ptr_bread->StopBaking();
				bread_is_cooked = ptr_bread->IsCooked();
			}
			CheckReadiness(ec);
		}
		catch (const std::invalid_argument& e) {
			std::cerr << "Обнаружено invalid_argument: " << e.what() << std::endl;
			// дополнительно можно вывести стек или предпринять меры
		}
		catch (const std::exception& e) {
			std::cerr << "Исключение: " << e.what() << std::endl;
		}

	}



private:

	void CheckReadiness(sys::error_code ec) {

		try {
			if (delivered_) {
				return;
			}
			if (ec) {
				Result<sys::error_code> result{ std::make_exception_ptr(ec) };
				return;
			}


			// Если все компоненты гамбургера готовы, упаковываем его
			if (IsReadyToPack()) {
				Pack(ec);
			}
		}
		catch (const std::invalid_argument& e) {
			std::cerr << "Обнаружено invalid_argument: " << e.what() << std::endl;
			// дополнительно можно вывести стек или предпринять меры
		}
		catch (const std::exception& e) {
			std::cerr << "Исключение: " << e.what() << std::endl;
		}
	}

	void Pack(sys::error_code ec) {

		try {
			delivered_ = true;

			// Просто потребляем ресурсы процессора в течение 0,5 с.
			auto start = steady_clock::now();
			while (steady_clock::now() - start < 200ms) {
			}


			handler_(std::move(HotDog(id_, std::move(ptr_sausage), std::move(ptr_bread))));
		}
		catch (const std::invalid_argument& e) {
			std::cerr << "Pack invalid_argument: " << e.what() << std::endl;
		}
		catch (const std::exception& e) {
			std::cerr << "Pack: " << e.what() << std::endl;
		}


	}


	[[nodiscard]] bool IsReadyToPack() const {
		// Если котлета обжарена и лук добавлен, как просили, гамбургер можно упаковывать
		return sausage_is_cooked && bread_is_cooked;
	}


private:
	bool sausage_is_cooked = false;
	bool bread_is_cooked = false;
	Store& store_;



	bool delivered_ = false; // Заказ доставлен?

	/************************/

	net::io_context& io_;
	net::strand<net::io_context::executor_type> strand_{ net::make_strand(io_) };
	/*Store& store_;*/

	const int id_;
	std::shared_ptr <GasCooker> gas_cooker_;

	Timer sausage_timer_{ io_,  HotDog::MIN_SAUSAGE_COOK_DURATION };
	Timer bread_timer_{ io_, HotDog::MIN_BREAD_COOK_DURATION };

	std::shared_ptr<Sausage> ptr_sausage;
	std::shared_ptr<Bread> ptr_bread;

	HotDogHandler handler_;
	std::mutex& store_mutex_;
};



// Класс "Кафетерий". Готовит хот-доги
class Cafeteria {
public:
	explicit Cafeteria(net::io_context& io)
		: io_{ io } {
	}

	// Асинхронно готовит хот-дог и вызывает handler, как только хот-дог будет готов.
	// Этот метод может быть вызван из произвольного потока
	//Result<HotDog> result
	void OrderHotDog(HotDogHandler handler) {
		// TODO: Реализуйте метод самостоятельно
		const int order_id = ++next_order_id_;
		std::make_shared<Order>(io_, order_id, store_, gas_cooker_, store_mutex, std::move(handler))->Execute();

	}

private:

private:
	net::io_context& io_;
	// Используется для создания ингредиентов хот-дога
	Store store_;
	// Газовая плита. По условию задачи в кафетерии есть только одна газовая плита на 8 горелок
	// Используйте её для приготовления ингредиентов хот-дога.
	// Плита создаётся с помощью make_shared, так как GasCooker унаследован от
	// enable_shared_from_this.
	std::shared_ptr<GasCooker> gas_cooker_ = std::make_shared<GasCooker>(io_);

	std::atomic<int> next_order_id_{ 0 };

	std::mutex store_mutex;
};
