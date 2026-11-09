#include "utils/FileHandler.h"
#include "utils/LibraryReports.h"
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QFile>
#include <cassert>
#include <iostream>
int main(int argc, char** argv) {
    QCoreApplication app(argc,argv);
    QTemporaryDir dir; assert(dir.isValid());
    const QString path = dir.path() + "/library.json";
    LibraryManager manager;
    assert(manager.addItem("Book","BK001","C++ Primer","Smith",2024));
    assert(manager.addItem("Journal","JR001","Science","Jones",2025));
    assert(manager.registerUser("U001","Ada"));
    assert(manager.registerUser("U002","Ben"));
    assert(manager.borrowItem("BK001","U001").ok);
    assert(manager.reserveItem("BK001","U002").ok);
    assert(FileHandler::save(manager.database(),path));
    LibraryManager restored;
    assert(FileHandler::load(restored,path));
    assert(restored.database().count()==2);
    assert(restored.database().users().size()==2);
    assert(restored.database().loans().size()==1);
    assert(!restored.database().find("BK001")->isAvailable());
    assert(restored.database().waitlistIfPresent("BK001")->front()=="U002");
    assert(restored.database().findUser("U001")->borrowingHistory().size()==1);
    assert(LibraryReports::categoryDistribution(restored.database()).contains("Journal: 1"));
    assert(LibraryReports::mostBorrowed(restored.database()).contains("BK001"));
    assert(LibraryReports::overdueCount(restored.database(),QDate::currentDate().addDays(30))==1);
    assert(restored.returnItem("BK001","U001").ok);
    assert(restored.borrowItem("BK001","U002").ok);
    QFile file(path); assert(file.open(QIODevice::WriteOnly|QIODevice::Truncate));
    file.write("{invalid-json"); file.close();
    assert(!FileHandler::load(restored,path));
    assert(restored.database().count()==2); // failed import must not alter data
    std::cout << "PASS: JSON round-trip, loans, queue, history, reports, invalid-file rollback\n";
}
