#include "Order.h"
#include "ui_Order.h"

Order::Order(QString clientOrderId,double Price,Operation OrderOp,double RemainedQuantity,QListWidgetItem *Item,QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Order)
{
    ui->setupUi(this);
    item=Item;
    _clientOrderId=clientOrderId;
    _Price=Price;
    _OrderOp=OrderOp;
    _RemainedQuantity=RemainedQuantity;
    ui->Price->setValue(_Price);
    if(OrderOp==BUY)
        ui->Side->setText("Покупка "+QString::number(RemainedQuantity));
    else
        ui->Side->setText("Продажа "+QString::number(RemainedQuantity));

}

Order::~Order()
{
    delete ui;
}

void Order::on_Cancel_clicked()
{
    emit OrderCancel(_clientOrderId,item);
}

void Order::on_Ok_clicked()
{
    emit OrderChange(_clientOrderId,ui->Price->value(),_OrderOp,_RemainedQuantity);
}

