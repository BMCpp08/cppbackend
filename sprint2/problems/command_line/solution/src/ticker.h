#pragma once

// boost.beast будет использовать std::string_view вместо boost::string_view
#define BOOST_BEAST_USE_STD_STRING_VIEW
#include "logger.h"
#include "sdk.h"
#include <chrono>
#include "boost_includes.h"

namespace ticker {
	using namespace boost_aliases;

	class Ticker : public std::enable_shared_from_this<Ticker> {
	public:
		using Strand = net::strand<net::io_context::executor_type>;
		using Handler = std::function<void(std::chrono::milliseconds delta)>;

		// Функция handler будет вызываться внутри strand с интервалом period
		Ticker(Strand strand, std::chrono::milliseconds period, Handler handler)
			: strand_{ strand }
			, period_{ period }
			, handler_{ std::move(handler) } {
		}

		void Start();

	private:
		void ScheduleTick();

		void OnTick(sys::error_code ec);

		using Clock = std::chrono::steady_clock;

		Strand strand_;
		std::chrono::milliseconds period_;
		net::steady_timer timer_{ strand_ };
		Handler handler_;
		std::chrono::steady_clock::time_point last_tick_;
	};
};