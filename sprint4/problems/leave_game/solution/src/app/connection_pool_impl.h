#pragma once

#include <string>
#include <vector>
#include "connection_pool.h"
#include "../app/retiree.h"

namespace app {
    using pqxx::operator"" _zv;
    constexpr auto tag_ins_retired = "insert_retired"_zv;
    constexpr auto tag_get_records = "get_records"_zv;
	using namespace std::literals;
	class RetireeRepositoryImpl : public app::RetireeRepository {
	public:
		explicit RetireeRepositoryImpl(pqxx::work& work)
			: work_(work) {
		}

		void Save(const app::Retiree& retiree) override {
			work_.exec_prepared(app::tag_ins_retired, retiree.GetId().ToString(), retiree.GetName(), retiree.GetScore(), retiree.GetPlayTime());
		}

		const std::vector<app::Retiree> GetRetirees(const int start_idx, const int count) override {
			std::vector<app::Retiree> retirees;
			auto result = work_.exec_prepared(app::tag_get_records, count, start_idx);

			for (const auto& row : result) {
				retirees.emplace_back(
					app::RetireeId::FromString(row["id"].as<std::string>()), 
					row["name"].is_null() ? "" : row["name"].as<std::string>(),
					row["score"].as<double>(0.0),
					row["play_time"].as<double>(0)
				);
			}
			return retirees;
		}

	private:
		pqxx::work& work_;
	};
class ConnectionPoolImpl {
public:
    ConnectionPoolImpl() = default;
    /*explicit ConnectionPoolImpl();*/

    void PrepareAllConnections(const std::string& db_url, const int size);
    ConnectionPool& GetPool();

	template <typename F>
	std::vector<app::Retiree> GetRetirees(F&& func) {
		auto conn = pool_->GetConnection();
		pqxx::work tx(*conn);
		RetireeRepositoryImpl repo(tx);
		try {
			auto result = func(repo);
			/*tx.commit();*/
			return result;
		}
		catch (...) {
			/*tx.abort();*/
			throw;
		}
	}

	template <typename F>
	void SaveRetirees(F&& func) {
		auto conn = pool_->GetConnection();
		pqxx::work tx(*conn);
		RetireeRepositoryImpl repo(tx);
		try {
			func(repo);
			tx.commit();
		}
		catch (...) {
			tx.abort();
			throw;
		}
	}
private:
    std::unique_ptr<ConnectionPool> pool_;
};
}  // namespace app
