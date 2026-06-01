#ifndef SOLITAIREWIDGET_H
#define SOLITAIREWIDGET_H

#include <QWidget>
#include <QVector>
#include <QTimer>
#include "SolitaireGame.h"

class SolitaireWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SolitaireWidget(QWidget* parent = nullptr);

    // Card geometry constants
    static constexpr int CARD_W = 71;
    static constexpr int CARD_H = 96;
    static constexpr int TABLEAU_SPACING = 25;

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void mouseDoubleClickEvent(QMouseEvent*) override;
    void keyPressEvent(QKeyEvent*) override;

public slots:
    void newGame();
    void undo();

private:
    // Geometry helpers
    QRect stockRect() const;
    QRect wasteRect() const;
    QRect foundationRect(int i) const;
    QRect tableauRect(int pile, int index) const;
    QRect tableauDropRect(int pile) const;

    // Hit testing
    bool hitTestTableau(const QPoint& pos, int& pileOut, int& indexOut) const;
    bool hitTestFoundation(const QPoint& pos, int& foundationOut) const;

    // Dragging
    bool dragging;
    int dragFromPile;
    int dragFromIndex;
    QPoint dragOffset;
    QVector<Card> dragCards;
    void clearDrag();

    // Game
    SolitaireGame game;

    // Win + fireworks
    bool gameWon;

    struct Particle {
        QPointF pos;
        QPointF vel;
        float life;
        QColor color;
    };

    QVector<Particle> particles;
    QTimer* fireworkTimer;

    void checkForWin();
    void startFireworks();
    void spawnFireworkBurst();
    void updateFireworks();
};

#endif // SOLITAIREWIDGET_H
