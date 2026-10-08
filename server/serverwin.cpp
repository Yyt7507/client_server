#include "serverwin.h"
#include "ui_serverwin.h"

serverWin::serverWin(QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::serverWin)
{
    ui->setupUi(this);
    if(!initDatabase())
    {
        qDebug()<<"数据库初始化失败，程序关闭";
        return;
    }

    //TCP
    server.listen(QHostAddress::AnyIPv4, 9999);
    connect(&server, &QTcpServer::newConnection, this, &serverWin::HasNewClientConnect);

    //UDP线程
    udpSocket = new QUdpSocket(this);
    udpThread = new QThread(this);
    udpSender = new UdpSender();
    udpSender->moveToThread(udpThread);
    connect(udpSender, &UdpSender::sendFinished, this, &serverWin::onUdpSendFinished);
    connect(this, &serverWin::sendUdpList, udpSender, &UdpSender::sendUserList);
    udpThread->start();
    qDebug() << "UDP发送线程已启动";
}

serverWin::~serverWin()
{
    if(UserDB.isOpen())
    {
        UserDB.close();
    }
    delete ui;
}

QStringList serverWin::getAllUsers()
{
    QStringList users;
    QSqlQuery query;
    query.exec("SELECT userzh FROM UserMsg");
    while (query.next())
    {
        users.append(query.value(0).toString());
    }
    return users;
}

void serverWin::HasNewClientConnect()
{
    qDebug() << "lianjie";
    QTcpSocket* sock = server.nextPendingConnection();
    connect(sock, &QTcpSocket::readyRead, this, &serverWin::readdata);
    connect(sock, &QTcpSocket::disconnected, this, &serverWin::SocketDisconnect);
}

void serverWin::SocketDisconnect()
{
    QTcpSocket* sock = (QTcpSocket*)sender();
    Offline.lineIP=sock->peerAddress().toString();
    Offline.linePort=QString::number(sock->peerPort()).toInt();
    Offline.lineZH=socketToUser.value(sock, "未知用户");
    udpPort.remove(sock);
    socketToUser.remove(sock);
    userToSocket.remove(Offline.lineZH);
    int index=list.indexOf(sock);
    if(index != -1)
    {
        list.takeAt(index);
    }
    else
    {
        qDebug() << "有客户端断连，但是没有在链表中找到该客户端！！！";
    }
    qDebug() << "客户断开，当前在线人数：" << list.size();
    broadcastUserList(true, false);
}

void serverWin::readdata()
{
    QTcpSocket* sock = (QTcpSocket*)sender();

    QByteArray  data = sock->readAll();
    qDebug() << "收到TCP数据：" << data;
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if(err.error != QJsonParseError::NoError)
    {
        qDebug() << "数据不是json  无法解析";
        return;
    }
    QJsonObject  return_obj;
    if(doc.object().value("type").toString() == "DL")
    {
        return_obj.insert("type", "DL");
        QString userzh = doc.object().value("userzh").toString();
        QString usermm = doc.object().value("usermm").toString();
        if(!checkUser(userzh, usermm))
        {
            return_obj.insert("msg", "fail");
            QJsonDocument doc(return_obj);
            sock->write(doc.toJson());
        }
        else
        {
            if(userToSocket.contains(userzh))
            {
                return_obj.insert("msg", "already_online");
                QJsonDocument doc(return_obj);
                sock->write(doc.toJson());
                return;
            }
            if(!list.contains(sock))
            {
                list.push_back(sock);


            }            
            return_obj.insert("msg", "success");
            udpPort.insert(sock, doc.object().value("port").toInt());
            socketToUser.insert(sock, userzh);
            userToSocket.insert(userzh, sock);
            qDebug() << "客户端UDP端口：" << udpPort;

            QJsonArray offlineArray;
            QSqlQuery query(UserDB);
            query.prepare("SELECT id, from_user, message, timestamp FROM OfflineMsg WHERE to_user = ? AND delivered = 0 ORDER BY id ASC");
            query.addBindValue(userzh);
            if(!query.exec())
            {
                qDebug() << "查询离线消息失败";
            }
            else
            {
                QList<int> idsToDelete;
                while(query.next())
                {
                    int msgId = query.value(0).toInt();
                    QJsonObject offlineMsg;
                    offlineMsg.insert("from", query.value(1).toString());
                    offlineMsg.insert("msg", query.value(2).toString());
                    offlineMsg.insert("time", query.value(3).toString());
                    offlineArray.append(offlineMsg);

                    idsToDelete.append(msgId);
                }

                if(!idsToDelete.isEmpty())
                {
                    for(int msgId : idsToDelete)
                    {
                        QSqlQuery deleteQuery(UserDB);
                        deleteQuery.prepare("DELETE FROM OfflineMsg WHERE id = ?");
                        deleteQuery.addBindValue(msgId);
                        if(!deleteQuery.exec())
                        {
                            qDebug() << "删除离线消息失败，id：" << msgId;
                        }
                    }
                    qDebug() << "登录响应中携带" << offlineArray.size() << "条离线消息，已从数据库删除";
                }
            }

            if(!offlineArray.isEmpty())
            {
                return_obj.insert("offlineMsg", offlineArray);
                qDebug() << "登录响应中携带" << offlineArray.size() << "条离线消息";
            }

            QStringList allUsers = getAllUsers();
            QJsonArray allUserArray;
            for (const QString& user : allUsers)
            {
                QJsonObject userObj;
                userObj.insert("zh", user);
                bool isOnline = userToSocket.contains(user);
                userObj.insert("isOnline", isOnline);
                if (isOnline)
                {
                    QTcpSocket* userSock = userToSocket.value(user);
                    userObj.insert("ip", userSock->peerAddress().toString());
                    userObj.insert("port", QString::number(userSock->peerPort()));
                }
                else
                {
                    userObj.insert("ip", "");
                    userObj.insert("port", "0");
                }
                allUserArray.append(userObj);
            }
            return_obj.insert("allUsers", allUserArray);

            QJsonDocument doc(return_obj);
            sock->write(doc.toJson());
            Online.lineIP=sock->peerAddress().toString();
            Online.linePort=QString::number(sock->peerPort()).toInt();
            Online.lineZH=userzh;
            broadcastUserList(false, true);
        }
    }
    if(doc.object().value("type").toString() == "ZC")
    {
        return_obj.insert("type", "ZC");
        QString userzh = doc.object().value("userzh").toString();
        QString usermm = doc.object().value("usermm").toString();
        if(addUser(userzh, usermm))//注册成功
        {
            return_obj.insert("msg", "success");
        }
        else//注册失败
        {
            return_obj.insert("msg", "fail");
        }
        QJsonDocument doc(return_obj);
        sock->write(doc.toJson());
        qDebug() << doc.toJson();
    }
    if(doc.object().value("type").toString()=="CHAT")
    {
        handleChatMessage(sock, doc);
    }
}

void serverWin::broadcastUserList(bool hasOffline, bool hasOnline)
{
    //发送给所有在线用户现在的在线用户链表list
    QJsonArray userArry;
    for(int i = 0; i < list.size(); i++)
    {
        if(list.at(i)->peerAddress().isNull() || list.at(i)->peerPort() == 0)
        {
            continue;
        }
        QJsonObject userObj;
        userObj.insert("ip", list.at(i)->peerAddress().toString());
        userObj.insert("port", QString::number(list.at(i)->peerPort()));
        userObj.insert("zh", socketToUser.value(list.at(i), "未知用户"));
        qDebug() << "port: " << QString::number(list.at(i)->peerPort());
        userArry.push_back(userObj);
    }

    qDebug() << "UDP发送线程，当前在线人数：" << list.size();
    emit sendUdpList(list, userArry, udpPort, Offline, hasOffline, Online, hasOnline);//线程信号
}

void serverWin::onUdpSendFinished()
{
    qDebug() << "UDP发送完成！！";
}

void serverWin::handleChatMessage(QTcpSocket *sock, const QJsonDocument &doc)
{
    QString fromUser = socketToUser.value(sock, "未知用户");
    QString toUser = doc.object().value("to").toString();
    QString msg = doc.object().value("msg").toString();
    qDebug()<<"聊天消息：" << fromUser << " → " << toUser << "：" << msg;

    QTcpSocket* targetSock = userToSocket.value(toUser, nullptr);
    //接收方不在线
    if(targetSock==nullptr)
    {
        qDebug()<<"用户不在线："<<toUser;
        QSqlQuery query(UserDB);
        query.prepare("INSERT INTO OfflineMsg (from_user, to_user, message) VALUES (?, ?, ?)");
        query.addBindValue(fromUser);
        query.addBindValue(toUser);
        query.addBindValue(msg);
        if(!query.exec())
        {
            qDebug() << "离线消息存储失败";
        }
        else
        {
            qDebug() << "离线消息已存储：" << fromUser << " → " << toUser;
        }

        QJsonObject reply;
        reply.insert("type", "CHAT_ERROR");
        reply.insert("msg", "用户 " + toUser + " 不在线，消息已暂存");
        QJsonDocument doc_fromUser(reply);
        sock->write(doc_fromUser.toJson() + "\n");
        return;
    }

    QJsonObject forward;
    forward.insert("type", "CHAT");
    forward.insert("from", fromUser);
    forward.insert("msg", msg);
    QJsonDocument doc_toUser(forward);
    targetSock->write(doc_toUser.toJson() + "\n");
    qDebug()<<"消息已转发";

}

bool serverWin::initDatabase()
{
    UserDB=QSqlDatabase::addDatabase("QSQLITE");
    UserDB.setDatabaseName("UserDB.db");
    if(!UserDB.open())
    {
        qDebug()<<"数据库打开失败，请检查！";
        return false;
    }

    QSqlQuery userQuery;
    QString createUserTable = "CREATE TABLE IF NOT EXISTS UserMsg "
                              "(id INTEGER PRIMARY KEY AUTOINCREMENT,userzh TEXT UNIQUE NOT NULL,"
                              "usermm TEXT NOT NULL,created_at DATETIME DEFAULT CURRENT_TIMESTAMP);";
    if(!userQuery.exec(createUserTable))
    {
        qDebug()<<"表格创建失败！";
        return false;
    }

    QSqlQuery offlineQuery;
    QString createOfflineTable = "CREATE TABLE IF NOT EXISTS OfflineMsg "
                                 "(id INTEGER PRIMARY KEY AUTOINCREMENT,from_user TEXT NOT NULL,"
                                 "to_user TEXT NOT NULL,message TEXT NOT NULL,"
                                 "timestamp DATETIME DEFAULT (datetime('now','localtime')),"
                                "delivered INTEGER DEFAULT 0)";
    if(!offlineQuery.exec(createOfflineTable))
    {
        qDebug()<<"离线消息表创建失败！";
        return false;
    }

    qDebug() << "数据库初始化成功";

    QSqlQuery countQuery;
    countQuery.exec("SELECT COUNT(*) FROM UserMsg");
    if(countQuery.next() && countQuery.value(0).toInt()==0)
    {
        qDebug() << "添加默认测试用户";
        addUser("jake", "123456");
        addUser("tom", "9876521");
        addUser("rose", "555555");
    }
    return true;
}

bool serverWin::checkUser(const QString &userzh, const QString &usermm)
{
    QSqlQuery query;
    query.prepare("SELECT * FROM UserMsg WHERE userzh = ? AND usermm = ?");
    query.addBindValue(userzh);
    query.addBindValue(usermm);
    if(!query.exec())
    {
        qDebug() << "查询失败：";
        return false;
    }
    return query.next();
}

bool serverWin::addUser(const QString &userzh, const QString &usermm)
{
    QSqlQuery query;
    query.prepare("INSERT INTO UserMsg (userzh, usermm) VALUES (?, ?)");
    query.addBindValue(userzh);
    query.addBindValue(usermm);
    if(!query.exec())
    {
        qDebug() << "注册失败";
        return false;
    }
    qDebug() << "注册成功";
    return true;
}
