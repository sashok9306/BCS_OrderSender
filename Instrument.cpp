#include "Instrument.h"
#include <QtMath>
#include <QThread>
Q_DECLARE_METATYPE(BookTrade)
Q_DECLARE_METATYPE(Candle)
Instrument::Instrument()
    : QObject()
{

    CurrentPriceStruct.Price=0;

    // Создаём поток для этого инструмента
    _thread = new QThread;
    _thread->setObjectName("InstrumentThread"); // для удобства отладки

}
Instrument::~Instrument()
{
    if (_thread) {
        _thread->quit();
        _thread->wait(3000);
        delete _thread;
        _thread = nullptr;
    }
}

void Instrument::SetParam(QString Ticker, QString ClassCode)
{
    _Ticker = Ticker;
    _ClassCode = ClassCode;
}


void Instrument::UpdateBook(double ASK, double BID)
{
    _ASK=ASK;_BID=BID;
    emit CurrentPrice(ASK,BID);
}

void Instrument::AppendTrade(BookTrade Trade)
{
    CurrentPriceStruct=Trade;
    //if(_delay==0)
    //    UpdateIndik();
    //emit CurrentPrice(0,0,Trade.Price);
}

QString Instrument::GetTicker()
{
    return _Ticker;
}

QString Instrument::GetClassCode()
{
    return _ClassCode;
}

void Instrument::SetOrderList(QList<MoexOrder> Orders)
{
    _Orders=Orders;
    std::sort(_Orders.begin(), _Orders.end(), [](const MoexOrder& a, const MoexOrder& b) {
        return a.Price > b.Price;
    });
    emit CurrentOrdersRecived(_Orders);
}

void Instrument::StartInThread()
{
    if (!_thread) return;
    this->moveToThread(_thread);
    _thread->start();
}



void Instrument::onBcsTrade(QString ticker, BookTrade trade)
{
    if (ticker != _Ticker) return; // Игнорируем чужие тикеры, просыпаемся только для своих
    AppendTrade(trade);
}


void Instrument::onBcsBook(QString ticker, double ask, double bid)
{
    if (ticker != _Ticker) return;
    UpdateBook(ask, bid);
}

