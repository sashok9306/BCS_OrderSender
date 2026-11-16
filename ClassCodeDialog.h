#ifndef CLASSCODEDIALOG_H
#define CLASSCODEDIALOG_H

#include <QDialog>

namespace Ui {
class ClassCodeDialog;
}

class ClassCodeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ClassCodeDialog(QWidget *parent = nullptr);
    ~ClassCodeDialog();
    QString ClassCode;

private slots:
    void on_Ok_clicked();

private:
    Ui::ClassCodeDialog *ui;
};

#endif // CLASSCODEDIALOG_H
