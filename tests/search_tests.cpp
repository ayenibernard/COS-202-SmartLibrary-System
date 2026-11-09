#include "controller/LibraryManager.h"
#include "controller/SearchEngine.h"
#include <cassert>
#include <iostream>
int main() {
    LibraryManager m;
    assert(m.addItem("Book", "B3", "Zebra", "Smith", 2020));
    assert(m.addItem("Journal", "J1", "Algorithms", "Ada", 2025));
    assert(m.addItem("Magazine", "M1", "Alpha", "Adams", 2018));
    assert(m.addItem("Book", "B4", "Alpine", "Ada", 2022));
    auto items = SearchEngine::catalogue(m.database());
    using F = SearchEngine::Field;
    using A = SearchEngine::Algorithm;
    using S = SearchEngine::SortAlgorithm;
    assert(SearchEngine::search(items, "alp", F::Title, A::Linear).size() == 2);
    assert(SearchEngine::search(items, "alp", F::Title, A::Recursive).size() == 2);
    assert(SearchEngine::search(items, "alp", F::Title, A::Binary).size() == 2);
    assert(SearchEngine::search(items, "ada", F::Author, A::Binary).size() == 3);
    assert(SearchEngine::search(items, "ithm", F::Title, A::Linear).size() == 1);
    assert(SearchEngine::search(items, "ithm", F::Title, A::Binary).empty()); // binary uses prefix matching
    assert(SearchEngine::search(items, "book", F::Type, A::Binary).size() == 2); // linear fallback
    assert(SearchEngine::search(items, "missing", F::Title, A::Recursive).empty());
    auto a = items, b = items;
    SearchEngine::sort(a, F::Title, S::Insertion);
    SearchEngine::sort(b, F::Title, S::Merge);
    for (std::size_t i = 0; i < a.size(); ++i) assert(a[i]->id() == b[i]->id());
    assert(a.front()->title() == "Algorithms");
    SearchEngine::sort(a, F::Year, S::Merge);
    assert(a.front()->year() == 2018 && a.back()->year() == 2025);
    SearchEngine::Results empty;
    SearchEngine::sort(empty, F::Title, S::Merge);
    assert(SearchEngine::search(empty, "a", F::Title, A::Binary).empty());
    std::cout << "PASS: search, sort, recursion, empty catalogue\n";
}
