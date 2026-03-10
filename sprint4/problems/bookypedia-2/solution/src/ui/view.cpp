#include "view.h"
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/trim_all.hpp>
#include <cassert>
#include <iostream>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include <algorithm>

#include "../app/use_cases.h"
#include "../menu/menu.h"

using namespace std::literals;
namespace ph = std::placeholders;

namespace ui {
	namespace detail {

		std::ostream& operator<<(std::ostream& out, const AuthorInfo& author) {
			out << author.name;
			return out;
		}

		std::ostream& operator<<(std::ostream& out, const BookInfo& book) {
			out << book.title << " by " << book.author << ", " << book.publication_year;
			return out;
		}

		std::ostream& operator<<(std::ostream& out, const TagInfo& tag) {

			out << "Title: " << tag.title << std::endl;
			out << "Author: " << tag.author << std::endl;
			out << "Publication year: " << tag.publication_year << std::endl;

			if (!tag.tags.empty()) {
				out << "Tags: " << tag.tags[0];
				for (int i = 1; i < tag.tags.size(); ++i) {
					out << ", " << tag.tags[i];
				}
				out << std::endl;
			}
			return out;
		}

	}  // namespace detail

	template <typename T>
	void PrintVector(std::ostream& out, const std::vector<T>& vector) {
		int i = 1;
		for (auto& value : vector) {
			out << i++ << " " << value << std::endl;
		}
	}

	View::View(menu::Menu& menu, app::UseCases& use_cases, std::istream& input, std::ostream& output)
		: menu_{ menu }
		, use_cases_{ use_cases }
		, input_{ input }
		, output_{ output } {
		menu_.AddAction(  //
			"AddAuthor"s, "name"s, "Adds author"s, std::bind(&View::AddAuthor, this, ph::_1)
			// либо
			// [this](auto& cmd_input) { return AddAuthor(cmd_input); }
		);
		menu_.AddAction("AddBook"s, "<pub year> <title>"s, "Adds book"s,
			std::bind(&View::AddBook, this, ph::_1));
		menu_.AddAction("ShowAuthors"s, {}, "Show authors"s, std::bind(&View::ShowAuthors, this));
		menu_.AddAction("ShowBooks"s, {}, "Show books"s, std::bind(&View::ShowBooks, this));
		menu_.AddAction("ShowAuthorBooks"s, {}, "Show author books"s,
			std::bind(&View::ShowAuthorBooks, this));
		menu_.AddAction(
			"DeleteAuthor"s, "name"s, "Delete author"s, std::bind(&View::DeleteAuthor, this, ph::_1)
		);
		menu_.AddAction(
			"EditAuthor"s, "name"s, "Edit Author"s, std::bind(&View::EditAuthor, this, ph::_1)
		);
		menu_.AddAction(
			"ShowBook"s, "name"s, "Show Book"s, std::bind(&View::ShowBook, this, ph::_1)
		);
		menu_.AddAction(
			"DeleteBook"s, "name"s, "Delete Book"s, std::bind(&View::DeleteBook, this, ph::_1)
		);
		menu_.AddAction(
			"EditBook"s, "name"s, "Edit Book"s, std::bind(&View::EditBook, this, ph::_1)
		);
	}
	std::string ReadInput(std::istream& cmd_input) {
		std::string str;
		std::getline(cmd_input, str);
		boost::algorithm::trim(str);
		return str;
	}

	bool View::EditBook(std::istream& cmd_input) const {
		try {
			
			std::string title = ReadInput(cmd_input);
			
			// 2. Получаем список книг
			std::vector<detail::BookInfo> books;
			if (!title.empty()) {
				books = GetBooksByTitle(title);
			}
			else {
				books = GetBooks();
			}

			// 3. Если книг нет
			if (books.empty()) {
				if (!title.empty()) {
					output_ << "Book not found" << std::endl;
				}
				return true;
			}

			int selectedIndex = -1;

			// 4. Определяем индекс книги
			if (!title.empty()) {
				if (books.size() == 1) {
					selectedIndex = 0;
				}
				else {
					selectedIndex = SelectBook(books);
					if (selectedIndex == -1) {
						output_ << "Book not found" << std::endl; 
						return true;
					}
				}
			}
			else {
				selectedIndex = SelectBook(books);
				if (selectedIndex == -1) {
					output_ << "Book not found" << std::endl;
					return true;
				}
			}
			

			// 5. Данные выбранной книги
			const auto& book = books[selectedIndex];
			std::string book_id = book.book_id;
			std::string current_title = book.title;
			int currentYear = book.publication_year;

			// 6. Текущие теги
			auto currentTags = use_cases_.GetAllTags(book_id);
			std::string currentTagsStr = boost::algorithm::join(currentTags, ", ");

			// 7. Ввод нового названия
			output_ << "Enter new title or empty line to use the current one (" << current_title << "): ";
			std::string new_title = ReadInput(input_);
			if (new_title.empty()) {
				new_title = current_title;
			}

			// 8. Ввод нового года
			output_ << "Enter publication year or empty line to use the current one (" << currentYear << "): ";
			std::string new_year;
			std::getline(input_, new_year);
			boost::algorithm::trim(new_year);
			int newYear = currentYear;
			if (!new_year.empty()) {
				try {
					newYear = std::stoi(new_year);
				}
				catch (...) {
					throw std::runtime_error("Invalid new_year");
				}
			}

			// 9. Ввод новых тегов
			output_ << "Enter tags (current tags: " << currentTagsStr << "): ";
			auto newTagsSet = ReadTags();
			// Преобразуем в вектор и сортируем для детерминированного порядка вставки
			std::vector<std::string> newTags(newTagsSet.begin(), newTagsSet.end());
			std::sort(newTags.begin(), newTags.end());

			// 10. Обновление книги в БД
			if (new_title != current_title || newYear != currentYear) {
				use_cases_.EditBook(domain::Book{
					domain::BookId::FromString(book_id),
					domain::AuthorId::FromString(book.author_id),
					new_title,
					newYear
					});
			}

			// 11. Обновление тегов: удаляем старые и добавляем новые (отсортированные)
			use_cases_.DeleteAllTags(book_id);
			for (const auto& tag : newTags) {
				use_cases_.AddTag(book_id, tag);
			}

			use_cases_.Commit();
			return true;

		}
		catch (const std::exception&) {
			output_ << "Book not found" << std::endl;
			use_cases_.Rollback();
			return true;
		}
	}
	int View::SelectBook(const std::vector<detail::BookInfo>& books) const {
		PrintVector(output_, books);
		output_ << "Enter the book # or empty line to cancel: ";

		std::string str;
		std::getline(input_, str);
		if (str.empty()) {
			return -1;
		}

		int author_idx;

		try {
			author_idx = std::stoi(str);
		}
		catch (std::exception const&) {
			throw std::runtime_error("Invalid book num");
		}

		--author_idx;
		if (author_idx < 0 or author_idx >= books.size()) {
			throw std::runtime_error("Invalid book num");
		}

		return author_idx;
	}

	std::vector<detail::BookInfo> View::GetBooksByTitle(const std::string& title) const {
		std::vector<detail::BookInfo> books;

		try {
			auto authors = GetAuthors();
			auto books_by_title = use_cases_.GetBooksByTitle(title);
			for (const auto& book : books_by_title) {
				auto it_author = std::find_if(authors.begin(), authors.end(), [&](const detail::AuthorInfo& author) {
					return author.id == book.GetAuthorId().ToString();
					});
				if (it_author != authors.end()) {
					books.emplace_back(book.GetTitle(), book.GetPublicationYear(), it_author->name, it_author->id, book.GetId().ToString());
				}
				else {
					/*throw std::runtime_error("Invalid author");*/
				}
			}

		}
		catch (const std::exception&) {
			/*output_ << "Invalid author"sv << std::endl;*/
		}
		return books;
	}

	bool View::DeleteBook(std::istream& cmd_input) const {
		try {
			std::string titile = ReadInput(cmd_input);
	
			std::vector<detail::BookInfo> books;
			bool delete_by_title = !titile.empty();
			int idx = 0;

			if (delete_by_title) {
				books = GetBooksByTitle(titile);
			}
			else {
				books = GetBooks();
				idx = SelectBook(books);
			}

			// Если книг нет, выходим без сообщения (кроме случая, когда название указано и не найдено)
			if (books.empty()) {
				if (delete_by_title) {
					/*output_ << "Book not found" << std::endl;*/
				}
				return true;
			}

			if (delete_by_title) {
				if (books.size() > 1) {
					idx = SelectBook(books);
					if (idx == -1) {
						return true;
					}
				}
			}
			use_cases_.DeleteAllTags(books[idx].book_id);
			use_cases_.DeleteBook(domain::BookId::FromString(books[idx].book_id));
			use_cases_.Commit();
		}
		catch (const std::exception&) {
			output_ << "Failed to delete book"sv << std::endl;
			use_cases_.Rollback();
		}
		return true;
	}

	bool View::ShowBook(std::istream& cmd_input) const {
		try {
			std::string title;
			std::getline(cmd_input, title);
			boost::algorithm::trim(title);

			std::vector<detail::BookInfo> books;
			int idx = 0;
			bool find_by_title = !title.empty();

			if (find_by_title) {
				books = GetBooksByTitle(title);
			}
			else {
				books = GetBooks();
				idx = SelectBook(books);
			}

			if (books.empty()) {
				return true;
			}

			if (find_by_title && books.size() > 1) {
				idx = SelectBook(books);
				if (idx == -1) return true;
			}

			auto tags = use_cases_.GetAllTags(books[idx].book_id);
			detail::TagInfo tag_info;
			tag_info.tags = tags;
			tag_info.author = books[idx].author;
			tag_info.publication_year = books[idx].publication_year;
			tag_info.title = books[idx].title;
			output_ << tag_info;
		}
		catch (const std::exception& e) {
			output_ << ""sv << std::endl;
			std::cerr << "ShowBook exception: " << e.what() << std::endl;
		}
		catch (...) {
			output_ << ""sv << std::endl;
			std::cerr << "ShowBook unknown exception" << std::endl;
		}
		return true;
	}
	/*bool View::ShowBook(std::istream& cmd_input) const {
		try {
			std::string titile;
			std::getline(cmd_input, titile);
			boost::algorithm::trim(titile);

			std::vector<detail::BookInfo> books;

			int idx = 0;
			bool find_by_title = !titile.empty();

			if (find_by_title) {
				books = GetBooksByTitle(titile);
			}
			else {
				books = GetBooks();
				idx = SelectBook(books);
			}

			if (books.empty()) {
				return false;
			}

			if (find_by_title) {
				if (books.size() > 1) {
					idx = SelectBook(books);
					if (idx == -1) {
						return false;
					}
				}

			}

			auto tags = use_cases_.GetAllTags(books[idx].book_id);
			detail::TagInfo tag_info_;
			tag_info_.tags = tags;
			tag_info_.author = books[idx].author;
			tag_info_.publication_year = books[idx].publication_year;
			tag_info_.title = books[idx].title;
			output_ << tag_info_;

		}
		catch (const std::exception&) {
			output_ << ""sv << std::endl;
		}
		return true;
	}*/

	bool View::EditAuthor(std::istream& cmd_input) const {
		try {
			std::string name;
			std::getline(cmd_input, name);
			boost::algorithm::trim(name);

			if (name.empty()) {
				if (auto author_id = SelectAuthor()) {
					output_ << "Enter new name:" << std::endl;
					std::string new_name;
					std::getline(input_, new_name);
					boost::algorithm::trim(new_name);

					if (new_name.empty()) {
						throw std::logic_error("New name is empty");
					}

					use_cases_.EditAuthor(author_id.value(), new_name);
					use_cases_.Commit();
				}
				else {
					throw std::logic_error("No authors found");
				}
			}
			else {
				boost::algorithm::trim(name);
				auto author = use_cases_.GetAuthor(name);
				if (!author.has_value()) {
					throw std::logic_error("Failed to edit author");
				}
				output_ << "Enter new name:" << std::endl;

				std::string new_name;
				std::getline(input_, new_name);
				boost::algorithm::trim(new_name);
				if (new_name.empty()) {
					throw std::logic_error("New name is empty");
				}

				use_cases_.EditAuthor(author->GetId().ToString(), new_name);
				use_cases_.Commit();
			}
		}
		catch (const std::exception&) {
			output_ << "Failed to edit author"sv << std::endl;
		}
		return true;
	}

	bool View::DeleteAuthor(std::istream& cmd_input) const {
		try {
			std::string name;
			std::getline(cmd_input, name);
			boost::algorithm::trim(name);

			if (name.empty()) {
				if (auto author_id = SelectAuthor()) {
					use_cases_.DeleteAuthor(author_id.value());
					use_cases_.Commit();
				}
				else {
					throw std::logic_error("No authors found");
				}
			}
			else {
				auto author = use_cases_.GetAuthor(name);
				if (!author.has_value()) {
					throw std::logic_error("Failed to delete author");
				}
				use_cases_.DeleteAuthor(author->GetId().ToString());
			}
			use_cases_.Commit();
		}
		catch (const std::exception&) {
			output_ << "Failed to delete author"sv << std::endl;
		}
		return true;
	}

	bool View::AddAuthor(std::istream& cmd_input) const {
		try {
			std::string name;
			std::getline(cmd_input, name);
			boost::algorithm::trim(name);

			if (name.empty()) {
				throw std::logic_error("Name is empty"s);
			}
			use_cases_.AddAuthor(std::move(name));
			use_cases_.Commit();
		}
		catch (const std::exception&) {
			output_ << "Failed to add author"sv << std::endl << std::flush;
			use_cases_.Rollback();
		}
		return true;
	}
	//bool View::AddBook(std::istream& cmd_input) const {
	//	try {
	//		auto params = GetBookParams(cmd_input);

	//		if (!params) {
	//			input_.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	//			throw std::logic_error("Failed to add book"s);
	//		}
	//	
	//		if (params->title.empty()) {
	//			input_.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	//			throw std::logic_error("Book title is empty"s);
	//		}

	//		output_ << "Enter tags (comma separated):";
	//		auto book_id = use_cases_.AddBook(params->author_id, params->title, params->publication_year);
	//		auto tags_set = ReadTags();

	//		if (!tags_set.empty()) {
	//			AddTag(book_id, tags_set);
	//		}
	//		use_cases_.Commit();
	//	}
	//	catch (const std::exception&) {
	//		use_cases_.Rollback();
	//		output_ << "Failed to add book"sv << std::endl;
	//	}
	//	return true;
	//}
	bool View::AddBook(std::istream& cmd_input) const {
		try {
			detail::AddBookParams params;
			cmd_input >> params.publication_year;
			std::getline(cmd_input, params.title);
			boost::algorithm::trim(params.title);

			output_ << "Enter author name or empty line to select from list:" << std::endl;
			std::string author_name;
			if (!std::getline(input_, author_name)) {
				return true;
			}

			boost::algorithm::trim(author_name);

			if (author_name.empty()) {
				auto author_id = SelectAuthor();

				if (!author_id) {
					return true;
				} else {
					params.author_id = author_id.value();
				}
			}
			else {
				auto author = use_cases_.GetAuthor(author_name);
				if (author.has_value()) {
					params.author_id = author->GetId().ToString();
				}
				else {
					output_ << "No author found. Do you want to add " << author_name << " (y/n)? ";

					char answer;
					input_ >> answer;
					input_.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

					if (answer == 'y' || answer == 'Y') {
						auto author_id = use_cases_.AddAuthor(std::move(author_name));
						if (!author_id.empty()) {
							params.author_id = author_id;
						}
						else {
							use_cases_.Rollback();
							return true;
						}
					}
					else {
						output_ << "Failed to add book"sv << std::endl;
						return true;
					}
				}
			}

			output_ << "Enter tags (comma separated):";
			auto book_id = use_cases_.AddBook(params.author_id, params.title, params.publication_year);
			auto tags_set = ReadTags();

			if (!tags_set.empty()) {
				AddTag(book_id, tags_set);
			}
			use_cases_.Commit();
		}
		catch (const std::exception&) {
			use_cases_.Rollback();
			output_ << "Failed to add book"sv << std::endl;
		}
		return true;
	}
	bool View::AddTag(const std::string& book_id, const std::vector<std::string>& tags) const {
		try {

			for (const auto& tag : tags) {
				use_cases_.AddTag(book_id, tag);
			}
		}
		catch (const std::exception& e) {
			use_cases_.Rollback();
			output_ << "Failed to add tags"sv << std::endl;
		}
		return true;
	}

	bool View::ShowAuthors() const {
		PrintVector(output_, GetAuthors());
		return true;
	}

	bool View::ShowBooks() const {
		try {
			auto books = GetBooks();
			PrintVector(output_, books);
			/*output_.flush();*/
		}
		catch (const std::exception& e) {
			std::cerr << "exception: " << e.what() << std::endl;
		}
		return true;
	}




	bool View::ShowAuthorBooks() const {
		// TODO: handle error
		try {
			if (auto author_id = SelectAuthor()) {
				PrintVector(output_, GetAuthorBooks(*author_id));
			}
		}
		catch (const std::exception&) {
			throw std::runtime_error("Failed to Show Books"s);
		}
		return true;
	}

	std::vector<std::string> View::ReadTags() const {
		std::string line;
		std::getline(input_, line);
		boost::algorithm::trim(line);

		std::vector<std::string>  tags;

		if (line.empty())
			return tags;

		std::vector<std::string> parts;
		boost::split(parts, line, boost::is_any_of(","));
		for (auto& token : parts) {
			boost::algorithm::trim_all(token);
			if (!token.empty()) {
				tags.emplace_back(token);
			}
		}

		std::sort(tags.begin(), tags.end());
		auto last = std::unique(tags.begin(), tags.end());
		tags.erase(last, tags.end());
		return tags;
	}

	std::optional<detail::AddBookParams> View::GetBookParams(std::istream& cmd_input) const {
		detail::AddBookParams params;

		cmd_input >> params.publication_year;
		std::getline(cmd_input, params.title);
		boost::algorithm::trim(params.title);

		output_ << "Enter author name or empty line to select from list:" << std::endl;
		std::string author_name;
		if (!std::getline(input_, author_name)) {
			return std::nullopt;
		}
		boost::algorithm::trim(author_name);

		if (author_name.empty()) {
			auto author_id = SelectAuthor();
			if (!author_id)
				return std::nullopt;
			else {
				params.author_id = author_id.value();
				return params;
			}
		}
		else {
			auto author = use_cases_.GetAuthor(author_name);
			if (author.has_value()) {
				params.author_id = author->GetId().ToString();
				return params;
			}
			else {
				output_ << "No author found. Do you want to add " << author_name << " (y/n)? ";
				
				char answer;
				input_ >> answer;
				input_.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

				if (answer == 'y' || answer == 'Y') {
					auto author_id = use_cases_.AddAuthor(std::move(author_name));
					if (!author_id.empty()) {

						params.author_id = author_id;
						return params;
					}
					return std::nullopt;
				}
				else {
					output_ << "Failed to add book"sv << std::endl;
					return std::nullopt;
				}
			}
		}
	}

	std::optional<std::string> View::SelectAuthor() const {
		output_ << "Select author:" << std::endl;
		auto authors = GetAuthors();
		PrintVector(output_, authors);
		output_ << "Enter author # or empty line to cancel" << std::endl;

		std::string str;
		if (!std::getline(input_, str) || str.empty()) {
			return std::nullopt;
		}

		int author_idx;
		try {
			author_idx = std::stoi(str);
		}
		catch (std::exception const&) {
			throw std::runtime_error("Invalid author num");
		}

		--author_idx;
		if (author_idx < 0 or author_idx >= authors.size()) {
			throw std::runtime_error("Invalid author num");
		}

		return authors[author_idx].id;
	}

	std::vector<detail::AuthorInfo> View::GetAuthors() const {
		std::vector<detail::AuthorInfo> dst_autors;

		for (const auto& author : use_cases_.GetAllAuthor()) {
			dst_autors.emplace_back(author.GetId().ToString(), author.GetName());
		}
		return dst_autors;
	}

	//std::vector<detail::BookInfo> View::GetBooks() const {
	//	std::vector<detail::BookInfo> books;
	//	try {
	//		
	//		auto authors = GetAuthors();
	//		auto all_books = use_cases_.GetAllBook();

	//		for (const auto& book : all_books) {
	//			auto it_author = std::find_if(authors.begin(), authors.end(), [&](const detail::AuthorInfo& author) {
	//				return author.id == book.GetAuthorId().ToString();
	//				});
	//			if (it_author != authors.end()) {
	//				books.emplace_back(book.GetTitle(), book.GetPublicationYear(), it_author->name, it_author->id, book.GetId().ToString());
	//			}
	//			else {
	//				throw std::runtime_error("Invalid author");
	//			}
	//		}

	//	}
	//	catch (const std::exception&) {
	//		output_ << "Invalid author"sv << std::endl;
	//	}
	//	return books;
	//}


	std::vector<detail::BookInfo> View::GetBooks() const {
		std::vector<detail::BookInfo> books;
		try {
			auto authors = GetAuthors();
			auto all_books = use_cases_.GetAllBook();

			for (const auto& book : all_books) {

				auto it_author = std::find_if(authors.begin(), authors.end(), [&](const detail::AuthorInfo& author) {
					return author.id == book.GetAuthorId().ToString();
					});

				if (it_author != authors.end()) {
					books.emplace_back(book.GetTitle(), book.GetPublicationYear(),
						it_author->name, it_author->id, book.GetId().ToString());
				}
				else {
					/*throw std::runtime_error("Invalid author");*/
				}
			}
		}
		catch (const std::exception& e) {
			/*std::cerr << ">>> View::GetBooks: exception: " << e.what() << std::endl;
			output_ << "Invalid author"sv << std::endl;*/
		}
		return books;
	}

	std::vector<detail::BookInfo> View::GetAuthorBooks(const std::string& author_id) const {
		std::vector<detail::BookInfo> books;

		for (const auto& book : use_cases_.GetAuthorBooks(author_id)) {
			books.emplace_back(book.GetTitle(), book.GetPublicationYear());
		}

		return books;
	}

}  // namespace ui
