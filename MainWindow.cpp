#include "MainWindow.hpp"

#include "TextAdventureWidget.hpp"
#include "TriviaWidget.hpp"
#include "DoodlePadWidget.hpp"
#include "SolitaireWidget.h"
#include "MoneyManagerWidget.hpp"
#include <QVector>
#include <QTabWidget>
#include <QCalendarWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QTextCursor>
#include <QTextCharFormat>
#include <QCloseEvent>
#include <QTabBar>
#include <QFont>
#include <QColor>

#include <algorithm>
#include <stdexcept>

static double evalExpr(const QString& expr); // forward

// =============================================================
//  Constructor
// =============================================================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Personal Manager");
    setWindowIcon(QIcon(":/resources/icons/pm_icon.png"));
    resize(880, 640);
    setMaximumWidth(1100);

    // ---- Central Widget + Layout ----
    QWidget *central = new QWidget(this);
    QVBoxLayout *rootLayout = new QVBoxLayout(central);

    // ---- Main Tabs ----
    tabs = new QTabWidget(this);
    rootLayout->addWidget(tabs);

    // // ---------------------------------------------------------
    // // Doodle Pad tab
    // // ---------------------------------------------------------
    // doodlePad = new DoodlePadWidget(this);
    // tabs->addTab(doodlePad, "Doodle Pad");

    // ---------------------------------------------------------
    // Calendar tab
    // ---------------------------------------------------------
    {
        QWidget* page = new QWidget;
        QVBoxLayout* layout = new QVBoxLayout(page);
        calendar = new QCalendarWidget(page);
        layout->addWidget(calendar);
        tabs->addTab(page, "Calendar");
    }

    // ---------------------------------------------------------
    // Calculator tab
    // ---------------------------------------------------------
    {
        QWidget* page = new QWidget;
        QVBoxLayout* layout = new QVBoxLayout(page);

        calcInput = new QLineEdit(page);
        calcInput->setReadOnly(true);
        calcInput->setAlignment(Qt::AlignRight);
        calcInput->setStyleSheet("font-size: 24px; padding: 8px;");
        layout->addWidget(calcInput);

        QPushButton* clearBtn = new QPushButton("C");
        clearBtn->setFixedHeight(40);
        clearBtn->setStyleSheet("font-size: 18px;");
        layout->addWidget(clearBtn);

        connect(clearBtn, &QPushButton::clicked, [this]() {
            calcInput->clear();
        });

        QGridLayout* grid = new QGridLayout;

        QString buttons[5][4] = {
            {"(", ")", "←", ""},
            {"7", "8", "9", "/"},
            {"4", "5", "6", "*"},
            {"1", "2", "3", "-"},
            {"0", ".", "=", "+"}
        };

        for (int r = 0; r < 5; r++) {
            for (int c = 0; c < 4; c++) {
                QString text = buttons[r][c];
                if (text.isEmpty()) continue;

                QPushButton* btn = new QPushButton(text);
                btn->setFixedSize(60, 60);
                btn->setStyleSheet("font-size: 20px;");
                grid->addWidget(btn, r, c);

                if (text == "=") {
                    connect(btn, &QPushButton::clicked, this, &MainWindow::onCalcEvaluate);
                } else if (text == "←") {
                    connect(btn, &QPushButton::clicked, [this]() {
                        QString t = calcInput->text();
                        if (!t.isEmpty())
                            calcInput->setText(t.left(t.length() - 1));
                    });
                } else {
                    connect(btn, &QPushButton::clicked, [this, text]() {
                        calcInput->setText(calcInput->text() + text);
                    });
                }
            }
        }

        layout->addLayout(grid);
        tabs->addTab(page, "Calculator");
    }

// Money Manager Tab
  {
    MoneyManagerWidget *money = new MoneyManagerWidget(this);
    tabs->addTab(money, "Money Manager");
}


    // ---------------------------------------------------------
    // Tasks tab
    // ---------------------------------------------------------
    {
        QWidget* page = new QWidget;
        QVBoxLayout* layout = new QVBoxLayout(page);

        taskInput = new QLineEdit(page);
        taskInput->setPlaceholderText("Enter task");

        taskDate = new QDateEdit(QDate::currentDate(), page);
        taskDate->setCalendarPopup(true);

        QPushButton* addBtn = new QPushButton("Add Task", page);
        QPushButton* rmBtn  = new QPushButton("Remove", page);

        QHBoxLayout* inputRow = new QHBoxLayout;
        inputRow->addWidget(taskInput);
        inputRow->addWidget(taskDate);
        inputRow->addWidget(addBtn);
        inputRow->addWidget(rmBtn);

        taskList = new QListWidget(page);

        layout->addLayout(inputRow);
        layout->addWidget(taskList);

        connect(addBtn, &QPushButton::clicked, this, &MainWindow::onTaskAdd);
        connect(rmBtn,  &QPushButton::clicked, this, &MainWindow::onTaskRemove);

        connect(calendar, &QCalendarWidget::clicked, [this](const QDate& d) {
            taskList->clear();
            for (const TaskItem& t : tasks) {
                if (t.due == d) {
                    QListWidgetItem* li = new QListWidgetItem(
                        t.text + "  [" + t.due.toString("yyyy-MM-dd") + "]"
                    );
                    if (isOverdue(t)) {
                        li->setForeground(Qt::red);
                        li->setFont(QFont("", -1, QFont::Bold));
                    }
                    taskList->addItem(li);
                }
            }
        });

        tabs->addTab(page, "Tasks");
        setMinimumSize(500, 300);

        loadTasks();
    }

    // ---------------------------------------------------------
    // Address Book tab
    // ---------------------------------------------------------
    {
        QWidget *page = new QWidget;
        QVBoxLayout *layout = new QVBoxLayout(page);

        contactName = new QLineEdit(page);
        contactName->setPlaceholderText("Name");

        contactPhone = new QLineEdit(page);
        contactPhone->setPlaceholderText("Phone");

        contactEmail = new QLineEdit(page);
        contactEmail->setPlaceholderText("Email");

        contactAddress = new QLineEdit(page);
        contactAddress->setPlaceholderText("Address");

        addContactBtn = new QPushButton("Add Contact", page);
        QPushButton *rmBtn  = new QPushButton("Remove", page);

        QGridLayout *inputGrid = new QGridLayout;
        inputGrid->addWidget(contactName,    0, 0);
        inputGrid->addWidget(contactPhone,   0, 1);
        inputGrid->addWidget(contactEmail,   1, 0);
        inputGrid->addWidget(contactAddress, 1, 1);
        inputGrid->addWidget(addContactBtn,  2, 0);
        inputGrid->addWidget(rmBtn,          2, 1);

        layout->addLayout(inputGrid);

        contactList = new QListWidget(page);
        layout->addWidget(contactList);

        connect(addContactBtn, &QPushButton::clicked, this, [this]() {
            QString name = contactName->text().trimmed();
            QString phone = contactPhone->text().trimmed();
            QString email = contactEmail->text().trimmed();
            QString addr = contactAddress->text().trimmed();

            if (name.isEmpty()) {
                QMessageBox::warning(this, "Error", "Name is required");
                return;
            }

            if (editingContactIndex == -1) {
                ContactItem c { name, phone, email, addr };
                contacts.append(c);

                QString line = QString("%1 — %2 — %3")
                    .arg(c.name)
                    .arg(c.phone)
                    .arg(c.email);

                contactList->addItem(line);
            } else {
                contacts[editingContactIndex].name = name;
                contacts[editingContactIndex].phone = phone;
                contacts[editingContactIndex].email = email;
                contacts[editingContactIndex].address = addr;

                QString line = QString("%1 — %2 — %3")
                    .arg(name)
                    .arg(phone)
                    .arg(email);

                contactList->item(editingContactIndex)->setText(line);

                editingContactIndex = -1;
                addContactBtn->setText("Add Contact");
            }

            contactName->clear();
            contactPhone->clear();
            contactEmail->clear();
            contactAddress->clear();

            saveContacts();
        });

        connect(rmBtn, &QPushButton::clicked, this, [this]() {
            int row = contactList->currentRow();
            if (row < 0 || row >= contacts.size()) return;

            contacts.removeAt(row);
            delete contactList->takeItem(row);

            if (editingContactIndex == row) {
                editingContactIndex = -1;
                addContactBtn->setText("Add Contact");
                contactName->clear();
                contactPhone->clear();
                contactEmail->clear();
                contactAddress->clear();
            }

            saveContacts();
        });

        connect(contactList, &QListWidget::itemDoubleClicked, this,
                [this](QListWidgetItem *item) {
            int row = contactList->row(item);
            if (row < 0 || row >= contacts.size()) return;

            editingContactIndex = row;

            contactName->setText(contacts[row].name);
            contactPhone->setText(contacts[row].phone);
            contactEmail->setText(contacts[row].email);
            contactAddress->setText(contacts[row].address);

            addContactBtn->setText("Save Changes");
        });

        tabs->addTab(page, "Address Book");

        loadContacts();
    }

    // ---------------------------------------------------------
    // Notepad tab
    // ---------------------------------------------------------
    {
        QWidget* page = new QWidget;
        QVBoxLayout* layout = new QVBoxLayout(page);

        QHBoxLayout *searchRow = new QHBoxLayout;

        searchBox = new QLineEdit();
        searchBox->setPlaceholderText("Search all notes...");

        QPushButton *findNextBtn = new QPushButton("Next");
        QPushButton *findAllBtn = new QPushButton("Find All");

        searchRow->addWidget(searchBox);
        searchRow->addWidget(findNextBtn);
        searchRow->addWidget(findAllBtn);

        layout->addLayout(searchRow);

        noteTabs = new QTabWidget(this);
        noteTabs->setTabsClosable(true);

        auto fixPlusTab = [this]() {
            int last = noteTabs->count() - 1;
            noteTabs->tabBar()->setTabButton(last, QTabBar::RightSide, nullptr);
            noteTabs->tabBar()->setTabButton(last, QTabBar::LeftSide, nullptr);
        };

        loadNotes();

        if (noteTabs->count() == 0) {
            QWidget *firstTab = new QWidget();
            QVBoxLayout *firstLayout = new QVBoxLayout(firstTab);
            QPlainTextEdit *firstEdit = new QPlainTextEdit();
            firstLayout->addWidget(firstEdit);
            firstTab->setLayout(firstLayout);
            noteTabs->addTab(firstTab, "Note 1");
        }

        QWidget *plusTab = new QWidget();
        noteTabs->addTab(plusTab, "+");
        fixPlusTab();

        connect(noteTabs, &QTabWidget::currentChanged, this,
                [this, fixPlusTab](int index) {
            if (noteTabs->tabText(index) == "+") {
                QWidget *newTab = new QWidget();
                QVBoxLayout *layout = new QVBoxLayout(newTab);
                QPlainTextEdit *editor = new QPlainTextEdit();
                layout->addWidget(editor);
                newTab->setLayout(layout);

                int newIndex = noteTabs->insertTab(
                    noteTabs->count() - 1,
                    newTab,
                    QString("Note %1").arg(noteTabs->count())
                );

                noteTabs->setCurrentIndex(newIndex);
                fixPlusTab();
            }
        });

        connect(noteTabs, &QTabWidget::tabCloseRequested, this,
                [this, fixPlusTab](int index) {
            if (noteTabs->tabText(index) == "+")
                return;

            QWidget *tab = noteTabs->widget(index);
            noteTabs->removeTab(index);
            delete tab;

            saveNotes();
            fixPlusTab();
        });

        connect(findNextBtn, &QPushButton::clicked, this, [this]() mutable {
            QString query = searchBox->text();
            if (query.isEmpty()) return;

            int tabCount = noteTabs->count() - 1;

            for (int i = 0; i < tabCount; i++) {
                QWidget *tab = noteTabs->widget(searchTabIndex);
                QPlainTextEdit *editor = tab->findChild<QPlainTextEdit *>();

                QString text = editor->toPlainText();

                int pos = text.indexOf(query, searchPos, Qt::CaseInsensitive);

                if (pos != -1) {
                    noteTabs->setCurrentIndex(searchTabIndex);

                    QTextCursor cursor = editor->textCursor();
                    cursor.setPosition(pos);
                    cursor.movePosition(QTextCursor::Right,
                                        QTextCursor::KeepAnchor,
                                        query.length());
                    editor->setTextCursor(cursor);

                    searchPos = pos + query.length();
                    return;
                }

                searchTabIndex = (searchTabIndex + 1) % tabCount;
                searchPos = 0;
            }
        });

        connect(findAllBtn, &QPushButton::clicked, this, [this]() {
            QString query = searchBox->text();
            if (query.isEmpty()) return;

            QString results;

            for (int i = 0; i < noteTabs->count() - 1; i++) {
                QWidget *tab = noteTabs->widget(i);
                QPlainTextEdit *editor = tab->findChild<QPlainTextEdit *>();

                QString text = editor->toPlainText();
                int count = text.count(query, Qt::CaseInsensitive);

                if (count > 0) {
                    results += QString("%1: %2 matches\n")
                        .arg(noteTabs->tabText(i))
                        .arg(count);
                }
            }

            if (results.isEmpty())
                QMessageBox::information(this, "Search", "No matches found.");
            else
                QMessageBox::information(this, "Search Results", results);
        });

        layout->addWidget(noteTabs);
        tabs->addTab(page, "Notepad");
    }

    // ---------------------------------------------------------
    // Games tab (Trivia, Text Adventure, DoodlePad, Solitaire)
    // ---------------------------------------------------------
    {
        QWidget *gamesPage = new QWidget;
        QVBoxLayout *gamesLayout = new QVBoxLayout(gamesPage);

        gamesTabs = new QTabWidget(gamesPage);
        gamesLayout->addWidget(gamesTabs);
        tabs->addTab(gamesPage, "Games");

        gamesTabs->addTab(new TriviaWidget(this), "Trivia");
        gamesTabs->addTab(new TextAdventureWidget(this), "Text Adventure");
        gamesTabs->addTab(new DoodlePadWidget(this), "Doodle Pad");

        solitaireWidget = new SolitaireWidget(this);
        gamesTabs->addTab(solitaireWidget, "Solitaire");
    }

    // ---------------------------------------------------------
    // History tab
    // ---------------------------------------------------------
    {
        QWidget* page = new QWidget;
        QVBoxLayout* layout = new QVBoxLayout(page);

        history = new QPlainTextEdit(page);
        history->setReadOnly(true);
        history->setStyleSheet("font-family: monospace; font-size: 14px;");

        QPushButton* clearHist = new QPushButton("Clear History", page);
        layout->addWidget(clearHist);

        connect(clearHist, &QPushButton::clicked, [this]() {
            history->clear();
        });

        // layout->addWidget(history);
        // tabs->addTab(page, "History");
    }

    // ---------------------------------------------------------
    // Menu bar (after widgets exist)
    // ---------------------------------------------------------
    QMenuBar *bar = menuBar();

    // File Menu
    {
        QMenu *fileMenu = bar->addMenu("File");
        fileMenu->addAction("Save", doodlePad, &DoodlePadWidget::saveImage);
        fileMenu->addAction("Clear", doodlePad, &DoodlePadWidget::clearCanvas);
    }

    // Edit Menu
    {
        QMenu *editMenu = bar->addMenu("Edit");
        editMenu->addAction("Undo", doodlePad, &DoodlePadWidget::undo);
        editMenu->addAction("Redo", doodlePad, &DoodlePadWidget::redo);
    }

    // Tools Menu
    {
        QMenu *toolsMenu = bar->addMenu("Tools");
        toolsMenu->addAction("Color", doodlePad, &DoodlePadWidget::chooseColor);
        toolsMenu->addAction("Eraser", doodlePad, &DoodlePadWidget::useEraser);
        toolsMenu->addAction("Glow Brush", [this]() { doodlePad->setBrushMode(GlowBrush); });
        toolsMenu->addAction("Spray Brush", [this]() { doodlePad->setBrushMode(SprayBrush); });
        toolsMenu->addAction("Smudge Brush", [this]() { doodlePad->setBrushMode(SmudgeBrush); });
    }

    // Game Menu
    {
        QMenu* gameMenu = bar->addMenu("Game");

        QAction* newGameAction = new QAction("New Game", this);
        QAction* undoAction    = new QAction("Undo", this);
        undoAction->setShortcut(QKeySequence("Ctrl+Z"));

        gameMenu->addAction(newGameAction);
        gameMenu->addAction(undoAction);

        connect(newGameAction, &QAction::triggered,
                solitaireWidget, &SolitaireWidget::newGame);

        connect(undoAction, &QAction::triggered,
                solitaireWidget, &SolitaireWidget::undo);
    }

    setCentralWidget(central);
}

// =============================================================
//  Slots
// =============================================================
void MainWindow::onCalcEvaluate() {
    QString expr = calcInput->text();
    try {
        double r = evalExpr(expr);
        calcInput->setText(QString::number(r));
        addHistory(expr + " = " + QString::number(r));
    } catch (...) {
        calcInput->setText("Error");
        addHistory(expr + " = Error");
    }
}

void MainWindow::onTaskAdd() {
    QString t = taskInput->text().trimmed();
    if (t.isEmpty()) return;

    TaskItem item;
    item.text = t;
    item.due = taskDate->date();

    tasks.append(item);

    QListWidgetItem* li = new QListWidgetItem(
        t + "  [" + item.due.toString("yyyy-MM-dd") + "]"
    );

    if (isOverdue(item)) {
        li->setForeground(Qt::red);
        li->setFont(QFont("", -1, QFont::Bold));
    }

    taskList->addItem(li);

    taskInput->clear();
    saveTasks();
    updateCalendarHighlights();
}

void MainWindow::onTaskRemove() {
    int row = taskList->currentRow();
    if (row >= 0 && row < tasks.size()) {

        tasks.removeAt(row);
        delete taskList->takeItem(row);

        saveTasks();
        updateCalendarHighlights();
    }
}

// =============================================================
//  Tasks JSON
// =============================================================
void MainWindow::loadTasks() {
    tasks.clear();
    if (taskList)
        taskList->clear();

    QFile f("tasks.json");
    if (!f.open(QIODevice::ReadOnly)) return;

    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    QJsonArray arr = doc.array();

    for (auto v : arr) {
        QJsonObject obj = v.toObject();
        TaskItem t;
        t.text = obj["text"].toString();
        t.due = QDate::fromString(obj["due"].toString(), Qt::ISODate);

        tasks.append(t);

        if (taskList) {
            QListWidgetItem* li = new QListWidgetItem(
                t.text + "  [" + t.due.toString("yyyy-MM-dd") + "]"
            );

            if (isOverdue(t)) {
                li->setForeground(Qt::red);
                li->setFont(QFont("", -1, QFont::Bold));
            }

            taskList->addItem(li);
        }
    }

    updateCalendarHighlights();
}

void MainWindow::saveTasks() {
    QJsonArray arr;
    for (const TaskItem& t : tasks) {
        QJsonObject obj;
        obj["text"] = t.text;
        obj["due"] = t.due.toString(Qt::ISODate);
        arr.append(obj);
    }

    QFile f("tasks.json");
    if (f.open(QIODevice::WriteOnly)) {
        f.write(QJsonDocument(arr).toJson());
    }
}

// =============================================================
//  Minimal evaluator
// =============================================================
static double evalExpr(const QString& expr) {
    std::string s = expr.toStdString();
    s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());

    auto prec = [](char op) {
        if (op == '+' || op == '-') return 1;
        if (op == '*' || op == '/') return 2;
        return 0;
    };

    std::vector<double> values;
    std::vector<char> ops;

    auto applyOp = [&](char op) {
        if (values.size() < 2) throw std::runtime_error("bad expr");
        double b = values.back(); values.pop_back();
        double a = values.back(); values.pop_back();
        switch (op) {
            case '+': values.push_back(a + b); break;
            case '-': values.push_back(a - b); break;
            case '*': values.push_back(a * b); break;
            case '/':
                if (b == 0) throw std::runtime_error("divide by zero");
                values.push_back(a / b);
                break;
        }
    };

    for (size_t i = 0; i < s.size(); i++) {
        char c = s[i];

        if (isdigit(c) || c == '.') {
            size_t j = i;
            while (j < s.size() && (isdigit(s[j]) || s[j] == '.')) j++;
            values.push_back(std::stod(s.substr(i, j - i)));
            i = j - 1;
        }
        else if (c == '(') {
            ops.push_back(c);
        }
        else if (c == ')') {
            while (!ops.empty() && ops.back() != '(') {
                applyOp(ops.back());
                ops.pop_back();
            }
            if (ops.empty()) throw std::runtime_error("bad expr");
            ops.pop_back();
        }
        else if (c == '+' || c == '-' || c == '*' || c == '/') {

            if (c == '-' && (i == 0 || s[i-1] == '(')) {
                size_t j = i + 1;
                while (j < s.size() && (isdigit(s[j]) || s[j] == '.')) j++;
                values.push_back(-std::stod(s.substr(i+1, j - (i+1))));
                i = j - 1;
                continue;
            }

            while (!ops.empty() && prec(ops.back()) >= prec(c)) {
                applyOp(ops.back());
                ops.pop_back();
            }

            ops.push_back(c);
        }
        else {
            throw std::runtime_error("bad expr");
        }
    }

    while (!ops.empty()) {
        applyOp(ops.back());
        ops.pop_back();
    }

    if (values.size() != 1) throw std::runtime_error("bad expr");
    return values[0];
}

// =============================================================
//  History
// =============================================================
void MainWindow::addHistory(const QString& line) {
    if (!history) return;
    history->appendPlainText(line);
}

void MainWindow::updateCalendarHighlights() {
    if (!calendar) return;

    QTextCharFormat normal;
    calendar->setWeekdayTextFormat(Qt::Monday, normal);

    // You can add per-date formatting here if desired
}

bool MainWindow::isOverdue(const TaskItem& t) const {
    return t.due < QDate::currentDate();
}

// =============================================================
//  Notepad JSON
// =============================================================
void MainWindow::saveNotes()
{
    if (!noteTabs) return;

    QJsonArray tabsArray;

    for (int i = 0; i < noteTabs->count() - 1; ++i) {
        QWidget *tab = noteTabs->widget(i);
        QPlainTextEdit *editor = tab->findChild<QPlainTextEdit *>();

        if (editor) {
            QJsonObject tabObj;
            tabObj["title"] = noteTabs->tabText(i);
            tabObj["content"] = editor->toPlainText();
            tabsArray.append(tabObj);
        }
    }

    QJsonObject root;
    root["tabs"] = tabsArray;

    QFile file("notes.json");
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(root).toJson());
        file.close();
    }
}

void MainWindow::loadNotes()
{
    if (!noteTabs) return;

    QFile file("notes.json");
    if (!file.open(QIODevice::ReadOnly))
        return;

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonArray tabsArray = doc.object()["tabs"].toArray();

    for (const QJsonValue &val : tabsArray) {
        QJsonObject obj = val.toObject();

        QWidget *tab = new QWidget();
        QVBoxLayout *layout = new QVBoxLayout(tab);
        QPlainTextEdit *editor = new QPlainTextEdit();
        editor->setPlainText(obj["content"].toString());
        layout->addWidget(editor);
        tab->setLayout(layout);

        noteTabs->addTab(tab, obj["title"].toString());
    }
}

// =============================================================
//  Contacts JSON
// =============================================================
void MainWindow::saveContacts() {
    QJsonArray arr;

    for (const ContactItem &c : contacts) {
        QJsonObject obj;
        obj["name"] = c.name;
        obj["phone"] = c.phone;
        obj["email"] = c.email;
        obj["address"] = c.address;
        arr.append(obj);
    }

    QFile f("contacts.json");
    if (f.open(QIODevice::WriteOnly)) {
        f.write(QJsonDocument(arr).toJson());
    }
}

void MainWindow::loadContacts() {
    contacts.clear();
    if (contactList)
        contactList->clear();

    QFile f("contacts.json");
    if (!f.open(QIODevice::ReadOnly)) return;

    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    QJsonArray arr = doc.array();

    for (auto v : arr) {
        QJsonObject obj = v.toObject();

        ContactItem c;
        c.name = obj["name"].toString();
        c.phone = obj["phone"].toString();
        c.email = obj["email"].toString();
        c.address = obj["address"].toString();

        contacts.append(c);

        if (contactList) {
            QString line = QString("%1 — %2 — %3")
                .arg(c.name)
                .arg(c.phone)
                .arg(c.email);

            contactList->addItem(line);
        }
    }
}

// =============================================================
//  Close event
// =============================================================
void MainWindow::closeEvent(QCloseEvent *event)
{
    saveNotes();
    saveTasks();
    saveContacts();
    event->accept();
}
