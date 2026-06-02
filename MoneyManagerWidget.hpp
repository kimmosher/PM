#pragma once

#include <QWidget>
#include <QVector>
#include <QDate>

class QLineEdit;
class QDoubleSpinBox;
class QDateEdit;
class QComboBox;
class QTableWidget;
class QCheckBox;
class QLabel;

struct ExpenseItem {
    QString description;
    double amount;
    QDate date;
    QString category;
    bool isIncome = false;   // NEW
};

class MoneyManagerWidget : public QWidget {
    Q_OBJECT
public:
    explicit MoneyManagerWidget(QWidget *parent = nullptr);

private slots:
    void addExpense();
    void removeExpense();
    void showSummary();
    void showMonthlyChart();
private:
    void loadExpenses();
    void saveExpenses();
    void updateRunningTotals();   // NEW
    void colorRow(int row, bool isIncome);
    void filterTable(const QString &text);

    QVector<ExpenseItem> expenses;
    QLineEdit *searchInput;
    QLineEdit *descInput = nullptr;
    QDoubleSpinBox *amountInput = nullptr;
    QDateEdit *dateInput = nullptr;
    QComboBox *categoryInput = nullptr;
    QCheckBox *incomeCheck = nullptr;   // NEW
    QTableWidget *table = nullptr;

    QLabel *totalBalanceLabel = nullptr;     // NEW
    QLabel *monthlyBalanceLabel = nullptr;   // NEW
};
