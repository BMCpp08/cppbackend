#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <iostream>
#include "tagged.h"
#include <stdexcept>
#include "loot_generator.h"
#include "extra_data.h"
#include "model_datails.h"

namespace model {
	using namespace std::literals;
	using namespace model_details;

	class Road;
	using ConstPtrRoad = std::shared_ptr<const Road>;
	using Dimension = int;
	using Coord = Dimension;

	enum Direction {
		DIR_NORTH = 0U,
		DIR_SOUTH,
		DIR_WEST,
		DIR_EAST,
	};

	struct Point {
		Coord x, y;
	};

	struct PointD {
		double x, y;
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

	using Speed = double;

	struct PlayerSpeed {
		Speed x, y;
	};

	struct Loot {
		int id;
		int type;
		int score;
		Point position;
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

		Road(HorizontalTag, Point start, Coord end_x) noexcept
			: start_{ start }
			, end_{ end_x, start.y } {
		}

		Road(VerticalTag, Point start, Coord end_y) noexcept
			: start_{ start }
			, end_{ start.x, end_y } {
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
	private:
		Point start_;
		Point end_;
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
		using LootsDescription = std::vector<std::shared_ptr<extra_data::LootDescription>>;


		Map(Id id, std::string name, Speed speed, int bag_capacity) noexcept
			: id_(std::move(id))
			, name_(std::move(name))
			, speed_(std::move(speed))
			, bag_capacity_(bag_capacity){
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

		void AddLootDescription(extra_data::LootDescription loot_description) {
			loot_description_.emplace_back(std::make_shared<extra_data::LootDescription>(std::move(loot_description)));
		}

		void AddLoot(Loot loot) {
			const size_t index = loots_.size();
			loot.id = index;
			loots_.emplace_back(std::move(loot));
		}

		Loots GetLoots() const noexcept {
			return loots_;
		}

		LootsDescription GetDescription() const noexcept {
			return loot_description_;
		}

		void ExtractLoot(size_t idx) {
			auto res = std::find_if(loots_.begin(), loots_.end(), [&](const Loot& loot) {
				return loot.id == idx;
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

	private:
		void CreateRoadmap(ConstPtrRoad road) {

			if (road->IsHorizontal()) {
				if (road->GetStart().x < road->GetEnd().x) {
					roadmap_[std::pair{ road->GetStart(), Direction::DIR_EAST }] = road;
					roadmap_[std::pair{ road->GetEnd(), Direction::DIR_WEST }] = road;
				}
				else {
					roadmap_[std::pair{ road->GetEnd(), Direction::DIR_EAST }] = road;
					roadmap_[std::pair{ road->GetStart(), Direction::DIR_WEST }] = road;
				}
			}
			else {
				if (road->GetStart().y < road->GetEnd().y) {
					roadmap_[std::pair{ road->GetStart(), Direction::DIR_SOUTH }] = road;
					roadmap_[std::pair{ road->GetEnd(), Direction::DIR_NORTH }] = road;
				}
				else {
					roadmap_[std::pair{ road->GetEnd(), Direction::DIR_SOUTH }] = road;
					roadmap_[std::pair{ road->GetStart(), Direction::DIR_NORTH }] = road;
				}
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
	};

	class Dog {
	public:

		Dog(PointD pos,
			std::string name,
			std::uint64_t id,
			ConstPtrRoad road, 
			size_t bag_capacity = 0,
			Direction dir = Direction::DIR_NORTH,
			PlayerSpeed speed = {0, 0}, int score = 0) noexcept
			: pos_(std::move(pos))
			, name_(std::move(name))
			, id_(std::move(id))
			, road_(road)
			, dir_(std::move(dir))
			, speed_(speed)
			, bag_capacity_(bag_capacity)
			, score_(score){
		}

		const std::string& GetName() const noexcept {
			return name_;
		}

		void SetPos(PointD pos) {
			pos_ = std::move(pos);
		}
		const PointD& GetPos() const noexcept {
			return pos_;
		}

		std::uint64_t GetId() const noexcept {
			return id_;
		}

		void SetDir(Direction new_dir) noexcept {
			dir_ = new_dir;
		}

		const Direction& GetDir() const noexcept {
			return dir_;
		}

		const PlayerSpeed& GetSpeed() const noexcept {
			return speed_;
		}

		void SetSpeed(PlayerSpeed speed) {
			speed_ = std::move(speed);
		}

		void SetNewRoad(ConstPtrRoad& road) {
			road_ = road;
		}

		const ConstPtrRoad GetCurrentRoad() const noexcept {
			return road_;
		}

		bool PutItemIntoBag(Loot item) {
			if (bag_.size() >= bag_capacity_) {
				return false;
			}
			bag_.emplace_back(std::move(item));
			return true;
		}

		Loots GetBag() const noexcept {
			return bag_;
		}

		void EraseBag() {
			bag_.clear();
		}

		bool BagIsFull() {
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

	private:
		PointD pos_;
		std::string name_;
		std::uint64_t id_;
		Direction dir_;
		PlayerSpeed speed_;
		ConstPtrRoad road_;
		size_t bag_capacity_;
		Loots bag_;
		int score_;
		
	};

	class GameSession {
	public:
		using Dogs = std::unordered_map<std::uint64_t, std::shared_ptr<Dog>>;

		explicit GameSession(std::shared_ptr<Map> map) noexcept
			: map_{ map } {
		}

		const std::shared_ptr<Dog> AddDog(PointD point, const std::string& name, ConstPtrRoad road, size_t capacity) {
			using namespace std::literals;
			if (!road) {
				throw std::invalid_argument("Invalid ptr road = nullptr");
			}

			const size_t index = dogs_.size();
			if (dogs_.count(index)) {
				throw std::invalid_argument("Dog with id "s + std::to_string(index));
			}
			else {
				try {
					dogs_.emplace(index, std::make_shared<Dog>(std::move(point), name, index, road, capacity));
					return dogs_[index];
				}
				catch (...) {
					dogs_.erase(index);
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
