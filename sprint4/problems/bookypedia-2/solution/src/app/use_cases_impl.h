#pragma once
#include "../domain/author_fwd.h"
#include "use_cases.h"
#include <vector>
#include <memory>
#include <optional>
#include "unit_of_work_factory.h"

namespace app {

class UseCasesImpl : public UseCases {
public:
    explicit UseCasesImpl(std::shared_ptr<UnitOfWorkFactory> unit_of_work_factory)
        : unit_of_work_factory_{ unit_of_work_factory } {
        unit_of_work_ = unit_of_work_factory_->CreateUnitOfWork();
    }

    void AddAuthor(const std::string& name) override {
        unit_of_work_->Authors().Save({ domain::AuthorId::New(), name });
    }

    std::string AddBook(const std::string& author_id, const std::string& title, const int publication_year) override {
        auto id = domain::BookId::New();
        unit_of_work_->Books().Save({ id, domain::AuthorId::FromString(author_id), title,  publication_year });
        return id.ToString();
    }

    const std::vector<domain::Author> GetAllAuthor() override {
        return unit_of_work_->Authors().GetAllAuthor();
    }

    const std::vector<domain::Book> GetAllBook() override {
        return unit_of_work_->Books().GetAllBooks();
    }
    const std::vector<domain::Book> GetAuthorBooks(const std::string& author_id) override {
        return unit_of_work_->Books().GetAuthorBooks(domain::AuthorId::FromString(author_id)); 
    }

    const std::optional<domain::Author> GetAuthor(const std::string& name) override {
        return unit_of_work_->Authors().GetAuthor(name);
    }


    void Commit() override {
        unit_of_work_->Commit();
        unit_of_work_ = unit_of_work_factory_->CreateUnitOfWork();
    }

    void AddTag(const std::string& book_id, const std::string& tag) override {
        unit_of_work_->Tags().Save(domain::Tag{ domain::BookId::FromString(book_id), tag });
    }

    void DeleteAuthor(const std::string& author_id) override {
        unit_of_work_->Authors().DeleteAuthor(domain::AuthorId::FromString(author_id));
    }

    void EditAuthor(const std::string& author_id, const std::string& name) override {
        unit_of_work_->Authors().EditAuthor(domain::Author{ domain::AuthorId::FromString(author_id), name });
    }

    const std::vector<std::string> GetAllTags(const std::string& book_id) override {
        return unit_of_work_->Tags().GetAllTags(domain::BookId::FromString(book_id));
    }

    const std::vector<domain::Book> GetBooksByTitle(const std::string& title) {
        return unit_of_work_->Books().GetBooksByTitle(title);
    }

    void DeleteBook(const domain::BookId& book_id) {
        unit_of_work_->Books().DeleteBook(book_id);
    }

    void EditBook(const domain::Book& book) {
        unit_of_work_->Books().EditBookById(book);
    }

    void DeleteAllTags(const std::string& book_id) override {
        unit_of_work_->Tags().DeleteAllTags(domain::BookId::FromString(book_id));
    }
private:
    /*UnitOfWork& unit_of_work_;*/
    std::shared_ptr<UnitOfWorkFactory> unit_of_work_factory_;
    std::shared_ptr<app::UnitOfWork> unit_of_work_;
};

}  // namespace app
