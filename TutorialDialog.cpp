#include "TutorialDialog.hpp"
#include <QTextEdit>
#include <QVBoxLayout>
#include <QPushButton>

TutorialDialog::TutorialDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Text Adventure Tutorial");
    resize(520, 420);

    QVBoxLayout *layout = new QVBoxLayout(this);

    QTextEdit *text = new QTextEdit(this);
    text->setReadOnly(true);
    text->setText(
        "Welcome to the Adventure!\n\n"
        "Basic Commands:\n"
        " • look — re-describe the room\n"
        " • go <direction> — move around\n"
        " • take <item> — pick up items\n"
        " • drop <item> — drop items\n"
        " • inventory — view what you're carrying\n"
        " • examine <item> — inspect items\n"
        " • solve <puzzle> — attempt puzzles\n"
        " • answer <text> — answer riddles\n\n"
        "Hints:\n"
        " • Some paths are locked until you find the right item.\n"
        " • Examine items carefully — many contain clues.\n"
        " • Puzzles may reward you with important items.\n"
        " • The world is larger than it first appears.\n\n"
        "Good luck, traveler."
    );

    QPushButton *closeBtn = new QPushButton("Begin Adventure", this);

    layout->addWidget(text);
    layout->addWidget(closeBtn);

    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
}
