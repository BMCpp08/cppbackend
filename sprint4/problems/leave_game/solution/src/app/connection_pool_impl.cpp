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

//void ConnectionPoolImpl::PrepareAllConnections(const std::string& db_url, const int size) {
//    
//    pool_ = std::make_unique<ConnectionPool>(size, [db_url] {
//        auto conn = std::make_shared<pqxx::connection>(db_url);
//        conn->prepare(tag_ins_retired,
//            "INSERT INTO retired_players (id, player_name, score, play_time_ms) VALUES ($1, $2, $3, $4)"_zv);
//
//        conn->prepare("get_records",
//            "SELECT id, player_name AS name, score, play_time_ms / 1000.0 AS playTime "
//            "FROM retired_players ORDER BY score DESC, play_time_ms, player_name LIMIT $1 OFFSET $2"_zv);
//
//       /* conn->prepare(tag_get_records,
//            "SELECT player_name AS id, name, score, play_time_ms / 1000.0 AS playTime "
//            "FROM retired_players ORDER BY score DESC, play_time_ms, player_name LIMIT $1 OFFSET $2"_zv);*/
//        return conn;
//        });
//}
    void ConnectionPoolImpl::PrepareAllConnections(const std::string& db_url, int size) {
        try {
            std::cerr << "Preparing connection pool with size " << size << std::endl;
            pool_ = std::make_unique<ConnectionPool>(size, [db_url] {
                try {
                    std::cerr << "Creating connection and preparing statements" << std::endl;
                    auto conn = std::make_shared<pqxx::connection>(db_url);
                    conn->prepare(tag_ins_retired,
                        "INSERT INTO retired_players (id, player_name, score, play_time_ms) VALUES ($1, $2, $3, $4)");
                    conn->prepare(tag_get_records,
                        "SELECT id, player_name AS name, score, play_time_ms / 1000.0 AS playTime "
                        "FROM retired_players ORDER BY score DESC, play_time_ms, player_name LIMIT $1 OFFSET $2");
                    std::cerr << "Connection prepared successfully" << std::endl;
                    return conn;
                }
                catch (const pqxx::sql_error& e) {
                    std::cerr << "SQL error in connection factory: " << e.what() << std::endl;
                    std::cerr << "Query: " << e.query() << std::endl;
                    throw;
                }
                catch (const std::exception& e) {
                    std::cerr << "Exception in connection factory: " << e.what() << std::endl;
                    throw;
                }
                });
            std::cerr << "Pool created successfully" << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << "Failed to prepare pool: " << e.what() << std::endl;
            throw; // или можно завершить программу, но лучше перебросить, чтобы main поймал
        }
    }
ConnectionPool& ConnectionPoolImpl::GetPool() {
    return *pool_;
}
}  // namespace app
