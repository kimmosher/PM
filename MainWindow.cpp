#include "MainWindow.hpp"

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
#include <stdexcept>

static double evalExpr(const QString& expr); // simple stub

// =============================================================
//  Constructor
// =============================================================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    tabs = new QTabWidget(this);
    setCentralWidget(tabs);
    setWindowTitle("Personal Manager");
    setWindowIcon(QIcon(":/icons/pm_icon.png"));

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

        QString buttons[6][4] = {
            {"(", ")", "←", ""},
            {"MC", "MR", "M+", "M-"},
            {"7", "8", "9", "/"},
            {"4", "5", "6", "*"},
            {"1", "2", "3", "-"},
            {"0", ".", "=", "+"}
        };

        for (int r = 0; r < 6; r++) {
            for (int c = 0; c < 4; c++) {
                QString text = buttons[r][c];
                if (text.isEmpty()) continue;

                QPushButton* btn = new QPushButton(text);
                btn->setFixedSize(60, 60);
                btn->setStyleSheet("font-size: 20px;");
                grid->addWidget(btn, r, c);

                if (text == "=") {
                    connect(btn, &QPushButton::clicked, this, &MainWindow::onCalcEvaluate);
                }
                else if (text == "←") {
                    connect(btn, &QPushButton::clicked, [this]() {
                        QString t = calcInput->text();
                        if (!t.isEmpty())
                            calcInput->setText(t.left(t.length() - 1));
                    });
                }
                else if (text == "MC") {
                    connect(btn, &QPushButton::clicked, [this]() {
                        this->memoryValue = 0.0;
                        addHistory("MC (memory cleared)");
                    });
                }
                else if (text == "MR") {
                    connect(btn, &QPushButton::clicked, [this]() {
                        addHistory("MR → " + QString::number(this->memoryValue));
                        calcInput->setText(calcInput->text() + QString::number(this->memoryValue));
                    });
                }
                else if (text == "M+") {
                    connect(btn, &QPushButton::clicked, [this]() {
                        try {
                            double v = evalExpr(calcInput->text());
                            this->memoryValue += v;
                            addHistory("M+ (added " + QString::number(v) + ")");
                        } catch (...) {
                            calcInput->setText("Error");
                        }
                    });
                }
                else if (text == "M-") {
                    connect(btn, &QPushButton::clicked, [this]() {
                        try {
                            double v = evalExpr(calcInput->text());
                            this->memoryValue -= v;
                            addHistory("M- (subtracted " + QString::number(v) + ")");
                        } catch (...) {
                            calcInput->setText("Error");
                        }
                    });
                }
                else {
                    connect(btn, &QPushButton::clicked, [this, text]() {
                        calcInput->setText(calcInput->text() + text);
                    });
                }
            }
        }

        layout->addLayout(grid);
        tabs->addTab(page, "Calculator");
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

    // ---- Notepad tab ----
{
    QWidget* page = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout(page);

    // --- Search bar ---
    QHBoxLayout *searchRow = new QHBoxLayout;

    searchBox = new QLineEdit();
    searchBox->setPlaceholderText("Search all notes...");

    QPushButton *findNextBtn = new QPushButton("Next");
    QPushButton *findAllBtn = new QPushButton("Find All");

    searchRow->addWidget(searchBox);
    searchRow->addWidget(findNextBtn);
    searchRow->addWidget(findAllBtn);

    layout->addLayout(searchRow);

    // --- Tab widget ---
    noteTabs = new QTabWidget(this);
    noteTabs->setTabsClosable(true);

    // Helper to keep "+" tab buttonless
    auto fixPlusTab = [=]() {
        int last = noteTabs->count() - 1;
        noteTabs->tabBar()->setTabButton(last, QTabBar::RightSide, nullptr);
        noteTabs->tabBar()->setTabButton(last, QTabBar::LeftSide, nullptr);
    };

    // Load saved tabs
    loadNotes();

    // If no tabs exist, create the first one
    if (noteTabs->count() == 0) {
        QWidget *firstTab = new QWidget();
        QVBoxLayout *firstLayout = new QVBoxLayout(firstTab);
        QPlainTextEdit *firstEdit = new QPlainTextEdit();
        firstLayout->addWidget(firstEdit);
        firstTab->setLayout(firstLayout);
        noteTabs->addTab(firstTab, "Note 1");
    }

    // "+" tab
    QWidget *plusTab = new QWidget();
    noteTabs->addTab(plusTab, "+");
    fixPlusTab();

    // Handle "+" tab creation
    connect(noteTabs, &QTabWidget::currentChanged, this, [=](int index) {
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

    // Handle closing tabs
    connect(noteTabs, &QTabWidget::tabCloseRequested, this, [=](int index) {
        if (noteTabs->tabText(index) == "+")
            return;

        QWidget *tab = noteTabs->widget(index);
        noteTabs->removeTab(index);
        delete tab;

        saveNotes();
        fixPlusTab();
    });

    // --- Find Next ---
    connect(findNextBtn, &QPushButton::clicked, this, [=]() mutable {
        QString query = searchBox->text();
        if (query.isEmpty()) return;

        int tabCount = noteTabs->count() - 1; // ignore "+"

        for (int i = 0; i < tabCount; i++) {
            QWidget *tab = noteTabs->widget(searchTabIndex);
            QPlainTextEdit *editor = tab->findChild<QPlainTextEdit *>();

            QString text = editor->toPlainText();

            int pos = text.indexOf(query, searchPos, Qt::CaseInsensitive);

            if (pos != -1) {
                noteTabs->setCurrentIndex(searchTabIndex);

                QTextCursor cursor = editor->textCursor();
                cursor.setPosition(pos);
                cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, query.length());
                editor->setTextCursor(cursor);

                searchPos = pos + query.length();
                return;
            }

            searchTabIndex = (searchTabIndex + 1) % tabCount;
            searchPos = 0;
        }
    });

    // --- Find All ---
    connect(findAllBtn, &QPushButton::clicked, this, [=]() {
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

        layout->addWidget(history);
        tabs->addTab(page, "History");
    }
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

        QListWidgetItem* li = new QListWidgetItem(
            t.text + "  [" + t.due.toString("yyyy-MM-dd") + "]"
        );

        if (isOverdue(t)) {
            li->setForeground(Qt::red);
            li->setFont(QFont("", -1, QFont::Bold));
        }

        taskList->addItem(li);
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
    s.erase(remove_if(s.begin(), s.end(), ::isspace), s.end());

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
    QTextCharFormat normal;
    calendar->setWeekdayTextFormat(Qt::Monday, normal);

    QTextCharFormat highlight;
    highlight.setBackground(QColor("#88c0ff"));
    highlight.setForeground(Qt::black);

    QTextCharFormat overdueFmt;
    overdueFmt.setBackground(QColor("#ffcccc"));
    overdueFmt.setForeground(Qt::red);
    overdueFmt.setFontWeight(QFont::Bold);

    for (const TaskItem& t : tasks) {
        // You can add highlight logic here if desired
    }
}

bool MainWindow::isOverdue(const TaskItem& t) const {
    return t.due < QDate::currentDate();
}

// =============================================================
//  Notepad JSON
// =============================================================
void MainWindow::saveNotes()
{
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
//  Close event
// =============================================================
void MainWindow::closeEvent(QCloseEvent *event)
{
    saveNotes();
    event->accept();
}
