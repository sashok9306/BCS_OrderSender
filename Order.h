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
    explicit Order(QString clientOrderId,double Price,Operation OrderOp,double RemainedQuantity,QListWidgetItem *Item,QWidget *parent = nullptr);
    ~Order();

private slots:
    void on_Cancel_clicked();

    void on_Ok_clicked();

signals:
    void OrderCancel(QString OrderId,QListWidgetItem *Item);
    void OrderChange(QString OrderId,double Price, Operation op,double quantity);
private:
    Ui::Order *ui;
    QString _clientOrderId;
    double _Price;
    Operation _OrderOp;
    double _RemainedQuantity;
    QListWidgetItem *item;
};

#endif // ORDER_H
