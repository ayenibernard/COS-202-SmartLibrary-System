#pragma once
#include "LibraryItem.h"
#include "UserAccount.h"
#include "FrequentItemCache.h"
#include <algorithm>
#include <memory>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

struct Loan {
    std::string itemId;
    std::string userId;
    std::string borrowedOn;
    std::string dueOn;
};

class LibraryDatabase {
public:
    bool add(std::unique_ptr<LibraryItem> item) {
        if (!item || find(item->id())) return false;
        items_.push_back(std::move(item));
        return true;
    }
    LibraryItem* find(const std::string& id) const {
        for (const auto& item : items_) if (item->id() == id) { frequentItems_.recordAccess(id); return item.get(); }
        return nullptr;
    }
    bool addUser(const std::string& id, const std::string& name) {
        if (id.empty() || name.empty() || findUser(id)) return false;
        users_.emplace_back(id, name);
        return true;
    }
    UserAccount* findUser(const std::string& id) {
        for (auto& user : users_) if (user.id() == id) return &user;
        return nullptr;
    }
    const UserAccount* findUser(const std::string& id) const {
        for (const auto& user : users_) if (user.id() == id) return &user;
        return nullptr;
    }
    Loan* findLoan(const std::string& itemId) {
        for (auto& loan : loans_) if (loan.itemId == itemId) return &loan;
        return nullptr;
    }
    const Loan* findLoan(const std::string& itemId) const {
        for (const auto& loan : loans_) if (loan.itemId == itemId) return &loan;
        return nullptr;
    }
    const std::vector<std::unique_ptr<LibraryItem>>& items() const { return items_; }
    const std::vector<UserAccount>& users() const { return users_; }
    const std::vector<Loan>& loans() const { return loans_; }
    std::queue<std::string>& waitlist(const std::string& id) { return reservations_[id]; }
    const std::queue<std::string>* waitlistIfPresent(const std::string& id) const {
        auto it = reservations_.find(id);
        return it == reservations_.end() ? nullptr : &it->second;
    }
    void addLoan(Loan loan) { loans_.push_back(std::move(loan)); }
    void removeLoan(const std::string& itemId) {
        loans_.erase(std::remove_if(loans_.begin(), loans_.end(), [&](const Loan& loan) {
            return loan.itemId == itemId;
        }), loans_.end());
    }
    bool removeItem(const std::string& id) {
        auto it = std::find_if(items_.begin(), items_.end(), [&](const auto& item) { return item->id() == id; });
        if (it == items_.end() || findLoan(id)) return false;
        const auto* queue = waitlistIfPresent(id);
        if (queue && !queue->empty()) return false;
        items_.erase(it);
        frequentItems_.remove(id);
        reservations_.erase(id);
        return true;
    }
    void clear() { items_.clear(); users_.clear(); loans_.clear(); reservations_.clear(); frequentItems_.clear(); }
    bool restoreLoan(const Loan& loan) {
        auto* item = find(loan.itemId);
        if (!item || !findUser(loan.userId) || !item->borrow() || findLoan(loan.itemId)) return false;
        loans_.push_back(loan); return true;
    }
    bool restoreReservation(const std::string& id, const std::string& userId) {
        if (!find(id) || !findUser(userId)) return false;
        reservations_[id].push(userId); return true;
    }
    std::size_t count() const { return items_.size(); }
    const FrequentItemCache& frequentItems() const { return frequentItems_; }
private:
    mutable FrequentItemCache frequentItems_;
    std::vector<std::unique_ptr<LibraryItem>> items_;
    std::vector<UserAccount> users_;
    std::vector<Loan> loans_;
    std::unordered_map<std::string, std::queue<std::string>> reservations_;
};
