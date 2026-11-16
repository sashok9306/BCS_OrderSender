#ifndef ORDER_H
#define ORDER_H

#include <BCS_CONNECTOR/Instrument.h>
#include <QWidget>
#include <QString>
#include <QListWidgetItem>
namespace Ui {
class Order;
}

class OrderWidget : public QWidget
{
    Q_OBJECT

public:
    explicit OrderWidget(MoexOrder ord,QListWidgetItem *Item,QWidget *parent = nullptr);
    void UpdateOrderData(MoexOrder ORDER);
    ~OrderWidget();

private slots:
    void on_Cancel_clicked();

    void on_Ok_clicked();
public slots:
    MoexOrder GetOrder();
    QListWidgetItem *GetItem();

signals:
    void OrderCancel(MoexOrder ORDER);
    void OrderChange(MoexOrder ORDER,double Price,double Offset,double Spread, double quantity);
public:
private:
    Ui::Order *ui;
    QListWidgetItem *item;
    MoexOrder _ORDER;
};

#endif // ORDER_H
