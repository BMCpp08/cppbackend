#include "connection_pool_impl.h"
#include <pqxx/pqxx>

namespace app {
void ConnectionPoolImpl::PrepareAllConnections(const std::string& db_url, const int size) {
    
    pool_ = std::make_unique<ConnectionPool>(size, [db_url] {
        auto conn = std::make_shared<pqxx::connection>(db_url);

        // Вставка записи о вышедшем на пенсию игроке
        conn->prepare(tag_ins_retired,
            "INSERT INTO retired_players (id, player_name, score, play_time) VALUES ($1, $2, $3, $4)"_zv);

        // Получение списка рекордов
        conn->prepare(tag_get_records,
            "SELECT id, player_name AS name, score, play_time "
            "FROM retired_players "
            "ORDER BY score DESC, play_time ASC, player_name ASC "
            "LIMIT $1 OFFSET $2"_zv);

        return conn;
        });
}

ConnectionPool& ConnectionPoolImpl::GetPool() {
    return *pool_;
}
}  // namespace app
