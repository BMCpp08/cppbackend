#pragma once
#include "../domain/author_fwd.h"
#include "use_cases.h"
#include <vector>

namespace app {

class UseCasesImpl : public UseCases {
public:
    explicit UseCasesImpl(domain::AuthorRepository& authors)
        : authors_{authors} {
    }

    void AddAuthor(const std::string& name) override;
    const std::vector<domain::Author> GetAllAuthor() override;

private:
    domain::AuthorRepository& authors_;
};

}  // namespace app
