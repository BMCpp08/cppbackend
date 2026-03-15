#define _USE_MATH_DEFINES

#include "../src/collision_detector.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_templated.hpp>
#include <stdexcept>
#include <cmath>
#include <sstream>

using namespace std::literals;

namespace Catch {
	template<>
	struct StringMaker<collision_detector::GatheringEvent> {
		static std::string convert(collision_detector::GatheringEvent const& value) {
			std::ostringstream tmp;
			tmp << "(" << value.gatherer_id << "," << value.item_id << "," << value.sq_distance << "," << value.time << ")";

			return tmp.str();
		}
	};
}  // namespace Catch 


template <typename Range, typename Predicate>
struct EqualMatcher : Catch::Matchers::MatcherGenericBase {
	EqualMatcher(Range range,Predicate predicate)
		: range_{ range }
		, predicate_(predicate) {
		}

	EqualMatcher(EqualMatcher&&) = default;

	template <typename OtherRange>
	bool match(OtherRange other) const {
		using std::begin;
		using std::end;

		return std::equal(begin(range_), end(range_), begin(other), end(other), predicate_);
	}

	std::string describe() const override {
		// Описание свойства, проверяемого матчером:
		return "Equal: "s + Catch::rangeToString(range_);
	}

private:
	Range range_;
	Predicate predicate_;
};

bool IsEqualWithEpsilon(const double a, const double b, const double eps = 1e-10) {
	return std::abs(a - b) < eps;
}

bool ComparatorEvents(const collision_detector::GatheringEvent& l, const collision_detector::GatheringEvent& r) {
	if (l.item_id != r.item_id || l.gatherer_id != r.gatherer_id) {
		return false;
	}

	if (!IsEqualWithEpsilon(l.sq_distance, r.sq_distance) || !IsEqualWithEpsilon(l.time, r.time)) {
		return false;
	}
	return true;
}


template<typename Range, typename Predicate>
EqualMatcher<Range, Predicate> EqualRangeMatcher(Range&& range, Predicate&& predicate) {
	return EqualMatcher<Range, Predicate>{std::forward<Range>(range), std::forward<Predicate>(predicate)};
}

class GathererProvider : public collision_detector::ItemGathererProvider {
public:

	GathererProvider(std::vector<collision_detector::Item> items, std::vector<collision_detector::Gatherer> gatherers)
		: items_(std::move(items))
		, gatherers_(std::move(gatherers)) {

	}

	size_t ItemsCount() const override {
		return items_.size();
	}

	collision_detector::Item GetItem(size_t idx) const override {
		if (idx >= items_.size()) {
			throw std::logic_error("Invalid item idx!");
		}
		return items_[idx];
	}

	size_t GatherersCount() const override {
		return gatherers_.size();
	}

	collision_detector::Gatherer GetGatherer(size_t idx) const override {
		if (idx >= gatherers_.size()) {
			throw std::logic_error("Invalid gatherer idx!");
		}
		return gatherers_[idx];
	} 
	
private:
	std::vector<collision_detector::Item> items_;
	std::vector<collision_detector::Gatherer> gatherers_;
};


SCENARIO("Collision detection") {
	using namespace collision_detector;

	//Нет предметов и нет собирателей
	WHEN("No items and no gatherers") {
		GathererProvider provider({}, {});

		THEN("No gathering event") {
			auto gather_events = collision_detector::FindGatherEvents(provider);
			CHECK(gather_events.empty());
		}
	}

	//Нет собирателей
	WHEN("No gatherers but three items") {
		GathererProvider provider({ {{1, 0}, 1.}, {{2, 0}, 1.}, {{3, 0}, 1.} }, {});

		THEN("No gathering event") {
			auto gather_events = collision_detector::FindGatherEvents(provider);
			CHECK(gather_events.empty());
		}
	}
	
	//Нет предметов
	WHEN("No items but three gatherers") {
		GathererProvider provider({}, { {{0, 0}, {10, 0}, 1.}, {{10, 0}, {20, 0}, 1.}, {{20, 0}, {30, 0}, 1.} });

		THEN("No gathering event") {
			auto gather_events = collision_detector::FindGatherEvents(provider);
			CHECK(gather_events.empty());
		}
	}

	//Никто не двигается
	WHEN("Gatherers stay put") {
		GathererProvider provider{ {{{0, 0}, 0.2}},
									{{{ 0, 1 }, { 0, 1 }, 0.6},
									{{ -40, -10 }, { -40, -10 }, 0.6},
									{{ -40, 0 }, { -40, 0 }, 0.6}} };
		THEN("No gathering event") {
			auto events = collision_detector::FindGatherEvents(provider);
			CHECK(events.empty());
		}
	}

	WHEN("Eight items and one gatherers") {
		GathererProvider provider({ 
			{{ 0, 0.0 }, 0.2},		// Предмет 0
			{{ 1, 0.9 }, 0.2},		// Предмет 1 -
			{{ 5, -0.7 }, 0.2},		// Предмет 2 
			{{ 7, 0.7 }, 0.2},		// Предмет 3
			{{ 9, 0.2 }, 0.2},		// Предмет 4
			{{ 10, 0.0 }, 0.2},		// Предмет 5
			{{ 20, 0.5 }, 0.2},	    // Предмет 6
			{{ 20, 0.9 }, 0.2}},	// Предмет 7 -
			{{{0, 0}, {20, 0}, 0.6}}); //x0 -> x20
		
		THEN("No gathering event") {
			auto gather_events = collision_detector::FindGatherEvents(provider);
			std::vector<collision_detector::GatheringEvent> expected_events{{0, 0, 0.0 * 0.0, 0.00},
																			{2, 0, 0.7 * 0.7, 0.25},
																			{3, 0, 0.7 * 0.7, 0.35},
																			{4, 0, 0.2 * 0.2, 0.45},
																			{5, 0, 0.0 * 0.0, 0.5},
																			{6, 0, 0.5 * 0.5, 1.0}};
			CHECK_THAT(gather_events, EqualRangeMatcher(expected_events, ComparatorEvents));
		}
	}

	//Кто самый быстрый
	WHEN("Who is faster gatherer") {
		GathererProvider provider{{{{0, 0}, 0.2}},
									{{{ 0, 1 }, { 0, -100 }, 0.6},
									{{ -40, -10 }, { 0, 0 }, 0.6},
									{{ -40, 0 }, { 40, 0 }, 0.6}}};
		THEN("Faster gatherer") {
			auto events = collision_detector::FindGatherEvents(provider);
			CHECK(events.front().gatherer_id == 0);
		}
	}
}
