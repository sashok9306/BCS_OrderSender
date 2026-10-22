#ifndef MOEXEXCHANGECONNECTOR_H
#define MOEXEXCHANGECONNECTOR_H
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QWebSocket>
#include <Instrument.h>
class StockConnector : public QObject
{
    Q_OBJECT

public:
    StockConnector()
    {
        connect(&Socket, &QWebSocket::connected, this, &StockConnector::SocketConnected,Qt::QueuedConnection);
        connect(&Socket, &QWebSocket::disconnected, this, &StockConnector::Disconnected,Qt::QueuedConnection);
        connect(&Socket, &QWebSocket::textFrameReceived, this, &StockConnector::DataFromEx,Qt::QueuedConnection);

        connect(&SocketOrders, &QWebSocket::connected, this, &StockConnector::SocketOrdersConnected,Qt::QueuedConnection);
        connect(&SocketOrders, &QWebSocket::disconnected, this, &StockConnector::Disconnected,Qt::QueuedConnection);
        connect(&SocketOrders, &QWebSocket::textFrameReceived, this, &StockConnector::DataFromExOrders,Qt::QueuedConnection);

        connect(&Socket, &QWebSocket::pong, this, &StockConnector::Pong,Qt::QueuedConnection);

        connect(&Socket, &QWebSocket::aboutToClose, this, &StockConnector::aboutToClose);
        connect(&Socket, &QWebSocket::alertReceived, this, &StockConnector::alertReceived);
        connect(&Socket, &QWebSocket::alertSent, this, &StockConnector::alertSent);
        connect(&Socket,
                &QWebSocket::authenticationRequired,
                this,
                &StockConnector::authenticationRequired);
        connect(&Socket, &QWebSocket::errorOccurred, this, &StockConnector::errorOccurred);
        connect(&Socket,
                &QWebSocket::handshakeInterruptedOnError,
                this,
                &StockConnector::handshakeInterruptedOnError);
        connect(&Socket, &QWebSocket::peerVerifyError, this, &StockConnector::peerVerifyError);
        connect(&Socket,
                &QWebSocket::readChannelFinished,
                this,
                &StockConnector::readChannelFinished);
        connect(&Socket, &QWebSocket::sslErrors, this, &StockConnector::sslErrors);
    }
private slots:
    virtual void SocketConnected() = 0;
    virtual void SocketOrdersConnected() = 0;
    virtual void DataFromEx(QString Data, bool last) = 0;
    virtual void DataFromExOrders(QString Data, bool last) = 0;
    virtual void Connect() = 0;
    void Disconnected()
    {
        emit MessageEvent("⚠️ОТКЛЮЧЕНО");
        Connect();
    }
    void aboutToClose() { emit MessageEvent("⚠️Вот вот закроемся"); }
    void Pong(quint64 elapsedTime, const QByteArray payload){Q_UNUSED(payload);emit MessageEvent("Задержка "+QString::number(elapsedTime)+" мс");}
    void alertReceived(QSsl::AlertLevel level, QSsl::AlertType type, const QString &description)
    {
        Q_UNUSED(level)
        Q_UNUSED(type)
        emit MessageEvent("⚠️Получено предупреждение " + description);
    }
    void alertSent(QSsl::AlertLevel level, QSsl::AlertType type, const QString &description)
    {
        Q_UNUSED(level)
        Q_UNUSED(type)
        emit MessageEvent("⚠️Отправлено предупреждение " + description);
    }
    void authenticationRequired(QAuthenticator *authenticator)
    {
        Q_UNUSED(authenticator)
        emit MessageEvent("⚠️Сервер просит аутентификацию");
    }
    void errorOccurred(QAbstractSocket::SocketError error)
    {
        emit MessageEvent("❌ Получена ошибка " + QString::number(error));
    }
    void handshakeInterruptedOnError(const QSslError &error)
    {
        emit MessageEvent("❌ Ошибка сертификата " + error.errorString());
    }
    void peerVerifyError(const QSslError &error)
    {
        emit MessageEvent("❌ Ошибка верификации " + error.errorString());
    }
    void readChannelFinished() { emit MessageEvent("⚠️Закрываем канал"); }
    void sslErrors(const QList<QSslError> &errors)
    {
        for (int i = 0; i < errors.size(); i++)
            emit MessageEvent("❌ Ошибка SSL " + errors.at(i).errorString());
    }

public slots:
    void ConnectToEX(QMap<QString, Instrument *> &Instrumments)
    {
        _Instrumments = Instrumments;
        Connect();
    }
    virtual void PlaceLimitOrder(Instrument &Pair, double price, int volume, Operation op)
        = 0;
    virtual void PlaceMarkettOrder(Instrument &Pair, int volume, Operation op)
        = 0;
signals:
    //void Ticker(Symbol);
    void MessageEvent(QString);

protected:
    QWebSocket Socket,SocketOrders;
    QNetworkAccessManager Manager[3];
    QMap<QString, Instrument *> _Instrumments;
};
#endif // MOEXEXCHANGECONNECTOR_H
