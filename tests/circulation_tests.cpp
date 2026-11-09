#include "controller/LibraryManager.h"
#include <cassert>
#include <iostream>
int main() {
    LibraryManager m;
    assert(m.addItem("Book", "BK001", "Introduction to C++", "John Smith", 2024));
    assert(!m.addItem("Book", "BK001", "Duplicate", "Someone", 2025));
    assert(m.registerUser("U001", "Ada"));
    assert(m.registerUser("U002", "Ben"));
    assert(!m.registerUser("U001", "Duplicate"));
    assert(!m.borrowItem("BAD", "U001").ok);
    assert(m.borrowItem("BK001", "U001").ok);
    assert(!m.borrowItem("BK001", "U002").ok);
    assert(!m.reserveItem("BK001", "U001").ok);
    assert(m.reserveItem("BK001", "U002").ok);
    assert(!m.reserveItem("BK001", "U002").ok);
    assert(!m.returnItem("BK001", "U002").ok);
    assert(m.returnItem("BK001", "U001").ok);
    assert(!m.borrowItem("BK001", "U001").ok);
    assert(m.borrowItem("BK001", "U002").ok);
    assert(m.database().findUser("U002")->borrowingHistory().size() == 1);
    assert(m.database().loans().size() == 1);
    assert(m.returnItem("BK001", "U002").ok);
    assert(m.database().loans().empty());
    assert(m.database().find("BK001")->isAvailable());
    std::cout << "PASS: Phase 3 circulation tests\n";
}
