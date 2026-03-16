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
			, play_time_(play_time) {
		}

		const RetireeId& GetId() const noexcept;

		const std::string& GetName() const noexcept;

		const int GetScore() const noexcept;

		const double GetPlayTime() const noexcept;
	private:
		RetireeId id_;
		std::string name_;
		int score_;
		double play_time_;
	};

	class RetireeRepository {
	public:
		virtual void Save(const Retiree& retiree) = 0;
		virtual const std::vector<Retiree> GetRetirees(const int start_idx, const int count) = 0;
	protected:
		~RetireeRepository() = default;
	};

}  // namespace app
