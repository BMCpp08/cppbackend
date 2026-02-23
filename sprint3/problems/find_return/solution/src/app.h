#pragma once

#define BOOST_BEAST_USE_STD_STRING_VIEW
#include "logger.h"
#include "sdk.h"
#include "model.h"
#include "tagged.h"
#include <string>
#include <random>
#include <unordered_map>
#include <memory>
#include "boost_includes.h"
#include "ticker.h"

namespace app {
	using namespace std::literals;
	using namespace boost_aliases;

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

	enum ErrorReason {
		FAILED_PARSE_JSON = 0u,
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

		const Id& GetId() const noexcept;

		model::GameSession* GetGameSession()  noexcept;

		std::shared_ptr<model::Dog> GetDogName();

		const model::PlayerSpeed& GetSpeed() const noexcept;

		void SetSpeed(model::PlayerSpeed speed);

		void SetDir(model::Direction dir);

		model::Direction GetDir() const;

	private:
		model::GameSession* game_session_;
		std::shared_ptr<model::Dog> dog_;
		Id id_;
	};

	class Players {
	public:
		Player& Add(std::shared_ptr<model::Dog> dog, model::GameSession* game_session);

		const Player* FindByDogIdAndMapId(std::string name, model::Map::Id map_id);

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

		std::shared_ptr<Player> FindPlayerByToken(Token token);

		Token FindTokenByPlayer(const Player* player) const;

	private:
		std::unordered_map<Token, std::shared_ptr<Player>, TokenHash> token_to_player_;
	};

	class GameResult {
	public:
		GameResult(Token token, Player::Id id)
			: token_(token)
			, id_(id) {
		}

		Token GetPlayerTokens() const noexcept;

		Player::Id GetPlayerId() const noexcept;
	private:
		Token token_;
		Player::Id id_;
	};

	class JoinGameUseCase {
	public:
		JoinGameUseCase(std::shared_ptr<model::Game> game,
			std::shared_ptr<PlayerTokens> player_tokens,
			std::shared_ptr<Players> players,
			bool is_random_positions = false)
			: game_(game)
			, player_tokens_(player_tokens)
			, players_(players)
			, is_random_positions_(is_random_positions) {
		}

		GameResult JoinGame(std::string map_id, std::string name);

		std::shared_ptr<Players> GetListPlayersUseCase() const noexcept;
	private:
		std::shared_ptr<model::Game> game_;
		std::shared_ptr<PlayerTokens> player_tokens_;
		std::shared_ptr<Players> players_;
		bool is_random_positions_;
	};

	class Authorization {
	public:
		Authorization(std::shared_ptr<PlayerTokens> player_tokens)
			: player_tokens_(player_tokens) {
		}

		Token TryExtractToken(std::string_view authorization_body);

		Token FindTokenByPlayer(const Player* player) const;

		const std::shared_ptr<Player> FindPlayerByToken(const Token& token) const;

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
				throw std::invalid_argument("Invalid ptr game_ = nullptr");
			}
			if (!player_tokens) {
				throw std::invalid_argument("Invalid ptr player_tokens = nullptr");
			}
		}

		const model::Game::Maps GetMaps() const noexcept;

		const model::Game::MapPtr FindMap(const model::Map::Id& id) const noexcept;

		GameResult JoinGame(std::string map_id, std::string name);

		void Tick(std::chrono::milliseconds delta);

		std::shared_ptr<Players> GetListPlayersUseCase() const noexcept;

		std::string GetPlayers(std::string_view authorization_body);

		std::string GetGameState(std::string_view authorization_body);

		std::string SetPlayerAction(std::string_view authorization_body, std::string base_body);

		std::string SetTimeDelta(std::string base_body);

		void UpdateGameState(std::chrono::milliseconds delta);

	private:
		void GoToSouth(model::Map::Roadmap& roadmap, const std::shared_ptr<model::Dog>& dog, double new_pos, double w_road);

		void GoToNorth(model::Map::Roadmap& roadmap, const std::shared_ptr<model::Dog>& dog, double new_pos, double w_road);

		void GoToWest(model::Map::Roadmap& roadmap, const std::shared_ptr<model::Dog>& dog, double new_pos, double w_road);

		void GoToEast(model::Map::Roadmap& roadmap, const std::shared_ptr<model::Dog>& dog, double new_pos, double w_road);
	private:
		std::shared_ptr<model::Game> game_;
		JoinGameUseCase& join_game_use_case_;
	};
}

