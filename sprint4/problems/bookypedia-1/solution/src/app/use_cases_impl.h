#pragma once
#include "../domain/author_fwd.h"
#include "use_cases.h"
#include <vector>

namespace app {

class UseCasesImpl : public UseCases {
public:
    explicit UseCasesImpl(domain::AuthorRepository& authors, domain::BookRepository& book)
        : authors_{authors} 
        , book_{book}{
    }

    void AddAuthor(const std::string& name) override;
    const std::vector<domain::Author> GetAllAuthor() override;

    void AddBook(const std::string& author_id, const std::string& title, const int publication_year) override;
    const std::vector<domain::Book> GetAllBook() override;
private:
    domain::AuthorRepository& authors_;
    domain::BookRepository& book_;
};

}  // namespace app
