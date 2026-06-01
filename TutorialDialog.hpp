#ifndef TUTORIALDIALOG_HPP
#define TUTORIALDIALOG_HPP

#include <QDialog>

class TutorialDialog : public QDialog {
    Q_OBJECT

public:
    explicit TutorialDialog(QWidget *parent = nullptr);
};

#endif