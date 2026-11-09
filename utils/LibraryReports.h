#pragma once
#include "model/LibraryDatabase.h"
#include <QDate>
#include <QString>
#include <QStringList>
#include <algorithm>
#include <map>
#include <vector>

class LibraryReports {
public:
    static QString overdue(const LibraryDatabase& db, QDate today = QDate::currentDate()) {
        QStringList rows;
        for (const auto& l : db.loans()) {
            const auto due = QDate::fromString(QString::fromStdString(l.dueOn), Qt::ISODate);
            if (due.isValid() && due < today) {
                const auto* user = db.findUser(l.userId);
                rows << QString("%1 (%2) | Item: %3 | Due: %4 | %5 days overdue")
                    .arg(user ? QString::fromStdString(user->name()) : "Unknown")
                    .arg(QString::fromStdString(l.userId))
                    .arg(QString::fromStdString(l.itemId))
                    .arg(due.toString(Qt::ISODate)).arg(due.daysTo(today));
            }
        }
        return rows.isEmpty() ? "No overdue loans." : rows.join("\n");
    }
    static QString categoryDistribution(const LibraryDatabase& db) {
        std::map<std::string,int> counts;
        for (const auto& p : db.items()) ++counts[p->type()];
        QStringList rows;
        for (const auto& [type,count] : counts) rows << QString::fromStdString(type) + ": " + QString::number(count);
        return rows.isEmpty() ? "No library items." : rows.join("\n");
    }
    static QString mostBorrowed(const LibraryDatabase& db) {
        std::map<std::string,int> counts;
        for (const auto& user : db.users()) for (const auto& id : user.borrowingHistory()) ++counts[id];
        std::vector<std::pair<std::string,int>> ranked(counts.begin(),counts.end());
        std::sort(ranked.begin(),ranked.end(),[](const auto& a,const auto& b) {
            return a.second != b.second ? a.second > b.second : a.first < b.first;
        });
        QStringList rows;
        for (const auto& [id,count] : ranked) {
            const auto* item = db.find(id);
            rows << QString("%1 | %2 | %3 borrow(s)").arg(QString::fromStdString(id))
                .arg(item ? QString::fromStdString(item->title()) : "Deleted item").arg(count);
        }
        return rows.isEmpty() ? "No borrowing history." : rows.join("\n");
    }
    static int overdueCount(const LibraryDatabase& db, QDate today = QDate::currentDate()) {
        int count = 0;
        for (const auto& l : db.loans()) {
            const auto due = QDate::fromString(QString::fromStdString(l.dueOn),Qt::ISODate);
            if (due.isValid() && due < today) ++count;
        }
        return count;
    }
};
