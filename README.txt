COS 202 SMART LIBRARY - PHASE 7

IMPORTANT: Back up your current project directory and your ORIGINAL unmodified JSON
working database before installing. Restore the original database after the overdue test.

REPLACEMENT:
 main.cpp
 CMakeLists.txt

NEW:
 utils/InputValidation.h
 tests/validation_tests.cpp

UNCHANGED: Other Phase 6 sources, including the corrected utils/FileHandler.h.

This package contains a full project tree for convenient installation. Its
FileHandler.h includes the Phase 6 compiler correction.

In MSYS2 UCRT64:
 cd "/c/Users/USER/Documents/COS 202 LAB"
 cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=/ucrt64/bin/g++.exe
 cmake --build build
 ctest --test-dir build --output-on-failure
 ./build/SmartLibrary.exe

Expected test suites: CirculationTests, SearchTests, AdminTests,
PersistenceTests, ValidationTests (5 total).

Manual acceptance: previous data survives restart; all tabs work; search Enter
key works; blank/invalid item and user IDs are rejected; valid records still
work; borrowing, returning, reservations, undo, JSON export/import and reports
still work; overdue reminders still work.
