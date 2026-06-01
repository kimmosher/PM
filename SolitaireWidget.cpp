#include "SolitaireWidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QCursor>
#include <QKeyEvent>
#include <QTimer>
#include <QRandomGenerator>

//
// Helper: Convert Card → image path
//
QString cardImagePath(const Card& c)
{
    static const char* ranks[] = {
        "A","2","3","4","5","6","7","8","9","10","J","Q","K"
    };
    static const char* suits[] = { "C","D","H","S" };

    return QString(":/resources/cards/%1%2.png")
        .arg(ranks[(int)c.rank])
        .arg(suits[(int)c.suit]);
}

SolitaireWidget::SolitaireWidget(QWidget* parent)
    : QWidget(parent),
      dragging(false),
      dragFromPile(-1),
      dragFromIndex(-1),
      gameWon(false),
      fireworkTimer(nullptr)
{
    setMouseTracking(true);
    setMinimumSize(800, 600);
}

//
// ─── GEOMETRY ───────────────────────────────────────────────────────────────
//

QRect SolitaireWidget::stockRect() const {
    return QRect(20, 20, CARD_W, CARD_H);
}

QRect SolitaireWidget::wasteRect() const {
    return QRect(20 + CARD_W + 15, 20, CARD_W, CARD_H);
}

QRect SolitaireWidget::foundationRect(int i) const {
    return QRect(300 + i * (CARD_W + 15), 20, CARD_W, CARD_H);
}

QRect SolitaireWidget::tableauRect(int pile, int index) const {
    int x = 20 + pile * (CARD_W + 15);
    int y = 150 + index * TABLEAU_SPACING;
    return QRect(x, y, CARD_W, CARD_H);
}

QRect SolitaireWidget::tableauDropRect(int pile) const {
    int x = 20 + pile * (CARD_W + 15);
    int y = 150;
    return QRect(x, y, CARD_W, CARD_H);
}

//
// ─── HIT TESTING ─────────────────────────────────────────────────────────────
//

bool SolitaireWidget::hitTestTableau(const QPoint& pos, int& pileOut, int& indexOut) const
{
    for (int p = 0; p < 7; ++p) {
        const auto& cards = game.tableau[p].cards;

        for (int i = (int)cards.size() - 1; i >= 0; --i) {

            QRect r;

            if (i == (int)cards.size() - 1) {
                r = tableauRect(p, i);
            } else {
                r = QRect(
                    tableauRect(p, i).x(),
                    tableauRect(p, i).y(),
                    CARD_W,
                    TABLEAU_SPACING
                );
            }

            if (r.contains(pos)) {
                pileOut = p;
                indexOut = i;
                return true;
            }
        }
    }
    return false;
}

bool SolitaireWidget::hitTestFoundation(const QPoint& pos, int& foundationOut) const
{
    for (int f = 0; f < 4; ++f) {
        if (foundationRect(f).contains(pos)) {
            foundationOut = f;
            return true;
        }
    }
    return false;
}

//
// ─── PAINT EVENT ─────────────────────────────────────────────────────────────
//

void SolitaireWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // GREEN FELT BACKGROUND
    p.fillRect(rect(), QColor(0, 128, 0));

    QPixmap back(":/resources/cards/back.png");

    //
    // STOCK
    //
    if (!game.stock.empty()) {
        p.drawPixmap(stockRect(), back);
    } else {
        p.setPen(Qt::white);
        p.setBrush(Qt::NoBrush);
        p.drawRect(stockRect().adjusted(2, 2, -2, -2));
    }

    //
    // WASTE
    //
    if (!game.waste.empty()) {
        const Card& c = game.waste.top();
        QPixmap img(cardImagePath(c));
        p.drawPixmap(wasteRect(), img);
    } else {
        p.setPen(Qt::white);
        p.setBrush(Qt::NoBrush);
        p.drawRect(wasteRect().adjusted(2, 2, -2, -2));
    }

    //
    // FOUNDATIONS
    //
    for (int f = 0; f < 4; ++f) {
        QRect r = foundationRect(f);

        if (!game.foundations[f].empty()) {
            const Card& c = game.foundations[f].top();
            QPixmap img(cardImagePath(c));
            p.drawPixmap(r, img);
        } else {
            p.setPen(Qt::white);
            p.setBrush(Qt::NoBrush);
            p.drawRect(r.adjusted(2, 2, -2, -2));
        }
    }

    //
    // TABLEAU
    //
    for (int t = 0; t < 7; ++t) {
        const auto& cards = game.tableau[t].cards;

        for (int i = 0; i < (int)cards.size(); ++i) {
            QRect r = tableauRect(t, i);

            if (dragging && t == dragFromPile && i >= dragFromIndex)
                continue;

            const Card& c = cards[i];

            if (!c.faceUp) {
                p.drawPixmap(r, back);
            } else {
                QPixmap img(cardImagePath(c));
                p.drawPixmap(r, img);
            }
        }
    }

    //
    // DRAGGED CARDS
    //
    if (dragging) {
        QPoint pos = mapFromGlobal(QCursor::pos()) - dragOffset;

        for (int i = 0; i < (int)dragCards.size(); ++i) {
            QRect r(pos.x(), pos.y() + i * TABLEAU_SPACING, CARD_W, CARD_H);

            QPixmap img(cardImagePath(dragCards[i]));
            p.drawPixmap(r, img);
        }
    }

    //
    // FIREWORKS
    //
    if (gameWon) {
        for (const auto& part : particles) {
            QColor c = part.color;
            c.setAlphaF(part.life);
            p.setBrush(c);
            p.setPen(Qt::NoPen);
            p.drawEllipse(part.pos, 4, 4);
        }
    }
}

//
// ─── MOUSE PRESS ─────────────────────────────────────────────────────────────
//

void SolitaireWidget::mousePressEvent(QMouseEvent* e)
{
    QPoint pos = e->pos();

    // Stock click
    if (stockRect().contains(pos)) {
        game.drawFromStock();
        checkForWin();
        update();
        return;
    }

    // Waste click → try moves
    if (wasteRect().contains(pos)) {
        for (int f = 0; f < 4; ++f) {
            if (game.moveWasteToFoundation(f)) {
                checkForWin();
                update();
                return;
            }
        }

        for (int t = 0; t < 7; ++t) {
            if (game.moveWasteToTableau(t)) {
                checkForWin();
                update();
                return;
            }
        }
    }

    // Tableau click
    int pile, index;
    if (hitTestTableau(pos, pile, index)) {
        Card& c = game.tableau[pile].cards[index];

        if (!c.faceUp) {
            game.flipTableauTop(pile);
            checkForWin();
            update();
            return;
        }

        dragging = true;
        dragFromPile = pile;
        dragFromIndex = index;

        dragCards.assign(
            game.tableau[pile].cards.begin() + index,
            game.tableau[pile].cards.end()
        );

        dragOffset = pos - tableauRect(pile, index).topLeft();
        update();
        return;
    }
}

//
// ─── MOUSE MOVE ─────────────────────────────────────────────────────────────
//

void SolitaireWidget::mouseMoveEvent(QMouseEvent*)
{
    if (dragging)
        update();
}

//
// ─── MOUSE RELEASE ───────────────────────────────────────────────────────────
//

void SolitaireWidget::mouseReleaseEvent(QMouseEvent* e)
{
    if (!dragging)
        return;

    QPoint pos = e->pos();

    //
    // Try dropping onto tableau piles
    //
    for (int t = 0; t < 7; ++t) {

        const auto& pile = game.tableau[t].cards;

        QRect dropRect;

        if (!pile.empty()) {
            int topIndex = pile.size() - 1;
            dropRect = tableauRect(t, topIndex);
        } else {
            dropRect = tableauDropRect(t);
        }

        if (dropRect.contains(pos)) {
            if (game.moveTableauRun(dragFromPile, dragFromIndex, t)) {
                clearDrag();
                checkForWin();
                update();
                return;
            }
        }
    }

    //
    // Try dropping onto foundations
    //
    int f;
    if (hitTestFoundation(pos, f)) {
        if (dragCards.size() == 1) {
            if (game.moveTableauToFoundation(dragFromPile, f)) {
                clearDrag();
                checkForWin();
                update();
                return;
            }
        }
    }

    clearDrag();
    update();
}

//
// ─── DOUBLE CLICK AUTO-MOVE ───────────────────────────────────────────────────
//

void SolitaireWidget::mouseDoubleClickEvent(QMouseEvent* e)
{
    QPoint pos = e->pos();

    if (wasteRect().contains(pos)) {
        for (int f = 0; f < 4; ++f) {
            if (game.moveWasteToFoundation(f)) {
                checkForWin();
                update();
                return;
            }
        }
        return;
    }

    int pile, index;
    if (hitTestTableau(pos, pile, index)) {
        const Card& c = game.tableau[pile].cards[index];

        if (!c.faceUp)
            return;

        if (index != (int)game.tableau[pile].cards.size() - 1)
            return;

        for (int f = 0; f < 4; ++f) {
            if (game.moveTableauToFoundation(pile, f)) {
                checkForWin();
                update();
                return;
            }
        }
    }
}

//
// ─── KEY PRESS (AUTO-COMPLETE) ───────────────────────────────────────────────
//

void SolitaireWidget::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Space) {
        game.autoCompleteAll();
        checkForWin();
        update();
    }
}

//
// ─── DRAG HELPERS ───────────────────────────────────────────────────────────
//

void SolitaireWidget::clearDrag()
{
    dragging = false;
    dragCards.clear();
    dragFromPile = -1;
    dragFromIndex = -1;
}

//
// ─── WIN CHECK + FIREWORKS ───────────────────────────────────────────────────
//

void SolitaireWidget::checkForWin()
{
    if (game.isWon() && !gameWon) {
        gameWon = true;
        startFireworks();
    }
}

void SolitaireWidget::startFireworks()
{
    if (fireworkTimer)
        return;

    fireworkTimer = new QTimer(this);
    connect(fireworkTimer, &QTimer::timeout, this, &SolitaireWidget::updateFireworks);
    fireworkTimer->start(16);
}

void SolitaireWidget::spawnFireworkBurst()
{
    QPointF center(width() / 2.0, height() / 3.0);

    int count = 40;
    for (int i = 0; i < count; ++i) {
        double angle = (2 * M_PI * i) / count;
        double speed = 3.0 + QRandomGenerator::global()->bounded(2.0);

        Particle part;
        part.pos = center;
        part.vel = QPointF(std::cos(angle) * speed,
                           std::sin(angle) * speed);
        part.life = 1.0f;

        part.color = QColor::fromHsv(
            QRandomGenerator::global()->bounded(360),
            255,
            255
        );

        particles.push_back(part);
    }
}

void SolitaireWidget::updateFireworks()
{
    if (QRandomGenerator::global()->bounded(5) == 0)
        spawnFireworkBurst();

    for (int i = particles.size() - 1; i >= 0; --i) {
        auto& p = particles[i];
        p.pos += p.vel;
        p.vel.setY(p.vel.y() + 0.1);
        p.life -= 0.02f;

        if (p.life <= 0.0f)
            particles.remove(i);
    }

    update();
}

void SolitaireWidget::newGame()
{
    game.newGame();
    dragging = false;
    dragCards.clear();
    gameWon = false;
    particles.clear();

    if (fireworkTimer) {
        fireworkTimer->stop();
        fireworkTimer->deleteLater();
        fireworkTimer = nullptr;
    }

    update();
}
void SolitaireWidget::undo()
{
    if (game.undo()) {
        dragging = false;
        gameWon = false;
        particles.clear();
        update();
    }
}