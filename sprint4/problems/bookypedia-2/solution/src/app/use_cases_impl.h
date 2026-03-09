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
            try {
                unit_of_work_->Authors().Save({ domain::AuthorId::New(), name });
            }
            catch (...) {
                unit_of_work_ = unit_of_work_factory_->CreateUnitOfWork();
                throw;
            }
        }

        std::string AddBook(const std::string& author_id, const std::string& title, const int publication_year) override {
            try {
                auto id = domain::BookId::New();
                unit_of_work_->Books().Save({ id, domain::AuthorId::FromString(author_id), title, publication_year });
                return id.ToString();
            }
            catch (...) {
                unit_of_work_ = unit_of_work_factory_->CreateUnitOfWork();
                throw;
            }
        }

        const std::vector<domain::Author> GetAllAuthor() override {
            try {
                return unit_of_work_->Authors().GetAllAuthor();
            }
            catch (...) {
                unit_of_work_ = unit_of_work_factory_->CreateUnitOfWork();
                throw;
            }
        }

        const std::vector<domain::Book> GetAllBook() override {
            try {
                return unit_of_work_->Books().GetAllBooks();
            }
            catch (...) {
                unit_of_work_ = unit_of_work_factory_->CreateUnitOfWork();
                throw;
            }
        }

        const std::vector<domain::Book> GetAuthorBooks(const std::string& author_id) override {
            try {
                return unit_of_work_->Books().GetAuthorBooks(domain::AuthorId::FromString(author_id));
            }
            catch (...) {
                unit_of_work_ = unit_of_work_factory_->CreateUnitOfWork();
                throw;
            }
        }

        const std::optional<domain::Author> GetAuthor(const std::string& name) override {
            try {
                return unit_of_work_->Authors().GetAuthor(name);
            }
            catch (...) {
                unit_of_work_ = unit_of_work_factory_->CreateUnitOfWork();
                throw;
            }
        }

        void Commit() override {
            try {
                unit_of_work_->Commit();
                unit_of_work_ = unit_of_work_factory_->CreateUnitOfWork();
            }
            catch (...) {
                unit_of_work_ = unit_of_work_factory_->CreateUnitOfWork();
                throw;
            }
        }

        void AddTag(const std::string& book_id, const std::string& tag) override {
            try {
                unit_of_work_->Tags().Save(domain::Tag{ domain::BookId::FromString(book_id), tag });
            }
            catch (...) {
                unit_of_work_ = unit_of_work_factory_->CreateUnitOfWork();
                throw;
            }
        }

        void DeleteAuthor(const std::string& author_id) override {
            try {
                unit_of_work_->Authors().DeleteAuthor(domain::AuthorId::FromString(author_id));
            }
            catch (...) {
                unit_of_work_ = unit_of_work_factory_->CreateUnitOfWork();
                throw;
            }
        }

        void EditAuthor(const std::string& author_id, const std::string& name) override {
            try {
                unit_of_work_->Authors().EditAuthor(domain::Author{ domain::AuthorId::FromString(author_id), name });
            }
            catch (...) {
                unit_of_work_ = unit_of_work_factory_->CreateUnitOfWork();
                throw;
            }
        }

        const std::vector<std::string> GetAllTags(const std::string& book_id) override {
            try {
                return unit_of_work_->Tags().GetAllTags(domain::BookId::FromString(book_id));
            }
            catch (...) {
                unit_of_work_ = unit_of_work_factory_->CreateUnitOfWork();
                throw;
            }
        }

        const std::vector<domain::Book> GetBooksByTitle(const std::string& title) override {
            try {
                return unit_of_work_->Books().GetBooksByTitle(title);
            }
            catch (...) {
                unit_of_work_ = unit_of_work_factory_->CreateUnitOfWork();
                throw;
            }
        }

        void DeleteBook(const domain::BookId& book_id) override {
            try {
                unit_of_work_->Books().DeleteBook(book_id);
            }
            catch (...) {
                unit_of_work_ = unit_of_work_factory_->CreateUnitOfWork();
                throw;
            }
        }

        void EditBook(const domain::Book& book) override {
            try {
                unit_of_work_->Books().EditBookById(book);
            }
            catch (...) {
                unit_of_work_ = unit_of_work_factory_->CreateUnitOfWork();
                throw;
            }
        }

        void DeleteAllTags(const std::string& book_id) override {
            try {
                unit_of_work_->Tags().DeleteAllTags(domain::BookId::FromString(book_id));
            }
            catch (...) {
                unit_of_work_ = unit_of_work_factory_->CreateUnitOfWork();
                throw;
            }
        }

    private:
        std::shared_ptr<UnitOfWorkFactory> unit_of_work_factory_;
        std::shared_ptr<app::UnitOfWork> unit_of_work_;
    };

}  // namespace app