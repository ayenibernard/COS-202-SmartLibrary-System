#include "model/LibraryDatabase.h"
#include <cassert>
#include <memory>
#include <string>
#include <iostream>
int main() {
    LibraryDatabase db;
    for (int i=1; i<=7; ++i) {
        const auto id = "BK" + std::to_string(i);
        assert(db.add(std::make_unique<Book>(id, "Title", "Author", 2026)));
    }
    for (int i=1; i<=7; ++i) {
        const auto id = "BK" + std::to_string(i);
        for (int j=0; j<i; ++j) assert(db.find(id));
    }
    const auto& cache=db.frequentItems();
    assert(cache.size()==5);
    assert(cache.at(0)=="BK7" && cache.at(4)=="BK3");
    assert(cache.accessCount("BK7")==7);
    assert(db.find("MISSING")==nullptr);
    assert(cache.size()==5);
    assert(db.removeItem("BK7"));
    assert(cache.accessCount("BK7")==0);
    assert(cache.at(0)=="BK6");
    db.clear();
    assert(cache.size()==0);
    std::cout << "Frequent item fixed-array cache tests passed\n";
}
