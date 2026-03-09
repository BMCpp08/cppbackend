#include "postgres.h"
#include <pqxx/pqxx>
#include <pqxx/zview.hxx>
#include <iostream>

namespace postgres {

	using namespace std::literals;
	using pqxx::operator"" _zv;

	void AuthorRepositoryImpl::Save(const domain::Author& author) {
		try {
			work_.exec_params(R"(
								INSERT INTO authors (id, name) VALUES ($1, $2)
								ON CONFLICT (id) DO UPDATE SET name=$2;
								)"_zv,
			author.GetId().ToString(), author.GetName());
		}
		catch (const pqxx::sql_error& e) {
			throw e;
		}
	}

	const std::optional<domain::Author> AuthorRepositoryImpl::GetAuthor(const std::string& name) {
		try {
			auto res = work_.exec_params("SELECT id, name FROM authors WHERE name = $1", name);
			if (res.empty()) {
				return std::nullopt;
			}

			const auto& row = res[0];
			std::string id = row[0].as<std::string>();
			std::string author_name = row[1].as<std::string>();
			return domain::Author(domain::AuthorId::FromString(id), author_name);
		}
		catch (const pqxx::sql_error& e) {
			throw e;
		}
	}

	void AuthorRepositoryImpl::DeleteAuthor(const domain::AuthorId& author_id) {
		try {
			/*work_.exec_params("DELETE FROM books WHERE author_id = $1", author_id.ToString());*/
			auto res = work_.exec_params("DELETE FROM authors WHERE id = $1 RETURNING id", author_id.ToString());
			if (res.empty()) {
				throw std::runtime_error("Author not found");
			}
		}
		catch (const pqxx::sql_error& e) {
			throw e;
		}
	}

	void AuthorRepositoryImpl::EditAuthor(const domain::Author& author) {
		try {
			auto res = work_.exec_params(
				"UPDATE authors SET name = $1 WHERE id = $2 RETURNING id",
				author.GetName(), author.GetId().ToString()
			);
			if (res.empty()) {
				throw std::runtime_error("Author not found");
			}
		}
		catch (const pqxx::sql_error& e) {
			throw e;
		}
	}

	void BookRepositoryImpl::Save(const domain::Book& book) {
		work_.exec_params(R"(
						INSERT INTO books (id, author_id, title, publication_year) VALUES ($1, $2, $3, $4)
						ON CONFLICT (id) DO UPDATE SET author_id=$2, title=$3, publication_year=$4;
						)"_zv,
						book.GetId().ToString(), book.GetAuthorId().ToString(), book.GetTitle(), book.GetPublicationYear());
	}

	const std::vector<domain::Author> AuthorRepositoryImpl::GetAllAuthor() {
		try {
			std::vector<domain::Author> res;
			auto rows = work_.query<std::string, std::optional<std::string>>("SELECT id, name FROM authors ORDER BY name;"_zv);

			for (auto& [id, name] : rows) {
				res.emplace_back(domain::AuthorId::FromString(id), name.value_or(""));
			}
			return res;
		}
		catch (const pqxx::sql_error& e) {
			throw e;
		}
	}

	/*const std::vector<domain::Book> BookRepositoryImpl::GetAllBooks() {
		try {
			std::vector<domain::Book>  res;

			auto rows = read_.query<std::string, std::string, std::optional<std::string>, std::optional<int>>("SELECT id, author_id, title, publication_year FROM books ORDER BY title ASC, publication_year DESC;"_zv);

			for (auto& [id, author_id, title, publication_year] : rows) {
				res.emplace_back(domain::BookId::FromString(id), domain::AuthorId::FromString(author_id), title.value_or(""), publication_year.value_or(-9999));
			}
			return res;
		}
		catch (const pqxx::sql_error& e) {
			throw e;
		}
	};*/
	const std::vector<domain::Book> BookRepositoryImpl::GetAllBooks() {
		try {
			std::vector<domain::Book> res;
			auto rows = work_.query<std::string, std::string, std::optional<std::string>, std::optional<int>>(
				"SELECT books.id, books.author_id, books.title, books.publication_year "
				"FROM books "
				"JOIN authors ON books.author_id = authors.id "
				"ORDER BY books.title ASC, authors.name ASC, books.publication_year ASC;"_zv
			);
			for (auto& [id, author_id, title, publication_year] : rows) {
				res.emplace_back(domain::BookId::FromString(id),
					domain::AuthorId::FromString(author_id),
					title.value_or(""),
					publication_year.value_or(-9999));
			}
			return res;
		}
		catch (const pqxx::sql_error& e) {
			throw e;
		}
	}

	const std::vector<domain::Book> BookRepositoryImpl::GetAuthorBooks(domain::AuthorId author_id) {
		try {
			std::vector<domain::Book>  res;

			auto rows = work_.query<std::string, std::string, std::optional<std::string>, std::optional<int>>("SELECT id, author_id, title, publication_year FROM books WHERE author_id = $1 ORDER BY publication_year ASC, title ASC;"_zv, author_id.ToString());
			for (auto& [id, author_id_, title, publication_year] : rows) {
				res.emplace_back(domain::BookId::FromString(id), domain::AuthorId::FromString(author_id_), title.value_or(""), publication_year.value_or(-9999));
			}

			return res;
		}
		catch (const pqxx::sql_error& e) {
			throw e;
		}
	}

	const std::vector<domain::Book> BookRepositoryImpl::GetBooksByTitle(const std::string& title) {
		try {
			std::vector<domain::Book> res;

			auto rows = work_.query<std::string, std::optional<std::string>, std::optional<int>, std::string>("SELECT books.id, books.title, books.publication_year, authors.id "
																											"FROM books "
																											"JOIN authors ON books.author_id = authors.id "
																											"WHERE books.title = $1 "
																											"ORDER BY books.title, authors.name, books.publication_year",
																											title);
			for (auto& [id, title, publication_year, author_id] : rows) {
				res.emplace_back(domain::BookId::FromString(id), domain::AuthorId::FromString(author_id), title.value_or(""), publication_year.value_or(-9999));
			}

			return res;
		}
		catch (const pqxx::sql_error& e) {
			throw e;
		}
	}
	void BookRepositoryImpl::DeleteBook(const domain::BookId& book_id) {
		try {
			auto res = work_.exec_params(
				"DELETE FROM books WHERE id = $1 RETURNING id",
				book_id.ToString()
			);
			if (res.empty()) {
				throw std::runtime_error("Book not found");
			}
		}
		catch (const pqxx::sql_error& e) {
			throw e;
		}
	}

	void BookRepositoryImpl::EditBookById(const domain::Book& book) {
		try {
			auto res = work_.exec_params(
				"UPDATE books SET title = $1, publication_year = $2 WHERE id = $3 RETURNING id",
				book.GetTitle(), book.GetPublicationYear(), book.GetId().ToString()
			);
			if (res.empty()) {
				throw std::runtime_error("Book not found");
			} 
		}
		catch (const pqxx::sql_error& e) {
			throw e;
		}
	}

	const std::shared_ptr<UnitOfWorkFactoryImpl> Database::GetUnitOfWorkImpl() const {
		return unit_of_work_impl_;
	}

	const std::vector<std::string> TagRepositoryImpl::GetAllTags(domain::BookId book_id) {
		try {
			std::vector<std::string> tags;
			auto rows = work_.query<std::optional<std::string>>(
				"SELECT tag FROM book_tags WHERE book_id = $1 ORDER BY tag;"_zv,
				book_id.ToString()
			);
			for (auto& [tag] : rows) {
				if (tag.has_value()) {
					tags.emplace_back(tag.value());
				}

			}
			return tags;
		}
		catch (const pqxx::sql_error& e) {
			throw e;
		}
	}

	void TagRepositoryImpl::DeleteAllTags(const domain::BookId& book_id) {
		try {
			work_.exec_params("DELETE FROM book_tags WHERE book_id = $1", book_id.ToString());
		}
		catch (const pqxx::sql_error& e) {
			throw e;
		}
	}
	

	Database::Database(pqxx::connection connection)
		: connection_{ std::move(connection) } 
		, unit_of_work_impl_(std::make_shared<UnitOfWorkFactoryImpl>(connection_)){
		pqxx::work work{ connection_ };

		//Таблица авторов
		work.exec(R"(
		CREATE TABLE IF NOT EXISTS authors (
			id UUID CONSTRAINT author_id_constraint PRIMARY KEY,
			name varchar(100) UNIQUE NOT NULL
		);
		)"_zv);

		//Таблица книг
		work.exec(R"(
			CREATE TABLE IF NOT EXISTS books (
			id UUID CONSTRAINT book_id_constraint PRIMARY KEY,
			author_id uuid references authors(id)  NOT NULL,
			title varchar(100) NOT NULL,
			publication_year integer NOT NULL
		);
		)"_zv);


		work.exec(R"(
			CREATE TABLE IF NOT EXISTS book_tags (
				book_id uuid REFERENCES books(id) ON DELETE CASCADE,
				tag varchar(30) NOT NULL,
				PRIMARY KEY (book_id, tag)
			);
		)"_zv);
		work.exec(R"(CREATE INDEX idx_book_tags_tag ON book_tags(tag);)"_zv);

		work.commit();
	}

}  // namespace postgres