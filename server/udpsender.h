#ifndef UDPSENDER_H
#define UDPSENDER_H

#include <QObject>
#include <QThread>
#include <QUdpSocket>
#include <QHostAddress>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QList>
#include <QTcpSocket>
#include "systemmsg.h"

class UdpSender: public QObject
{
    Q_OBJECT
public:
    explicit UdpSender(QObject* parent = nullptr);
    ~UdpSender();

signals:
    void sendFinished();

public slots:
    void sendUserList(const QList<QTcpSocket*>& clientList, const QJsonArray& userArray, const QMap<QTcpSocket*, quint16> &udpMap,
                      const SystemMsg &Offline, const bool hasOffline, const SystemMsg &Online, const bool hasOnline);

private:
    QUdpSocket* m_udpSocket;
};

#endif // UDPSENDER_H
