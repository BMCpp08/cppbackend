// main.cpp

#include <iostream>
#include <string>
#include <pqxx/pqxx>
#include <boost/json.hpp>               

using namespace std::literals;
// libpqxx использует zero-terminated символьные литералы вроде "abc"_zv;
using pqxx::operator"" _zv;
namespace json = boost::json;

struct ReqBook {
	std::string title;
	std::string author;
	int year;
	std::optional<std::string> isbn;
};

bool ParseReqAddBook(const json::object& payload, ReqBook& req_add_book) {
	bool result_is_ok = false;

	if (!payload.empty()) {
		if (auto it_title = payload.if_contains("title"); it_title) {
			req_add_book.title = it_title->as_string().c_str();

			if (auto it_author = payload.if_contains("author"); it_author) {
				req_add_book.author = it_author->as_string().c_str();

				if (auto it_year = payload.if_contains("year"); it_year) {
					if (it_year->is_int64()) {
						req_add_book.year = it_year->as_int64();

						if (auto it = payload.find("ISBN"); it != payload.end()) {
							auto const& val = it->value();
							if (!val.is_null()) {
								if (val.is_string()) {
									req_add_book.isbn = json::value_to<std::string>(val);
									result_is_ok = true;
								}
							}
						}
						else {
							result_is_ok = true;
						}
					}
				}
			}
		}
	}
	return result_is_ok;
}

int main(int argc, const char* argv[]) {


	try {
		if (argc == 1) {
			std::cout << "Usage: db_example <conn-string>\n"sv;
			return EXIT_SUCCESS;
		}
		else if (argc != 2) {
			std::cerr << "Invalid command line\n"sv;
			return EXIT_FAILURE;
		}

		// Подключаемся к БД, указывая её параметры в качестве аргумента
		pqxx::connection conn{ argv[1] };

		// Создаём транзакцию. Это понятие будет разобрано в следующих уроках.
		// Транзакция нужна, чтобы выполнять запросы.
		//pqxx::work w(conn);
		pqxx::work  r(conn);

		// Используя транзакцию создадим таблицу в выбранной базе данных:
		r.exec(
			"CREATE TABLE IF NOT EXISTS books (id SERIAL PRIMARY KEY, title varchar(100) NOT NULL, author varchar(100) NOT NULL, year integer NOT NULL, ISBN char(13) NULL UNIQUE);"_zv);

		// Применяем все изменения
		r.commit();

		constexpr auto tag_add_book = "add_book"_zv;
		conn.prepare(tag_add_book, "INSERT INTO books (title, author, year, ISBN) VALUES ($1, $2, $3, $4)"_zv);

		constexpr auto tag_all_books = "all_books"_zv;


		constexpr auto tag_exit = "exit"_zv;

		std::string line;
		while (std::getline(std::cin, line)) {
			if (line.empty()) {
				continue;
			}
			std::cout << json::serialize(json::object{ {"result", true} });
			//try {
			//	json::value value = json::parse(line);

			//	if (value.is_object()) {

			//		auto const& data = value.as_object();
			//		auto it_action = data.if_contains("action");
			//		auto it_payload = data.if_contains("payload");

			//		if (it_action && it_payload) {

			//			auto cmd = it_action->as_string().c_str();
			//			if (cmd == tag_add_book) {

			//				if (it_payload->is_object()) {
			//					ReqBook req_add_book;
			//					try {
			//						if (ParseReqAddBook(it_payload->as_object(), req_add_book)) {
			//							r.exec_prepared(tag_add_book, req_add_book.title, req_add_book.author, req_add_book.year,
			//								req_add_book.isbn ? pqxx::to_string(*req_add_book.isbn) : nullptr);
			//							
			//							std::cout << json::serialize(json::object{ {"result", true} });
			//							r.commit();
			//						}
			//						else {
			//							std::cout << json::serialize(json::object{ {"result", false} });
			//						}
			//					}
			//					catch (const pqxx::sql_error& e) {
			//						std::cout << json::serialize(json::object{ {"result", false} });
			//					}
			//				}

			//			}
			//			else if (cmd == tag_all_books && it_payload->is_object() && it_payload->as_object().empty()) {
			//				if (1) {

			//					json::array arr;
			//					for (auto [id, title, author, year, ISBN] :
			//						r.query<std::optional<int>, std::optional<std::string>, std::optional<std::string>, std::optional<int>, std::optional<std::string>>("SELECT id, title, author, year, isbn FROM books ORDER BY ORDER BY year DESC, title ASC, author ASC, isbn ASC;"_zv)) {
			//						arr.emplace_back(json::array{ json::object{ {"id", id.value_or(-9999)},
			//																	{"title", title.value_or("")},
			//																	{"author", author.value_or("")},
			//																	{"year", year.value_or(-9999)},
			//																	{"ISBN", ISBN.value_or("null")}} });
			//					}
			//					std::cout << json::serialize(arr);
			//				}
			//			}
			//			else if (cmd == tag_exit && it_payload->is_object() && it_payload->as_object().empty()) {
			//				break;
			//			}
			//		}
			//	}
			//}
			//catch (const pqxx::sql_error& e) {

			//}
			//std::cin >> std::ws;
		}

	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}