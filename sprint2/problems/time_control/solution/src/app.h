#pragma once

// boost.beast будет использовать std::string_view вместо boost::string_view
#define BOOST_BEAST_USE_STD_STRING_VIEW
#include "logger.h"
#include "sdk.h"
#include "model.h"
#include "tagged.h"
#include <string>
#include <boost/json.hpp>
#include <random>
#include <unordered_map>
#include <memory>

namespace app {
	namespace json = boost::json;
	using namespace std::literals;

	enum JoinGameErrorReason {
		INVALIDE_NAME = 0u,
		INVALIDE_MAP,
	};

	enum AuthorizationGameErrorReason {
		AUTHORIZATION_HEADER_MISSING = 0u,
		AUTHORIZATION_TOKEN_NOT_FOUND,
		AUTHORIZATION_HEADER_REQ,
		AUTHORIZATION_INVALIDE_TOKEN,
	};

	enum ActionGameErrorReason {
		FAILED_PARSE_ACTION = 0u,
	};

	struct PlayersHash {
		size_t operator()(const std::pair<std::string, std::string>& key) const {
			size_t hash_f = std::hash<std::string>()(key.first);
			size_t hash_c = std::hash<std::string>()(key.second);
			return hash_f ^ (hash_c << 1);
		}
	};

	template<class ErrorReason>
	class GameError {
	public:
		GameError(ErrorReason reason)
			:reason_(reason) {
		}
		ErrorReason GetErrorReason() const noexcept {
			return reason_;
		}
	private:
		ErrorReason reason_;
	};

	namespace detail {
		struct TokenTag {};
	}  // namespace detail

	using Token = util::Tagged<std::string, detail::TokenTag>;

	struct TokenHash {
		size_t operator()(const Token& token) const {
			return std::hash<std::string>()(*token);
		}
	};


	class Player {
	public:
		using Id = util::Tagged<std::uint64_t, Player>;

		Player(Id id,
			model::GameSession* game_session,
			std::shared_ptr<model::Dog> dog) noexcept
			: id_(std::move(id))
			, game_session_(game_session)
			, dog_(dog) {
		}

		Player()
			: id_(Id(0))
			, game_session_(nullptr) {
		};

		const Id& GetId() const noexcept {
			return id_;
		}

		model::GameSession* GetGameSession()  noexcept {
			return game_session_;
		}

		std::shared_ptr<model::Dog> GetDogName() {
			if (!dog_) {
				throw std::invalid_argument("”казатель dog_ равен nullptr");
			}
			return dog_;
		}

		const model::PlayerSpeed& GetSpeed() const noexcept {
			return dog_->GetSpeed();
		}

		void SetSpeed(model::PlayerSpeed speed) {
			dog_->SetSpeed(std::move(speed));
		}

		void SetDir(model::Direction dir) {
			dog_->SetDir(std::move(dir));
		}

		model::Direction GetDir() const {
			return dog_->GetDir();
		}

	private:
		model::GameSession* game_session_;
		std::shared_ptr<model::Dog> dog_;
		Id id_;
	};

	class Players {
	public:
		Player& Add(std::shared_ptr<model::Dog> dog, model::GameSession* game_session) {

			if (!game_session) {
				throw std::invalid_argument("”казатель game_session равен nullptr");
			}
			std::cout << game_session;
			auto key = std::pair{ dog->GetName(), *(game_session->GetMap()->GetId()) };
			players_.emplace(key, std::move(Player(Player::Id(dog->GetId()), game_session, dog)));
			return players_[key];

		};

		const Player* FindByDogIdAndMapId(std::string name, model::Map::Id map_id) {
			if (auto it_player = players_.find(std::pair{ name, *map_id }); it_player != players_.end()) {
				return &it_player->second;
			}
			return nullptr;
		}

	private:
		std::unordered_map<std::pair<std::string, std::string>, Player, PlayersHash> players_;
	};

	class PlayerTokens {
	private:
		std::random_device random_device_;
		std::mt19937_64 generator1_{ [this] {
			std::uniform_int_distribution<std::mt19937_64::result_type> dist;
			return dist(random_device_);
		}() };
		std::mt19937_64 generator2_{ [this] {
			std::uniform_int_distribution<std::mt19937_64::result_type> dist;
			return dist(random_device_);
		}() };

	public:

		template<typename NewPlayer>
		Token AddPlayer(NewPlayer&& player) {

			std::uint64_t num1 = generator1_();
			std::uint64_t num2 = generator2_();
			std::stringstream ss;
			ss << std::hex << std::setfill('0') << std::setw(16) << num1
				<< std::hex << std::setfill('0') << std::setw(16) << num2;

			auto token = Token(ss.str());
			token_to_player_[token] = std::make_shared<Player>(std::forward<NewPlayer>(player));
			return token;
		}

		std::shared_ptr<Player> FindPlayerByToken(Token token) {
			if (token_to_player_.count(token)) {
				return token_to_player_[token];
			}
			return nullptr;
		}

		Token FindTokenByPlayer(const Player* player) const {
			auto it = std::find_if(token_to_player_.begin(), token_to_player_.end(), [&](const auto& pair) {
				return pair.second.get() == player;
				}
			);
			if (it != token_to_player_.end()) {
				return it->first;
			}

			throw GameError(AuthorizationGameErrorReason::AUTHORIZATION_TOKEN_NOT_FOUND);
		}

	private:
		std::unordered_map<Token, std::shared_ptr<Player>, TokenHash> token_to_player_;
	};

	class GameResult {
	public:
		GameResult(Token token, Player::Id id)
			: token_(token)
			, id_(id) {
		}

		Token GetPlayerTokens() const noexcept {
			return token_;
		}

		Player::Id GetPlayerId() const noexcept {
			return id_;
		}
	private:
		Token token_;
		Player::Id id_;
	};


	class JoinGameUseCase {
	public:
		JoinGameUseCase(std::shared_ptr<model::Game> game, std::shared_ptr<PlayerTokens> player_tokens, std::shared_ptr<Players> players)
			: game_(game)
			, player_tokens_(player_tokens)
			, players_(players) {
		}

		GameResult JoinGame(std::string map_id, std::string name) {
			
			if (name.empty()) {
				throw GameError(JoinGameErrorReason::INVALIDE_NAME);
			}
			if (auto* session = game_->FindGameSessions(model::Map::Id(map_id)); session) {

				auto spawn_point = model::PointD{ 0.,0. };
				model::ConstPtrRoad road = nullptr;


				if (!session->GetMap()->GetRoads().empty()) {
					auto road_ = session->GetMap()->GetRoads()[0];
					spawn_point = model::PointD{ static_cast<double>(road_->GetStart().x), static_cast<double>(road_->GetStart().y) };
			

					road = (road_);
				}

				if (!players_) {
					//throw TODO ERROR
				}

				auto& player = players_->Add(session->AddDog(spawn_point, std::move(name), road), session);
	
				if (!player_tokens_) {
					//throw TODO ERROR
				}
				auto token = player_tokens_->AddPlayer(player);
				return { token, player.GetId() };
			}
			throw GameError(JoinGameErrorReason::INVALIDE_MAP);
		}

		std::shared_ptr<Players> GetListPlayersUseCase() const noexcept {
			return players_;
		}
	private:
		std::shared_ptr<model::Game> game_;
		std::shared_ptr<PlayerTokens> player_tokens_;
		std::shared_ptr<Players> players_;
	};

	class Authorization {
	public:
		Authorization(std::shared_ptr<PlayerTokens> player_tokens)
			: player_tokens_(player_tokens) {
		}

		model::GameSession::Dogs AuthorizationGame(std::string_view authorization_body) {
			if (!player_tokens_) {
				throw std::invalid_argument("”казатель player_tokens_ равен nullptr");
			}

			std::string name;
			std::string token;

			if (authorization_body.empty()) {
				throw GameError(AuthorizationGameErrorReason::AUTHORIZATION_HEADER_MISSING);
			}

			auto pos = authorization_body.find(' ');
			if (pos == std::string::npos) {
				throw GameError(AuthorizationGameErrorReason::AUTHORIZATION_HEADER_MISSING);
			}

			name = authorization_body.substr(0, pos);
			token = authorization_body.substr(pos + 1);

			if (name.empty() || token.empty() || name != "Bearer") {
				throw GameError(AuthorizationGameErrorReason::AUTHORIZATION_HEADER_MISSING);
			}

			auto player = player_tokens_->FindPlayerByToken((Token(token)));

			if (!player) {
				throw GameError(AuthorizationGameErrorReason::AUTHORIZATION_TOKEN_NOT_FOUND);
			}

			auto game_session = player->GetGameSession();
			return game_session->GetDogs();
		}

		Token TryExtractToken(std::string_view authorization_body) {
			std::string bearer;
			std::string token;


			if (authorization_body.empty()) {
				throw GameError(AuthorizationGameErrorReason::AUTHORIZATION_INVALIDE_TOKEN);
			}

			auto pos = authorization_body.find(' ');
			if (pos == std::string::npos) {
				throw GameError(AuthorizationGameErrorReason::AUTHORIZATION_INVALIDE_TOKEN);
			}

			bearer = authorization_body.substr(0, pos);
			token = authorization_body.substr(pos + 1);

			if (bearer.empty() || token.empty() || bearer != "Bearer" || token.length() != 32) {

				throw GameError(AuthorizationGameErrorReason::AUTHORIZATION_INVALIDE_TOKEN);
			}
			return Token(token);
		}



		//StringResponse GetPlayers(const StringRequest& request) {
		//	return ExecuteAuthorized([](const Token& token) {
		//		/* ¬ыполн€ем действие A, использу€ токен, и возвращаем ответ */
		//		});
		//}

		//StringResponse SetPlayerAction(const StringRequest& request) {
		//	return ExecuteAuthorized([](const Token& token) {
		//		/* ¬ыполн€ем действие B, использу€ токен, и возвращаем ответ */
		//		});
		//}

		//StringResponse GetGameState(const StringRequest& request) {
		//	return ExecuteAuthorized([](const Token& token) {
		//		/* ¬ыполн€ем действие C, использу€ токен, и возвращаем ответ */
		//		});
		//}

		Token FindTokenByPlayer(const Player* player) const {
			return player_tokens_->FindTokenByPlayer(player);
		}

		const std::shared_ptr<Player> FindTokenByPlayer(const Token& token) const {
			if (auto player = player_tokens_->FindPlayerByToken(token); player) {
				return player;
			}
			throw GameError(AuthorizationGameErrorReason::AUTHORIZATION_TOKEN_NOT_FOUND);
		}


	private:
		std::shared_ptr<PlayerTokens> player_tokens_;

	};


	class Application : public Authorization {
	public:
		Application(std::shared_ptr<model::Game> game, JoinGameUseCase& join_game_use_case, std::shared_ptr<PlayerTokens> player_tokens)
			: Authorization(player_tokens)
			, game_(game)
			, join_game_use_case_(join_game_use_case) {
			if (!game_) {
				throw std::invalid_argument("”казатель game_ равен nullptr");
			}
		}

		const model::Game::Maps GetMaps() const noexcept {
			return game_->GetMaps();
		}

		const model::Game::MapPtr FindMap(const model::Map::Id& id) const noexcept {
			return game_->FindMap(id);
		}

		GameResult JoinGame(std::string map_id, std::string name) {
			try {
				return join_game_use_case_.JoinGame(std::move(map_id), std::move(name));
			}
			catch (...) {
				throw;
			}
		}

		std::shared_ptr<Players> GetListPlayersUseCase() const noexcept {
			return join_game_use_case_.GetListPlayersUseCase();
		}

		std::string GetGameState(std::string_view authorization_body) {
			try {

				auto token = TryExtractToken(authorization_body);
				auto player = FindTokenByPlayer(token);
				auto game_session = player->GetGameSession();
				auto dogs = game_session->GetDogs();
				json::object obj;
				obj["players"s] = json::object();

				for (const auto& dog : dogs) {
					json::object player;
					std::string dir;
					switch (dog.second->GetDir()) {
					case model::Direction::DIR_NORTH:
						dir = "U"s;
						break;
					case model::Direction::DIR_SOUTH:
						dir = "D"s;
						break;
					case model::Direction::DIR_EAST:
						dir = "R"s;
						break;
					case model::Direction::DIR_WEST:
						dir = "L"s;
						break;
					}

					json::array arr_pos;
					arr_pos.push_back(dog.second->GetPos().x);
					arr_pos.push_back(dog.second->GetPos().y);

					json::array arr_speed;
					arr_speed.push_back(dog.second->GetSpeed().x);
					arr_speed.push_back(dog.second->GetSpeed().y);

					obj["players"s].as_object()[std::to_string(dog.second->GetId())] =
						json::object{ {"pos"s, arr_pos},
										{"speed"s, arr_speed},
										{"dir"s, dir} };

				}
				return json::serialize(obj);
			}
			catch (app::GameError<app::AuthorizationGameErrorReason> err) {
				if (err.GetErrorReason() == AUTHORIZATION_INVALIDE_TOKEN) {
					throw GameError(AuthorizationGameErrorReason::AUTHORIZATION_HEADER_REQ);
				}
				else {
					throw err;
				}
			}
			catch (...) {
				throw;
			}
		}

		std::string SetPlayerAction(std::string_view authorization_body, std::string base_body) {
			try {
				 
				auto token = TryExtractToken(authorization_body);
				auto player = FindTokenByPlayer(token);
				auto json_obj = json::parse(base_body).as_object();
				auto dir = json_obj.at("move").as_string().c_str();
				auto speed = player->GetGameSession()->GetMap()->GetSpeed();
				
				if (std::strcmp(dir, "") == 0) {
					player->SetSpeed({ 0,0 });
				}
				else if (std::strcmp(dir, "L") == 0) {
					player->SetSpeed({ -speed,0 });
					player->SetDir(model::Direction::DIR_WEST);
				}
				else if (std::strcmp(dir, "R") == 0) {
					player->SetSpeed({ speed,0 });
					player->SetDir(model::Direction::DIR_EAST);
				}
				else if (std::strcmp(dir, "U") == 0) {
					player->SetSpeed({ 0,-speed });
					player->SetDir(model::Direction::DIR_NORTH);
				}
				else if (std::strcmp(dir, "D") == 0) {
					player->SetSpeed({ 0,speed });
					player->SetDir(model::Direction::DIR_SOUTH);
				}
				else {
					throw GameError(ActionGameErrorReason::FAILED_PARSE_ACTION);
				}

				return json::serialize(json::object());
			}
			catch (app::GameError<app::AuthorizationGameErrorReason> err) {

				if (err.GetErrorReason() == AUTHORIZATION_INVALIDE_TOKEN) {
					throw GameError(AuthorizationGameErrorReason::AUTHORIZATION_HEADER_REQ);
				}
				else {
					throw err;
				}
			}
			catch (...) {
				throw;
			}
		}

		std::string SetTimeDelta(std::string base_body) {
			try {
				auto json_obj = json::parse(base_body).as_object();
				std::chrono::milliseconds time(json_obj.at("timeDelta").as_int64());
				UpdateGameState(time);
				return json::serialize(json::object());
			}
			catch (...) {
				throw;
			}
		}

		void UpdateGameState(std::chrono::milliseconds delta) {
			try {
				auto time = delta.count();
				if (time <= 0) {
					throw;
				}

				auto maps = game_->GetMaps();

				for (auto map : maps) {

					if (auto* session = game_->FindGameSessions(map->GetId()); session) {
						auto dogs = session->GetDogs();

						for (auto dog : dogs) {
							auto dog_ = dog.second;

							auto roads = map->GetRoadmap();
							auto new_x = dog_->GetPos().x + (dog_->GetSpeed().x * time / 1000);
							auto new_y = dog_->GetPos().y + (dog_->GetSpeed().y * time / 1000);
							auto cur_dir = dog_->GetDir();
							double w_road = 0.4;

							switch (cur_dir) {
							case model::Direction::DIR_NORTH:
								if (dog_->GetSpeed().y != 0) {
									GoToNorth(roads, dog_, new_y, w_road);
								}
								break;
							case model::Direction::DIR_SOUTH:
								if (dog_->GetSpeed().y != 0) {
									GoToSouth(roads, dog_, new_y, w_road);
								}
								break;
							case model::Direction::DIR_WEST:
								if (dog_->GetSpeed().x != 0) {
									GoToWest(roads, dog_, new_x, w_road);
								}
								break;
							case model::Direction::DIR_EAST:
								if (dog_->GetSpeed().x != 0) {
									GoToEast(roads, dog_, new_x, w_road);
								}
								break;
							}
						}
					}
				}
			}
			catch (...) {
				throw;
			}
		}

	private:
		void GoToSouth(model::Map::Roadmap& roadmap, const std::shared_ptr<model::Dog>& dog, double new_pos, double w_road) {
			auto curr_road = dog->GetCurrentRoad();
			auto cur_dog_pos = dog->GetPos();
			int max_pos = std::max(curr_road->GetStart().y, curr_road->GetEnd().y);

			if (new_pos <= max_pos + w_road) {
				dog->SetPos(model::PointD(dog->GetPos().x, new_pos));
				return;
			}

			model::Map::Roadmap::iterator road;

			if (curr_road->IsHorizontal()) {
				if (cur_dog_pos.x >= curr_road->GetStart().x - w_road && cur_dog_pos.x <= curr_road->GetStart().x + w_road) {
					road = roadmap.find(std::pair{ curr_road->GetStart(), model::Direction::DIR_SOUTH });

				}
				else if (cur_dog_pos.x >= curr_road->GetEnd().x - w_road && cur_dog_pos.x <= curr_road->GetEnd().x + w_road) {
					road = roadmap.find(std::pair{ curr_road->GetEnd(), model::Direction::DIR_SOUTH });
				}
			}
			else {
				road = roadmap.find(std::pair{ model::Point{curr_road->GetEnd().x, max_pos}, model::Direction::DIR_SOUTH });
			}

			while (road != roadmap.end() && new_pos > (max_pos = std::max(road->second->GetStart().y, road->second->GetEnd().y))) {
				road = roadmap.find(std::pair{ model::Point{curr_road->GetEnd().x, max_pos}, model::Direction::DIR_SOUTH });
			}

			if (road != roadmap.end()) {
				dog->SetNewRoad(road->second);
				dog->SetPos(model::PointD(dog->GetPos().x, new_pos));
			}
			else {
				dog->SetSpeed(model::PlayerSpeed{ 0, 0 });
				dog->SetPos(model::PointD(dog->GetPos().x, max_pos + w_road));
			}
		}
		  

		void GoToNorth(model::Map::Roadmap& roadmap, const std::shared_ptr<model::Dog>& dog, double new_pos, double w_road) {
			auto curr_road = dog->GetCurrentRoad();
			auto cur_dog_pos = dog->GetPos();
			int min_pos = std::min(curr_road->GetStart().y, curr_road->GetEnd().y);

			if (new_pos >= min_pos - w_road) {
				dog->SetPos(model::PointD(dog->GetPos().x, new_pos));
				return;
			}

			model::Map::Roadmap::iterator road;

			if (curr_road->IsHorizontal()) {
				if (cur_dog_pos.x >= curr_road->GetStart().x - w_road && cur_dog_pos.x <= curr_road->GetStart().x + w_road) {
					road = roadmap.find(std::pair{ curr_road->GetStart(), model::Direction::DIR_NORTH });

				}
				else if (cur_dog_pos.x >= curr_road->GetEnd().x - w_road && cur_dog_pos.x <= curr_road->GetEnd().x + w_road) {
					road = roadmap.find(std::pair{ curr_road->GetEnd(), model::Direction::DIR_NORTH });
				}
			}
			else {
				road = roadmap.find(std::pair{ model::Point{curr_road->GetEnd().x, min_pos}, model::Direction::DIR_NORTH });
			}

			while (road != roadmap.end() && new_pos > (min_pos = std::min(road->second->GetStart().y, road->second->GetEnd().y))) {
				road = roadmap.find(std::pair{ model::Point{curr_road->GetEnd().x, min_pos}, model::Direction::DIR_NORTH });
			}

			if (road != roadmap.end()) {
				dog->SetNewRoad(road->second);
				dog->SetPos(model::PointD(dog->GetPos().x, new_pos));
			}
			else {
				dog->SetSpeed(model::PlayerSpeed{ 0, 0 });
				dog->SetPos(model::PointD(dog->GetPos().x, min_pos - w_road));
			}
		}

		void GoToWest(model::Map::Roadmap& roadmap, const std::shared_ptr<model::Dog>& dog, double new_pos, double w_road) {
			auto curr_road = dog->GetCurrentRoad();
			auto cur_dog_pos = dog->GetPos();
			int min_pos = std::min(curr_road->GetStart().x, curr_road->GetEnd().x);

			if (new_pos >= min_pos - w_road) {
				dog->SetPos(model::PointD(new_pos, dog->GetPos().y));
				return;
			}

			model::Map::Roadmap::iterator road;

			if (curr_road->IsVertical()) {

				if (cur_dog_pos.y >= curr_road->GetStart().y - w_road && cur_dog_pos.y <= curr_road->GetStart().y + w_road) {
					road = roadmap.find(std::pair{ curr_road->GetStart(), model::Direction::DIR_WEST });

				}
				else if (cur_dog_pos.y >= curr_road->GetEnd().y - w_road && cur_dog_pos.y <= curr_road->GetEnd().y + w_road) {
					road = roadmap.find(std::pair{ curr_road->GetEnd(), model::Direction::DIR_WEST });
				}
			}
			else {
				road = roadmap.find(std::pair{ model::Point{min_pos, curr_road->GetEnd().y}, model::Direction::DIR_WEST });
			}

			while (road != roadmap.end() && new_pos > (min_pos = std::min(road->second->GetStart().x, road->second->GetEnd().x))) {
				road = roadmap.find(std::pair{ model::Point{min_pos, curr_road->GetEnd().y}, model::Direction::DIR_WEST });
			}

			if (road != roadmap.end()) {
				dog->SetNewRoad(road->second);
				dog->SetPos(model::PointD(new_pos, dog->GetPos().y));
			}
			else {
				dog->SetSpeed(model::PlayerSpeed{ 0, 0 });
				dog->SetPos(model::PointD(min_pos - w_road, dog->GetPos().y));
			}
		}

		void GoToEast(model::Map::Roadmap& roadmap, const std::shared_ptr<model::Dog>& dog, double new_pos, double w_road) {
			auto curr_road = dog->GetCurrentRoad();
			auto cur_dog_pos = dog->GetPos();
			int max_pos = std::max(curr_road->GetStart().x, curr_road->GetEnd().x);

			if (new_pos <= max_pos + w_road) {
				dog->SetPos(model::PointD(new_pos, dog->GetPos().y));
				return;
			}

			model::Map::Roadmap::iterator road;

			if (curr_road->IsVertical()) {


				if (cur_dog_pos.y >= curr_road->GetStart().y - w_road && cur_dog_pos.y <= curr_road->GetStart().y + w_road) {
					road = roadmap.find(std::pair{ curr_road->GetStart(), model::Direction::DIR_EAST });

				}
				else if (cur_dog_pos.y >= curr_road->GetEnd().y - w_road && cur_dog_pos.y <= curr_road->GetEnd().y + w_road) {
					road = roadmap.find(std::pair{ curr_road->GetEnd(), model::Direction::DIR_EAST });
				}
			}
			else {
				road = roadmap.find(std::pair{ model::Point{max_pos, curr_road->GetEnd().y}, model::Direction::DIR_EAST });
			}


			while (road != roadmap.end() && new_pos > (max_pos = std::max(road->second->GetStart().x, road->second->GetEnd().x))) {

				road = roadmap.find(std::pair{ model::Point{max_pos, road->second->GetEnd().y}, model::Direction::DIR_EAST });
			}

			if (road != roadmap.end()) {
				dog->SetNewRoad(road->second);
				dog->SetPos(model::PointD(new_pos, dog->GetPos().y));
			}
			else {
				dog->SetSpeed(model::PlayerSpeed{ 0, 0 });
				dog->SetPos(model::PointD(max_pos + w_road, dog->GetPos().y));
			}
		}

	
private:
		std::shared_ptr<model::Game> game_;
		JoinGameUseCase& join_game_use_case_;
	};
}

