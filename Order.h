#ifndef ORDER_H
#define ORDER_H

#include "Instrument.h"
#include <QWidget>
#include <QString>
#include <QListWidgetItem>
namespace Ui {
class Order;
}

class Order : public QWidget
{
    Q_OBJECT

public:
    explicit Order(MoexOrder ord,QListWidgetItem *Item,QWidget *parent = nullptr);
    ~Order();

private slots:
    void on_Cancel_clicked();

    void on_Ok_clicked();

signals:
    void OrderCancel(MoexOrder ORDER,QListWidgetItem *Item);
    void OrderChange(MoexOrder ORDER,double Price,double Offset,double Spread, double quantity);
private:
    Ui::Order *ui;
    MoexOrder _ORDER;
    QListWidgetItem *item;
};

#endif // ORDER_H
