#pragma once
#include "../domain/author_fwd.h"
#include "use_cases.h"
#include <vector>
#include <memory>
#include <optional>
#include "unit_of_work_factory.h"
#include <iostream>

namespace app {

    class UseCasesImpl : public UseCases {
    public:
        explicit UseCasesImpl(std::shared_ptr<UnitOfWorkFactory> unit_of_work_factory)
            : unit_of_work_factory_{ unit_of_work_factory } {
            unit_of_work_ = unit_of_work_factory_->CreateUnitOfWork();
        }

        std::string AddAuthor(const std::string& name) override {
            try {
                auto id = domain::AuthorId::New();
                unit_of_work_->Authors().Save({ id, name });
                return id.ToString();
            }
            catch (...) {
                /*Rollback();*/
                throw;
            }
        }

        std::string AddBook(const std::string& author_id, const std::string& title, int publication_year) override {
            try {

                auto id = domain::BookId::New();
                unit_of_work_->Books().Save({ id, domain::AuthorId::FromString(author_id), title, publication_year });
                return id.ToString();
            }
            catch (...) {
                /*Rollback();*/
                throw;
            }
        }
        const std::vector<domain::Author> GetAllAuthor() override {
            try {
                return unit_of_work_->Authors().GetAllAuthor();
            }
            catch (...) {
                Rollback();
                throw;
            }
        }

        const std::vector<domain::Book> GetAllBook() override {
            try {
                return unit_of_work_->Books().GetAllBooks();
            }
            catch (...) {
                Rollback();
                throw;
            }
        }

        const std::vector<domain::Book> GetAuthorBooks(const std::string& author_id) override {
            try {
                return unit_of_work_->Books().GetAuthorBooks(domain::AuthorId::FromString(author_id));
            }
            catch (...) {
                Rollback();
                throw;
            }
        }

        const std::optional<domain::Author> GetAuthor(const std::string& name) override {
            try {
                return unit_of_work_->Authors().GetAuthor(name);
            }
            catch (...) {
                Rollback();
                throw;
            }
        }

        //void Commit() override {
        //    try {
        //        unit_of_work_->Commit();
        //        unit_of_work_ = unit_of_work_factory_->CreateUnitOfWork();
        //    }
        //    catch (...) {
        //        unit_of_work_->Rollback();
        //        unit_of_work_ = unit_of_work_factory_->CreateUnitOfWork();
        //        throw;
        //    }
        //}

        void Rollback() override {
            if (unit_of_work_) {
                unit_of_work_->Rollback();
            }
            unit_of_work_ = unit_of_work_factory_->CreateUnitOfWork();
        }

        void Commit() override {
            try {
                std::cerr << ">>> Commit: starting" << std::endl;

                if (!unit_of_work_) {
                    std::cerr << ">>> Commit: ERROR - unit_of_work_ is null, creating new one" << std::endl;
                    unit_of_work_ = unit_of_work_factory_->CreateUnitOfWork();
                    return;
                }

                std::cerr << ">>> Commit: calling unit_of_work_->Commit()" << std::endl;
                unit_of_work_->Commit();
                std::cerr << ">>> Commit: successful" << std::endl;

                std::cerr << ">>> Commit: creating new UnitOfWork" << std::endl;
                unit_of_work_ = unit_of_work_factory_->CreateUnitOfWork();
                std::cerr << ">>> Commit: new UnitOfWork created" << std::endl;
            }
            catch (const std::exception& e) {
                std::cerr << ">>> Commit: EXCEPTION: " << e.what() << std::endl;
                try {
                    Rollback();
                }
                catch (...) {
                    std::cerr << ">>> Commit: Rollback also failed" << std::endl;
                }
                throw;
            }
        }

        //void AddTag(const std::string& book_id, const std::string& tag) override {
        //    try {
        //        unit_of_work_->Tags().Save(domain::Tag{ domain::BookId::FromString(book_id), tag });
        //    }
        //    catch (...) {
        //        unit_of_work_->Rollback();
        //        unit_of_work_ = unit_of_work_factory_->CreateUnitOfWork();
        //        throw;
        //    }
        //}
        void AddTag(const std::string& book_id, const std::string& tag) override {
            try {

                if (tag.empty()) {
                    return;
                }
                unit_of_work_->Tags().Save(domain::Tag{ domain::BookId::FromString(book_id), tag });
            }
            catch (const std::exception& e) {
                /*Rollback();*/
                throw;
            }
        }

        void DeleteAuthor(const std::string& author_id) override {
            try {
                unit_of_work_->Authors().DeleteAuthor(domain::AuthorId::FromString(author_id));
            }
            catch (...) {
                Rollback();
                throw;
            }
        }

        void EditAuthor(const std::string& author_id, const std::string& name) override {
            try {
                unit_of_work_->Authors().EditAuthor(domain::Author{ domain::AuthorId::FromString(author_id), name });
            }
            catch (...) {
                Rollback();
                throw;
            }
        }

        const std::vector<std::string> GetAllTags(const std::string& book_id) override {
            try {
                return unit_of_work_->Tags().GetAllTags(domain::BookId::FromString(book_id));
            }
            catch (...) {
                Rollback();
                throw;
            }
        }

        const std::vector<domain::Book> GetBooksByTitle(const std::string& title) override {
            try {
                return unit_of_work_->Books().GetBooksByTitle(title);
            }
            catch (...) {
                Rollback();
                throw;
            }
        }

        void DeleteBook(const domain::BookId& book_id) override {
            try {
                unit_of_work_->Books().DeleteBook(book_id);
            }
            catch (...) {
                /*Rollback();*/
                throw;
            }
        }

        void EditBook(const domain::Book& book) override {
            try {
                unit_of_work_->Books().EditBookById(book);
            }
            catch (...) {
                Rollback();
                throw;
            }
        }

        void DeleteAllTags(const std::string& book_id) override {
            try {
                unit_of_work_->Tags().DeleteAllTags(domain::BookId::FromString(book_id));
            }
            catch (...) {
               /* Rollback();*/
                throw;
            }
        }

    private:
        std::shared_ptr<UnitOfWorkFactory> unit_of_work_factory_;
        std::shared_ptr<app::UnitOfWork> unit_of_work_;
    };

}  // namespace app