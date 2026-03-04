#include "postgres.h"
#include <pqxx/pqxx>
#include <pqxx/zview.hxx>

namespace postgres {

using namespace std::literals;
using pqxx::operator"" _zv;

void AuthorRepositoryImpl::Save(const domain::Author& author) {

    // Пока каждое обращение к репозиторию выполняется внутри отдельной транзакции
    // В будущих уроках вы узнаете про паттерн Unit of Work, при помощи которого сможете несколько
    // запросов выполнить в рамках одной транзакции.
    // Вы также может самостоятельно почитать информацию про этот паттерн и применить его здесь.
    pqxx::work work{connection_};
    //constexpr auto tag_author = "add_author"_zv;

    //connection_.prepare(tag_author, "INSERT INTO authors (id, name) VALUES ($1, $2) ON CONFLICT (id) DO UPDATE SET name=$2"_zv);
    //work.exec_prepared(tag_author, author.GetId().ToString(), author.GetName());
    //work.commit();

    work.exec_params(
        R"(
INSERT INTO authors (id, name) VALUES ($1, $2)
ON CONFLICT (id) DO UPDATE SET name=$2;
)"_zv,
        author.GetId().ToString(), author.GetName());
    work.commit();
}

const domain::Author AuthorRepositoryImpl::LoadAuthorById() {
    return { domain::AuthorId::FromString("id"), ("") };
}

const std::vector<domain::Author> AuthorRepositoryImpl::GetAllAuthor() {
    try {
        std::vector<domain::Author> res;
        pqxx::read_transaction read(connection_);
        auto r = read.query< std::string, std::optional<std::string>>("SELECT id, name FROM authors ORDER BY name;"_zv);
        
        for (auto& [id, name] : r){
            res.emplace_back(domain::AuthorId::FromString(id), name.value_or(""));
        }

        read.commit();
        return res;
    }
    catch (const pqxx::sql_error& e) {
        throw e;
    }
}


Database::Database(pqxx::connection connection)
    : connection_{std::move(connection)} {
    pqxx::work work{connection_};
    work.exec(R"(
CREATE TABLE IF NOT EXISTS authors (
    id UUID CONSTRAINT author_id_constraint PRIMARY KEY,
    name varchar(100) UNIQUE NOT NULL
);
)"_zv);
    work.commit();
    // ... создать другие таблицы
    work.exec(
        R"(CREATE TABLE IF NOT EXISTS books (
        id UUID CONSTRAINT book_id_constraint PRIMARY KEY, 
        author_id UUID NOT NULL, 
        title varchar(100) NOT NULL, 
        publication_year integer NOT NULL,
        CONSTRAINT f_books FOREIGN KEY(author_id) REFERENCES authors(id);)"_zv);

    //// коммитим изменения
    work.commit();
}

}  // namespace postgres