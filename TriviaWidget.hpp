#ifndef TRIVIAWIDGET_HPP
#define TRIVIAWIDGET_HPP

#include <QWidget>
#include <QVector>
#include <QStringList>
#include <QNetworkAccessManager>
#include <QJsonArray>
#include <QRandomGenerator>
#include <algorithm>
struct TriviaQuestion {
    QString question;
    QString correct;
    QStringList incorrect;
};

class QLabel;
class QPushButton;

class TriviaWidget : public QWidget {
    Q_OBJECT
public:
    explicit TriviaWidget(QWidget *parent = nullptr);

private slots:
    void handleAnswerClicked();
    void handleNetworkFinished(QNetworkReply *reply);

private:
    QVector<TriviaQuestion> questions;
    int currentIndex = 0;
    int score = 0;

    QNetworkAccessManager *manager;

    QLabel *questionLabel;
    QLabel *scoreLabel;
    QPushButton *answerButtons[4];
    QPushButton *nextButton;

    void fetchQuestions();
    void processQuestions(const QJsonArray &results);
    void showNextQuestion();
    void updateScoreLabel();
    void setAnswersEnabled(bool enabled);
};

#endif // TRIVIAWIDGET_HPP
