#pragma once

#include <string>
#include <vector>
#include "../domain/author.h"
namespace app {

class UseCases {
public:
    virtual void AddAuthor(const std::string& name) = 0;
    virtual const std::vector<domain::Author> GetAllAuthor() = 0;
protected:
    ~UseCases() = default;
};

}  // namespace app
