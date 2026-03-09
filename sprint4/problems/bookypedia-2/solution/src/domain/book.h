#pragma once
#include <string>
#include <vector>
#include "../util/tagged_uuid.h"
#include "author.h"

namespace domain {

namespace detail {
struct BookTag {};
}  // namespace detail

using BookId = util::TaggedUUID<detail::BookTag>;

class Book{
public:
    Book(BookId id, AuthorId author_id, std::string title, int publication_year)
        : id_(std::move(id))
        , author_id_(std::move(author_id))
        , title_(std::move(title))
        , publication_year_(std::move(publication_year)) {
    }

    const BookId& GetId() const noexcept {
        return id_;
    }

    const std::string& GetTitle() const noexcept {
        return title_;
    }

    const AuthorId& GetAuthorId() const noexcept {
        return author_id_;
    }

    const int GetPublicationYear() const noexcept {
        return publication_year_;
    }
private:
    BookId id_;
    AuthorId author_id_;
    std::string title_;
    int publication_year_;
}; 

class BookRepository {
public:
    virtual void Save(const Book& book) = 0;
    virtual const std::vector<domain::Book> GetAllBooks() = 0;
    virtual const std::vector<domain::Book> GetAuthorBooks(domain::AuthorId author_id) = 0;
    virtual const std::vector<domain::Book> GetBooksByTitle(const std::string& title) = 0;
    virtual void DeleteBook(const domain::BookId& book_id) = 0;
    virtual void EditBookById(const domain::Book& book) = 0;
protected:
    ~BookRepository() = default;
};
}  // namespace domain
