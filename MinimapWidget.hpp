#ifndef MINIMAPWIDGET_HPP
#define MINIMAPWIDGET_HPP

#include <QWidget>
#include <QMap>
#include <QSet>
#include <QPoint>

class MinimapWidget : public QWidget {
    Q_OBJECT

public:
    explicit MinimapWidget(QWidget *parent = nullptr);

    void setCurrentRoom(const QString &room);
    void discoverRoom(const QString &room);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    struct Node {
        QPoint pos;      // grid position
        QString name;    // internal id
        QString label;   // display label
    };

    QMap<QString, Node> nodes;
    QString currentRoom;
    QSet<QString> discovered;
    QMap<QString, QPixmap> icons;
    QMap<QString, QPixmap> whiteIcons;
    void setupNodes();
    void loadIcons();               // <-- NEW
};

#endif
