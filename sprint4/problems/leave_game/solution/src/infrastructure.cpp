#include "infrastructure.h"

namespace infrastructure {

	void SerializingListener::OnTick(std::chrono::milliseconds timestamp) {
		if (timestamp - time_since_save_ >= save_period_) {
			SaveState();
			time_since_save_ = timestamp;
		}
	}

	void SerializingListener::SaveState() {
		std::string tmp_file = state_file_ + ".tmp"s;
		try {
			std::ofstream ofs(tmp_file, std::ios::binary);
			if (!ofs) {
				throw std::runtime_error("Cannot open temp file!");
			}
			OutputArchive output_archive(ofs);
			const auto& game = app_.GetGame();
			const auto& maps = game->GetMaps();

			if (!game) {
				throw std::logic_error("Ptr game if null!");
			}

			auto players = app_.GetListPlayersUseCase();
			std::vector<serialization::MapRepr> maps_repr;
			for (const auto& map : maps) {
				std::vector<serialization::DogRepr> dogs_repr;
				std::vector<serialization::LootRepr> loots_repr;
				std::vector<serialization::PlayerRepr> players_repr;

				if (auto* session = game->FindGameSessions(map->GetId()); session) {
					auto dogs = session->GetDogs();

					for (const auto& dog : dogs) {
						dogs_repr.emplace_back(*dog.second);
						const app::Player* player = players->FindByDogIdAndMapId(dog.second->GetName(), map->GetId());
						players_repr.emplace_back(player, player->GetToken());
					}

					auto loots = map->GetLoots();
					for (const auto& loot : loots) {
						loots_repr.emplace_back(loot);
					}
				}
				maps_repr.push_back(serialization::MapRepr{ map->GetId(), players_repr, loots_repr, dogs_repr });
			}

			output_archive << maps_repr;

			ofs.flush();
			ofs.close();

			std::filesystem::rename(tmp_file, state_file_);

			std::filesystem::permissions(state_file_,
				std::filesystem::perms::owner_read | std::filesystem::perms::owner_write,
				std::filesystem::perm_options::replace);
		}
		catch (const std::exception& exc) {
			std::cerr << "Save error: " << exc.what() << std::endl;
			std::filesystem::remove(tmp_file);
		}
	}

	void SerializingListener::RestoreGameState(const std::string& state_file) {
		try {
			if (!std::filesystem::exists(state_file_)) {
				return;
			}
			std::ifstream ifs(state_file);
			if (!ifs.is_open()) {
				return;
			}
			InputArchive input_archive(ifs);
			std::vector<serialization::MapRepr> maps_repr;

			input_archive >> maps_repr;
			const auto& game = app_.GetGame();
			for (auto& map_repr : maps_repr) {
				auto map_data = map_repr.Restore();
				if (auto* session = game->FindGameSessions(map_data.id_); session) {

					const auto& map = session->GetMap();
					std::vector<app::Player> players;
					for (auto player_repr : map_data.players_) {
						players.emplace_back(player_repr.Restore());
					}

					for (auto dog_repr : map_data.dogs_) {
						model::Dog dog = dog_repr.Restore();

						auto road = map->GetRoads().at(*dog.GetRoadId());
						dog.SetNewRoad(road);
						auto f_player = std::find_if(players.begin(), players.end(), [&](const app::Player& player) {
							return *dog.GetId() == *player.GetId();
							});

						if (f_player != players.end()) {
							app_.JoinGame(session->AddDog(dog), session, f_player->GetToken());
						}
					}
					for (auto loot_repr : map_data.loots_) {
						map->AddLoot(loot_repr.Restore());
					}
				}
			}
		}
		catch (const std::exception& exc) {
			std::cerr << exc.what() << std::endl;
		}
	}
}

