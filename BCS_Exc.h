#ifndef BCS_EXC_H
#define BCS_EXC_H
#include <QEventLoop>
#include <QTimer>
#include <QUuid>
#include <QJsonDocument>
#include <MoexExchangeConnector.h>
#include <QThread>
#include <QUrlQuery>
//#define EMULATION
struct LastOrder{
    Instrument *inst;
    int volume; Operation op;
};
class BCS_Exc : public StockConnector
{
    Q_OBJECT
public:
    explicit BCS_Exc(QObject *parent = nullptr);

private:
    QString AccesToken;
    QString RefreshToken;
    //QTimer ConnectIfFails;
    QNetworkRequest request[2];
private slots:
    void OrderResult();
    void OrderListResult();
    void SocketStartConnection();
    void PlaceOrder(Instrument &Pair, double price, int volume, Operation op,QString Type);
public slots:
    void GetAccesToken();
    void SetRefrashToken(QString Token);
    void SetTimeFrame(QString TimeFrame);
    void GetOrders(Instrument &Pair);
    void CancelOrder(QString OrderID);
signals:
    void GetLine();
    void ClearCandles();
    void tradeDataReady(QString ticker, BookTrade trade);
    void candleDataReady(QString ticker, Candle candle);
    void bookDataReady(QString ticker, double ask, double bid);
    void AppendCandle(QString ticker, Candle candle);
private slots:
    void SocketConnected();
    void DataFromEx(QString Data, bool last);
    void SocketOrdersConnected();
    void DataFromExOrders(QString Data, bool last);

public slots:
    void Connect();
    void PlaceLimitOrder(Instrument &Pair, double price, int volume, Operation op);
    void PlaceMarkettOrder(Instrument &Pair, int volume, Operation op);
    void PlaceTakeProvit(Instrument &Pair,double price,double offset,double spread, int volume, Operation op);
    void ChangeOrder(MoexOrder Order,double NewPrice,double NewOffset,double NewSpread, int NewVolume);
private:
    QThread BaseTh;
    quint64 counter=0;
    Candle CurrentCandle;
    QString _TimeFrame;
    int OrdersPage=0;
    QList<MoexOrder> _InstOrders;
public:
};

#endif // BCS_EXC_H
