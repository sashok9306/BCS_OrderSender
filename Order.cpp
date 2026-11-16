#include "Order.h"
#include "ui_Order.h"

OrderWidget::OrderWidget(MoexOrder ord, QListWidgetItem *Item, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Order)
{
    ui->setupUi(this);
    item=Item;
    UpdateOrderData(ord);
}

void OrderWidget::UpdateOrderData(MoexOrder ORDER)
{
    _ORDER=ORDER;
    ui->Price->setValue(_ORDER.Price);
    ui->Vol->setValue(_ORDER.Volume);
    if(_ORDER.Op==BUY)
        ui->Side->setStyleSheet("background-color: rgb(0, 255, 0);");
    else
        ui->Side->setStyleSheet("background-color: rgb(255, 0, 0);");
    if(_ORDER.type)
    {
        ui->offset->setValue(_ORDER.Offset);
        ui->Spread->setValue(_ORDER.Spread);
    }
    else
    {
        ui->offset->setVisible(false);
        ui->Spread->setVisible(false);
        ui->Side_3->setVisible(false);
        ui->Side_4->setVisible(false);
    }
}

OrderWidget::~OrderWidget()
{
    delete ui;
}

void OrderWidget::on_Cancel_clicked()
{
    emit OrderCancel(_ORDER);
}

void OrderWidget::on_Ok_clicked()
{
    emit OrderChange(_ORDER,ui->Price->value(),ui->offset->value(),ui->Spread->value(),ui->Vol->value());
}

MoexOrder OrderWidget::GetOrder()
{
    return _ORDER;
}

QListWidgetItem *OrderWidget::GetItem()
{
    return item;
}

