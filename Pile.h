#pragma once
#include "Card.h"
#include <vector>
#include <stack>

enum class PileType {
    Stock,
    Waste,
    Foundation,
    Tableau
};

struct Pile {
    PileType type;
    std::vector<Card> cards;

    Pile(PileType t = PileType::Tableau) : type(t) {}

    bool empty() const { return cards.empty(); }

    Card& top() { return cards.back(); }
    const Card& top() const { return cards.back(); }

    void push(const Card& c) { cards.push_back(c); }
    void pop() { cards.pop_back(); }

    void clear() { cards.clear(); }
};
