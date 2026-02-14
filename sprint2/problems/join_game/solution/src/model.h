#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

#include "tagged.h"
#include <stdexcept>

namespace model {

	namespace model_details {
		const std::string key_maps = "maps";
		const std::string key_id = "id";
		const std::string key_name = "name";
		const std::string key_roads = "roads";
		const std::string key_x1 = "x1";
		const std::string key_x0 = "x0";
		const std::string key_y0 = "y0";
		const std::string key_y1 = "y1";
		const std::string key_buildings = "buildings";
		const std::string key_w = "w";
		const std::string key_h = "h";
		const std::string key_x = "x";
		const std::string key_y = "y";
		const std::string key_offices = "offices";
		const std::string key_offset_x = "offsetX";
		const std::string key_offset_y = "offsetY";
		const std::string key_auth_token = "authToken";
		const std::string key_player_id = "playerId";
	}

	using Dimension = int;
	using Coord = Dimension;

	struct Point {
		Coord x, y;
	};

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

	struct SpeedCoord {
		Speed x, y;
	};

	enum Direction {
		DIR_NORTH = 0U,
		DIR_SOUTH,
		DIR_WEST,
		DIR_EAST,
	};

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
		using Roads = std::vector<Road>;
		using Buildings = std::vector<Building>;
		using Offices = std::vector<Office>;

		Map(Id id, std::string name) noexcept
			: id_(std::move(id))
			, name_(std::move(name)) {
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

		void AddRoad(const Road& road) {
			roads_.emplace_back(road);
		}

		void AddBuilding(const Building& building) {
			buildings_.emplace_back(building);
		}

		void AddOffice(Office office);

	private:
		using OfficeIdToIndex = std::unordered_map<Office::Id, size_t, util::TaggedHasher<Office::Id>>;

		Id id_;
		std::string name_;
		Roads roads_;
		Buildings buildings_;

		OfficeIdToIndex warehouse_id_to_index_;
		Offices offices_;
	};
	//*************************************************

	class Dog {
	public:

		Dog(Point pos, std::string name,
			std::uint64_t id,
			Direction dir = Direction::DIR_NORTH,
			SpeedCoord speed = {0., 0.}) noexcept
			: pos_(std::move(pos))
			, name_(std::move(name))
			, id_(std::move(id))
			, dir_(std::move(dir))
			, speed_(speed){
		}

		const std::string& GetName() const noexcept {
			return name_;
		}

		const Point& GetPos() const noexcept {
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

		const SpeedCoord& GetSpeed() const noexcept {
			return speed_;
		}
	private:
		Point pos_;
		std::string name_;
		std::uint64_t id_;
		
		Direction dir_;
		SpeedCoord speed_;
	};

	class GameSession {
	public:
		using Dogs = std::unordered_map<std::uint64_t, std::shared_ptr<Dog>>;

		explicit GameSession(Map* map) noexcept
			: map_{ map } {
		}

		const std::shared_ptr<Dog> AddDog(Point point, std::string name) {
			using namespace std::literals;
			const size_t index = dogs_.size();

			if (dogs_.count(index)) {
				throw std::invalid_argument("Dog with id "s + std::to_string(index));
			}
			else {
				try {

					dogs_.emplace(index, std::make_shared<Dog>(std::move(Dog(std::move(point), std::move(name), index))));
					return dogs_[index];
				}
				catch (...) {
					dogs_.erase(index);
					throw;
				}
			}
		}

		const Map* GetMap() const noexcept {
			return map_;
		}

		const Dogs& GetDogs() const noexcept {
			return dogs_;
		}

	private:
		Dogs dogs_;
		Map* map_;
	};

	//*************************************************

	class Game {
	public:
		using Maps = std::vector<Map>;
		using GameSessions = std::vector<GameSession>;

		void AddMap(Map map);

		const Maps& GetMaps() const noexcept {
			return maps_;
		}

		const Map* FindMap(const Map::Id& id) const noexcept {
			if (auto it = map_id_to_index_.find(id); it != map_id_to_index_.end()) {
				return &maps_.at(it->second);
			}
			return nullptr;
		}

		GameSession* FindGameSessions(const Map::Id& id) noexcept {
			if (auto it = map_id_to_index_.find(id); it != map_id_to_index_.end()) {
				return &sessions_.at(it->second);
			}
			return nullptr;
		}

	private:
		using MapIdHasher = util::TaggedHasher<Map::Id>;
		using MapIdToIndex = std::unordered_map<Map::Id, size_t, MapIdHasher>;

		std::vector<Map> maps_;
		MapIdToIndex map_id_to_index_;
		std::vector<GameSession> sessions_;
	};

}  // namespace model
