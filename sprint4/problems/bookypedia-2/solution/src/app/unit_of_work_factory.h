#pragma once

#include <string>
#include <vector>
#include "../domain/author.h"
#include "../domain/book.h"

namespace app {
	class UnitOfWork {
	public:
		virtual void Commit() = 0;
		virtual domain::AuthorRepository& Authors() = 0;
		virtual domain::BookRepository& Books() = 0;
		virtual domain::TagRepository& Tags() = 0;
		virtual void Rollback() = 0;
	protected:
		~UnitOfWork() = default;
	};

	class UnitOfWorkFactory {
	public:
		virtual std::shared_ptr<UnitOfWork> CreateUnitOfWork() = 0;

	protected:
		~UnitOfWorkFactory() = default;
	};

}  // namespace app
