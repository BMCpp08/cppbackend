#pragma once

// boost.beast будет использовать std::string_view вместо boost::string_view
#define BOOST_BEAST_USE_STD_STRING_VIEW
#include "ticker.h"


namespace ticker {
	using namespace boost_aliases;

	void Ticker::Start() {
		last_tick_ = Clock::now();
		net::dispatch(strand_, [self = shared_from_this()] {
			self->ScheduleTick();
			});
	}

	void Ticker::ScheduleTick() {
		assert(strand_.running_in_this_thread());
		timer_.expires_after(period_);
		timer_.async_wait([self = shared_from_this()](sys::error_code ec) {
			self->OnTick(ec);
			});
	}

	void Ticker::OnTick(sys::error_code ec) {
		using namespace std::chrono;
		assert(strand_.running_in_this_thread());

		if (!ec) {
			auto this_tick = Clock::now();
			auto delta = duration_cast<milliseconds>(this_tick - last_tick_);
			last_tick_ = this_tick;
			try {
				handler_(delta);
			}
			catch (...) {
			}
			ScheduleTick();
		}
	}
};