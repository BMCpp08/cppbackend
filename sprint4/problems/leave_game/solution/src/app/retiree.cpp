#include "retiree.h"

namespace app {
	const RetireeId& Retiree::GetId() const noexcept {
		return id_;
	}

	const std::string& Retiree::GetName() const noexcept {
		return name_;
	}

	const int Retiree::GetScore() const noexcept {
		return score_;
	}

	const double Retiree::GetPlayTime() const noexcept {
		return play_time_;
	}
}  // namespace app