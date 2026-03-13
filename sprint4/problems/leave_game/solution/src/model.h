#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <iostream>
#include "util/tagged.h"
#include <stdexcept>
#include "loot_generator.h"
#include "game_details.h"
#include "model_datails.h"
#include "geom.h"

namespace model {
	using namespace std::literals;
	using namespace model_details;
	using namespace game_details;
	class Road;
	using ConstPtrRoad = std::shared_ptr<const Road>;
	using Dimension = int;
	using Coord = Dimension;
	using Speed = double;
	using TimePoint = std::chrono::time_point<std::chrono::steady_clock>;

	enum class Direction {
		DIR_NORTH = 0U,
		DIR_SOUTH,
		DIR_WEST,
		DIR_EAST,
	};

	struct Point {
		Coord x, y;
	};

	struct HashPoint {
		size_t operator()(const Point& p) const noexcept {
			size_t h1 = std::hash<Coord>()(p.x);
			size_t h2 = std::hash<Coord>()(p.y);
			return h1 ^ (h2 << 1); 
		}
	};

	struct HashPointDir {
		size_t operator()(const std::pair<Point, Direction>& p) const noexcept {
			size_t h1 = HashPoint()(p.first);
			std::hash<Direction> hasher;
			size_t h2 = hasher(p.second);
			return h1 ^ (h2 << 1);
		}
	};

	inline bool operator==(const Point& a, const Point& b) {
		return a.x == b.x && a.y == b.y;
	}

	struct Size {
		Dimension width, height;
	};

	struct Rectangle {
		Point position;
		Size size;
	};

	struct Offset {
		Dimension dx, dy;
	};

	struct Loot {
		using Id = util::Tagged<size_t, Loot>;
		Id id{ 0u };
		LootType type;
		Score score;
		Point position;
		[[nodiscard]] auto operator<=>(const Loot&) const = default;
	};

	using Loots = std::vector<Loot>;

	class Road {
		struct HorizontalTag {
			explicit HorizontalTag() = default;
		};

		struct VerticalTag {
			explicit VerticalTag() = default;
		};
	public:
		constexpr static HorizontalTag HORIZONTAL{};
		constexpr static VerticalTag VERTICAL{};
		using Id = util::Tagged<int, Road>;

		Road(HorizontalTag, Point start, Coord end_x, Id id) noexcept
			: start_{ start }
			, end_{ end_x, start.y }
			, id_(id){
		}

		Road(VerticalTag, Point start, Coord end_y, Id id) noexcept
			: start_{ start }
			, end_{ start.x, end_y }
			, id_(id) {
		}

		bool IsHorizontal() const noexcept {
			return start_.y == end_.y;
		}

		bool IsVertical() const noexcept {
			return start_.x == end_.x;
		}

		Point GetStart() const noexcept {
			return start_;
		}

		Point GetEnd() const noexcept {
			return end_;
		}

		const Id& GetId() const noexcept {
			return id_;
		}

	private:
		Point start_;
		Point end_;
		Id id_;
	};

	class Building {
	public:
		explicit Building(Rectangle bounds) noexcept
			: bounds_{ bounds } {
		}

		const Rectangle& GetBounds() const noexcept {
			return bounds_;
		}
	private:
		Rectangle bounds_;
	};

	class Office {
	public:
		using Id = util::Tagged<std::string, Office>;

		Office(Id id, Point position, Offset offset) noexcept
			: id_{ std::move(id) }
			, position_{ position }
			, offset_{ offset } {
		}

		const Id& GetId() const noexcept {
			return id_;
		}

		Point GetPosition() const noexcept {
			return position_;
		}

		Offset GetOffset() const noexcept {
			return offset_;
		}
	private:
		Id id_;
		Point position_;
		Offset offset_;
	};

	class Map {
	public:
		using Id = util::Tagged<std::string, Map>;
		using Buildings = std::vector<Building>;
		using Offices = std::vector<Office>;
		using Roads = std::vector<ConstPtrRoad>;
		using Roadmap = std::unordered_map<std::pair<Point, Direction>, ConstPtrRoad, HashPointDir>;
		using LootsDescription = std::vector<std::shared_ptr<LootDescription>>;

		Map(Id id, std::string name, Speed speed, int bag_capacity, double dog_retirement_time = 1.) noexcept
			: id_(std::move(id))
			, name_(std::move(name))
			, speed_(std::move(speed))
			, bag_capacity_(bag_capacity)
			, dog_retirement_time_(dog_retirement_time){
		}

		const Id& GetId() const noexcept {
			return id_;
		}

		const std::string& GetName() const noexcept {
			return name_;
		}

		const Buildings& GetBuildings() const noexcept {
			return buildings_;
		}

		const Roads& GetRoads() const noexcept {
			return roads_;
		}

		const Offices& GetOffices() const noexcept {
			return offices_;
		}

		void AddRoad(const Road road) {
			roads_.emplace_back(std::make_shared<Road>(road));
			CreateRoadmap(roads_.back());
		}

		void AddBuilding(const Building& building) {
			buildings_.emplace_back(building);
		}

		void AddOffice(Office office);

		const Speed& GetSpeed() const noexcept {
			return speed_;
		}

		void SetSpeed(Speed speed) {
			speed_ = std::move(speed);
		}

		const Roadmap& GetRoadmap() const {
			return roadmap_;
		}

		void AddLootDescription(LootDescription loot_description) {
			loot_description_.emplace_back(std::make_shared<LootDescription>(std::move(loot_description)));
		}

		void AddLoot(Loot loot) {
			loot.id = Loot::Id{ loots_.size() };
			loots_.emplace_back(std::move(loot));
		}

		Loots GetLoots() const noexcept {
			return loots_;
		}

		LootsDescription GetDescription() const noexcept {
			return loot_description_;
		}

		void ExtractLoot(Loot::Id idx) {
			auto res = std::find_if(loots_.begin(), loots_.end(), [&](const Loot& loot) {
				return loot.id == idx ;
				});
			if (res != loots_.end()) {
				loots_.erase(res);
			}
		}

		size_t GetLootCount() const noexcept {
			return loots_.size();
		}

		size_t GetBagCapacity() const noexcept {
			return bag_capacity_;
		}

		double GetDogRetirementTime() const noexcept {
			return dog_retirement_time_;
		}
	private:
		template<typename Comparator>
		void LoadRoadmap(const ConstPtrRoad& road, Comparator comp, std::pair<Direction, Direction> dir) {
			if (comp(road)) {
				roadmap_[std::pair{ road->GetStart(), dir.second }] = road;
				roadmap_[std::pair{ road->GetEnd(), dir.first }] = road;
			}
			else {
				roadmap_[std::pair{ road->GetEnd(), dir.second }] = road;
				roadmap_[std::pair{ road->GetStart(), dir.first }] = road;
			}
		}

		void CreateRoadmap(ConstPtrRoad road) {
			auto comparator_x = [](ConstPtrRoad road) { return road->GetStart().x < road->GetEnd().x; };
			auto comparator_y = [](ConstPtrRoad road) { return road->GetStart().y < road->GetEnd().y; };

			if (road->IsHorizontal()) {
				LoadRoadmap(road, comparator_x, std::pair<Direction, Direction>{Direction::DIR_WEST, Direction::DIR_EAST});
			}
			else {
				LoadRoadmap(road, comparator_y, std::pair<Direction, Direction>{Direction::DIR_NORTH, Direction::DIR_SOUTH});
			}
		}
	private:
		using OfficeIdToIndex = std::unordered_map<Office::Id, size_t, util::TaggedHasher<Office::Id>>;
		Id id_;
		std::string name_;
		Roads roads_;
		Buildings buildings_;
		OfficeIdToIndex warehouse_id_to_index_;
		Offices offices_;
		Speed speed_;
		Roadmap roadmap_;
		Loots loots_;
		LootsDescription loot_description_;
		size_t bag_capacity_;
		double dog_retirement_time_;
	};

	class Dog {
	public:
		using Id = util::Tagged<uint32_t, Dog>;

		Dog(geom::Point2D pos,
			std::string name,
			Id id,
			ConstPtrRoad road, 
			size_t bag_capacity = 0,
			Direction dir = Direction::DIR_NORTH,
			int score = 0) noexcept
			: pos_(std::move(pos))
			, name_(std::move(name))
			, id_(std::move(id))
			, road_(road)
			, dir_(std::move(dir))
			, bag_capacity_(bag_capacity)
			, score_(score){
			
			if (road_) {
				SetRoadId(road->GetId());
			}
		}

		const std::string GetName() const noexcept {
			return name_;
		}

		void SetPosition(geom::Point2D pos) {
			pos_ = std::move(pos);
		}

		const geom::Point2D& GetPosition() const noexcept {
			return pos_;
		}

		const Id& GetId() const noexcept {
			return id_;
		}

		void SetDirection(Direction new_dir) noexcept {
			dir_ = new_dir;
		}

		Direction GetDirection() const noexcept {
			return dir_;
		}

		const geom::Vec2D GetSpeed() const noexcept {
			return speed_;
		}

		void SetSpeed(geom::Vec2D speed) {
			speed_ = std::move(speed);
		}

		void SetNewRoad(ConstPtrRoad& road) {
			road_ = road;
			SetRoadId(road->GetId());
		}

		void SetRoadId(Road::Id id) {
			road_id_ = *id;
		}

		const ConstPtrRoad GetCurrentRoad() const noexcept {
			return road_;
		}

		[[nodiscard]] bool PutItemIntoBag(Loot item) {
			if (IsBagFull()) {
				return false;
			}
			bag_.emplace_back(std::move(item));
			return true;
		}

		bool IsBagFull() const noexcept {
			return bag_.size() >= bag_capacity_;
		}

		Loots GetBagContent() const noexcept {
			return bag_;
		}

		void EraseBag() {
			bag_.clear();
		}

		bool IsBagFull() {
			return bag_.size() >= bag_capacity_;
		}

		void CalcScoreAndEraseBag() {
			for (const auto& item : bag_) {
				score_ += item.score;
			}
			EraseBag();
		}

		int GetScore()  const noexcept {
			return score_;
		}

		void AddScore(Score score) noexcept {
			score_ += score;
		}

		size_t GetBagCapacity() const noexcept {
			return bag_capacity_;
		}

		Road::Id GetRoadId() const noexcept {
			return Road::Id{ road_id_ };
		}

		void SetStopTimestamp(TimePoint new_timestamp) {
			stop_timestamp_ = new_timestamp;
		}

		TimePoint GetStopTimestamp() const noexcept {
			return stop_timestamp_;
		}

		TimePoint GetStartTime() const noexcept {
			return start_time_;
		}
	private:
		geom::Point2D pos_;
		std::string name_;
		Id id_;
		Direction dir_;
		geom::Vec2D speed_;
		ConstPtrRoad road_;
		size_t bag_capacity_;
		Loots bag_;
		Score score_;
		int road_id_;
		TimePoint start_time_ = std::chrono::steady_clock::now();
		TimePoint stop_timestamp_ = std::chrono::steady_clock::now();
	};

	class GameSession {
	public:
		using Dogs = std::unordered_map<std::uint64_t, std::shared_ptr<Dog>>;

		explicit GameSession(std::shared_ptr<Map> map) noexcept
			: map_{ map } {
		}

		const std::shared_ptr<Dog> AddDog(geom::Point2D point, const std::string& name, ConstPtrRoad road, size_t capacity) {
			using namespace std::literals;
			if (!road) {
				throw std::invalid_argument("Invalid ptr road = nullptr"s);
			}

			const size_t index = dogs_.size();
			if (dogs_.count(index)) {
				throw std::invalid_argument("Dog with id "s + std::to_string(index));
			}
			else {
				try {
					dogs_.emplace(index, std::make_shared<Dog>(std::move(point), name, static_cast<model::Dog::Id>( index ), road, capacity));
					return dogs_[index];
				}
				catch (...) {
					dogs_.erase(index);
					throw;
				}
			}
		}

		const std::shared_ptr<Dog> AddDog(Dog dog) {
			auto idx = *dog.GetId();
			if (dogs_.count(idx)) {
				throw std::invalid_argument("Dog with id "s + std::to_string(idx));
			}
			else {
				try {
					dogs_.emplace(idx, std::make_shared<Dog>(std::move(dog)));
					return dogs_[idx];
				}
				catch (...) {
					dogs_.erase(idx);
					throw;
				}
			}
		}

		const std::shared_ptr<Map> GetMap() const noexcept {
			return map_;
		}

		const Dogs& GetDogs() const noexcept {
			return dogs_;
		}

		void RemovePlayer(Dog::Id dog_id) {
			if (auto it = dogs_.find(*dog_id); it != dogs_.end()) {
				dogs_.erase(it);
			}
		}
	private:
		Dogs dogs_;
		std::shared_ptr<Map> map_;
	};

	class Game {
	public:
		using Maps = std::vector<std::shared_ptr<Map>>;
		using MapPtr = std::shared_ptr<Map>;
		using GameSessions = std::vector<GameSession>;

		void AddMap(Map map);

		const Maps& GetMaps() const noexcept {
			return maps_;
		}

		const MapPtr FindMap(const Map::Id& id) const noexcept {
			if (auto it = map_id_to_index_.find(id); it != map_id_to_index_.end()) {
				return maps_.at(it->second);
			}
			return nullptr;
		}

		GameSession* FindGameSessions(const Map::Id& id) noexcept {
			if (auto it = map_id_to_index_.find(id); it != map_id_to_index_.end()) {
				return &sessions_.at(it->second);
			}
			return nullptr;
		}

		void AddLootGenerator(loot_gen::LootGenerator loot_generator);

		const std::shared_ptr<loot_gen::LootGenerator> GetLootGenerator() const;
	private:
		using MapIdHasher = util::TaggedHasher<Map::Id>;
		using MapIdToIndex = std::unordered_map<Map::Id, size_t, MapIdHasher>;
		Maps maps_;
		MapIdToIndex map_id_to_index_;
		std::vector<GameSession> sessions_;
		std::shared_ptr<loot_gen::LootGenerator> loot_generator_;
	};

}  // namespace model
