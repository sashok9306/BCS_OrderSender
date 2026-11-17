#include "mainwindow.h"
#include "ClassCodeDialog.h"
#include "ui_mainwindow.h"
#define VERSION "v1.2.1"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QCoreApplication::setApplicationVersion(VERSION);

    setWindowTitle("Лимитки на срочный рынок "+QCoreApplication::applicationVersion());
    connect(&TimForSendPos,&QTimer::timeout,this,&MainWindow::SendOrder);

    connect(&BCS, &StockConnector::MessageEvent, this, &MainWindow::PrintLog,Qt::QueuedConnection);
    //connect(&CurrentInstrument, &Instrument::CurrentOrdersRecived, this, &MainWindow::OrdersRecived,Qt::QueuedConnection);

    //ui->ClassCode->addItem("Фьючерс","SPBFUT");
    //ui->ClassCode->addItem("Опцион","SPBOPT");
    LoadSettings();
    BCS.SetRefrashToken(GlobalToken);
    BCS.Connect();

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
#ifndef Q_OS_ANDROID
    QString ProgDir=QApplication::applicationDirPath();
    QString FileName=QDir(ProgDir).filePath("Config.txt");
    QSettings Param(FileName,QSettings::IniFormat);
#else
    QSettings Param("MakarovDeveloper","LimitSender");
#endif

    QStringList Tickers=Param.value("Tickers").toStringList();
    if(Tickers.size()%2==0)
    {
        while(Tickers.size())
        {
            QString Ticker=Tickers.at(0);
            QString ClassCode=Tickers.at(1);
            Tickers.remove(0,2);
            ControlledInstrumments[Ticker]=BCS.CreateInstrument(Ticker,ClassCode);
            connect(ControlledInstrumments[Ticker],&Instrument::CurrentOrders,this,&MainWindow::OrdersRecive,Qt::QueuedConnection);
            connect(ControlledInstrumments[Ticker],&Instrument::NewOrderState,this,&MainWindow::OrderUpdate,Qt::QueuedConnection);
            ui->Instrumments->addItem(Ticker,ClassCode);
        }
    }
    ui->PositionVolume->setValue(Param.value("PositionVolume", 10).toUInt());
    ui->StepVolume->setValue(Param.value("StepVolume", 1).toUInt());
    ui->StepPrice->setValue(Param.value("StepPrice", 1).toDouble());
    ui->StartPrice->setValue(Param.value("StartPrice", 100).toDouble());
    ui->VolumeType->setCurrentIndex(Param.value("VolumeType", 1).toUInt());
    on_VolumeType_currentIndexChanged(Param.value("VolumeType", 1).toUInt());
    ui->OrderType->setCurrentIndex(Param.value("OrderType", 0).toUInt());
    on_OrderType_currentIndexChanged(Param.value("OrderType", 0).toUInt());
    ui->spread->setValue(Param.value("Spread", 0).toDouble());
    ui->Offset->setValue(Param.value("Offset", 0).toDouble());
    GlobalToken=Param.value("Token","").toString();
    if(GlobalToken.size()==0)
    {
        SetupKey KeyWindow;
        KeyWindow.show();
        KeyWindow.exec();
        GlobalToken=KeyWindow.Key;
        SaveSettings();
        exit(1);
    }
}

void MainWindow::SaveSettings()
{
#ifndef Q_OS_ANDROID
    QString ProgDir=QApplication::applicationDirPath();
    QString FileName=QDir(ProgDir).filePath("Config.txt");
    QSettings Param(FileName,QSettings::IniFormat);
#else
    QSettings Param("MakarovDeveloper","LimitSender");
#endif
    Param.setValue("Application","Размещение Лимиток");

    QStringList Data;
    for (int i=0;i<ui->Instrumments->count();i++)
    {
        Data<<ui->Instrumments->itemText(i);
        Data<<ui->Instrumments->itemData(i,Qt::UserRole).toString();
    }
    Param.setValue("Tickers",Data);
    Param.setValue("PositionVolume",ui->PositionVolume->value());
    Param.setValue("StepVolume",ui->StepVolume->value());
    Param.setValue("StepPrice",ui->StepPrice->value());
    Param.setValue("StartPrice",ui->StartPrice->value());
    Param.setValue("VolumeType",ui->VolumeType->currentIndex());
    Param.setValue("OrderType",ui->OrderType->currentIndex());
    Param.setValue("Spread",ui->spread->value());
    Param.setValue("Offset",ui->Offset->value());
    Param.setValue("Token",GlobalToken);
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
            BCS.PlaceLimitOrder(ui->Instrumments->currentText(),ui->Instrumments->currentData(Qt::UserRole).toString(),OrderPrice,OrderValue,BUY);
        else
            BCS.PlaceTakeProvit(ui->Instrumments->currentText(),ui->Instrumments->currentData(Qt::UserRole).toString(),OrderPrice,ui->Offset->value(),ui->spread->value(),OrderValue,BUY);
    }
    else
    {
        ui->StartPrice->setValue(OrderPrice+ui->StepPrice->value());
        if(ui->OrderType->currentIndex()==0)
            BCS.PlaceLimitOrder(ui->Instrumments->currentText(),ui->Instrumments->currentData(Qt::UserRole).toString(),OrderPrice,OrderValue,SELL);
        else
            BCS.PlaceTakeProvit(ui->Instrumments->currentText(),ui->Instrumments->currentData(Qt::UserRole).toString(),OrderPrice,ui->Offset->value(),ui->spread->value(),OrderValue,SELL);
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

void MainWindow::OrderUpdate(MoexOrder UpOrder)
{
    Instrument* InstrumentSender=qobject_cast<Instrument*>(sender());
    if(InstrumentSender==nullptr)
        return;
    if(InstrumentSender->GetTicker()!=ui->Instrumments->currentText())
        return;
    if(UpOrder.State==CANCELED||UpOrder.State==COMPLEATE)
    {
        OrderWidget*OrderWidget=WidgetOrderControll.take(UpOrder.MoexOrderID);
        if(!OrderWidget)
            return;
        delete OrderWidget->GetItem();
        delete OrderWidget;

    }
    if(UpOrder.State==PART_COMPLEATE)
    {
        OrderWidget*OrderWidget=WidgetOrderControll.take(UpOrder.MoexOrderID);
        if(!OrderWidget)
            return;
        OrderWidget->UpdateOrderData(UpOrder);
    }
    // если ордер новый, нужно посчитать куда его запихнуть
    // в списке ордеров и в списке виджетов они все отсортированы
    // надо пробежать по списку и найти куда вставить
    if(UpOrder.State==NEW)
    {
        if(ui->OrderList->count()==0)
        {
            CreateWidgetInOrderList(0,UpOrder);
            return;
        }
        for(int i=0;i<ui->OrderList->count();i++)
        {
            OrderWidget * CurOrder=qobject_cast<OrderWidget*>(ui->OrderList->itemWidget(ui->OrderList->item(i)));
            if(CurOrder)
                if(UpOrder.Price>CurOrder->GetOrder().Price)
                {
                    CreateWidgetInOrderList(i,UpOrder);
                    return;
                }
        }
        CreateWidgetInOrderList(ui->OrderList->count(),UpOrder);
    }
}

void MainWindow::OrdersRecive(QHash<QString,MoexOrder> orders)
{
    Instrument* InstrumentSender=qobject_cast<Instrument*>(sender());
    if(InstrumentSender==nullptr)
        return;
    if(InstrumentSender->GetTicker()!=ui->Instrumments->currentText())
        return;
    QList<MoexOrder>PrintOrders=orders.values();
    std::sort(PrintOrders.begin(), PrintOrders.end(), [](const MoexOrder& a, const MoexOrder& b) {
        return a.Price > b.Price;
    });

    for(int i=0;i<PrintOrders.size();i++)
    {
        MoexOrder order=PrintOrders.at(i);
        CreateWidgetInOrderList(i,order);
    }

}


void MainWindow::on_VolumeType_currentIndexChanged(int index)
{
    if(index)
        ui->PositionVolume->setSuffix(" лот");
    else
        ui->PositionVolume->setSuffix(" руб");
}

void MainWindow::on_Instrumments_currentIndexChanged(int index)
{

    WidgetOrderControll.clear();
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
    // добавление нового инструмммента в отслеживаемые
    if(ControlledInstrumments.value(ui->Instrumments->currentText())==nullptr)
    {
        ClassCodeDialog Dialog;
        Dialog.setModal(true);
        Dialog.show();
        Dialog.exec();
        ui->Instrumments->setItemData(ui->Instrumments->currentIndex(),Dialog.ClassCode,Qt::UserRole);
        QString tmp=ui->Instrumments->currentData(Qt::UserRole).toString();
        ControlledInstrumments[ui->Instrumments->currentText()]=BCS.CreateInstrument(ui->Instrumments->currentText(),ui->Instrumments->currentData(Qt::UserRole).toString());
        connect(ControlledInstrumments[ui->Instrumments->currentText()],&Instrument::CurrentOrders,this,&MainWindow::OrdersRecive,Qt::QueuedConnection);
        connect(ControlledInstrumments[ui->Instrumments->currentText()],&Instrument::NewOrderState,this,&MainWindow::OrderUpdate,Qt::QueuedConnection);
    }
    else
        // перезапросим ордера на всякий случай, так как поменяли инструммент(если инструммент добавили то запрос ордеров происходит автоматически
        BCS.GetOrders();
}


void MainWindow::CreateWidgetInOrderList(int index, MoexOrder DataOrder)
{
    QListWidgetItem *Item = new QListWidgetItem;
    OrderWidget *NewOrder=new OrderWidget(DataOrder,Item);
    WidgetOrderControll[DataOrder.MoexOrderID]=NewOrder;
    Item->setSizeHint(NewOrder->sizeHint());
    ui->OrderList->insertItem(index,Item);
    ui->OrderList->setItemWidget(Item,NewOrder);
    connect(NewOrder,&OrderWidget::OrderCancel,&BCS,&BCS_Exc::CancelOrder);
    connect(NewOrder,&OrderWidget::OrderChange,&BCS,&BCS_Exc::ChangeOrder);
}


void MainWindow::on_DeleteInstrumment_clicked()
{
    if(ui->Instrumments->count()<=1)
        return;
    ui->Instrumments->removeItem(ui->Instrumments->currentIndex());
}

