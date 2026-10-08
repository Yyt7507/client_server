#include "udpsender.h"
#include <QDebug>

UdpSender::UdpSender(QObject *parent) : QObject(parent)
{
    m_udpSocket = new QUdpSocket(this);
}

UdpSender::~UdpSender()
{
    m_udpSocket->close();
}

void UdpSender::sendUserList(const QList<QTcpSocket*> &onlineList, const QJsonArray &userArray, const QMap<QTcpSocket*, quint16> &udpMap,
                             const SystemMsg &Offline, const bool hasOffline, const SystemMsg &Online, const bool hasOnline)
{
    QJsonObject broadcast;
    broadcast.insert("type", "USER_LIST_UPDATE");
    broadcast.insert("list", userArray);
    if(hasOffline)
    {
        broadcast.insert("OfflineIP", Offline.lineIP);
        broadcast.insert("OfflinePort", Offline.linePort);
        broadcast.insert("OfflineZH", Offline.lineZH);
    }
    if(hasOnline)
    {
        broadcast.insert("OnlineIP", Online.lineIP);
        broadcast.insert("OnlinePort", Online.linePort);
        broadcast.insert("OnlineZH", Online.lineZH);
    }

    QJsonDocument doc(broadcast);
    QByteArray sendData = doc.toJson();
    qDebug() << "UDP发送线程开始，目标客户端数：" << onlineList.size();

    for (int i = 0; i < onlineList.size(); i++)
    {
        QTcpSocket *sock = onlineList.at(i);
        if (sock == nullptr) continue;

        QHostAddress clientIP = sock->peerAddress();
        quint16 clientPort = udpMap.value(sock,0);
        if (clientIP.isNull()) continue;

        qint64 sent = m_udpSocket->writeDatagram(sendData, clientIP, clientPort);
        if (sent == -1)
        {
            qDebug() << "UDP发送给" << clientIP.toString() << "失败：" << m_udpSocket->errorString();
        } else {
            qDebug() << "UDP发送给" << clientIP.toString() << "成功，字节数：" << sent;
        }
    }
    emit sendFinished();
    qDebug() << "UDP发送线程结束";
}
