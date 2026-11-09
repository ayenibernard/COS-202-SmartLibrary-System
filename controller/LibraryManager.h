#pragma once
#include "model/LibraryDatabase.h"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <memory>
#include <sstream>
#include <stack>
#include <string>

struct OperationResult {
    bool ok;
    std::string message;
};

class LibraryManager {
public:
    bool addItem(const std::string& type, const std::string& id,
                 const std::string& title, const std::string& author, int year) {
        if (id.empty() || title.empty() || author.empty() || year < 1 || year > 9999) return false;
        std::unique_ptr<LibraryItem> item;
        if (type == "Book") item = std::make_unique<Book>(id, title, author, year);
        else if (type == "Magazine") item = std::make_unique<Magazine>(id, title, author, year);
        else if (type == "Journal") item = std::make_unique<Journal>(id, title, author, year);
        else return false;
        if (!database_.add(std::move(item))) return false;
        undo_.push({AdminAction::Added, id, type, title, author, year});
        return true;
    }
    bool registerUser(const std::string& id, const std::string& name) {
        return database_.addUser(id, name);
    }
    OperationResult borrowItem(const std::string& itemId, const std::string& userId) {
        auto* item = database_.find(itemId);
        auto* user = database_.findUser(userId);
        if (!item || !user) return {false, "Enter an existing item ID and registered user ID."};
        if (!item->isAvailable()) return {false, "Item is already borrowed. Use Reserve to join the waitlist."};
        auto& queue = database_.waitlist(itemId);
        if (!queue.empty() && queue.front() != userId)
            return {false, "Item is reserved for the first user in the waitlist."};
        if (!item->borrow()) return {false, "Item is unavailable."};
        if (!queue.empty()) queue.pop();
        const auto now = std::chrono::system_clock::now();
        const auto due = now + std::chrono::hours(24 * 14);
        database_.addLoan({itemId, userId, date(now), date(due)});
        user->recordBorrow(itemId);
        return {true, "Borrowed successfully. Loan period: 14 days."};
    }
    OperationResult returnItem(const std::string& itemId, const std::string& userId) {
        auto* item = database_.find(itemId);
        const auto* loan = database_.findLoan(itemId);
        if (!item || !loan) return {false, "No active loan exists for this item."};
        if (loan->userId != userId) return {false, "This item is borrowed by a different user."};
        item->returnItem();
        database_.removeLoan(itemId);
        const auto* queue = database_.waitlistIfPresent(itemId);
        if (queue && !queue->empty())
            return {true, "Returned successfully. Reserved for next user: " + queue->front()};
        return {true, "Returned successfully. Item is now available."};
    }
    OperationResult reserveItem(const std::string& itemId, const std::string& userId) {
        auto* item = database_.find(itemId);
        if (!item || !database_.findUser(userId)) return {false, "Enter an existing item ID and registered user ID."};
        if (const auto* loan = database_.findLoan(itemId); loan && loan->userId == userId)
            return {false, "You already have this item on loan."};
        auto& queue = database_.waitlist(itemId);
        auto copy = queue;
        while (!copy.empty()) {
            if (copy.front() == userId) return {false, "This user is already on the waitlist."};
            copy.pop();
        }
        if (item->isAvailable() && queue.empty()) return {false, "Item is available. Borrow it directly."};
        queue.push(userId);
        return {true, "Reservation added. Queue position: " + std::to_string(queue.size())};
    }
    OperationResult deleteItem(const std::string& id) {
        auto* item = database_.find(id);
        if (!item) return {false, "Item not found."};
        if (!item->isAvailable() || database_.findLoan(id))
            return {false, "Cannot delete a borrowed item."};
        const auto* queue = database_.waitlistIfPresent(id);
        if (queue && !queue->empty()) return {false, "Cannot delete an item with pending reservations."};
        AdminAction action{AdminAction::Deleted, item->id(), item->type(), item->title(), item->author(), item->year()};
        if (!database_.removeItem(id)) return {false, "Unable to delete this item."};
        undo_.push(std::move(action));
        return {true, "Item deleted. Use Undo Last Admin Action to restore it."};
    }
    OperationResult undoLastAdminAction() {
        if (undo_.empty()) return {false, "There is no admin operation to undo."};
        const auto& action = undo_.top();
        if (action.kind == AdminAction::Added) {
            if (!database_.removeItem(action.id))
                return {false, "Cannot undo addition while the item is borrowed or reserved."};
        } else {
            std::unique_ptr<LibraryItem> item;
            if (action.type == "Book") item = std::make_unique<Book>(action.id, action.title, action.author, action.year);
            else if (action.type == "Magazine") item = std::make_unique<Magazine>(action.id, action.title, action.author, action.year);
            else if (action.type == "Journal") item = std::make_unique<Journal>(action.id, action.title, action.author, action.year);
            if (!item || !database_.add(std::move(item)))
                return {false, "Unable to restore the deleted item."};
        }
        undo_.pop();
        return {true, "Last admin item operation undone successfully."};
    }
    void replaceDatabase(LibraryDatabase&& restored) {
        database_ = std::move(restored);
        undo_ = std::stack<AdminAction>{}; // undo history is session-only
    }
    std::size_t undoCount() const { return undo_.size(); }
    const LibraryDatabase& database() const { return database_; }
private:
    struct AdminAction {
        enum Kind { Added, Deleted } kind;
        std::string id, type, title, author;
        int year;
    };
    std::stack<AdminAction> undo_;
    static std::string date(const std::chrono::system_clock::time_point& when) {
        const std::time_t raw = std::chrono::system_clock::to_time_t(when);
        std::tm local{};
#ifdef _WIN32
        localtime_s(&local, &raw);
#else
        localtime_r(&raw, &local);
#endif
        std::ostringstream output;
        output << std::put_time(&local, "%Y-%m-%d");
        return output.str();
    }
    LibraryDatabase database_;
};
