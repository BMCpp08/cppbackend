#pragma once
#include <iostream>
#include "../app/connection_pool_impl.h"
#include "../app/retiree.h"

namespace postgres {


	class Database {
	public:
		explicit Database(const std::string& db_url);
	
		/*template <typename F>
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
		}*/

		/*void PreparePollConnections(const std::string& db_url, const int size) {
			pool_.PrepareAllConnections(db_url, size);
		}*/
	private:
		/*app::ConnectionPoolImpl pool_;*/
	};
	
}  // namespace postgres