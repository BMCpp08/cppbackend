#pragma once
#include <string>
#include <vector>
#include "../util/tagged_uuid.h"
#include <optional>

namespace domain {

namespace detail {
struct AuthorTag {};

}  // namespace detail

using AuthorId = util::TaggedUUID<detail::AuthorTag>;

class Author {
public:
    Author(AuthorId id, std::string name)
        : id_(std::move(id))
        , name_(std::move(name)) {
    }

    const AuthorId& GetId() const noexcept {
        return id_;
    }

    const std::string& GetName() const noexcept {
        return name_;
    }

private:
    AuthorId id_;
    std::string name_;
};

class AuthorRepository {
public:
    virtual void Save(const Author& author) = 0;
    virtual const std::vector<domain::Author> GetAllAuthor() = 0;
    virtual const std::optional<domain::Author> GetAuthor(const std::string& name) = 0;
    virtual void DeleteAuthor(const AuthorId& author_id) = 0;
    virtual void EditAuthor(const domain::Author& author) = 0;
    //virtual const std::optional<domain::Author> GetAuthorById(const domain::AuthorId& author_id) = 0;
protected:
    ~AuthorRepository() = default;
};


}  // namespace domain
