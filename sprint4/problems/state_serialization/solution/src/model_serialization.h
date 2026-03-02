#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>
#include "model.h"
#include "app.h"
namespace geom {

template <typename Archive>
void serialize(Archive& ar, geom::Point2D& point, [[maybe_unused]] const unsigned version) {
    ar& point.x;
    ar& point.y;
}

template <typename Archive>
void serialize(Archive& ar, geom::Vec2D& vec, [[maybe_unused]] const unsigned version) {
    ar& vec.x;
    ar& vec.y;
}
}  // namespace geom

namespace model {

template <typename Archive>
void serialize(Archive& ar, Loot& obj, [[maybe_unused]] const unsigned version) {
    ar&(*obj.id);
    ar&(obj.type);
}

template <typename Archive>
void serialize(Archive& ar, model::Point& point, [[maybe_unused]] const unsigned version) {
    ar& point.x;
    ar& point.y;
}
}  // namespace model

namespace serialization {

// DogRepr (DogRepresentation) - сериализованное представление класса Dog
class DogRepr {
public:
    DogRepr() = default;

    explicit DogRepr(const model::Dog& dog)
        : id_(dog.GetId())
        , name_(dog.GetName())
        , pos_(dog.GetPosition())
        , bag_capacity_(dog.GetBagCapacity())
        , speed_(dog.GetSpeed())
        , direction_(dog.GetDirection())
        , score_(dog.GetScore())
        , bag_content_(dog.GetBagContent()) 
        , road_id_(dog.GetRoadId()){
    }

    [[nodiscard]] model::Dog Restore() const {
        model::Dog dog{ pos_, name_, id_, nullptr, bag_capacity_ };

        dog.SetSpeed(speed_);
        dog.SetDirection(direction_);
        dog.AddScore(score_);
        dog.SetRoadId(road_id_);
        for (const auto& item : bag_content_) {
            if (!dog.PutItemIntoBag(item)) {
                throw std::runtime_error("Failed to put bag content");
            }
        }
        return dog;
    }

    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar&* id_;
        ar& name_;
        ar& pos_;
        ar& bag_capacity_;
        ar& speed_;
        ar& direction_;
        ar& score_;
        ar& bag_content_;
        ar& *road_id_;
    }

private:
    model::Dog::Id id_ = model::Dog::Id{0u};
    std::string name_;
    geom::Point2D pos_;
    size_t bag_capacity_ = 0;
    geom::Vec2D speed_;
    model::Direction direction_ = model::Direction::DIR_NORTH;
    model::Score score_ = 0;
    model::Loots bag_content_;
    model::Road::Id road_id_ = model::Road::Id{ 0u };
};

class LootRepr {
public:
    LootRepr() = default;

    explicit LootRepr(const model::Loot& loot)
        : id_(loot.id)
        , type_(loot.type)
        , score_(loot.score)
        , position_(loot.position) {
    }

    [[nodiscard]] model::Loot Restore() const {
        model::Loot loot{ id_, type_, score_, position_ };
        return loot;
    }

    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar&* id_;
        ar& type_;
        ar& score_;
        ar& position_;
    }

private:
    model::Loot::Id id_ = model::Loot::Id{ 0u };
    model::LootType type_;
    model::Score score_;
    model::Point position_;
};


class PlayerRepr {
public:
    PlayerRepr() = default;

    explicit PlayerRepr(const app::Player* player, const app::Token& token)
        : id_(player->GetId())
        , token_(player->GetToken()) {
    }
 
    [[nodiscard]] app::Player Restore() const {
        app::Player player(id_);
        player.SetToken(token_);
        return player;
    }

    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar&* id_;
        ar&* token_;
    }

private:
    app::Player::Id id_ = app::Player::Id{ 0u };
    app::Token token_ = app::Token{ std::to_string(0)};
   
};

class MapRepr {
public:

    MapRepr() = default;

    explicit MapRepr(const model::Map::Id& id, 
        const std::vector<PlayerRepr>& players, 
        const std::vector<LootRepr>& loots, 
        const std::vector<DogRepr>& dogs)
        : id_(id)
        , players_(players)
        , loots_(loots)
        , dogs_(dogs) {
    }

    [[nodiscard]] MapRepr Restore() const {
        MapRepr map{ id_, players_, loots_, dogs_ };
        return map;
    }

    template<class Archive>
    void serialize(Archive& ar, const unsigned int) {
        ar& *id_;
        ar& players_;
        ar& loots_;
        ar& dogs_;
    }
public:
    model::Map::Id id_ = model::Map::Id{ std::to_string(0) };
    std::vector<PlayerRepr> players_;
    std::vector<LootRepr> loots_;
    std::vector<DogRepr> dogs_;
};
}  // namespace serialization


