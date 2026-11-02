#include "mainwindow.h"
#include "ui_mainwindow.h"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(&TimForSendPos,&QTimer::timeout,this,&MainWindow::SendOrder);

    connect(&BCS, &StockConnector::MessageEvent, this, &MainWindow::PrintLog,Qt::QueuedConnection);
    connect(&CurrentInstrument, &Instrument::CurrentOrdersRecived, this, &MainWindow::OrdersRecived,Qt::QueuedConnection);

    ui->ClassCode->addItem("Фьючерс","SPBFUT");
    ui->ClassCode->addItem("Опцион","SPBOPT");
    LoadSettings();
    BCS.SetRefrashToken(GlobalToken);
    BCS.GetAccesToken();



    //InstForOpenOrders=new Instrument();
    //InstForOpenOrders->SetParam(ui->OptName->text(),ui->ClassCode->currentData(Qt::UserRole).toString());
    //BCS.GetOrders(*InstForOpenOrders);


}

void MainWindow::PrintLog(QString Message)
{
    qDebug()<<QDateTime::currentDateTime().toString("hh.mm.ss: ") + Message;
}
MainWindow::~MainWindow()
{
    SaveSettings();
    delete ui;
}

void MainWindow::LoadSettings()
{
    QString ProgDir=QApplication::applicationDirPath();
    QString FileName=QDir(ProgDir).filePath("Config.txt");
    QSettings Param(FileName,QSettings::IniFormat);
    QStringList Tickers=Param.value("Tickers").toStringList();
    ui->OrderList->addItems(Tickers);
    ui->PositionVolume->setValue(Param.value("PositionVolume", 10).toUInt());
    ui->StepVolume->setValue(Param.value("StepVolume", 1).toUInt());
    ui->StepPrice->setValue(Param.value("StepPrice", 1).toDouble());
    ui->StartPrice->setValue(Param.value("StartPrice", 100).toDouble());
    ui->ClassCode->setCurrentIndex(Param.value("Calss", 1).toUInt());
    ui->VolumeType->setCurrentIndex(Param.value("VolumeType", 1).toUInt());
    on_VolumeType_currentIndexChanged(Param.value("VolumeType", 1).toUInt());
    ui->OrderType->setCurrentIndex(Param.value("OrderType", 0).toUInt());
    on_OrderType_currentIndexChanged(Param.value("OrderType", 0).toUInt());
    ui->spread->setValue(Param.value("Spread", 0).toDouble());
    ui->Offset->setValue(Param.value("Offset", 0).toDouble());
    GlobalToken=Param.value("Token","").toString();
    if(GlobalToken.size()==0)
    {
        QMessageBox::critical(this,"Ошибка","Пропишите ключ доступа в конфиг файл\nи перезапустите приложение",QMessageBox::Ok);
        SaveSettings();
        exit(1);
    }
}

void MainWindow::SaveSettings()
{
    QString ProgDir=QApplication::applicationDirPath();
    QString FileName=QDir(ProgDir).filePath("Config.txt");
    QSettings Param(FileName,QSettings::IniFormat);
    Param.setValue("Application","Размещение Лимиток");
    QStringList Data;
    for (int i=0;i<ui->OrderList->count();i++)
        Data<<ui->OrderList->item(i)->text();
    Param.setValue("Tickers",Data);
    Param.setValue("PositionVolume",ui->PositionVolume->value());
    Param.setValue("StepVolume",ui->StepVolume->value());
    Param.setValue("StepPrice",ui->StepPrice->value());
    Param.setValue("StartPrice",ui->StartPrice->value());
    Param.setValue("Calss",ui->ClassCode->currentIndex());
    Param.setValue("VolumeType",ui->VolumeType->currentIndex());
    Param.setValue("OrderType",ui->OrderType->currentIndex());
    Param.setValue("Spread",ui->spread->value());
    Param.setValue("Offset",ui->Offset->value());
    ui->OrderType->setCurrentIndex(Param.value("OrderType", 0).toUInt());
    Param.sync();
}

void MainWindow::on_Buy_clicked()
{
    Bay=true;
    TimForSendPos.start(200);
}
void MainWindow::on_Sell_clicked()
{
    Bay=false;
    TimForSendPos.start(200);
}

void MainWindow::SendOrder()
{
    if(ui->PositionVolume->value()==0)
    {
        TimForSendPos.stop();
        QTimer::singleShot(500,this,&MainWindow::UpdateInstData);
        return;
    }
    double OrderPrice=ui->StartPrice->value();
    int OrderValue;
    if(ui->VolumeType->currentIndex())
    {
        OrderValue=ui->StepVolume->value();
        if(OrderValue>ui->PositionVolume->value())
            OrderValue=ui->PositionVolume->value();
        ui->PositionVolume->setValue(ui->PositionVolume->value()-OrderValue);

    }else
    {
        OrderValue=ui->StepVolume->value();
        double MoneyValue=OrderValue*ui->StartPrice->value();
        while(MoneyValue>ui->PositionVolume->value())
        {
            OrderValue--;
            MoneyValue=OrderValue*ui->StartPrice->value();
            if(OrderValue==0)
            {
                ui->PositionVolume->setValue(0);
                TimForSendPos.stop();
                return;
            }
        }
        ui->StartPrice->setValue(OrderPrice-ui->StepPrice->value());
        ui->PositionVolume->setValue(ui->PositionVolume->value()-MoneyValue);

    }        //тут отправка ордера
    if(Bay)
    {
        ui->StartPrice->setValue(OrderPrice-ui->StepPrice->value());
        if(ui->OrderType->currentIndex()==0)
            BCS.PlaceLimitOrder(CurrentInstrument,OrderPrice,OrderValue,BUY);
        else
            BCS.PlaceTakeProvit(CurrentInstrument,OrderPrice,ui->Offset->value(),ui->spread->value(),OrderValue,BUY);
    }
    else
    {
        ui->StartPrice->setValue(OrderPrice+ui->StepPrice->value());
        if(ui->OrderType->currentIndex()==0)
            BCS.PlaceLimitOrder(CurrentInstrument,OrderPrice,OrderValue,SELL);
        else
            BCS.PlaceTakeProvit(CurrentInstrument,OrderPrice,ui->Offset->value(),ui->spread->value(),OrderValue,SELL);
    }
}


void MainWindow::on_OrderType_currentIndexChanged(int index)
{
    if (index==0)
    {
        ui->label_7->setVisible(false);
        ui->Offset->setVisible(false);
        ui->label_8->setVisible(false);
        ui->spread->setVisible(false);
    }
    else
    {
        ui->label_7->setVisible(true);
        ui->Offset->setVisible(true);
        ui->label_8->setVisible(true);
        ui->spread->setVisible(true);
    }
}

void MainWindow::CancelOrder(QString OrderId, QListWidgetItem *Item)
{
    QWidget *widget = qobject_cast<QWidget *>(sender());
    if(!widget)
        return;
    if(!Item)
        return;
    delete Item;
    widget->deleteLater();
    BCS.CancelOrder(OrderId);
    //QTimer::singleShot(500,this,&MainWindow::UpdateInstData);
}

void MainWindow::OrderChange(QString OrderId,int _type,double Price,double Offset,double Spread, Operation op,double quantity)
{
    BCS.CancelOrder(OrderId);
    if(_type)
        BCS.PlaceTakeProvit(CurrentInstrument,Price,Offset,Spread,quantity,op);
    else
        BCS.PlaceLimitOrder(CurrentInstrument,Price,quantity,op);
    QTimer::singleShot(500,this,&MainWindow::UpdateInstData);
}

void MainWindow::on_ClassCode_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    //UpdateInstData();
}

void MainWindow::UpdateInstData()
{
    CurrentInstrument.SetParam(ui->OrderList->currentItem()->text(),ui->ClassCode->currentData(Qt::UserRole).toString());
    BCS.GetOrders(CurrentInstrument);
}

void MainWindow::OrdersRecived(QList<MoexOrder> orders)
{

    // Удаляем с конца, чтобы индексы не сбивались
    while (ui->OrderList->count() > 0)
    {
        // Получаем item и виджет
        QListWidgetItem *item = ui->OrderList->item(0);
        QWidget *widget = ui->OrderList->itemWidget(item);

        // Удаляем item (QListWidget очистит связи)
        delete item;
        // Удаляем виджет (можно delete, потому что мы НЕ внутри сигнала виджета)
        if (widget)
            delete widget;
    }
    for(int i=0;i<orders.size();i++)
    {
        MoexOrder order=orders.at(i);
        QListWidgetItem *Item = new QListWidgetItem;
        Order *NewOrder=new Order(order,Item);
        Item->setSizeHint(NewOrder->sizeHint());
        ui->OrderList->addItem(Item);
        ui->OrderList->setItemWidget(Item,NewOrder);
        connect(NewOrder,&Order::OrderCancel,this,&MainWindow::CancelOrder);
        connect(NewOrder,&Order::OrderChange,this,&MainWindow::OrderChange);
    }

}


void MainWindow::on_VolumeType_currentIndexChanged(int index)
{
    if(index)
        ui->PositionVolume->setSuffix(" лот");
    else
        ui->PositionVolume->setSuffix(" руб");
}


void MainWindow::on_Instrumments_currentTextChanged(const QString &arg1)
{
    UpdateInstData();
}

