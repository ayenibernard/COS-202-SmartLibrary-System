#include "utils/InputValidation.h"
#include "controller/LibraryManager.h"
#include <cassert>
#include <iostream>
int main() {
    assert(InputValidation::validId("BK001"));
    assert(InputValidation::validId("USER-1_A"));
    assert(!InputValidation::validId(""));
    assert(!InputValidation::validId("A B"));
    assert(!InputValidation::validId("A/B"));
    assert(!InputValidation::validId(std::string(33, 'A')));
    assert(InputValidation::meaningfulText("Introduction to C++"));
    assert(!InputValidation::meaningfulText("   "));
    assert(!InputValidation::meaningfulText("Hello\nWorld"));
    assert(InputValidation::item("BK001", "C++", "Ada", 2026).empty());
    assert(!InputValidation::item("BAD ID", "C++", "Ada", 2026).empty());
    assert(!InputValidation::item("BK001", "", "Ada", 2026).empty());
    assert(!InputValidation::item("BK001", "C++", "Ada", 0).empty());
    assert(InputValidation::user("U001", "Ada").empty());
    assert(!InputValidation::user("U001", "  ").empty());
    LibraryManager manager;
    assert(manager.addItem("Book", "BK001", "C++", "Ada", 2026));
    assert(manager.registerUser("U001", "Ada"));
    assert(manager.borrowItem("BK001", "U001").ok);
    assert(!manager.borrowItem("BK001", "U001").ok);
    assert(manager.returnItem("BK001", "U001").ok);
    assert(manager.deleteItem("BK001").ok);
    assert(manager.undoLastAdminAction().ok);
    std::cout << "Phase 7 validation and integration regression passed\n";
}
