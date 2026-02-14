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

	enum JoinGameErrorReason {
		INVALIDE_NAME = 0u,
		INVALIDE_MAP,
	};

	enum AuthorizationGameErrorReason {
		AUTHORIZATION_HEADER_MISSING = 0u,
		AUTHORIZATION_HEADER_REQ = 0u,
		AUTHORIZATION_TOKEN_NOT_FOUND,
	};

	struct PlayersHash {
		size_t operator()(const std::pair<std::string, std::string>& key) const {
			size_t hash_f = std::hash<std::string>()(key.first);
			size_t hash_c = std::hash<std::string>()(key.second);
			return hash_f ^ (hash_c << 1);
		}
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

		 Id& GetId()  noexcept {
			return id_;
		}

		 model::GameSession* GetGameSession()  noexcept {
			return game_session_;
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
			token_to_player_[token] = std::make_shared<Player>(std::forward<NewPlayer>(player));;
			return token;
		}

		std::shared_ptr<Player> FindPlayerByToken(Token token) {
			if (token_to_player_.count(token)) {
				return token_to_player_[token];
			}
			return nullptr;
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
				
				auto spawn_point = model::Point{ 0,0 };
				if (!players_) {
					//throw TODO ERROR
				}

				auto& player = players_->Add(session->AddDog(spawn_point, std::move(name)), session);
			
				if (!player_tokens_) {
					//throw TODO ERROR
				}
		
				auto token = player_tokens_->AddPlayer(player);

				std::cout <<"token " << *token << std::endl;
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
		: player_tokens_(player_tokens){
		}

		model::GameSession::Dogs AuthorizationGame(std::string_view authorization_body)  {
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
			model::GameSession::Dogs dogs = game_session->GetDogs();
			return dogs;
		}

	private:
		std::shared_ptr<PlayerTokens> player_tokens_;

	};

	class Application : public Authorization{
	public:
		Application(std::shared_ptr<model::Game> game, JoinGameUseCase& join_game_use_case, std::shared_ptr<PlayerTokens> player_tokens)
			: Authorization(player_tokens)
			, game_(game)
			, join_game_use_case_(join_game_use_case) {
			if (!game_) {
				throw std::invalid_argument("”казатель game_ равен nullptr");
			}
		}

		const model::Game::Maps& GetMaps() const noexcept {
			return game_->GetMaps();
		}

		const model::Map* FindMap(const model::Map::Id& id) const noexcept {
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
	private:
		std::shared_ptr<model::Game> game_;
		JoinGameUseCase& join_game_use_case_;
	};
}

