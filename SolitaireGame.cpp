#include "SolitaireGame.h"
#include <algorithm>

SolitaireGame::SolitaireGame()
{
    rng.seed(std::random_device{}());
    newGame();
}

void SolitaireGame::initDeck()
{
    deck.clear();
    deck.reserve(52);

    for (int s = 0; s < 4; ++s)
    {
        for (int r = 0; r < 13; ++r)
        {
            Card c;
            c.suit = static_cast<Suit>(s);
            c.rank = static_cast<Rank>(r);
            c.faceUp = false;
            deck.push_back(c);
        }
    }
}

void SolitaireGame::shuffleDeck()
{
    std::shuffle(deck.begin(), deck.end(), rng);
}

void SolitaireGame::newGame()
{
    stock.cards.clear();
    waste.cards.clear();

    for (auto &p : tableau)
        p.cards.clear();

    for (auto &f : foundations)
        f.cards.clear();

    history.clear();

    initDeck();
    shuffleDeck();
    deal();
}

void SolitaireGame::deal()
{
    int deckIndex = 0;

    for (int col = 0; col < 7; ++col)
    {
        for (int row = 0; row <= col; ++row)
        {
            Card c = deck[deckIndex++];
            c.faceUp = (row == col);
            tableau[col].push(c);
        }
    }

    for (; deckIndex < static_cast<int>(deck.size()); ++deckIndex)
    {
        Card c = deck[deckIndex];
        c.faceUp = false;
        stock.push(c);
    }
}

bool SolitaireGame::drawFromStock()
{
    if (stock.empty())
    {
        resetStock();
        return true;
    }

    MoveRecord rec;
    rec.type = MoveRecord::DrawFromStock;
    history.push_back(rec);

    Card c = stock.top();
    stock.pop();
    c.faceUp = true;
    waste.push(c);
    return true;
}

void SolitaireGame::resetStock()
{
    std::vector<Card> temp;

    while (!waste.empty())
    {
        Card c = waste.top();
        waste.pop();
        c.faceUp = false;
        temp.push_back(c);
    }

    std::reverse(temp.begin(), temp.end());

    for (auto &c : temp)
        stock.push(c);
}

bool SolitaireGame::canMoveToFoundation(const Card& c, int foundationIndex) const
{
    const auto& pile = foundations[foundationIndex].cards;

    if (pile.empty())
        return c.rank == Rank::Ace;

    const Card& top = pile.back();

    return (c.suit == top.suit) &&
           (static_cast<int>(c.rank) == static_cast<int>(top.rank) + 1);
}

bool SolitaireGame::canMoveToTableau(const Card& c, int tableauIndex) const
{
    const auto& pile = tableau[tableauIndex].cards;

    if (pile.empty())
        return c.rank == Rank::King;

    const Card& top = pile.back();

    bool oppositeColor =
        (c.isRed() && !top.isRed()) ||
        (!c.isRed() && top.isRed());

    bool correctRank =
        static_cast<int>(c.rank) == static_cast<int>(top.rank) - 1;

    return oppositeColor && correctRank;
}

bool SolitaireGame::moveWasteToFoundation(int foundationIndex)
{
    if (waste.empty())
        return false;

    Card c = waste.top();
    if (!canMoveToFoundation(c, foundationIndex))
        return false;

    MoveRecord rec;
    rec.type = MoveRecord::WasteToFoundation;
    rec.foundation = foundationIndex;
    rec.movedCards = { c };
    history.push_back(rec);

    waste.pop();
    foundations[foundationIndex].push(c);
    return true;
}

bool SolitaireGame::moveWasteToTableau(int tableauIndex)
{
    if (waste.empty())
        return false;

    Card c = waste.top();
    if (!canMoveToTableau(c, tableauIndex))
        return false;

    MoveRecord rec;
    rec.type = MoveRecord::WasteToTableau;
    rec.toPile = tableauIndex;
    rec.movedCards = { c };
    history.push_back(rec);

    waste.pop();
    tableau[tableauIndex].push(c);
    return true;
}

bool SolitaireGame::moveTableauToFoundation(int fromTableau, int foundationIndex)
{
    auto& src = tableau[fromTableau].cards;

    if (src.empty())
        return false;

    Card c = src.back();
    if (!c.faceUp)
        return false;

    if (!canMoveToFoundation(c, foundationIndex))
        return false;

    MoveRecord rec;
    rec.type = MoveRecord::TableauToFoundation;
    rec.fromPile = fromTableau;
    rec.foundation = foundationIndex;
    rec.movedCards = { c };
    history.push_back(rec);

    src.pop_back();
    foundations[foundationIndex].push(c);

    if (!src.empty() && !src.back().faceUp)
        src.back().faceUp = true;

    return true;
}

bool SolitaireGame::moveTableauRun(int fromTableau, int cardIndexInPile, int toTableau)
{
    auto& src = tableau[fromTableau].cards;
    auto& dst = tableau[toTableau].cards;

    if (cardIndexInPile < 0 || cardIndexInPile >= static_cast<int>(src.size()))
        return false;

    const Card& first = src[cardIndexInPile];

    if (!dst.empty())
    {
        const Card& top = dst.back();

        bool oppositeColor =
            (first.isRed() && !top.isRed()) ||
            (!first.isRed() && top.isRed());

        bool correctRank =
            static_cast<int>(first.rank) == static_cast<int>(top.rank) - 1;

        if (!(oppositeColor && correctRank))
            return false;
    }
    else
    {
        if (first.rank != Rank::King)
            return false;
    }

    MoveRecord rec;
    rec.type = MoveRecord::TableauRun;
    rec.fromPile = fromTableau;
    rec.fromIndex = cardIndexInPile;
    rec.toPile = toTableau;
    rec.movedCards = QVector<Card>(src.begin() + cardIndexInPile, src.end());
    history.push_back(rec);

    std::vector<Card> moving(src.begin() + cardIndexInPile, src.end());
    src.erase(src.begin() + cardIndexInPile, src.end());
    dst.insert(dst.end(), moving.begin(), moving.end());

    if (!src.empty() && !src.back().faceUp)
        src.back().faceUp = true;

    return true;
}

bool SolitaireGame::flipTableauTop(int tableauIndex)
{
    auto& pile = tableau[tableauIndex].cards;

    if (pile.empty())
        return false;

    Card& top = pile.back();

    if (!top.faceUp)
    {
        MoveRecord rec;
        rec.type = MoveRecord::FlipTableau;
        rec.fromPile = tableauIndex;
        rec.movedCards = { top };
        history.push_back(rec);

        top.faceUp = true;
        return true;
    }

    return false;
}

bool SolitaireGame::isWon() const
{
    int total = 0;
    for (int f = 0; f < 4; ++f)
        total += foundations[f].cards.size();
    return total == 52;
}

bool SolitaireGame::canAutoComplete() const
{
    for (const auto& pile : tableau)
        for (const auto& c : pile.cards)
            if (!c.faceUp)
                return false;

    return true;
}

void SolitaireGame::autoCompleteStep()
{
    if (!waste.empty())
    {
        for (int f = 0; f < 4; ++f)
            if (moveWasteToFoundation(f))
                return;
    }

    for (int t = 0; t < 7; ++t)
    {
        if (!tableau[t].cards.empty())
        {
            for (int f = 0; f < 4; ++f)
                if (moveTableauToFoundation(t, f))
                    return;
        }
    }
}

void SolitaireGame::autoCompleteAll()
{
    if (!canAutoComplete())
        return;

    while (true)
    {
        int before = 0;
        for (int f = 0; f < 4; ++f)
            before += foundations[f].cards.size();

        autoCompleteStep();

        int after = 0;
        for (int f = 0; f < 4; ++f)
            after += foundations[f].cards.size();

        if (after == before)
            break;
    }
}

bool SolitaireGame::undo()
{
    if (history.isEmpty())
        return false;

    MoveRecord rec = history.back();
    history.pop_back();

    switch (rec.type)
    {
    case MoveRecord::WasteToFoundation:
        foundations[rec.foundation].cards.pop_back();
        waste.push(rec.movedCards[0]);
        break;

    case MoveRecord::WasteToTableau:
        tableau[rec.toPile].cards.pop_back();
        waste.push(rec.movedCards[0]);
        break;

    case MoveRecord::TableauToFoundation:
        foundations[rec.foundation].cards.pop_back();
        tableau[rec.fromPile].cards.push_back(rec.movedCards[0]);
        break;

    case MoveRecord::TableauRun:
        for (int i = 0; i < rec.movedCards.size(); ++i)
            tableau[rec.toPile].cards.pop_back();

        for (const Card& c : rec.movedCards)
            tableau[rec.fromPile].cards.push_back(c);
        break;

    case MoveRecord::DrawFromStock:
        if (!waste.empty())
        {
            Card c = waste.top();
            waste.pop();
            c.faceUp = false;
            stock.push(c);
        }
        break;

    case MoveRecord::FlipTableau:
        tableau[rec.fromPile].cards.back().faceUp = false;
        break;
    }

    return true;
}
