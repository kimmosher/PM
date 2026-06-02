#include "TriviaWidget.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>
#include <QDebug>
#include <algorithm>

TriviaWidget::TriviaWidget(QWidget *parent)
    : QWidget(parent),
      manager(new QNetworkAccessManager(this))
{
    // UI setup
    questionLabel = new QLabel("Loading trivia...", this);
    questionLabel->setWordWrap(true);

    scoreLabel = new QLabel("Score: 0", this);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(scoreLabel);
    mainLayout->addWidget(questionLabel);

    QVBoxLayout *answersLayout = new QVBoxLayout;
    for (int i = 0; i < 4; ++i) {
        answerButtons[i] = new QPushButton(this);
        answerButtons[i]->setEnabled(false);
        connect(answerButtons[i], &QPushButton::clicked,
                this, &TriviaWidget::handleAnswerClicked);
        answersLayout->addWidget(answerButtons[i]);
    }
    mainLayout->addLayout(answersLayout);

    nextButton = new QPushButton("Next Question", this);
    nextButton->setEnabled(false);
    connect(nextButton, &QPushButton::clicked,
            this, &TriviaWidget::showNextQuestion);
    mainLayout->addWidget(nextButton);

    setLayout(mainLayout);

    // Networking
    connect(manager, &QNetworkAccessManager::finished,
            this, &TriviaWidget::handleNetworkFinished);

    fetchQuestions();
}

void TriviaWidget::fetchQuestions() {
    QUrl url("https://opentdb.com/api.php?amount=1000&type=multiple");
    manager->get(QNetworkRequest(url));
}

void TriviaWidget::handleNetworkFinished(QNetworkReply *reply) {
    auto data = reply->readAll();
    reply->deleteLater();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonArray results = doc["results"].toArray();
    processQuestions(results);
    currentIndex = 0;
    showNextQuestion();
}

void TriviaWidget::processQuestions(const QJsonArray &results) {
    questions.clear();

    for (const QJsonValue &val : results) {
        QJsonObject obj = val.toObject();

        TriviaQuestion q;
        q.question = obj["question"].toString();
        q.correct = obj["correct_answer"].toString();

        for (const QJsonValue &inc : obj["incorrect_answers"].toArray())
            q.incorrect << inc.toString();

        questions.append(q);
    }

    // Correct shuffle using URBG
    auto *rng = QRandomGenerator::global();
    std::shuffle(questions.begin(), questions.end(), *rng);
}

void TriviaWidget::showNextQuestion() {
    if (questions.isEmpty()) {
        questionLabel->setText("No questions loaded.");
        setAnswersEnabled(false);
        nextButton->setEnabled(false);
        return;
    }

    if (currentIndex >= questions.size()) {
        questionLabel->setText("No more questions.\nFinal score: " + QString::number(score));
        setAnswersEnabled(false);
        nextButton->setEnabled(false);
        return;
    }

    const TriviaQuestion &q = questions[currentIndex];

    questionLabel->setText(decodeHtml(q.question));

    QStringList answers;
    answers << q.correct << q.incorrect;

    // Correct shuffle using URBG
    auto *rng = QRandomGenerator::global();
    std::shuffle(answers.begin(), answers.end(), *rng);

    for (int i = 0; i < 4; ++i) {
        answerButtons[i]->setText(answers[i]);
        answerButtons[i]->setProperty("isCorrect", answers[i] == q.correct);
        answerButtons[i]->setEnabled(true);
        answerButtons[i]->setStyleSheet("");
    }

    nextButton->setEnabled(false);
    currentIndex++;
}

void TriviaWidget::handleAnswerClicked() {
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;

    bool isCorrect = btn->property("isCorrect").toBool();
    if (isCorrect) {
        score++;
        updateScoreLabel();
        btn->setStyleSheet("background-color: #4CAF50; color: white;");
    } else {
        btn->setStyleSheet("background-color: #F44336; color: white;");
        // highlight correct one
        for (int i = 0; i < 4; ++i) {
            if (answerButtons[i]->property("isCorrect").toBool()) {
                answerButtons[i]->setStyleSheet("background-color: #4CAF50; color: white;");
            }
        }
    }

    setAnswersEnabled(false);
    nextButton->setEnabled(true);
}

void TriviaWidget::updateScoreLabel() {
    scoreLabel->setText("Score: " + QString::number(score));
}

void TriviaWidget::setAnswersEnabled(bool enabled) {
    for (int i = 0; i < 4; ++i) {
        answerButtons[i]->setEnabled(enabled);
        if (enabled)
            answerButtons[i]->setStyleSheet("");
    }
}

QString TriviaWidget::decodeHtml(const QString &html)
{
    QTextDocument doc;
    doc.setHtml(html);
    return doc.toPlainText();
}

