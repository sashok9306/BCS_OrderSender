#ifndef INSTRUMENT_H
#define INSTRUMENT_H
#include <QList>
#include <QMap>
#include <QObject>
#include<QTimer>
#include <QDateTime>
//#include <QString.h>//
enum Operation { BUY = 0, SELL };
struct BookTrade
{
    QDateTime TimeTrade;
    double Price;
    double Volume;
};
struct LimitOrder
{
    QString OrderId;
    double Price;
    double Volume;
    Operation Op;
};
struct Candle
{
    QDateTime Time;
    double Open;
    double Close;
    double High;
    double Low;
    double Volume;
};
struct IndicatorValues {
    bool isReady = false; // Флаг "прогрева"

    // Bollinger Bands
    double bbUpper = 0.0;
    double bbMiddle = 0.0;
    double bbLower = 0.0;
    double bbWidthPercent= 0.0;
    double volatilityChangePercent=0;

    // RSI
    double rsi = 0.0;

    // CCI
    double cci = 0.0;

    // EMA
    double ema = 0.0;

    QDateTime Time;
};
class Instrument : public QObject
{
    Q_OBJECT
public:
    explicit Instrument();
    ~Instrument();
    void SetParam(QString Ticker, QString ClassCode);
    QString GetTicker();
    QString GetClassCode();
    void SetOrderList(QList<LimitOrder> Orders);
    // Запуск в отдельном потоке
    void StartInThread();
public slots:
public slots:
    void onBcsTrade(QString ticker, BookTrade trade);
    void onBcsBook(QString ticker, double ask, double bid);

signals:
    void CurrentPrice(double Ask,double Bid);
    void SendOrder(int volume, Operation op);
    void Message(QString Message);
    void CurrentOrdersRecived(QList<LimitOrder> orders);
private slots:
    void AppendTrade(BookTrade Trade);
    void UpdateBook(double  ASK, double BID);
private:
    QString _Ticker;
    QString _ClassCode;
    int _CurrentPos=0;
    double AverPrice=0;
    double _ASK=0;
    double _BID=0;
    int Volume;
    BookTrade CurrentPriceStruct;
    QList<LimitOrder> _Orders;
    QThread *_thread;
    //BIDs;
};

#endif // INSTRUMENT_H
