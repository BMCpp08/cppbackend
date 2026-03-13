#include "postgres.h"
#include <iostream>

namespace postgres {
	using namespace std::literals;
	using pqxx::operator"" _zv;

	Database::Database(const std::string& db_url)
		: pool_(db_url, 100){
		pqxx::connection conn(db_url);
		pqxx::work work{ conn };

		//Таблица игры
		work.exec(R"(
		CREATE TABLE IF NOT EXISTS retired_players (
			    id UUID CONSTRAINT id_constraint PRIMARY KEY,
				player_name TEXT NOT NULL,
				score DOUBLE PRECISION NOT NULL,
				play_time_ms BIGINT NOT NULL
		);
		)"_zv);
		work.exec(R"(CREATE INDEX IF NOT EXISTS idx_retired_players
			ON retired_players(score DESC, play_time_ms, player_name);)"_zv);

		work.commit();
	}
}  // namespace postgres