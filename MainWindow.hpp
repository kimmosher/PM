#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP
#include <QDateEdit>
#include <QMainWindow>
#include <QTabWidget>
#include <QCalendarWidget>
#include <QLineEdit>
#include <QListWidget>
#include <QPlainTextEdit>

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
struct TaskItem {
    QString text;
    QDate due;
};
private slots:
    void onCalcEvaluate();
    void onTaskAdd();
    void onTaskRemove();
    void onNoteSave();
    void onNoteLoad();

private:
    void loadTasks();
    void saveTasks();
    void addHistory(const QString& line);
    void updateCalendarHighlights();
    bool isOverdue(const TaskItem& t) const;
    QTabWidget* tabs = nullptr;
    QCalendarWidget* calendar = nullptr;

    QLineEdit* calcInput = nullptr;

    QListWidget* taskList = nullptr;
    QLineEdit* taskInput = nullptr;
    QDateEdit* taskDate = nullptr;
    QLineEdit* noteFile = nullptr;
    QPlainTextEdit* noteEdit = nullptr;

    QPlainTextEdit* history = nullptr;
    
    double memoryValue = 0.0;
    QVector<TaskItem> tasks;
};

#endif // MAINWINDOW_HPP
