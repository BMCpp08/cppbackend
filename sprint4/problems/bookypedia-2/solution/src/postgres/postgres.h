#pragma once
#include <pqxx/connection>
#include <pqxx/transaction>
#include <iostream>
#include "../domain/author.h"
#include "../domain/book.h"
#include "../domain/tag.h"
#include "../app/unit_of_work_factory.h"

namespace postgres {
	using namespace std::literals;
	class AuthorRepositoryImpl : public domain::AuthorRepository {
	public:
		explicit AuthorRepositoryImpl(pqxx::work& work, pqxx::read_transaction& read)
			: work_(work)
			, read_(read) {
		}

		void Save(const domain::Author& author) override;
		const std::vector<domain::Author> GetAllAuthor() override;

		const std::optional<domain::Author> GetAuthor(const std::string& name) override;

		void DeleteAuthor(const domain::AuthorId& author_id) override;
		void EditAuthor(const domain::Author& author) override;
	private:
		pqxx::work& work_;
		pqxx::read_transaction& read_;
	};

	class BookRepositoryImpl : public domain::BookRepository {
	public:
		explicit BookRepositoryImpl(pqxx::work& work, pqxx::read_transaction& read)
			: work_(work)
			, read_(read) {
		}

		void Save(const domain::Book& book) override;
		const std::vector<domain::Book> GetAllBooks() override;
		const std::vector<domain::Book> GetAuthorBooks(domain::AuthorId author_id) override;
		const std::vector<domain::Book> GetBooksByTitle(const std::string& title) override;
		void DeleteBook(const domain::BookId& book_id) override;
		void EditBookById(const domain::Book& book) override;
	private:
		pqxx::work& work_;
		pqxx::read_transaction& read_;
	};


	class TagRepositoryImpl : public domain::TagRepository {
	public:
		explicit TagRepositoryImpl(pqxx::work& work, pqxx::read_transaction& read)
			: work_(work)
			, read_(read) {
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
		pqxx::read_transaction& read_;
	};


	class UnitOfWorkImpl : public app::UnitOfWork {
	public:
		explicit UnitOfWorkImpl(pqxx::connection& connection)
			: work_(connection)
			, read_(connection)
			, authors_(work_, read_)
			, books_(work_, read_)
			, tags_(work_, read_){
		}

		void Commit() override {
			
			try {
				work_.commit();
			}
			catch (...) {
				Rollback();
				throw std::logic_error("Error Commit"s);
			}
		}

		void Rollback() override {
			work_.abort();
		}
		
		domain::AuthorRepository& Authors() override {
			return authors_;
		}

		domain::BookRepository& Books() override {
			return books_;
		}

		domain::TagRepository& Tags() override {
			return tags_;
		}

		void SaveAuthor(const domain::Author& author) {
			try {
				authors_.Save(author);
			}
			catch (...) {
				Rollback();
				throw std::logic_error("Error SaveAuthor"s);
			}
		}

		void SaveBook(const domain::Book& book) {
			try {
				books_.Save(book);
			}
			catch (...) {
				Rollback();
				throw std::logic_error("Error SaveBook"s);
			}
		}

		void SaveTag(const domain::Tag& tag) {
			try {
				tags_.Save(tag);
			}
			catch (...) {
				Rollback();
				throw std::logic_error("Error SaveTag"s);
			}
		}

		void DeleteAuthor(const domain::AuthorId& author_id) {
			try {
				authors_.DeleteAuthor(author_id);
			}
			catch (...) {
				Rollback();
				throw std::logic_error("Error DeleteAuthor"s);
			}
		} 
		void EditBook(const domain::Book& book) {
			try {
				books_.EditBookById(book);
			}
			catch (...) {
				Rollback();
				throw std::logic_error("Error EditBook"s);
			}
		}
		
		void DeleteBook(const domain::BookId& book_id) {
			try {
				books_.DeleteBook(book_id);
			}
			catch (...) {
				Rollback();
				throw std::logic_error("Error DeleteAuthor"s);
			}
		}

		void EditAuthor(const domain::Author& author) {
			try {
				authors_.EditAuthor(author);
			}
			catch (...) {
				Rollback();
				throw std::logic_error("Error EditAuthor"s);
			}
		}

		const std::vector<std::string> GetAllTags(domain::BookId book_id) {
			try {
				return tags_.GetAllTags(book_id);
			}
			catch (...) {
				Rollback();
				throw std::logic_error("Error EditAuthor"s);
			}
		}

		~UnitOfWorkImpl() {
			try {
				work_.abort();
			}
			catch (...) {

			}
		}

	private:
		AuthorRepositoryImpl authors_;
		BookRepositoryImpl books_;
		TagRepositoryImpl tags_;
		pqxx::work work_;
		pqxx::read_transaction read_;

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