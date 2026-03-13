#pragma once

#include <string>
#include <vector>
#include "../util/tagged_uuid.h"

namespace app {
	namespace detail {
		struct RetireeTag {};

	}  // namespace detail

	using RetireeId = util::TaggedUUID<detail::RetireeTag>;

	class Retiree {
	public:
		Retiree(RetireeId id, std::string name, int score, double play_time)
			: id_(std::move(id))
			, name_(std::move(name))
			, score_(score)
			, play_time_(play_time){
		}

		const RetireeId& GetId() const noexcept {
			return id_;
		}

		const std::string& GetName() const noexcept {
			return name_;
		}

		const int GetScore() const noexcept {
			return score_;
		}

		const double GetPlayTime() const noexcept {
			return play_time_;
		}
	private:
		RetireeId id_;
		std::string name_;
		int score_;
		double play_time_;
	};
}  // namespace app
