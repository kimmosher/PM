#include "MinimapWidget.hpp"
#include <QPainter>
#include <QFile>
#include <QDir>
QPixmap makeWhite(const QPixmap &pix) {
    
    QImage img = pix.toImage();
    // for (int y = 0; y < img.height(); ++y) {
    //     QRgb *line = reinterpret_cast<QRgb*>(img.scanLine(y));
    //     for (int x = 0; x < img.width(); ++x) {
    //         int alpha = qAlpha(line[x]);
    //         line[x] = qRgba(255, 255, 255, alpha);
    //     }
    // }
    return QPixmap::fromImage(img);
}

MinimapWidget::MinimapWidget(QWidget *parent)
    : QWidget(parent)
{
    
    setMinimumSize(220, 320);
    setMinimumWidth(450);
    setMaximumWidth(500);   // optional, keeps it tidy
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    setupNodes();
    loadIcons();
    
}

void MinimapWidget::setupNodes() {
    nodes["cabin"]           = {{0, 3}, "cabin", "Cabin"};
    nodes["path1"]            = {{1, 3}, "path1", "Path1"};
    nodes["village_gate"]    = {{2, 3}, "village_gate", "Gate"};
    nodes["village_square"]  = {{3, 3}, "village_square", "Square"};

    nodes["forest"]          = {{1, 2}, "forest", "Forest"};
    nodes["clearing"]        = {{1, 1}, "clearing", "Clear"};
    nodes["rocky_trail"]     = {{1, 0}, "rocky_trail", "Trail"};
    nodes["mountain_peak"]   = {{1,-1}, "mountain_peak", "Peak"};

    nodes["riverbank"]       = {{2, 2}, "riverbank", "River"};
    nodes["old_bridge"]      = {{3, 2}, "old_bridge", "Bridge"};
    nodes["ruins_entrance"]  = {{4, 2}, "ruins_entrance", "Ruins"};

    nodes["cave"]            = {{0, 2}, "cave", "Cave"};
    nodes["underground_lake"]= {{0, 1}, "underground_lake", "Lake"};
}

void MinimapWidget::setCurrentRoom(const QString &room) {
    currentRoom = room;
    discoverRoom(room);
    update();
}

void MinimapWidget::discoverRoom(const QString &room) {
    discovered.insert(room);
    
    update();
}

void MinimapWidget::loadIcons() {
    
    icons["cabin"]            = QPixmap(":/resources/icons/cabin.png");
    whiteIcons["cabin"] = makeWhite(icons["cabin"]);
    
    icons["forest"]           = QPixmap(":/resources/icons/forest.png");
    whiteIcons["forest"] = makeWhite(icons["forest"]);
    icons["clearing"]         = QPixmap(":/resources/icons/clearing.png");
    whiteIcons["clearing"] = makeWhite(icons["clearning"]);
    icons["rocky_trail"]      = QPixmap(":/resources/icons/trail.png");
    whiteIcons["rocky_trail"] = makeWhite(icons["rocky_trail"]);
    icons["mountain_peak"]    = QPixmap(":/resources/icons/mountain.png");
    whiteIcons["mountain_peak"] = makeWhite(icons["mountain_peak"]);
    icons["cave"]             = QPixmap(":/resources/icons/cave.png");
    whiteIcons["cave"] = makeWhite(icons["cave"]);
    icons["underground_lake"] = QPixmap(":/resources/icons/lake.png");
    whiteIcons["underground_lake"] = makeWhite(icons["underground_lake"]);
    icons["riverbank"]        = QPixmap(":/resources/icons/river.png");
    whiteIcons["riverbank"] = makeWhite(icons["riverbank"]);
    icons["old_bridge"]       = QPixmap(":/resources/icons/bridge.png");
    whiteIcons["old_bridge"] = makeWhite(icons["old_bridge"]);
    icons["ruins_entrance"]   = QPixmap(":/resources/icons/ruins.png");
    whiteIcons["ruins_entrance"] = makeWhite(icons["ruins_entrance"]);
    icons["deep_ruins"]       = QPixmap(":/resources/icons/deepruins.png");
    whiteIcons["deep_ruins"] = makeWhite(icons["deep_ruins"]);
    icons["path1"]             = QPixmap(":/resources/icons/path1.png");
    whiteIcons["path1"] = makeWhite(icons["path1"]);
    icons["village_gate"]     = QPixmap(":/resources/icons/gate.png");
    whiteIcons["village_gate"] = makeWhite(icons["village_gate"]);
    icons["village_square"]   = QPixmap(":/resources/icons/village.png");
    whiteIcons["village_square"] = makeWhite(icons["village_square"]);
}

void MinimapWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // Background
    p.fillRect(rect(), QColor(20, 20, 30));

    const int cell = 55;
    const QPoint center(width() / 4, height() / 2);

    // -------------------------
    // Draw connections
    // -------------------------
    p.setPen(QPen(QColor(80, 80, 120), 2));

    for (const auto &node : nodes) {
        for (const auto &other : nodes) {
            if (node.pos == other.pos) continue;

            bool adjacent =
                (node.pos.x() == other.pos.x() && qAbs(node.pos.y() - other.pos.y()) == 1) ||
                (node.pos.y() == other.pos.y() && qAbs(node.pos.x() - other.pos.x()) == 1);

            if (adjacent) {
                QPoint a = center + QPoint(node.pos.x() * cell, node.pos.y() * cell);
                QPoint b = center + QPoint(other.pos.x() * cell, other.pos.y() * cell);
                p.drawLine(a, b);
            }
        }
    }

    // -------------------------
    // Draw nodes + icons + labels
    // -------------------------
    for (const auto &node : nodes) {

        QPoint c = center + QPoint(node.pos.x() * cell, node.pos.y() * cell);

        bool isCurrent = (node.name == currentRoom);
        bool isSeen    = discovered.contains(node.name);

        // --- Blue glow for current room ---
        if (isCurrent) {
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(0, 128, 255, 100));
            p.drawEllipse(c, 18, 18);
        }

        // --- Pick icon (white if discovered, color if not) ---
        QPixmap icon;
        if (isSeen)
            icon = whiteIcons[node.name];
        else
            icon = icons[node.name];

        icon = icon.scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        // Draw icon centered
        p.drawPixmap(c.x() - 12, c.y() - 12, icon);

        // --- Draw label only if seen or current ---
        if (isSeen || isCurrent) {
            p.setPen(Qt::white);
            p.drawText(c + QPoint(-18, 20), node.label);
        }

        // --- Draw node circle behind icon ---
        // QColor fill;
        // if (isCurrent)
        //     fill = QColor(0, 200, 255);
        // else if (isSeen)
        //     fill = QColor(140, 140, 220);
        // else
        //     fill = QColor(100, 100, 45);

        // p.setBrush(fill);
        // p.setPen(Qt::black);
        //p.drawEllipse(c, 10, 10);
    }
}




