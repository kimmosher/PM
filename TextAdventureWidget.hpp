#ifndef TEXTADVENTUREWIDGET_HPP
#define TEXTADVENTUREWIDGET_HPP
#include "MinimapWidget.hpp"
#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMap>

class TextAdventureWidget : public QWidget {
    Q_OBJECT

public:
    explicit TextAdventureWidget(QWidget *parent = nullptr);

private slots:
    void onCommandEntered();

private:
    QTextEdit *output;
    QLineEdit *input;
    MinimapWidget *minimap;
    QString currentRoom;

    struct Room {
        QString description;
        QMap<QString, QString> exits; // direction -> room name
        QStringList items;            // NEW: items in this room
        QMap<QString, QString> lockedExits; 
        
    };

    QMap<QString, Room> rooms;
    QMap<QString, bool> puzzleSolved;
    QStringList inventory;            // NEW: player inventory
    QMap<QString, QString> itemDescriptions;   // NEW: item descriptions

    void setupWorld();
    void showRoom(const QString &roomName);
    void processCommand(const QString &cmd);
};

#endif // TEXTADVENTUREWIDGET_HPP
