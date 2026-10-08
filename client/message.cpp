#include "message.h"
#include "ui_message.h"

message::message(QWidget* parent, QJsonDocument* doc, QTcpSocket* sock, QString zh) :
    QMainWindow(parent),
    ui(new Ui::message)
{
    ui->setupUi(this);
    this->zh = zh;
    list = new QList<UserData>;
    allUserList = new QList<UserData>;
    handleJson(doc);
    ui->listWidget_2->addItem("系统消息");
    this->socket = sock;

    if(socket != nullptr && socket->state() == QAbstractSocket::ConnectedState)
    {
        ui->label_6->setText(zh);
        ui->label_2->setText(socket->localAddress().toString());
        ui->label_3->setText(QString::number(socket->localPort()));
        qDebug() << "message显示IP：" << socket->peerAddress().toString();
    }
    else
    {
        qDebug() << " message: socket无效或未连接";
        ui->label_2->setText("未连接");
        ui->label_3->setText("0");
    }

    connect(ui->listWidget, &QListWidget::itemDoubleClicked, this, &message::on_listWidget_itemDoubleClicked);
}

message::~message()
{
    delete ui;
}

void message::RefreshUserlist()
{
    qDebug() << "===== RefreshUserlist 开始 =====";
    qDebug() << "allUserList size:" << (allUserList ? allUserList->size() : -1);

    ui->listWidget->clear();
    ui->listWidget->setSpacing(1);  // 设置项目间距
    ui->listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    // 设置 listWidget 样式
    ui->listWidget->setStyleSheet(
                    "QListWidget {"
                    "   background: qlineargradient("
                    "       x1: 0, y1: 0, x2: 1, y2: 1,"
                    "       stop: 0 #FFF8D2,"
                    "       stop: 1 #FFEAA7);"
                    "   border: 1px solid #e0e0e0;"
                    "   border-radius: 4px;"
                    "}"
                    "QListWidget::item {"
                    "   border: none;"
                    "   padding: 0px;"
                    "}"
                    "QListWidget::item:selected {"
                    "   background-color: transparent;"
                    "}"
    );

    if(!allUserList || allUserList->isEmpty())
    {
        qDebug() << "allUserList 为空！";
        QListWidgetItem* emptyItem = new QListWidgetItem("暂无好友");
        emptyItem->setFlags(Qt::NoItemFlags);
        emptyItem->setTextAlignment(Qt::AlignCenter);
        emptyItem->setForeground(QColor("#999999"));
        ui->listWidget->addItem(emptyItem);
        return;
    }

    for(const UserData& user : *allUserList)
    {
        qDebug() << "用户:" << user.zh << "在线:" << user.isOnline;
        UserTable* UT = new UserTable(this);
        if(!UT)
        {
            qDebug() << "UserTable 创建失败！";
            continue;
        }
        QListWidgetItem* item = new QListWidgetItem();
        item->setData(Qt::UserRole, user.zh);  // 使用 UserRole 存储用户名
        item->setSizeHint(QSize(390, 50));

        ui->listWidget->addItem(item);
        //        item->setSizeHint(UT->sizeHint());
        ui->listWidget->setItemWidget(item, UT);
        QString displayText;

        if(user.zh == zh)
        {
            displayText = user.zh + "(" + user.ip + " - " + QString::number(user.port) + ")";
            UT->setUser(displayText);
            UT->setUnread(false);
            UT->setUserColor("blue");
        }
        else if(user.isOnline)
        {
            displayText = user.zh + "(" + user.ip + " - " + QString::number(user.port) + ")";
            UT->setUser(displayText);
            UT->setUserColor("#2e7d32");
            if(whoUnread.contains(user.zh))
            {
                UT->setUnread(true);
            }
        }
        else
        {
            displayText = user.zh + " [离线]";
            UT->setUser(displayText);
            UT->setUserColor("gray");
            if(whoUnread.contains(user.zh))
            {
                UT->setUnread(true);
            }
        }
    }

    if(!Offline.lineIP.isEmpty())
    {
        ui->listWidget_2->addItem(Offline.lineZH + "(" + Offline.lineIP + " - " + QString::number(Offline.linePort) + ")" + "好友已离线！");
    }
    if(!Online.lineIP.isEmpty())
    {
        ui->listWidget_2->addItem(Online.lineZH + "(" + Online.lineIP + " - " + QString::number(Online.linePort) + ")" + "好友已上线！");
    }

    qDebug() << "===== RefreshUserlist 结束 =====";
}

void message::readTCPData()
{
    QByteArray data = socket->readAll();
    qDebug() << "message::readTCPData 收到数据：" << data;

    ui->label_2->setText(socket->localAddress().toString());
    ui->label_3->setText(QString::number(socket->localPort()));

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if(err.error != QJsonParseError::NoError)
    {
        qDebug() << "message: 数据不是json，无法解析";
        return;
    }

    QString type = doc.object().value("type").toString();
    qDebug() << "message::readTCPData 解析到类型：" << type;
    if(type == "DL")
    {
        qDebug() << "message 收到 DL 响应";
        if(doc.object().value("msg").toString() == "success")
        {
            QJsonArray allUsersArray = doc.object().value("allUsers").toArray();
            allUserList->clear();
            for(int i = 0; i < allUsersArray.size(); i++)
            {
                QJsonObject userObj = allUsersArray.at(i).toObject();
                UserData user;
                user.zh = userObj.value("zh").toString();
                user.isOnline = userObj.value("isOnline").toBool();
                user.ip = userObj.value("ip").toString();
                user.port = userObj.value("port").toString().toInt();
                allUserList->append(user);
            }
            qDebug() << "allUserList 已更新，size:" << allUserList->size();

            QJsonArray offlineArray = doc.object().value("offlineMsg").toArray();
            QStringList unreadUsers;
            for(int i = 0; i < offlineArray.size(); i++)
            {
                QJsonObject offlineMsg = offlineArray.at(i).toObject();
                QString fromUser = offlineMsg.value("from").toString();
                QString msg = offlineMsg.value("msg").toString();
                QString time = offlineMsg.value("time").toString();

                ChatDatabase::instance()->saveMessage(fromUser, zh, msg, time);

                if(!unreadUsers.contains(fromUser))
                {
                    unreadUsers.append(fromUser);
                }
            }

            for(const QString& user : unreadUsers)
            {
                whoUnread[user] = true;
            }
            RefreshUserlist();
        }
    }
    else if(type == "CHAT" || type == "CHAT_ERROR" || type == "USER_LIST_UPDATE")
    {
        qDebug() << "准备调用 handleJson";
        handleJson(&doc);
    }
    else
    {
        qDebug() << "未知类型，不处理：" << type;
    }
    qDebug() << "==========================================";
}

void message::handleJson(QJsonDocument* doc)
{
    qDebug() << "handleJson被调用，type：" << doc->object().value("type").toString();
    if(doc->object().value("type").toString() == "USER_LIST_UPDATE")
    {
        list->clear();
        QJsonArray arry = doc->object().value("list").toArray();
        for(int i = 0; i < arry.size(); i++)
        {
            QString ip = arry.at(i).toObject().value("ip").toString();
            int port = arry.at(i).toObject().value("port").toString().toInt();
            QString zh = arry.at(i).toObject().value("zh").toString();
            qDebug() << "ip：" << ip << "    " << "port：" << port  << "zh：" << zh << "size：" << arry.size();
            list->push_back(UserData(ip, port, zh));
        }

        for(int i = 0; i < allUserList->size(); i++)
        {
            (*allUserList)[i].isOnline = false;
        }
        for(int i = 0; i < arry.size(); i++)
        {
            QString zh = arry.at(i).toObject().value("zh").toString();
            bool found = false;
            for(int j = 0; j < allUserList->size(); j++)
            {
                if((*allUserList)[j].zh == zh)
                {
                    (*allUserList)[j].isOnline = true;
                    (*allUserList)[j].ip = arry.at(i).toObject().value("ip").toString();
                    (*allUserList)[j].port = arry.at(i).toObject().value("port").toString().toInt();
                    found = true;
                    break;
                }
            }
        }

        Offline.lineZH = doc->object().value("OfflineZH").toString();
        Offline.lineIP = doc->object().value("OfflineIP").toString();
        Offline.linePort = doc->object().value("OfflinePort").toInt();
        Online.lineZH = doc->object().value("OnlineZH").toString();
        Online.lineIP = doc->object().value("OnlineIP").toString();
        Online.linePort = doc->object().value("OnlinePort").toInt();

        if(Offline.lineZH == zh)
        {
            Offline.lineZH.clear();
            Offline.lineIP.clear();
            Offline.linePort = 0;
        }
        if(Online.lineZH == zh)
        {
            Online.lineZH.clear();
            Online.lineIP.clear();
            Online.linePort = 0;
        }
    }
    if(doc->object().value("type").toString() == "CHAT_ERROR")
    {
        QString msg = doc->object().value("msg").toString();
        qDebug() << msg;
    }
    if(doc->object().value("type").toString() == "CHAT")
    {
        QString fromUser = doc->object().value("from").toString();
        QString msg = doc->object().value("msg").toString();
        QString time = doc->object().value("time").toString();
        ChatDatabase::instance()->saveMessage(fromUser, zh, msg, time);
        if(chatWinMap.contains(fromUser))
        {
            chatWinMap[fromUser]->appendMessage(fromUser, msg, time);
        }
        else
        {
            whoUnread.insert(fromUser, true);
            qDebug() << fromUser << "的新消息，但聊天窗口未打开!";
            RefreshUserlist();
        }
    }
}

void message::setAllUsers(const QList<UserData>& allUsers)
{
    allUserList->clear();
    for(const UserData& user : allUsers)
    {
        allUserList->append(user);
    }
    RefreshUserlist();
}

void message::setUnreadUsers(const QStringList& users)
{
    qDebug() << "setUnreadUsers 被调用，用户列表：" << users;
    for(const QString& user : users)
    {
        whoUnread[user] = true;
        qDebug() << "标记未读：" << user;
    }
    RefreshUserlist();
}

void message::connectReadData()
{
    connect(socket, &QTcpSocket::readyRead, this, &message::readTCPData);
}

void message::on_listWidget_itemDoubleClicked(QListWidgetItem* item)
{
    qDebug() << "双击了列表项";
    UserTable* UT = qobject_cast<UserTable*>(ui->listWidget->itemWidget(item));
    if(!UT)
    {
        qDebug() << "未获取到 UserTable";
        return;
    }

    //    QString text = UT->getText();
    //    QString toUser = text.split("(").first();
    //    toUser = toUser.split(" [").first();
    //    toUser = toUser.trimmed();
    QString toUser = item->data(Qt::UserRole).toString();
    if(toUser.isEmpty())
    {
        qDebug() << "用户名为空";
        return;
    }
    qDebug() << "双击用户:" << toUser;

    if(toUser == zh)
    {
        return;
    }
    if(whoUnread.contains(toUser))
    {
        whoUnread.remove(toUser);
        UT->setUnread(false);
    }
    if(chatWinMap.contains(toUser))
    {
        chatWinMap[toUser]->show();
        chatWinMap[toUser]->raise();
        chatWinMap[toUser]->activateWindow();
        return;
    }
    ChatWin* chatWin = new ChatWin(toUser, socket, zh, this);
    chatWinMap.insert(toUser, chatWin);
    chatWin->show();
    chatWin->raise();
    chatWin->activateWindow();
    qDebug() << "创建聊天窗口：" << toUser;
}
