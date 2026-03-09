#pragma once
#include <iosfwd>
#include <optional>
#include <string>
#include <vector>
#include <unordered_set>

namespace menu {
class Menu;
}

namespace app {
class UseCases;
}

namespace ui {
namespace detail {

struct AddBookParams {
    std::string title;
    std::string author_id;
    int publication_year = 0;
};

struct AuthorInfo {
    std::string id;
    std::string name;
};

struct BookInfo {
    std::string title;
    int publication_year;
    std::string author;
    std::string author_id;
    std::string book_id;
};

struct TagInfo {
    std::string title;
    int publication_year;
    std::string author;
    std::vector<std::string> tags;
};
}  // namespace detail

class View {
public:
    View(menu::Menu& menu, app::UseCases& use_cases, std::istream& input, std::ostream& output);

private:
    bool AddAuthor(std::istream& cmd_input) const;
    bool AddBook(std::istream& cmd_input) const;
    bool ShowAuthors() const;
    bool ShowBooks() const;
    bool ShowAuthorBooks() const;
    bool DeleteAuthor(std::istream& cmd_input) const;
    bool AddTag(const std::string& book_id, const std::unordered_set<std::string>& tags) const;
    bool EditAuthor(std::istream& cmd_input) const;
    bool ShowBook(std::istream& cmd_input) const;
    bool DeleteBook(std::istream& cmd_input) const;
    bool EditBook(std::istream& cmd_input) const;

    std::optional<detail::AddBookParams> GetBookParams(std::istream& cmd_input) const;
    std::optional<std::string> SelectAuthor() const;
    std::vector<detail::AuthorInfo> GetAuthors() const;
    std::vector<detail::BookInfo> GetBooks() const;
    std::vector<detail::BookInfo> GetAuthorBooks(const std::string& author_id) const;
    std::unordered_set<std::string> ReadTags() const;
    int SelectBook(const std::vector<detail::BookInfo>& books) const;
    std::vector<detail::BookInfo> GetBooksByTitle(const std::string& title) const;

    bool AddTag(const std::string& book_id, const std::vector<std::string>& tags) const;
    menu::Menu& menu_;
    app::UseCases& use_cases_;
    std::istream& input_;
    std::ostream& output_;
};

}  // namespace ui