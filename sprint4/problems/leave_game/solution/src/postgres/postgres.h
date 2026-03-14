#pragma once
#include <iostream>
#include "../app/connection_pool_impl.h"
#include "../app/retiree.h"

namespace postgres {
	using namespace std::literals;
	class RetireeRepositoryImpl : public app::RetireeRepository {
	public:
		explicit RetireeRepositoryImpl(pqxx::work& work)
			: work_(work) {
		}

		void Save(const app::Retiree& retiree) override {
			work_.exec_prepared(app::tag_ins_retired, retiree.GetName(), retiree.GetScore(), retiree.GetPlayTime());
		}

		const std::vector<app::Retiree> GetRetirees(const int start_idx, const int count) override {
			std::vector<app::Retiree> retirees;
			auto result = work_.exec_prepared(app::tag_get_records, count, start_idx);

			for (const auto& row : result) {
				retirees.emplace_back(app::RetireeId::FromString(row["id"].as<std::string>()), row["name"].is_null() ? "" : row["name"].as<std::string>(),
									row["score"].as<double>(0.0), 
									row["play_time_ms"].as<int64_t>(0)
									);
			}
			return retirees;
		}

	private:
		pqxx::work& work_;
	};

	class Database {
	public:
		explicit Database(const std::string& db_url);
	
		template <typename F>
		const std::vector<app::Retiree> GetRetirees(F&& func) {
			auto conn = pool_.GetPool().GetConnection();          
			pqxx::work tx(*conn);
			postgres::RetireeRepositoryImpl repo(tx);
			try {
				auto result = func(repo);              
				tx.commit();
				return result;
			}
			catch (...) {
				tx.abort();
				throw;
			}
		}

		template <typename F>
		void SaveRetirees(F&& func) {
			auto conn = pool_.GetPool().GetConnection();
			pqxx::work tx(*conn);
			postgres::RetireeRepositoryImpl repo(tx);
			try {
				func(repo);                              
				tx.commit();
			}
			catch (...) {
				tx.abort();
				throw;
			}
		}

		void PreparePollConnections(const std::string& db_url, const int size) {
			pool_.PrepareAllConnections(db_url, size);
		}
	private:
		app::ConnectionPoolImpl pool_;
	};
	
}  // namespace postgres