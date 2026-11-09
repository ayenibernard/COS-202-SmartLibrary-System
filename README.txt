# COS 202 — Smart Library Circulation & Automation System

## Project Overview

The Smart Library Circulation & Automation System is a desktop application developed in C++17 using the Qt6 Widgets framework.

The application helps manage library items, registered users, borrowing, returning, reservations, searching, sorting, and library reports.

**Course:** COS 202  
**Institution:** Miva Open University  
**Student:** Ayeni Oluwamuyiwa Bernard  
**Department:** Software Engineering  
**Matriculation Number:** 2025/A/SENG/0250

## Main Features

- Management of books, magazines, and journals
- Library user registration
- Borrowing and returning of library items
- Reservation management using FIFO queues
- Searching using linear, binary, and recursive search
- Sorting using insertion sort and merge sort
- Administrative item management with undo functionality
- JSON data persistence, import, and export
- Reports on borrowed items, overdue users, and library categories
- Overdue reminders
- Frequently accessed items cache using a fixed-size array

## Technologies Used

- C++17
- Qt6 Widgets
- CMake
- Ninja
- MSYS2 UCRT64
- JSON-based data storage

## Project Structure

- `controller/` — Application logic and search algorithms
- `model/` — Library data models and cache
- `utils/` — Validation, persistence, and reporting
- `tests/` — Automated C++ test suites
- `Screenshots/` — Images of the running application
- `main.cpp` — Graphical application entry point
- `CMakeLists.txt` — Build configuration

## Build Instructions

Open MSYS2 UCRT64 with Qt6, CMake, Ninja, and GCC installed.

From the project directory, run:

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=/ucrt64/bin/g++.exe
cmake --build build
./build/SmartLibrary.exe
```

## Automated Testing

The project includes six automated test suites:

1. CirculationTests
2. SearchTests
3. AdminTests
4. PersistenceTests
5. ValidationTests
6. CacheTests

Run the tests using:

```bash
ctest --test-dir build --output-on-failure
```

**Final verification — November 9, 2026**

- Tests passed: 6/6
- Success rate: 100%
- Total execution time: 1.80 seconds

## Application Screenshots

Five screenshots are available in the `Screenshots` folder, demonstrating the catalogue, circulation management, administration, searching and sorting, and reporting features.

## Conclusion

The project demonstrates practical application of object-oriented programming, data structures, searching and sorting algorithms, file handling, input validation, and graphical user interface development using C++.
