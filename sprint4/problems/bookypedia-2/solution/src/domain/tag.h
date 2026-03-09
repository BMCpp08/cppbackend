#pragma once
#include <string>
#include <vector>
#include "../util/tagged_uuid.h"
#include "author.h"

namespace domain {

class Tag{
public:
    Tag(domain::BookId book_id, std::string tag)
        : book_id_(std::move(book_id))
        , tag_(std::move(tag)) {
    }

    const BookId& GetBookId() const noexcept {
        return book_id_;
    }

    const std::string& GetTag() const noexcept {
        return tag_;
    }
private:
    BookId book_id_;
    std::string tag_;
}; 

class TagRepository {
public:
    virtual void Save(const Tag& book) = 0;
    virtual const std::vector<std::string> GetAllTags(domain::BookId book_id) = 0;
    virtual void DeleteAllTags(const domain::BookId& book_id) = 0;
    //virtual const std::vector<domain::Book> GetAllBooks() = 0;
    //virtual const std::vector<domain::Book> GetAuthorBooks(domain::AuthorId author_id) = 0;
protected:
    ~TagRepository() = default;
};
}  // namespace domain
