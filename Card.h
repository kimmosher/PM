#pragma once
#include <QString>

enum class Suit {
    Clubs = 0,
    Diamonds,
    Hearts,
    Spades
};

enum class Rank {
    Ace = 0, Two, Three, Four, Five, Six, Seven,
    Eight, Nine, Ten, Jack, Queen, King
};

struct Card {
    Suit suit;
    Rank rank;
    bool faceUp = false;

    bool isRed() const {
        return suit == Suit::Hearts || suit == Suit::Diamonds;
    }

    QString toString() const {
        static const char* ranks[] = {
            "A","2","3","4","5","6","7","8","9","10","J","Q","K"
        };
        static const char* suits[] = { "C","D","H","S" };
        return QString("%1%2").arg(ranks[(int)rank]).arg(suits[(int)suit]);
    }
};
