#pragma once
#include <string>
#include <utility>

class Borrowable {
public:
    virtual ~Borrowable() = default;
    virtual bool borrow() = 0;
    virtual void returnItem() = 0;
    virtual bool isAvailable() const = 0;
};

class LibraryItem : public Borrowable {
public:
    LibraryItem(std::string id, std::string title, std::string author, int year)
        : id_(std::move(id)), title_(std::move(title)), author_(std::move(author)), year_(year) {}
    virtual ~LibraryItem() = default;
    const std::string& id() const { return id_; }
    const std::string& title() const { return title_; }
    const std::string& author() const { return author_; }
    int year() const { return year_; }
    virtual std::string type() const = 0;
    bool borrow() override { if (!available_) return false; available_ = false; return true; }
    void returnItem() override { available_ = true; }
    bool isAvailable() const override { return available_; }
private:
    std::string id_, title_, author_;
    int year_;
    bool available_ = true;
};

class Book final : public LibraryItem {
public:
    using LibraryItem::LibraryItem;
    std::string type() const override { return "Book"; }
};
class Magazine final : public LibraryItem {
public:
    using LibraryItem::LibraryItem;
    std::string type() const override { return "Magazine"; }
};
class Journal final : public LibraryItem {
public:
    using LibraryItem::LibraryItem;
    std::string type() const override { return "Journal"; }
};
