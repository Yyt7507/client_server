#ifndef CLIENTWIN_H
#define CLIENTWIN_H

#include <QMainWindow>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonDocument>
#include <QTcpSocket>
#include <QSettings>
#include "zhucewin.h"
#include "message.h"
#include <QList>
#include "userdata.h"
#include <QJsonArray>
#include <QJsonParseError>
#include <QUdpSocket>
#include <QMessageBox>
#include <QDebug>
#include <QThread>
#include <QTimer>
#include <QCoreApplication>

namespace Ui
{
class ClientWin;
}

class ClientWin : public QMainWindow
{
    Q_OBJECT

public:
    explicit ClientWin(QWidget* parent = nullptr);
    ~ClientWin();
private slots:
    void on_pushButton_2_clicked();
    void readTCPData();
    void readUDPData();
    void on_pushButton_clicked();
    void connectReadData();
signals:
    void sendZCMsg();
    void sendMsg();

private:
    Ui::ClientWin* ui;
    QTcpSocket tcpsock;
    QUdpSocket udpsock;
    zhucewin* win = nullptr;
    message* msgwin = nullptr;
    QList<UserData>* list = nullptr;
    quint16 udpPort;
    QString zh = "";
    QList<UserData> allUsers;
    QStringList unreadUsers;
};

#endif // CLIENTWIN_H
