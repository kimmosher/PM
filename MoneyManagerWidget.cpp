#include "MoneyManagerWidget.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QDateEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QMessageBox>
#include <QLabel>
#include <QBarSeries>
#include <QBarSet>
#include <QChart>
#include <QBarCategoryAxis>
#include <QValueAxis>
#include <QPainter>
#include <QChartView>
#include <QHBoxLayout>


MoneyManagerWidget::MoneyManagerWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    // --- Input row ---
    descInput = new QLineEdit(this);
    descInput->setPlaceholderText("Description");

    amountInput = new QDoubleSpinBox(this);
    amountInput->setPrefix("$");
    amountInput->setMaximum(1000000);
    amountInput->setDecimals(2);

    dateInput = new QDateEdit(QDate::currentDate(), this);
    dateInput->setCalendarPopup(true);

    categoryInput = new QComboBox(this);
    categoryInput->addItems({"Food", "Bills", "Transport", "Entertainment", "Shopping", "Medical", "Other"});

    incomeCheck = new QCheckBox("Income", this);   // NEW

    QPushButton *addBtn = new QPushButton("Add", this);
    QPushButton *rmBtn  = new QPushButton("Remove", this);
    QPushButton *summaryBtn = new QPushButton("Summary", this);  // NEW

    QHBoxLayout *row = new QHBoxLayout;
    row->addWidget(descInput);
    row->addWidget(amountInput);
    row->addWidget(dateInput);
    row->addWidget(categoryInput);
    row->addWidget(incomeCheck);   // NEW
    row->addWidget(addBtn);
    row->addWidget(rmBtn);
    row->addWidget(summaryBtn);    // NEW

    layout->addLayout(row);

    searchInput = new QLineEdit(this);
    searchInput->setPlaceholderText("Search...");
    layout->addWidget(searchInput);

    connect(searchInput, &QLineEdit::textChanged,
        this, &MoneyManagerWidget::filterTable);
    // --- Table ---
    table = new QTableWidget(this);
    table->setColumnCount(4);
    table->setHorizontalHeaderLabels({"Date", "Category", "Amount" , "Description"});
    table->horizontalHeader()->setStretchLastSection(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);

    layout->addWidget(table);

    // --- Running totals ---
    totalBalanceLabel = new QLabel("Total Balance: $0.00", this);
    monthlyBalanceLabel = new QLabel("Monthly Balance: $0.00", this);

    layout->addWidget(totalBalanceLabel);
    layout->addWidget(monthlyBalanceLabel);

    QPushButton *chartBtn = new QPushButton("Monthly Chart");
    row->addWidget(chartBtn);

    connect(chartBtn, &QPushButton::clicked, this, &MoneyManagerWidget::showMonthlyChart);

    connect(addBtn, &QPushButton::clicked, this, &MoneyManagerWidget::addExpense);
    connect(rmBtn,  &QPushButton::clicked, this, &MoneyManagerWidget::removeExpense);
    connect(summaryBtn, &QPushButton::clicked, this, &MoneyManagerWidget::showSummary);

    loadExpenses();
    updateRunningTotals();
}

void MoneyManagerWidget::addExpense()
{
    ExpenseItem e;
    e.description = descInput->text().trimmed();
    e.amount = amountInput->value();
    e.date = dateInput->date();
    e.category = categoryInput->currentText();
    e.isIncome = incomeCheck->isChecked();

    if (e.description.isEmpty() || e.amount <= 0.0)
        return;

    expenses.append(e);

    int row = table->rowCount();
    table->insertRow(row);

    // 0: Date
    table->setItem(row, 0, new QTableWidgetItem(e.date.toString("yyyy-MM-dd")));

    // 1: Category
    table->setItem(row, 1, new QTableWidgetItem(e.category));

    // 2: Amount
    QString amountText = QString("$%1").arg(e.amount, 0, 'f', 2);
    if (e.isIncome)
        amountText = "+" + amountText;
    else
        amountText = "-" + amountText;

    table->setItem(row, 2, new QTableWidgetItem(amountText));

    // 3: Description
    table->setItem(row, 3, new QTableWidgetItem(e.description));

    descInput->clear();
    amountInput->setValue(0.0);
    incomeCheck->setChecked(false);

    saveExpenses();
    updateRunningTotals();
}


void MoneyManagerWidget::removeExpense()
{
    int row = table->currentRow();
    if (row < 0 || row >= expenses.size())
        return;

    expenses.removeAt(row);
    table->removeRow(row);

    saveExpenses();
    updateRunningTotals();
}

void MoneyManagerWidget::saveExpenses()
{
    QJsonArray arr;

    for (const ExpenseItem &e : expenses) {
        QJsonObject obj;
        obj["description"] = e.description;
        obj["amount"] = e.amount;
        obj["date"] = e.date.toString(Qt::ISODate);
        obj["category"] = e.category;
        obj["isIncome"] = e.isIncome;   // NEW
        arr.append(obj);
    }

    QFile f("expenses.json");
    if (f.open(QIODevice::WriteOnly)) {
        f.write(QJsonDocument(arr).toJson());
        f.close();
    }
}

void MoneyManagerWidget::loadExpenses()
{
    expenses.clear();
    table->setRowCount(0);

    QFile f("expenses.json");
    if (!f.open(QIODevice::ReadOnly))
        return;

    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    f.close();

    QJsonArray arr = doc.array();

    for (auto v : arr) {
        QJsonObject obj = v.toObject();

        ExpenseItem e;
        e.description = obj["description"].toString();
        e.amount = obj["amount"].toDouble();
        e.date = QDate::fromString(obj["date"].toString(), Qt::ISODate);
        e.category = obj["category"].toString();
        e.isIncome = obj["isIncome"].toBool();   // NEW

        expenses.append(e);

        int row = table->rowCount();
        table->insertRow(row);

        table->setItem(row, 0, new QTableWidgetItem(e.date.toString("yyyy-MM-dd")));
        
        table->setItem(row, 1, new QTableWidgetItem(e.category));
        table->setSortingEnabled(true);
        QString amountText = QString("$%1").arg(e.amount, 0, 'f', 2);
        if (e.isIncome)
            amountText = "+" + amountText;
        else
            amountText = "-" + amountText;

        table->setItem(row, 2, new QTableWidgetItem(amountText));
        table->setItem(row, 3, new QTableWidgetItem(e.description));
        colorRow(row, e.isIncome);
    }
}

void MoneyManagerWidget::colorRow(int row, bool isIncome)
{
    QColor bg = isIncome ? QColor(200, 255, 200)   // light green
                         : QColor(255, 220, 220);  // light red

    for (int col = 0; col < table->columnCount(); ++col) {
        QTableWidgetItem *item = table->item(row, col);
        if (item)
            item->setBackground(bg);
    }
}

void MoneyManagerWidget::updateRunningTotals()
{
    double totalIncome = 0, totalExpenses = 0;
    double monthIncome = 0, monthExpenses = 0;

    QDate today = QDate::currentDate();
    QDate firstOfMonth(today.year(), today.month(), 1);

    for (const ExpenseItem &e : expenses) {
        if (e.isIncome) {
            totalIncome += e.amount;
            if (e.date >= firstOfMonth)
                monthIncome += e.amount;
        } else {
            totalExpenses += e.amount;
            if (e.date >= firstOfMonth)
                monthExpenses += e.amount;
        }
    }

    double totalBalance = totalIncome - totalExpenses;
    double monthlyBalance = monthIncome - monthExpenses;

    totalBalanceLabel->setText(QString("Total Balance: $%1").arg(totalBalance, 0, 'f', 2));
    monthlyBalanceLabel->setText(QString("Monthly Balance: $%1").arg(monthlyBalance, 0, 'f', 2));
}

void MoneyManagerWidget::showSummary()
{
    double totalIncome = 0;
    double totalExpenses = 0;

    double monthIncome = 0;
    double monthExpenses = 0;

    QMap<QString, double> categoryTotals;

    QDate today = QDate::currentDate();
    QDate firstOfMonth(today.year(), today.month(), 1);

    for (const ExpenseItem &e : expenses) {

        if (e.isIncome) {
            totalIncome += e.amount;
            if (e.date >= firstOfMonth)
                monthIncome += e.amount;
        } else {
            totalExpenses += e.amount;
            if (e.date >= firstOfMonth)
                monthExpenses += e.amount;

            categoryTotals[e.category] += e.amount;
        }
    }

    double totalBalance = totalIncome - totalExpenses;
    double monthlyBalance = monthIncome - monthExpenses;

    QString summary;
    summary += "📊 <b>Financial Summary</b><br><br>";

    summary += QString("<b>Total Income:</b> $%1<br>").arg(totalIncome, 0, 'f', 2);
    summary += QString("<b>Total Expenses:</b> $%1<br>").arg(totalExpenses, 0, 'f', 2);
    summary += QString("<b>Total Balance:</b> $%1<br><br>").arg(totalBalance, 0, 'f', 2);

    summary += QString("<b>Monthly Income:</b> $%1<br>").arg(monthIncome, 0, 'f', 2);
    summary += QString("<b>Monthly Expenses:</b> $%1<br>").arg(monthExpenses, 0, 'f', 2);
    summary += QString("<b>Monthly Balance:</b> $%1<br><br>").arg(monthlyBalance, 0, 'f', 2);

    summary += "<b>Category Totals (Expenses Only):</b><br>";
    for (auto it = categoryTotals.begin(); it != categoryTotals.end(); ++it) {
        summary += QString("%1: $%2<br>").arg(it.key()).arg(it.value(), 0, 'f', 2);
    }

    
    for (const ExpenseItem &e : expenses) {
        QString key = e.date.toString("yyyy-MM");       
    }

    QMessageBox box;
    box.setWindowTitle("Summary");
    box.setText(summary);
    box.setTextFormat(Qt::RichText);
    box.exec();
}

void MoneyManagerWidget::showMonthlyChart()
{
    QMap<QString, double> monthIncome;
    QMap<QString, double> monthExpenses;

    for (const ExpenseItem &e : expenses) {
        QString key = e.date.toString("yyyy-MM");

        if (e.isIncome)
            monthIncome[key] += e.amount;
        else
            monthExpenses[key] += e.amount;
    }

    // Create bar sets
    QBarSet *incomeSet = new QBarSet("Income");
    QBarSet *expenseSet = new QBarSet("Expenses");

    QStringList categories;

    for (const QString &month : monthIncome.keys() + monthExpenses.keys()) {
        if (!categories.contains(month))
            categories.append(month);
    }

    std::sort(categories.begin(), categories.end());

    for (const QString &month : categories) {
        *incomeSet << monthIncome.value(month, 0);
        *expenseSet << monthExpenses.value(month, 0);
    }

    QBarSeries *series = new QBarSeries();
    series->append(incomeSet);
    series->append(expenseSet);

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Monthly Income vs Expenses");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Amount ($)");
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    QChartView *view = new QChartView(chart);
    view->setRenderHint(QPainter::Antialiasing);

    // Show in dialog
    QDialog *dlg = new QDialog(this);
    QVBoxLayout *layout = new QVBoxLayout(dlg);
    layout->addWidget(view);
    dlg->resize(800, 600);
    dlg->setWindowTitle("Monthly Chart");
    dlg->exec();
}

void MoneyManagerWidget::filterTable(const QString &text)
{
    QString query = text.trimmed().toLower();

    for (int row = 0; row < table->rowCount(); ++row) {
        bool match = false;

        for (int col = 0; col < table->columnCount(); ++col) {
            QTableWidgetItem *item = table->item(row, col);
            if (!item) continue;

            if (item->text().toLower().contains(query)) {
                match = true;
                break;
            }
        }

        table->setRowHidden(row, !match);
    }
}


