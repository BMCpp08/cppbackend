#pragma once
#include <pqxx/connection>
#include <pqxx/transaction>
#include <iostream>
#include "../domain/author.h"
#include "../domain/book.h"
#include "../domain/tag.h"
#include "../app/unit_of_work_factory.h"
#include <atomic>
namespace postgres {
	using namespace std::literals;
	class AuthorRepositoryImpl : public domain::AuthorRepository {
	public:
		explicit AuthorRepositoryImpl(pqxx::work& work)
			: work_(work) {
		}

		void Save(const domain::Author& author) override;
		const std::vector<domain::Author> GetAllAuthor() override;

		const std::optional<domain::Author> GetAuthor(const std::string& name) override;

		void DeleteAuthor(const domain::AuthorId& author_id) override;
		void EditAuthor(const domain::Author& author) override;
	private:
		pqxx::work& work_;
	};

	class BookRepositoryImpl : public domain::BookRepository {
	public:
		explicit BookRepositoryImpl(pqxx::work& work)
			: work_(work) {
		}

		void Save(const domain::Book& book) override;
		const std::vector<domain::Book> GetAllBooks() override;
		const std::vector<domain::Book> GetAuthorBooks(domain::AuthorId author_id) override;
		const std::vector<domain::Book> GetBooksByTitle(const std::string& title) override;
		void DeleteBook(const domain::BookId& book_id) override;
		void EditBookById(const domain::Book& book) override;
	private:
		pqxx::work& work_;
	};


	class TagRepositoryImpl : public domain::TagRepository {
	public:
		explicit TagRepositoryImpl(pqxx::work& work)
			: work_(work) {
		}

		void Save(const domain::Tag& book) override {
			work_.exec_params(
				"INSERT INTO book_tags (book_id, tag) VALUES ($1, $2) ON CONFLICT DO NOTHING",
				book.GetBookId().ToString(), book.GetTag()
			);
		}

		const std::vector<std::string> GetAllTags(domain::BookId book_id) override;
		void DeleteAllTags(const domain::BookId& book_id) override;
	private:
		pqxx::work& work_;
		//pqxx::read_transaction& read_;
	};


	class UnitOfWorkImpl : public app::UnitOfWork {
	public:
		/*explicit UnitOfWorkImpl(pqxx::connection& connection)
			: work_(connection)
			, authors_(work_)
			, books_(work_)
			, tags_(work_){
		}*/
		explicit UnitOfWorkImpl(pqxx::connection& connection)
			: id_(++next_id_)
			, work_(connection)
			, authors_(work_)
			, books_(work_)
			, tags_(work_)
		{
			std::cerr << ">>> UnitOfWorkImpl CREATED with id=" << id_ << std::endl;
		}

		~UnitOfWorkImpl() {
			std::cerr << ">>> UnitOfWorkImpl DESTROYED with id=" << id_ << std::endl;
		}

		void Commit() override {
			std::cerr << ">>> UnitOfWorkImpl::Commit id=" << id_ << " STARTING" << std::endl;
			work_.commit();
			std::cerr << ">>> UnitOfWorkImpl::Commit id=" << id_ << " COMPLETED" << std::endl;
		}

		void Rollback() override {
			std::cerr << ">>> UnitOfWorkImpl::Rollback id=" << id_ << std::endl;
			work_.abort();
		}

	/*	void Commit() override {
			work_.commit();
		}*/

	/*	void Rollback() override {
			work_.abort();
		}*/
		
		domain::AuthorRepository& Authors() override {
			return authors_;
		}

		domain::BookRepository& Books() override {
			return books_;
		}

		domain::TagRepository& Tags() override {
			return tags_;
		}

		//void SaveAuthor(const domain::Author& author) {
		//	authors_.Save(author);
		//}

		//void SaveBook(const domain::Book& book) {
		//	books_.Save(book);
		//}

		//void SaveTag(const domain::Tag& tag) {
		//	tags_.Save(tag);
		//}

		//void DeleteAuthor(const domain::AuthorId& author_id) {
		//	authors_.DeleteAuthor(author_id);			
		//} 
		//void EditBook(const domain::Book& book) {
		//	books_.EditBookById(book);
		//}
		//
		//void DeleteBook(const domain::BookId& book_id) {
		//	books_.DeleteBook(book_id);
		//}

		//void EditAuthor(const domain::Author& author) {
		//	authors_.EditAuthor(author);
		//}

		//const std::vector<std::string> GetAllTags(domain::BookId book_id) {
		//	return tags_.GetAllTags(book_id);
		//}

		//~UnitOfWorkImpl() = default;
		//~UnitOfWorkImpl() {
		//	try {
		//		work_.abort();
		//	}
		//	catch (...) {

		//	}
		//}

	private:
		AuthorRepositoryImpl authors_;
		BookRepositoryImpl books_;
		TagRepositoryImpl tags_;
		pqxx::work work_;
		int id_;
		static inline std::atomic<int> next_id_{0};
	};


	class UnitOfWorkFactoryImpl : public app::UnitOfWorkFactory {
	public:
		explicit UnitOfWorkFactoryImpl(pqxx::connection& connection)
			: connection_(connection) {
		}

		std::shared_ptr<app::UnitOfWork> CreateUnitOfWork() override {
			return std::make_shared<UnitOfWorkImpl>(connection_);
		}

	private:
		pqxx::connection& connection_;
	};

	class Database {
	public:
		explicit Database(pqxx::connection connection);
		const std::shared_ptr<UnitOfWorkFactoryImpl> GetUnitOfWorkImpl() const;

	private:
		pqxx::connection connection_;
		std::shared_ptr<UnitOfWorkFactoryImpl> unit_of_work_impl_;
	};
	
}  // namespace postgres