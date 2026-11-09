#pragma once
#include "controller/LibraryManager.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QString>
#include <QDate>
#include <QFileInfo>

class FileHandler {
public:
    static bool save(const LibraryDatabase& db, const QString& path, QString* error = nullptr) {
        QJsonObject root; root["schemaVersion"] = 1;
        QJsonArray items, users, loans, reservations;
        for (const auto& ptr : db.items()) {
            const auto& x = *ptr;
            items.append(QJsonObject{{"id", QString::fromStdString(x.id())}, {"type", QString::fromStdString(x.type())},
                {"title", QString::fromStdString(x.title())}, {"author", QString::fromStdString(x.author())}, {"year", x.year()}});
            if (const auto* q = db.waitlistIfPresent(x.id())) {
                auto copy = *q;
                while (!copy.empty()) {
                    reservations.append(QJsonObject{{"itemId", QString::fromStdString(x.id())},
                        {"userId", QString::fromStdString(copy.front())}});
                    copy.pop();
                }
            }
        }
        for (const auto& u : db.users()) {
            QJsonArray history;
            for (const auto& id : u.borrowingHistory()) history.append(QString::fromStdString(id));
            users.append(QJsonObject{{"id", QString::fromStdString(u.id())}, {"name", QString::fromStdString(u.name())}, {"history", history}});
        }
        for (const auto& l : db.loans()) loans.append(QJsonObject{{"itemId", QString::fromStdString(l.itemId)},
            {"userId", QString::fromStdString(l.userId)}, {"borrowedOn", QString::fromStdString(l.borrowedOn)},
            {"dueOn", QString::fromStdString(l.dueOn)}});
        root["items"] = items; root["users"] = users; root["loans"] = loans; root["reservations"] = reservations;
        QSaveFile file(path);
        if (!file.open(QIODevice::WriteOnly)) { if (error) *error = file.errorString(); return false; }
        if (file.write(QJsonDocument(root).toJson(QJsonDocument::Indented)) < 0 || !file.commit()) {
            if (error) *error = file.errorString(); return false;
        }
        return true;
    }
    static bool load(LibraryManager& manager, const QString& path, QString* error = nullptr) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) { if (error) *error = file.errorString(); return false; }
        QJsonParseError parse;
        const auto doc = QJsonDocument::fromJson(file.readAll(), &parse);
        if (parse.error != QJsonParseError::NoError || !doc.isObject()) { if (error) *error = "Invalid JSON document"; return false; }
        const auto root = doc.object();
        if (root.value("schemaVersion").toInt() != 1 || !root.value("items").isArray() ||
            !root.value("users").isArray() || !root.value("loans").isArray() || !root.value("reservations").isArray()) {
            if (error) *error = "Unsupported or incomplete data format"; return false;
        }
        LibraryDatabase restored;
        auto fail = [&](const QString& why) { if (error) *error = why; return false; };
        for (const auto& value : root.value("items").toArray()) {
            if (!value.isObject()) return fail("Invalid item entry");
            const auto x = value.toObject();
            if (!x.value("id").isString() || !x.value("title").isString() || !x.value("author").isString() ||
                !x.value("type").isString() || !x.value("year").isDouble()) return fail("Invalid item fields");
            const auto id = x.value("id").toString().toStdString();
            const auto title = x.value("title").toString().toStdString();
            const auto author = x.value("author").toString().toStdString();
            const auto type = x.value("type").toString();
            const int year = x.value("year").toInt();
            if (id.empty() || title.empty() || author.empty() || year < 1 || year > 9999) return fail("Invalid item values");
            std::unique_ptr<LibraryItem> item;
            if (type == "Book") item = std::make_unique<Book>(id,title,author,year);
            else if (type == "Magazine") item = std::make_unique<Magazine>(id,title,author,year);
            else if (type == "Journal") item = std::make_unique<Journal>(id,title,author,year);
            else return fail("Unknown item type");
            if (!restored.add(std::move(item))) return fail("Duplicate item ID");
        }
        for (const auto& value : root.value("users").toArray()) {
            if (!value.isObject()) return fail("Invalid user entry");
            const auto x = value.toObject();
            if (!x.value("id").isString() || !x.value("name").isString() || !x.value("history").isArray()) return fail("Invalid user fields");
            const auto id = x.value("id").toString().toStdString();
            if (!restored.addUser(id,x.value("name").toString().toStdString())) return fail("Duplicate or invalid user");
            std::vector<std::string> history;
            for (const auto& h : x.value("history").toArray()) {
                if (!h.isString()) return fail("Invalid history entry");
                history.push_back(h.toString().toStdString());
            }
            restored.findUser(id)->restoreHistory(history);
        }
        for (const auto& value : root.value("loans").toArray()) {
            if (!value.isObject()) return fail("Invalid loan entry");
            const auto x = value.toObject();
            if (!x.value("itemId").isString() || !x.value("userId").isString() ||
                !x.value("borrowedOn").isString() || !x.value("dueOn").isString()) return fail("Invalid loan fields");
            Loan loan{x.value("itemId").toString().toStdString(),x.value("userId").toString().toStdString(),
                x.value("borrowedOn").toString().toStdString(),x.value("dueOn").toString().toStdString()};
            const auto borrowed = QDate::fromString(x.value("borrowedOn").toString(),Qt::ISODate);
            const auto due = QDate::fromString(x.value("dueOn").toString(),Qt::ISODate);
            if (!borrowed.isValid() || !due.isValid() || due < borrowed || !restored.restoreLoan(loan)) return fail("Invalid or duplicate loan");
        }
        for (const auto& value : root.value("reservations").toArray()) {
            if (!value.isObject()) return fail("Invalid reservation entry");
            const auto x = value.toObject();
            if (!x.value("itemId").isString() || !x.value("userId").isString() ||
                !restored.restoreReservation(x.value("itemId").toString().toStdString(),x.value("userId").toString().toStdString()))
                return fail("Invalid reservation");
        }
        manager.replaceDatabase(std::move(restored));
        return true;
    }
};
