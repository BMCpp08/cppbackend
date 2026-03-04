#pragma once

#include <string>
#include <vector>
#include "../domain/author.h"

namespace app {

class UseCases {
public:
    virtual void AddAuthor(const std::string& name) = 0;
    virtual const std::vector<domain::Author> GetAllAuthor() = 0;

    virtual void AddBook(const std::string& title, const std::string& author_id, const int publication_year) = 0;
    virtual const std::vector<domain::Book> GetAllBook() = 0;
    virtual const std::vector<domain::Book> GetAuthorBooks(const std::string& author_id) = 0;
protected:
    ~UseCases() = default;
};

}  // namespace app
