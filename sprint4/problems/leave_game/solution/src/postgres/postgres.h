#pragma once
#include <iostream>

namespace postgres {
	class Database {
	public:
		explicit Database(const std::string& db_url);
	private:
	};
	
}  // namespace postgres