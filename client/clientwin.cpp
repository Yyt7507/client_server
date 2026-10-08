#include "clientwin.h"
#include "ui_clientwin.h"

ClientWin::ClientWin(QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::ClientWin)
{
    ui->setupUi(this);
    //TCP
    QString exePath = QCoreApplication::applicationDirPath();
    QString configPath = exePath + "/ipconfig.ini";
    QSettings set(configPath, QSettings::IniFormat);
    QString serverip = set.value("ip/serverip").toString();
    quint16 port = set.value("port/serverport").toInt();
    qDebug() << "读取到 IP：" << serverip << " 端口：" << port;
    tcpsock.connectToHost(serverip, port);
    connect(&tcpsock, &QTcpSocket::readyRead, this, &ClientWin::readTCPData);
    list = new QList<UserData>;

    //UDP
    bool bindOk = udpsock.bind(QHostAddress::AnyIPv4, 0, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    udpPort = udpsock.localPort();
    qDebug() << "UDP bind结果：" << bindOk << "   端口：" << udpPort;;
    connect(&udpsock, &QUdpSocket::readyRead, this, &ClientWin::readUDPData);
}

ClientWin::~ClientWin()
{
    delete ui;
    delete win;
    delete list;
}
//登录按钮
void ClientWin::on_pushButton_2_clicked()
{
    //发送登录的数据
    QString userzh = ui->lineEdit->text();
    QString usermm = ui->lineEdit_2->text();
    QJsonObject dlmsg_obj;
    dlmsg_obj.insert("type", "DL");
    dlmsg_obj.insert("userzh", userzh);
    dlmsg_obj.insert("usermm", usermm);
    dlmsg_obj.insert("port", udpPort);
    qDebug() << "发送登录请求，UDP 端口：" << udpPort;
    QJsonDocument doc(dlmsg_obj);
    tcpsock.write(doc.toJson());
    zh = userzh;
}

void ClientWin::readTCPData()
{
    QByteArray data = tcpsock.readAll();
    qDebug() << "ClientWin::readTCPData    :   " << data;

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if(err.error != QJsonParseError::NoError)
    {
        qDebug() << "数据不是json  无法解析";
        data.clear();
        return;
    }

    QString type = doc.object().value("type").toString();
    qDebug() << "readTCPData 解析到类型：" << type;

    if(type == "DL")
    {
        if(doc.object().value("msg") == "success")
        {
            if(!ChatDatabase::instance()->initDatabase(zh))
            {
                qDebug() << "数据库初始化失败";
            }

            QJsonArray allUsersArray = doc.object().value("allUsers").toArray();
            QList<UserData> tempAllUsers;
            for(int i = 0; i < allUsersArray.size(); i++)
            {
                QJsonObject userObj = allUsersArray.at(i).toObject();
                UserData user;
                user.zh = userObj.value("zh").toString();
                user.isOnline = userObj.value("isOnline").toBool();
                user.ip = userObj.value("ip").toString();
                user.port = userObj.value("port").toString().toInt();
                tempAllUsers.append(user);
            }
            allUsers = tempAllUsers;

            unreadUsers.clear();
            QJsonArray offlineArray = doc.object().value("offlineMsg").toArray();
            for(int i = 0; i < offlineArray.size(); i++)
            {
                QJsonObject offlineMsg = offlineArray.at(i).toObject();
                QString fromUser = offlineMsg.value("from").toString();
                QString msg = offlineMsg.value("msg").toString();
                QString time = offlineMsg.value("time").toString();

                ChatDatabase::instance()->saveMessage(fromUser, zh, msg, time);
                qDebug() << "收到离线消息：" << fromUser << "：" << msg;
                if(!unreadUsers.contains(fromUser))
                {
                    unreadUsers.append(fromUser);
                }
            }
            if(!offlineArray.isEmpty())
            {
                qDebug() << "共收到" << offlineArray.size() << "条离线消息";
                qDebug() << "未读消息用户：" << unreadUsers;
            }
            QMessageBox::information(this, "登录消息", "登录成功");
        }
        else if(doc.object().value("msg") == "fail")
        {
            zh = "";
            QMessageBox::warning(this, "登录消息", "登录失败");
        }
        else if(doc.object().value("msg") == "already_online")
        {
            QMessageBox::warning(this, "登录消息", "请勿重复登陆！");
        }
    }
    else if(type == "CHAT" || type == "ERROR_CHAT")
    {
        qDebug() << "收到CHAT消息，msgwin = " << msgwin;
        if(msgwin == nullptr)
        {
            msgwin = new message(nullptr, &doc, &tcpsock, zh);
            msgwin->setAllUsers(allUsers);
            connect(this, &ClientWin::sendMsg, msgwin, &message::connectReadData);
            msgwin->show();
        }
        else
        {
            msgwin->handleJson(&doc);
        }
    }
    else
    {
        qDebug() << "未知类型：" << type;
    }

}

void ClientWin::readUDPData()
{
    qDebug() << "UDP readyRead触发了！";
    QByteArray data;
    data.resize(udpsock.pendingDatagramSize());//一开始data缓冲区为0，故resize下一个待读取数据报的字节数
    QHostAddress writerip;
    quint16 writerport;
    qint64 len = udpsock.readDatagram(data.data(), data.size(), &writerip, &writerport);
    qDebug() << "ClientWin::readUDPData   :   " << data;
    if(len == -1)
    {
        qDebug() << "UDP读取失败：" << udpsock.errorString();
        return;
    }
    qDebug() << "UDP收到数据长度：" << len << "内容：" << data;
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if(err.error != QJsonParseError::NoError)
    {
        qDebug() << "数据不是json  无法解析";
        data.clear();
        return;
    }
    if(doc.object().value("type").toString() == "USER_LIST_UPDATE")
    {
        if(msgwin == nullptr)
        {
            disconnect(&tcpsock, &QTcpSocket::readyRead, this, &ClientWin::readTCPData);
            qDebug() << "已断开 ClientWin 的 TCP 接收";
            msgwin = new message(nullptr, &doc, &tcpsock, zh);
            msgwin->setAllUsers(allUsers);
            if(!unreadUsers.isEmpty())
            {
                msgwin->setUnreadUsers(unreadUsers);
                qDebug() << "设置未读用户列表：" << unreadUsers;
            }
            connect(this, &ClientWin::sendMsg, msgwin, &message::connectReadData);
        }
        else
        {
            msgwin->handleJson(&doc);
            msgwin->RefreshUserlist();
        }
        msgwin->show();
        this->hide();

        emit sendMsg();
    }
}

//弹出注册页面
void ClientWin::on_pushButton_clicked()
{
    if(win == nullptr)
    {
        win = new zhucewin(this, &tcpsock);
        connect(this, &ClientWin::sendZCMsg, win, &zhucewin::connectReadData);
        connect(win, &zhucewin::sendDLwinMsg, this, &ClientWin::connectReadData);
    }
    this->hide();
    win->show();
    //由于后续的消息要在注册页面发送和接收   所以要把登录页面的接受消息槽函数解绑
    disconnect(&tcpsock, &QTcpSocket::readyRead, this, &ClientWin::readTCPData);
    emit sendZCMsg();
}

void ClientWin::connectReadData()
{
    connect(&tcpsock, &QTcpSocket::readyRead, this, &ClientWin::readTCPData);
}

