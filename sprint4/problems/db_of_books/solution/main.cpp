// main.cpp

#include <iostream>
#include <pqxx/pqxx>

using namespace std::literals;
// libpqxx использует zero-terminated символьные литералы вроде "abc"_zv;
using pqxx::operator"" _zv;

int main(int argc, const char* argv[]) {
    try {
        if (argc == 1) {
            std::cout << "Usage: db_example <conn-string>\n"sv;
            return EXIT_SUCCESS;
        } else if (argc != 2) {
            std::cerr << "Invalid command line\n"sv;
            return EXIT_FAILURE;
        }

        // Подключаемся к БД, указывая её параметры в качестве аргумента
        pqxx::connection conn{argv[1]};

        // Создаём транзакцию. Это понятие будет разобрано в следующих уроках.
        // Транзакция нужна, чтобы выполнять запросы.
        pqxx::work w(conn);

        // Используя транзакцию создадим таблицу в выбранной базе данных:
        w.exec(
            "CREATE TABLE movies (id SERIAL PRIMARY KEY, title varchar(200) NOT NULL, year integer NOT NULL);"_zv);

        // Применяем все изменения
        w.commit();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}