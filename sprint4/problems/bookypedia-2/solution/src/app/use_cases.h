#pragma once

#include <string>
#include <vector>
#include <unordered_set>
#include "../domain/author.h"
#include "../domain/book.h"
#include "../domain/tag.h"

namespace app {

class UseCases {
public:
    virtual std::string AddAuthor(const std::string& name) = 0;
    virtual const std::vector<domain::Author> GetAllAuthor() = 0;
    virtual const std::optional<domain::Author> GetAuthor(const std::string& name) = 0;

    virtual std::string AddBook(const std::string& title, const std::string& author_id, const int publication_year) = 0;
    virtual const std::vector<domain::Book> GetAllBook() = 0;
    virtual const std::vector<domain::Book> GetAuthorBooks(const std::string& author_id) = 0;

    virtual void AddTag(const std::string& book_id, const std::string& tag) = 0;
    virtual void Commit() = 0;
    virtual void DeleteAuthor(const std::string& author_id) = 0;
    virtual void EditAuthor(const std::string& author_id, const std::string& name) = 0;

    virtual const std::vector<std::string> GetAllTags(const std::string& book_id) = 0;
    virtual const std::vector<domain::Book> GetBooksByTitle(const std::string& title) = 0;
    virtual void DeleteBook(const domain::BookId& book_id) = 0;
    virtual void EditBook(const domain::Book& book) = 0;
    virtual void DeleteAllTags(const std::string& book_id) = 0;
protected:
protected:
    ~UseCases() = default;
};

}  // namespace app
