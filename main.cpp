#include <QApplication>
#include <QAbstractItemView>
#include <QGroupBox>
#include <QFont>
#include <QShortcut>
#include <QKeySequence>
#include "utils/InputValidation.h"
#include <QTimer>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDir>
#include <QTextEdit>
#include <QDate>
#include "utils/FileHandler.h"
#include "utils/LibraryReports.h"
#include <QComboBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QStatusBar>
#include <QTabWidget>
#include <QTableWidget>
#include <QVBoxLayout>
#include <functional>
#include "controller/LibraryManager.h"
#include "controller/SearchEngine.h"

static QTableWidget* makeTable(const QStringList& headings, QWidget* parent) {
    auto* table = new QTableWidget(0, headings.size(), parent);
    table->setHorizontalHeaderLabels(headings);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setAlternatingRowColors(true);
    table->setSortingEnabled(false);
    table->verticalHeader()->setVisible(false);
    return table;
}
static void fillRow(QTableWidget* table, const QStringList& values) {
    int row = table->rowCount();
    table->insertRow(row);
    for (int column = 0; column < values.size(); ++column)
        table->setItem(row, column, new QTableWidgetItem(values[column]));
}
static QString qs(const std::string& value) { return QString::fromStdString(value); }

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    LibraryManager manager;
    QMainWindow window;
    const QString dataFolder = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataFolder);
    const QString dataPath = dataFolder + "/library_data.json";
    QString startupError;
    if (QFile::exists(dataPath) && !FileHandler::load(manager, dataPath, &startupError))
        QMessageBox::warning(nullptr, "Data Load Warning", "Saved data could not be loaded: " + startupError);
    const auto saveCurrent = [&]() {
        QString error;
        if (!FileHandler::save(manager.database(), dataPath, &error))
            QMessageBox::warning(&window, "Save Failed", error);
    };
    window.setWindowTitle("COS 202 - Smart Library Circulation & Automation System");
    window.resize(1100, 720);
    window.setMinimumSize(850, 600);
    app.setStyleSheet("QPushButton { min-height: 28px; padding: 3px 10px; } "
        "QLineEdit, QComboBox, QSpinBox { min-height: 25px; } "
        "QTabWidget::pane { border: 1px solid palette(mid); } "
        "QTableWidget { alternate-background-color: palette(alternate-base); }");
    auto* tabs = new QTabWidget(&window);

    auto* catalogue = new QWidget(tabs);
    auto* catalogueLayout = new QVBoxLayout(catalogue);
    auto* itemsTable = makeTable({"ID", "Type", "Title", "Author", "Year", "Status", "Reserved"}, catalogue);
    catalogueLayout->addWidget(itemsTable);
    tabs->addTab(catalogue, "View Items");

    auto* borrowPage = new QWidget(tabs);
    auto* borrowLayout = new QVBoxLayout(borrowPage);
    auto* circulationForm = new QFormLayout();
    auto* userId = new QLineEdit(borrowPage);
    auto* itemId = new QLineEdit(borrowPage);
    userId->setPlaceholderText("e.g. U001");
    itemId->setPlaceholderText("e.g. BK001");
    circulationForm->addRow("Registered user ID", userId);
    circulationForm->addRow("Library item ID", itemId);
    borrowLayout->addLayout(circulationForm);
    auto* borrowButton = new QPushButton("Borrow Item", borrowPage);
    auto* returnButton = new QPushButton("Return Item", borrowPage);
    auto* reserveButton = new QPushButton("Reserve Item", borrowPage);
    borrowLayout->addWidget(borrowButton);
    borrowLayout->addWidget(returnButton);
    borrowLayout->addWidget(reserveButton);
    borrowLayout->addWidget(new QLabel("Active loans (14-day loan period):", borrowPage));
    auto* loansTable = makeTable({"Item ID", "User ID", "Borrowed", "Due"}, borrowPage);
    borrowLayout->addWidget(loansTable);
    tabs->addTab(borrowPage, "Borrow / Return");

    auto* adminPage = new QWidget(tabs);
    auto* adminLayout = new QVBoxLayout(adminPage);
    auto* itemForm = new QFormLayout();
    auto* type = new QComboBox(adminPage);
    type->addItems({"Book", "Magazine", "Journal"});
    auto* id = new QLineEdit(adminPage);
    auto* title = new QLineEdit(adminPage);
    auto* author = new QLineEdit(adminPage);
    auto* year = new QSpinBox(adminPage);
    year->setRange(1, 9999);
    year->setValue(2026);
    id->setPlaceholderText("Unique ID, e.g. BK004");
    title->setPlaceholderText("Library item title");
    author->setPlaceholderText("Author or publisher");
    itemForm->addRow("Item type", type);
    itemForm->addRow("Unique ID", id);
    itemForm->addRow("Title", title);
    itemForm->addRow("Author", author);
    itemForm->addRow("Publication year", year);
    adminLayout->addLayout(itemForm);
    auto* addButton = new QPushButton("Add Library Item", adminPage);
    adminLayout->addWidget(addButton);
    auto* deleteForm = new QFormLayout();
    auto* deleteId = new QLineEdit(adminPage);
    deleteId->setPlaceholderText("ID of an available, unreserved item");
    deleteForm->addRow("Item ID to delete", deleteId);
    adminLayout->addLayout(deleteForm);
    auto* deleteButton = new QPushButton("Delete Library Item", adminPage);
    auto* undoButton = new QPushButton("Undo Last Admin Item Action", adminPage);
    adminLayout->addWidget(deleteButton);
    adminLayout->addWidget(undoButton);
    adminLayout->addWidget(new QLabel("Register a library user:", adminPage));
    auto* userForm = new QFormLayout();
    auto* registerId = new QLineEdit(adminPage);
    auto* registerName = new QLineEdit(adminPage);
    registerId->setPlaceholderText("e.g. U003");
    registerName->setPlaceholderText("Full name");
    userForm->addRow("Unique user ID", registerId);
    userForm->addRow("Full name", registerName);
    adminLayout->addLayout(userForm);
    auto* registerButton = new QPushButton("Register User", adminPage);
    adminLayout->addWidget(registerButton);
    auto* usersTable = makeTable({"User ID", "Name", "Borrowing History (item IDs)"}, adminPage);
    adminLayout->addWidget(usersTable);
    tabs->addTab(adminPage, "Admin");

    auto* searchPage = new QWidget(tabs);
    auto* searchLayout = new QVBoxLayout(searchPage);
    auto* searchForm = new QFormLayout();
    auto* query = new QLineEdit(searchPage);
    query->setPlaceholderText("Enter a title, author, type, or year");
    auto* field = new QComboBox(searchPage);
    field->addItems({"Title", "Author", "Type", "Year"});
    auto* method = new QComboBox(searchPage);
    method->addItems({"Linear Search (contains)", "Binary Search (starts with)", "Recursive Search (contains)"});
    auto* sortField = new QComboBox(searchPage);
    sortField->addItems({"Title", "Author", "Year"});
    auto* sortMethod = new QComboBox(searchPage);
    sortMethod->addItems({"Insertion Sort", "Merge Sort"});
    searchForm->addRow("Search query", query);
    searchForm->addRow("Search field", field);
    searchForm->addRow("Search algorithm", method);
    searchForm->addRow("Sort field", sortField);
    searchForm->addRow("Sort algorithm", sortMethod);
    searchLayout->addLayout(searchForm);
    auto* searchButton = new QPushButton("Search", searchPage);
    auto* sortButton = new QPushButton("Sort All Items", searchPage);
    auto* showAllButton = new QPushButton("Show All", searchPage);
    searchLayout->addWidget(searchButton);
    searchLayout->addWidget(sortButton);
    searchLayout->addWidget(showAllButton);
    auto* resultsLabel = new QLabel("Search results: 0", searchPage);
    searchLayout->addWidget(resultsLabel);
    auto* resultsTable = makeTable({"ID", "Type", "Title", "Author", "Year", "Status"}, searchPage);
    searchLayout->addWidget(resultsTable);
    tabs->addTab(searchPage, "Search & Sort");

    auto* reportsPage = new QWidget(tabs);
    auto* reportsLayout = new QVBoxLayout(reportsPage);
    auto* reportsText = new QTextEdit(reportsPage);
    reportsText->setReadOnly(true);
    auto* reportsButton = new QPushButton("Refresh Library Reports", reportsPage);
    auto* exportButton = new QPushButton("Export JSON Backup", reportsPage);
    auto* importButton = new QPushButton("Import JSON Backup", reportsPage);
    reportsLayout->addWidget(reportsButton);
    reportsLayout->addWidget(exportButton);
    reportsLayout->addWidget(importButton);
    reportsLayout->addWidget(reportsText);
    tabs->addTab(reportsPage, "Reports & Data");
    const auto updateReports = [&]() {
        reportsText->setPlainText("MOST BORROWED ITEMS\n" + LibraryReports::mostBorrowed(manager.database()) +
            "\n\nUSERS WITH OVERDUE ITEMS\n" + LibraryReports::overdue(manager.database()) +
            "\n\nCATEGORY DISTRIBUTION\n" + LibraryReports::categoryDistribution(manager.database()));
    };
    const auto refresh = [&]() {
        itemsTable->setRowCount(0);
        for (const auto& ptr : manager.database().items()) {
            const auto& item = *ptr;
            const auto* queue = manager.database().waitlistIfPresent(item.id());
            const int reserved = queue ? static_cast<int>(queue->size()) : 0;
            fillRow(itemsTable, {qs(item.id()), qs(item.type()), qs(item.title()), qs(item.author()),
                QString::number(item.year()), item.isAvailable() ? "Available" : "Borrowed",
                QString::number(reserved)});
        }
        loansTable->setRowCount(0);
        for (const auto& loan : manager.database().loans())
            fillRow(loansTable, {qs(loan.itemId), qs(loan.userId), qs(loan.borrowedOn), qs(loan.dueOn)});
        usersTable->setRowCount(0);
        for (const auto& user : manager.database().users()) {
            QStringList history;
            for (const auto& borrowedId : user.borrowingHistory()) history << qs(borrowedId);
            fillRow(usersTable, {qs(user.id()), qs(user.name()), history.join(", ")});
        }
        updateReports();
        window.statusBar()->showMessage(QString("Catalogue: %1 items | Users: %2 | Active loans: %3")
            .arg(manager.database().count()).arg(manager.database().users().size()).arg(manager.database().loans().size()) + QString(" | Undo actions: %1").arg(manager.undoCount()));
    };
    QObject::connect(addButton, &QPushButton::clicked, &window, [&]() {
        const auto validation = InputValidation::item(id->text().toStdString(),
            title->text().toStdString(), author->text().toStdString(), year->value());
        if (!validation.empty()) { QMessageBox::warning(&window, "Invalid Item", qs(validation)); return; }
        if (!manager.addItem(type->currentText().toStdString(), id->text().trimmed().toStdString(),
                title->text().trimmed().toStdString(), author->text().trimmed().toStdString(), year->value())) {
            QMessageBox::warning(&window, "Invalid Item", "Enter a unique ID, title and author. Duplicate IDs are not allowed.");
            return;
        }
        id->clear(); title->clear(); author->clear();
        refresh(); saveCurrent(); tabs->setCurrentWidget(catalogue);
    });
    QObject::connect(deleteButton, &QPushButton::clicked, &window, [&]() {
        const auto value = deleteId->text().trimmed().toStdString();
        if (value.empty()) { QMessageBox::warning(&window, "Delete Item", "Enter an item ID."); return; }
        if (QMessageBox::question(&window, "Confirm Deletion", "Delete item " + qs(value) + "?") != QMessageBox::Yes) return;
        const auto result = manager.deleteItem(value);
        if (result.ok) { deleteId->clear(); QMessageBox::information(&window, "Delete Item", qs(result.message)); }
        else QMessageBox::warning(&window, "Delete Item", qs(result.message));
        refresh(); if (result.ok) saveCurrent();
    });
    QObject::connect(undoButton, &QPushButton::clicked, &window, [&]() {
        const auto result = manager.undoLastAdminAction();
        if (result.ok) QMessageBox::information(&window, "Undo", qs(result.message));
        else QMessageBox::warning(&window, "Undo", qs(result.message));
        refresh(); if (result.ok) saveCurrent();
    });
    QObject::connect(registerButton, &QPushButton::clicked, &window, [&]() {
        const auto validation = InputValidation::user(registerId->text().toStdString(), registerName->text().toStdString());
        if (!validation.empty()) { QMessageBox::warning(&window, "Invalid User", qs(validation)); return; }
        if (!manager.registerUser(registerId->text().trimmed().toStdString(), registerName->text().trimmed().toStdString())) {
            QMessageBox::warning(&window, "Invalid User", "Enter a unique user ID and non-empty name.");
            return;
        }
        registerId->clear(); registerName->clear(); refresh(); saveCurrent();
        QMessageBox::information(&window, "User Registered", "Library user registered successfully.");
    });
    const auto action = [&](const std::function<OperationResult(const std::string&, const std::string&)>& operation) {
        if (!InputValidation::validId(itemId->text().toStdString()) ||
            !InputValidation::validId(userId->text().toStdString())) {
            QMessageBox::warning(&window, "Invalid IDs", "Enter valid item and user IDs (letters, numbers, underscore or hyphen).");
            return;
        }
        const auto result = operation(itemId->text().trimmed().toStdString(), userId->text().trimmed().toStdString());
        if (result.ok) QMessageBox::information(&window, "Circulation", qs(result.message));
        else QMessageBox::warning(&window, "Circulation Error", qs(result.message));
        refresh(); if (result.ok) saveCurrent();
    };
    QObject::connect(borrowButton, &QPushButton::clicked, &window, [&]() {
        action([&](const std::string& item, const std::string& user) { return manager.borrowItem(item, user); });
    });
    QObject::connect(returnButton, &QPushButton::clicked, &window, [&]() {
        action([&](const std::string& item, const std::string& user) { return manager.returnItem(item, user); });
    });
    QObject::connect(reserveButton, &QPushButton::clicked, &window, [&]() {
        action([&](const std::string& item, const std::string& user) { return manager.reserveItem(item, user); });
    });
    const auto fieldFromIndex = [](int index) {
        switch (index) {
            case 1: return SearchEngine::Field::Author;
            case 2: return SearchEngine::Field::Type;
            case 3: return SearchEngine::Field::Year;
            default: return SearchEngine::Field::Title;
        }
    };
    const auto showResults = [&](const SearchEngine::Results& results) {
        resultsTable->setRowCount(0);
        for (const auto* item : results)
            fillRow(resultsTable, {qs(item->id()), qs(item->type()), qs(item->title()), qs(item->author()),
                QString::number(item->year()), item->isAvailable() ? "Available" : "Borrowed"});
        resultsLabel->setText(QString("Search results: %1").arg(results.size()));
    };
    QObject::connect(searchButton, &QPushButton::clicked, &window, [&]() {
        const auto algorithm = static_cast<SearchEngine::Algorithm>(method->currentIndex());
        const auto result = SearchEngine::search(SearchEngine::catalogue(manager.database()),
            query->text().trimmed().toStdString(), fieldFromIndex(field->currentIndex()), algorithm);
        showResults(result);
    });
    QObject::connect(sortButton, &QPushButton::clicked, &window, [&]() {
        auto items = SearchEngine::catalogue(manager.database());
        SearchEngine::sort(items, fieldFromIndex(sortField->currentIndex()),
            static_cast<SearchEngine::SortAlgorithm>(sortMethod->currentIndex()));
        showResults(items);
    });
    QObject::connect(showAllButton, &QPushButton::clicked, &window, [&]() {
        query->clear();
        showResults(SearchEngine::catalogue(manager.database()));
    });
    QObject::connect(tabs, &QTabWidget::currentChanged, &window, [&](int index) {
        if (tabs->widget(index) == searchPage) showResults(SearchEngine::catalogue(manager.database()));
    });
    QObject::connect(reportsButton, &QPushButton::clicked, &window, updateReports);
    QObject::connect(query, &QLineEdit::returnPressed, searchButton, &QPushButton::click);
    QObject::connect(itemId, &QLineEdit::returnPressed, borrowButton, &QPushButton::click);
    QObject::connect(registerName, &QLineEdit::returnPressed, registerButton, &QPushButton::click);
    QObject::connect(title, &QLineEdit::returnPressed, addButton, &QPushButton::click);
    QObject::connect(exportButton, &QPushButton::clicked, &window, [&]() {
        const auto path = QFileDialog::getSaveFileName(&window, "Export Library JSON", "library_backup.json", "JSON (*.json)");
        if (path.isEmpty()) return;
        QString error;
        if (!FileHandler::save(manager.database(), path, &error)) QMessageBox::warning(&window, "Export Failed", error);
        else QMessageBox::information(&window, "Export", "Backup exported successfully.");
    });
    QObject::connect(importButton, &QPushButton::clicked, &window, [&]() {
        const auto path = QFileDialog::getOpenFileName(&window, "Import Library JSON", QString(), "JSON (*.json)");
        if (path.isEmpty()) return;
        if (QMessageBox::question(&window, "Replace Data", "Import will replace the current catalogue, users and loans. Continue?") != QMessageBox::Yes) return;
        QString error;
        if (!FileHandler::load(manager, path, &error)) QMessageBox::warning(&window, "Import Failed", error);
        else { refresh(); saveCurrent(); QMessageBox::information(&window, "Import", "Data imported successfully."); }
    });
    QTimer reminderTimer;
    QObject::connect(&reminderTimer, &QTimer::timeout, &window, [&]() {
        const int count = LibraryReports::overdueCount(manager.database());
        if (count > 0) window.statusBar()->showMessage(QString("OVERDUE REMINDER: %1 active overdue loan(s). See Reports & Data.").arg(count), 30000);
    });
    reminderTimer.start(60000);
    window.setCentralWidget(tabs);
    refresh();
    window.show();
    return app.exec();
}
