#pragma once
#include <string>
#include <vector>
#include <utility>
class UserAccount {
public:
    UserAccount(std::string id, std::string name) : id_(std::move(id)), name_(std::move(name)) {}
    const std::string& id() const { return id_; }
    const std::string& name() const { return name_; }
    void recordBorrow(const std::string& itemId) { borrowingHistory_.push_back(itemId); }
    const std::vector<std::string>& borrowingHistory() const { return borrowingHistory_; }
    void restoreHistory(const std::vector<std::string>& history) { borrowingHistory_ = history; }
private:
    std::string id_, name_;
    std::vector<std::string> borrowingHistory_;
};
