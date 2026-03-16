#pragma once
#include "app/app.h"
#include <chrono>
#include <boost/serialization/vector.hpp>
#include <filesystem>
#include "model_serialization.h"
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

using namespace std::literals;

namespace infrastructure {
	using InputArchive = boost::archive::text_iarchive;
	using OutputArchive = boost::archive::text_oarchive;

	class SerializingListener : public app::ApplicationListener {
	public:
		SerializingListener(const std::string& state_file, app::Application& app, std::chrono::milliseconds save_period)
			: state_file_(state_file)
			, app_(app)
			, save_period_(save_period)
			, time_since_save_(0ms){
		}

		void OnTick(std::chrono::milliseconds timestamp) override;

		void SaveState();

		void RestoreGameState(const std::string& state_file);
	private:
		std::string state_file_;
		app::Application& app_;
		std::chrono::milliseconds save_period_;
		std::chrono::milliseconds time_since_save_;
	};
}
