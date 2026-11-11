#ifndef SETUPKEY_H
#define SETUPKEY_H

#include <QDialog>

namespace Ui {
class SetupKey;
}

class SetupKey : public QDialog
{
    Q_OBJECT

public:
    explicit SetupKey(QWidget *parent = nullptr);
    ~SetupKey();

private slots:
    void on_Ok_clicked();

    void on_Cancel_clicked();
public:
    QString Key;

private:
    Ui::SetupKey *ui;
};

#endif // SETUPKEY_H
