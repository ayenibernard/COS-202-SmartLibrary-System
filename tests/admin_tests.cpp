#include "controller/LibraryManager.h"
#include <cassert>
#include <iostream>
int main() {
    LibraryManager m;
    assert(!m.undoLastAdminAction().ok);
    assert(m.addItem("Book", "BK001", "C++", "A", 2024));
    assert(m.addItem("Journal", "JR001", "Research", "B", 2025));
    assert(!m.addItem("Book", "BK001", "Duplicate", "C", 2026));
    assert(m.undoCount() == 2);
    assert(m.deleteItem("BK001").ok);
    assert(m.database().find("BK001") == nullptr);
    assert(m.undoLastAdminAction().ok);
    assert(m.database().find("BK001") != nullptr);
    assert(m.undoLastAdminAction().ok); // undo JR001 addition
    assert(m.database().find("JR001") == nullptr);
    assert(m.registerUser("U001", "Ada"));
    assert(m.borrowItem("BK001", "U001").ok);
    assert(!m.deleteItem("BK001").ok);
    assert(!m.undoLastAdminAction().ok); // cannot undo borrowed BK001 addition
    assert(m.returnItem("BK001", "U001").ok);
    assert(m.undoLastAdminAction().ok);
    assert(m.database().count() == 0);
    assert(!m.undoLastAdminAction().ok);
    assert(m.addItem("Book", "BK002", "Algorithms", "A", 2025));
    assert(m.borrowItem("BK002", "U001").ok);
    assert(m.registerUser("U002", "Ben"));
    assert(m.reserveItem("BK002", "U002").ok);
    assert(m.returnItem("BK002", "U001").ok);
    assert(!m.deleteItem("BK002").ok); // reservation still pending
    assert(!m.undoLastAdminAction().ok);
    assert(m.borrowItem("BK002", "U002").ok);
    assert(m.returnItem("BK002", "U002").ok);
    assert(m.deleteItem("BK002").ok);
    assert(m.undoLastAdminAction().ok);
    assert(m.database().find("BK002") != nullptr);
    std::cout << "PASS: admin delete, LIFO undo, blocked operations, circulation regression\n";
}
