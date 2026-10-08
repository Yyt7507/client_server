#ifndef SERVERWIN_H
#define SERVERWIN_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QList>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QJsonValue>
#include <QJsonDocument>
#include <QSettings>
#include <QThread>
#include <QMap>
#include <QTimer>
#include <QSqlDatabase>
#include <QSqlQuery>
#include "userdata.h"
#include "udpsender.h"
#include "systemmsg.h"

namespace Ui {
class serverWin;
}

class serverWin : public QMainWindow
{
    Q_OBJECT

public:
    explicit serverWin(QWidget *parent = 0);
    ~serverWin();
    QStringList getAllUsers();

signals:
    void sendUdpList(const QList<QTcpSocket*> &list, const QJsonArray &userArray, const QMap<QTcpSocket*, quint16> &udpPort,
                     const SystemMsg &Offline, const bool hasOffline, const SystemMsg &Online, const bool hasOnline);

private slots:
    void HasNewClientConnect();

    void readdata();

    void SocketDisconnect();

    void broadcastUserList(bool hasOffline, bool hasOnline);

    void onUdpSendFinished();

    void handleChatMessage(QTcpSocket* sock, const QJsonDocument &doc);

private:
    Ui::serverWin *ui;
    QTcpServer server;
    QUdpSocket *udpSocket;
    QMap<QTcpSocket *, quint16>udpPort;
    QList<QTcpSocket *> list;
    QMap<QTcpSocket*, QString> socketToUser;
    QMap<QString, QTcpSocket*> userToSocket;
    QThread *udpThread = nullptr;
    UdpSender *udpSender = nullptr;
    SystemMsg Offline;
    SystemMsg Online;
    QSqlDatabase UserDB;

    bool initDatabase();
    bool checkUser(const QString &userzh, const QString &usermm);
    bool addUser(const QString &userzh, const QString &usermm);
};

#endif // SERVERWIN_H
