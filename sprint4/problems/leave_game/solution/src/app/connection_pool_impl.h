#pragma once

#include <string>
#include <vector>
#include "connection_pool.h"

namespace app {
    using pqxx::operator"" _zv;
    constexpr auto tag_ins_retired = "insert_retired"_zv;
    constexpr auto tag_get_records = "get_records"_zv;

class ConnectionPoolImpl {
public:
    ConnectionPoolImpl() = default;
    /*explicit ConnectionPoolImpl();*/
    void PrepareAllConnections(const std::string& db_url, const int size);
    ConnectionPool& GetPool();

private:
    std::unique_ptr<ConnectionPool> pool_;
};
}  // namespace app
