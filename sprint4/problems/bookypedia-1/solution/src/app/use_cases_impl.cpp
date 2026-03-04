#include "use_cases_impl.h"



namespace app {
using namespace domain;

void UseCasesImpl::AddAuthor(const std::string& name) {
    authors_.Save({AuthorId::New(), name});
}
const std::vector<domain::Author> UseCasesImpl::GetAllAuthor() {
    return authors_.GetAllAuthor();
}
}  // namespace app
