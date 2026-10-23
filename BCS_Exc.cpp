#include "BCS_Exc.h"
#include <QDebug>
BCS_Exc::BCS_Exc(QObject *parent)
    : StockConnector{}
{
    Q_UNUSED(parent)
    //connect(&BaseTh,&QThread::started,&Base,&TradeBase::Start);
    //Base.moveToThread(&BaseTh);
    //BaseTh.start();
}

void BCS_Exc::SocketConnected()
{
    emit MessageEvent("✅ Соединение к данным установлено");
    Socket.ping(QByteArray());
    return;
    // подпишемся на стакан
    QJsonObject subscribeOrderBookRequest;
    subscribeOrderBookRequest["subscribeType"] = 0;
    subscribeOrderBookRequest["dataType"] = 0;
    subscribeOrderBookRequest["depth"] =1;


    QJsonArray Instr;
    foreach (Instrument *Element, _Instrumments) {
        QJsonObject Ticker;
        Ticker["ticker"] = Element->GetTicker();
        Ticker["classCode"] = Element->GetClassCode();
        Instr.append(Ticker);
    }
    subscribeOrderBookRequest["instruments"] = Instr;
    QString req = QJsonDocument(subscribeOrderBookRequest).toJson();
    subscribeOrderBookRequest["dataType"] = 2;
    QString req2 = QJsonDocument(subscribeOrderBookRequest).toJson();
    Socket.sendTextMessage(req);
    Socket.sendTextMessage(req2);
}

void BCS_Exc::DataFromEx(QString Data, bool last)
{
    Q_UNUSED(last)
    QJsonDocument doc = QJsonDocument::fromJson(Data.toUtf8());
    QJsonObject Root = doc.object();
    if (Root.value("responseType").toString() == "OrderBookSuccess") {
        emit MessageEvent("✅ Подписались на стакан " + Root.value("ticker").toString());
        return;
    }
    if (Root.value("responseType").toString() == "CandleStickSuccess") {
        emit MessageEvent("✅ Подписались на свечу " + Root.value("ticker").toString());
        return;
    }
#ifndef EMULATION
    //Base.AddTicker(Data);
#else
    emit GetLine();
#endif
    //
    if (Root.value("responseType").toString() == "OrderBook") {
        QList<BookTrade> ASKs, BIDs;
        double A, B;
        A = Root.value("asks").toArray().at(0).toObject().value("price").toDouble();
        B = Root.value("bids").toArray().at(0).toObject().value("price").toDouble();
        QString Ticker = Root.value("ticker").toString();
        emit bookDataReady(Ticker, A, B);
        //_Instrumments[Ticker]->UpdateBook(A, B);
        return;
    }
    if (Root.value("responseType").toString() == "CandleStick") {
        Candle c;
        // Парсим время (формат зависит от API, тут пример с ISO строкой)
        c.Time = QDateTime::fromString(Root["dateTime"].toString(), Qt::ISODate);
        c.Open  = Root["open"].toDouble();
        c.High  = Root["high"].toDouble();
        c.Low   = Root["low"].toDouble();
        c.Close = Root["close"].toDouble();
        c.Volume = Root["volume"].toDouble();
        QString Ticker = Root.value("ticker").toString();
        emit candleDataReady(Ticker,c);
        //_Instrumments[Ticker]->SetLastCandle(c);

        return;
    }
    if (Root.value("responseType").toString() == "LastTrades") {
        BookTrade LastTrade;
        QString Ticker = Root.value("ticker").toString();
        QDateTime DateTime=QDateTime::fromString(Root.value("dateTime").toString(),Qt::ISODate);
        LastTrade.Price = Root.value("price").toDouble();
        LastTrade.Volume = Root.value("quantity").toDouble();
        LastTrade.TimeTrade=DateTime;
        if (Root.value("side").toString() != "BUY")
            LastTrade.Volume *= -1;
        emit tradeDataReady(Ticker, LastTrade);
        //_Instrumments[Ticker]->AppendTrade(LastTrade);

    }
}

void BCS_Exc::SocketOrdersConnected()
{
    emit MessageEvent("✅ Соединение к ленте ордеров установлено");
}

void BCS_Exc::DataFromExOrders(QString Data, bool last)
{
    Q_UNUSED(last)
    QJsonDocument doc = QJsonDocument::fromJson(Data.toUtf8());
    QJsonArray Portfel = doc.array();
}
void BCS_Exc::Connect()
{
    //BaseTh.start();
#ifndef EMULATION
    GetAccesToken();
#else
    emit GetLine();
#endif
    /*Symbol s;
    s.Ask = 0;
    s.Bid = 0;
    s.AskVolume = 0;
    s.BidVolume = 0;
    emit Ticker(s);*/
}

void BCS_Exc::PlaceLimitOrder(
    Instrument &Pair, double price, int volume, Operation op)
{
    PlaceOrder(Pair, price, volume, op,"2");
}

void BCS_Exc::PlaceMarkettOrder(Instrument &Pair, int volume, Operation op)
{
    Order.inst=&Pair;
    Order.volume=volume;
    Order.op=op;
    PlaceOrder(Pair, 0,volume, op,"1");
}

void BCS_Exc::PlaceTakeProvit(Instrument &Pair, double price, double offset, double spread, int volume, Operation op)
{
#ifdef EMULATION
    return;
#endif
    if(volume<0)
        volume*=-1;
    QJsonObject Order;
    QString OpStr;
    Order["clientOrderId"] = QUuid::createUuid().toString(QUuid::WithoutBraces);
    if (op == BUY)
    {
        OpStr="Покупка ";
        Order["side"] = "1";
    }
    else
    {
        OpStr="Продажа ";
        Order["side"] = "2";
    }
    Order["orderType"] = "5";
    Order["orderQuantity"] = volume;
    Order["ticker"] = Pair.GetTicker();
    Order["classCode"] = Pair.GetClassCode();
    Order["takePrice"] = price;
    Order["executionTakeProfitType"] = 2;
    Order["spread"] = spread;
    Order["offset"] = offset;

    QNetworkRequest requestLocal;
    requestLocal.setRawHeader("Accept", "application/json");
    requestLocal.setRawHeader("Content-Type", "application/json");
    requestLocal.setRawHeader("Authorization", ("Bearer " + AccesToken).toUtf8());

    requestLocal.setUrl(QUrl("https://be.broker.ru/trade-api-bff-operations/api/v1/orders"));
    QString Reguest = QJsonDocument(Order).toJson();
    QNetworkReply *rep = Manager[0].post(requestLocal, Reguest.toUtf8());
    connect(rep, &QNetworkReply::finished, this, &BCS_Exc::OrderResult,Qt::QueuedConnection);
    emit MessageEvent(OpStr + Pair.GetTicker());
}

void BCS_Exc::GetAccesToken()
{
    QNetworkRequest RequestLocal;
    RequestLocal.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    RequestLocal.setRawHeader("Accept", "application/json");
    RequestLocal.setUrl(QUrl(
        "https://be.broker.ru/trade-api-keycloak/realms/tradeapi/protocol/openid-connect/token"));
    QString Reguest = "client_id=trade-api-write&grant_type=refresh_token&refresh_token="
                      + RefreshToken;
    QEventLoop LoopForWait;
    QTimer TimeOutTimer;
    QNetworkReply *rep = Manager[0].post(RequestLocal, Reguest.toUtf8());
    connect(rep, &QNetworkReply::finished, &LoopForWait, &QEventLoop::quit);
    connect(&TimeOutTimer, &QTimer::timeout, &LoopForWait, &QEventLoop::quit);
    TimeOutTimer.start(500);
    LoopForWait.exec();
    if (!TimeOutTimer.isActive()) {
        rep->deleteLater();
        emit MessageEvent("❌ Таймаут получения токена");
        QTimer::singleShot(1000, this, &BCS_Exc::GetAccesToken);
        return;
    }
    // ️Сначала проверяем ошибку!
    if (rep->error() != QNetworkReply::NoError) {
        emit MessageEvent("❌ Ошибка получения токена " + rep->errorString());
        rep->deleteLater();
        QTimer::singleShot(1000, this, &BCS_Exc::GetAccesToken);
        return;
    }
    QByteArray ReplayData = rep->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(ReplayData);
    QJsonObject Root = doc.object();
    if (Root.contains("access_token") && Root["access_token"].isString()) {
        AccesToken = Root["access_token"].toString();
        RefreshToken = Root["refresh_token"].toString();
        emit MessageEvent("✅ Новый токен");
        emit ClearCandles();
        QTimer::singleShot(10, this, &BCS_Exc::SocketStartConnection);

    } else {
        emit MessageEvent("❌ Что-то пошло не так " + rep->errorString());
        QTimer::singleShot(1000, this, &BCS_Exc::GetAccesToken);
        return;
    }
    rep->deleteLater();
}

void BCS_Exc::SetRefrashToken(QString Token)
{
    RefreshToken = Token;
}

void BCS_Exc::SetTimeFrame(QString TimeFrame)
{
    _TimeFrame=TimeFrame;
}

void BCS_Exc::GetOrders(Instrument &Pair)
{
    QNetworkRequest requestLocal;
    requestLocal.setRawHeader("Accept", "application/json");
    requestLocal.setRawHeader("Content-Type", "application/json");
    requestLocal.setRawHeader("Authorization", ("Bearer " + AccesToken).toUtf8());

    QJsonObject Order;
    QJsonArray Status;
    Status.append(3);
    QJsonArray orderTypes;
    orderTypes.append(2);
    orderTypes.append(5);
    QJsonArray tickers;
    tickers.append(Pair.GetTicker());
    QJsonArray ClassCode;
    ClassCode.append(Pair.GetClassCode());
    Order["orderStatus"]=Status;
    Order["tickers"]=tickers;
    Order["classCodes"]=ClassCode;
    //Order["orderTypes"]=orderTypes;

    requestLocal.setUrl(QUrl("https://be.broker.ru/trade-api-bff-order-details/api/v1/orders/search"));
    QString Reguest = QJsonDocument(Order).toJson();
    QNetworkReply *rep = Manager[0].post(requestLocal, Reguest.toUtf8());
    rep->setParent(&Pair);
    connect(rep, &QNetworkReply::finished, this, &BCS_Exc::OrderListResult,Qt::QueuedConnection);
}

void BCS_Exc::CancelOrder(QString OrderID)
{
    QNetworkRequest requestLocal;
    requestLocal.setRawHeader("Accept", "application/json");
    requestLocal.setRawHeader("Content-Type", "application/json");
    requestLocal.setRawHeader("Authorization", ("Bearer " + AccesToken).toUtf8());
    QJsonObject Order;
    Order["orderIdType"]="2";
    Order["orderId"]=OrderID;
    Order["clientOrderId"] = QUuid::createUuid().toString(QUuid::WithoutBraces);
    requestLocal.setUrl(QUrl("https://be.broker.ru/trade-api-bff-operations/api/v1/orders/cancel"));
    QString Reguest = QJsonDocument(Order).toJson();
    Manager[0].post(requestLocal, Reguest.toUtf8());
}

void BCS_Exc::OrderResult()
{
    QNetworkReply *rep = qobject_cast<QNetworkReply *>(sender());
    if (!rep)
        return;
    // ️Сначала проверяем ошибку!
    if (rep->error() != QNetworkReply::NoError) {
        qDebug()<<rep->readAll();
        emit MessageEvent("❌ Ошибка выставления ордера " + rep->errorString());
        rep->deleteLater();
        return;
    }
    QByteArray ReplayData = rep->readAll();
    rep->deleteLater();
    QJsonDocument doc = QJsonDocument::fromJson(ReplayData);
    QJsonObject Root = doc.object();
    if (Root.contains("status")) {
        if (Root["status"].toString() == "OK") {
            emit MessageEvent("✅ Выставление ордера: " + Root["status"].toString());
        } else {
            emit MessageEvent("❌ Выставление ордера: " + Root["type"].toString());
        }
    }
}

void BCS_Exc::OrderListResult()
{
    QNetworkReply *rep = qobject_cast<QNetworkReply *>(sender());
    if (!rep)
        return;
    // ️Сначала проверяем ошибку!
    if (rep->error() != QNetworkReply::NoError) {
        qDebug()<<rep->readAll();
        emit MessageEvent("❌ Ошибка выставления ордера " + rep->errorString());
        rep->deleteLater();
        return;
    }
    QByteArray ReplayData = rep->readAll();
    rep->deleteLater();
    QJsonDocument doc = QJsonDocument::fromJson(ReplayData);
    QJsonObject Root = doc.object();
    QJsonArray Orders=Root["records"].toArray();
    QList<MoexOrder> InstOrders;
    for(int i=0;i<Orders.size();i++)
    {
        QString OrderId,Ticker,ClassCode; Operation op;
        double Price,Offset=0,Spread=0; int Side; double remainedQuantityLots;
        int type;
        QJsonObject CurrentOrder=Orders.at(i).toObject();
        OrderId=CurrentOrder["orderId"].toString();
        Ticker=CurrentOrder["ticker"].toString();
        ClassCode=CurrentOrder["classCode"].toString();
        Side=CurrentOrder["side"].toInt();
        if(CurrentOrder["orderType"]==2)
        {
            type=0;
            Price=CurrentOrder["price"].toDouble();
        }
        else
        {
            type=1;
            Price=CurrentOrder["takePrice"].toDouble();
            Offset=CurrentOrder["offset"].toDouble();
            Spread=CurrentOrder["spread"].toDouble();
        }
        if(Side==1)
            op=BUY;
        else
            op=SELL;
        remainedQuantityLots=CurrentOrder["remainedQuantityLots"].toInt();
        MoexOrder newOrder={OrderId,type,Price,Offset,Spread,remainedQuantityLots,op};
        InstOrders.append(newOrder);
    }
    Instrument *inst = qobject_cast<Instrument *>(rep->parent());
    if(inst)
        inst->SetOrderList(InstOrders);
}

void BCS_Exc::SocketStartConnection()
{
    request[0].setRawHeader("Authorization", ("Bearer " + AccesToken).toUtf8());
    request[0].setRawHeader("Accept", "application/json");
    request[0].setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request[0].setUrl(QUrl("wss://ws.broker.ru/trade-api-market-data-connector/api/v1/market-data/ws"));
    request[1].setRawHeader("Authorization", ("Bearer " + AccesToken).toUtf8());
    request[1].setRawHeader("Accept", "application/json");
    request[1].setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request[1].setUrl(QUrl("wss://ws.broker.ru/trade-api-bff-operations/api/v1/orders/execution/ws"));
    Socket.open(request[0]);
    SocketOrders.open(request[1]);
}

void BCS_Exc::PlaceOrder(Instrument &Pair, double price, int volume, Operation op, QString Type)
{
#ifdef EMULATION
    return;
#endif
    if(volume<0)
        volume*=-1;
    QJsonObject Order;
    QString OpStr;
    Order["clientOrderId"] = QUuid::createUuid().toString(QUuid::WithoutBraces);
    if (op == BUY)
    {
        OpStr="Покупка ";
        Order["side"] = "1";
    }
    else
    {
        OpStr="Продажа ";
        Order["side"] = "2";
    }
    Order["orderType"] = Type;
    Order["orderQuantity"] = volume;
    Order["ticker"] = Pair.GetTicker();
    Order["classCode"] = Pair.GetClassCode();
    if(Type=="2")
    {
        Order["price"] = price;
    }
    QNetworkRequest requestLocal;
    requestLocal.setRawHeader("Accept", "application/json");
    requestLocal.setRawHeader("Content-Type", "application/json");
    requestLocal.setRawHeader("Authorization", ("Bearer " + AccesToken).toUtf8());

    requestLocal.setUrl(QUrl("https://be.broker.ru/trade-api-bff-operations/api/v1/orders"));
    QString Reguest = QJsonDocument(Order).toJson();
    QNetworkReply *rep = Manager[0].post(requestLocal, Reguest.toUtf8());
    connect(rep, &QNetworkReply::finished, this, &BCS_Exc::OrderResult,Qt::QueuedConnection);
    emit MessageEvent(OpStr + Pair.GetTicker());
}
