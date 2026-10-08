#ifndef MESSAGE_H
#define MESSAGE_H

#include <QMainWindow>
#include <QList>
#include <QTcpSocket>
#include "userdata.h"
#include <QHostAddress>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonParseError>
#include <QDebug>
#include "systemmsg.h"
#include "chatwin.h"
#include <QListWidgetItem>
#include "usertable.h"

namespace Ui
{
class message;
}

class message : public QMainWindow
{
    Q_OBJECT

public:
    explicit message(QWidget* parent = nullptr, QJsonDocument* doc = nullptr, QTcpSocket* sock = nullptr, QString zh = "");
    ~message();
    void RefreshUserlist();
    void readTCPData();
    void handleJson(QJsonDocument* doc);
    void setAllUsers(const QList<UserData>& allUsers);
    void setUnreadUsers(const QStringList& users);
public slots:
    void connectReadData();
    void on_listWidget_itemDoubleClicked(QListWidgetItem* item);
private:
    Ui::message* ui;
    QJsonDocument* userdoc = nullptr;
    QTcpSocket* socket = nullptr;
    QList<UserData>* list = nullptr;
    QString Onlinezh;
    SystemMsg Offline;
    SystemMsg Online;
    QString zh = "";
    QMap<QString, ChatWin*> chatWinMap;
    QList<UserData>* allUserList;
    QMap<QString, bool> whoUnread;
};

#endif // MESSAGE_H
