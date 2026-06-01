#pragma once

#include "Pile.h"
#include <array>
#include <random>
#include <QVector>

// One single, global MoveRecord used by the game and undo system
struct MoveRecord {
    enum Type {
        WasteToFoundation,
        WasteToTableau,
        TableauToFoundation,
        TableauRun,
        DrawFromStock,
        FlipTableau
    } type;

    int fromPile   = -1;
    int fromIndex  = -1;
    int toPile     = -1;
    int foundation = -1;

    QVector<Card> movedCards;
};

class SolitaireGame
{
public:
    // Draw-3 Windows-style Klondike

    Pile stock{PileType::Stock};
    Pile waste{PileType::Waste};
    std::array<Pile, 4> foundations;   // 0..3
    std::array<Pile, 7> tableau;       // 0..6

    std::vector<Card> deck;
    std::mt19937 rng;

    SolitaireGame();

    void newGame();
    void initDeck();
    void shuffleDeck();
    void deal();

    // Stock / waste
    bool drawFromStock();   // draw 3 to waste (Windows-style)
    void resetStock();      // recycle waste → stock

    // Rules
    bool canMoveToFoundation(const Card& c, int foundationIndex) const;
    bool canMoveToTableau(const Card& c, int tableauIndex) const;

    // Moves
    bool moveWasteToFoundation(int foundationIndex);
    bool moveWasteToTableau(int tableauIndex);

    bool moveTableauToFoundation(int fromTableau, int foundationIndex);
    bool moveTableauRun(int fromTableau, int cardIndexInPile, int toTableau);

    bool flipTableauTop(int tableauIndex);

    bool canAutoComplete() const;
    void autoCompleteStep();
    void autoCompleteAll();
    bool isWon() const;

    bool undo();

private:
    QVector<MoveRecord> history;
};
