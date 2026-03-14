#pragma once

#include "connection_pool_impl.h"
#include <pqxx/pqxx>

namespace app {


//ConnectionPoolImpl::ConnectionPoolImpl(const std::string& db_url, const int size)
//    : pool_(size, [db_url] {
//        auto conn = std::make_shared<pqxx::connection>(db_url);
//
//        conn->prepare(tag_ins_retired,
//            "INSERT INTO retired_players (id, player_name, score, play_time_ms) VALUES ($1, $2, $3, $4)"_zv);
//
//        conn->prepare(tag_get_records,
//            "SELECT player_name AS id, name, score, play_time_ms / 1000.0 AS playTime "
//            "FROM retired_players ORDER BY score DESC, play_time_ms, player_name LIMIT $1 OFFSET $2"_zv);
//        return conn;
//    }){
//}

void ConnectionPoolImpl::PrepareAllConnections(const std::string& db_url, const int size) {
    
    pool_ = std::make_unique<ConnectionPool>(size, [db_url] {
        auto conn = std::make_shared<pqxx::connection>(db_url);
        conn->prepare(tag_ins_retired,
            "INSERT INTO retired_players (id, player_name, score, play_time_ms) VALUES ($1, $2, $3, $4)"_zv);

        conn->prepare(tag_get_records,
            "SELECT player_name AS id, name, score, play_time_ms / 1000.0 AS playTime "
            "FROM retired_players ORDER BY score DESC, play_time_ms, player_name LIMIT $1 OFFSET $2"_zv);
        return conn;
        });
}

ConnectionPool& ConnectionPoolImpl::GetPool() {
    return *pool_;
}
}  // namespace app
