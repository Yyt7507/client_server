#include "chatwin.h"
#include "ui_chatwin.h"

ChatWin::ChatWin(const QString& peerName, QTcpSocket* sock, const QString& zh, QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::ChatWin)
{
    ui->setupUi(this);
    this->peerName = peerName;
    this->sock = sock;
    this->zh = zh;
    connect(ui->pushButton, &QPushButton::clicked, this, &ChatWin::on_pushButton_clicked);
    ui->label->setText(peerName);
    localHistory();
}

ChatWin::~ChatWin()
{
    delete ui;
}

void ChatWin::appendMessage(const QString& fromUser, const QString& msg, const QString& time)
{
    QString userColor = (fromUser == "我") ? "#2a82da" : "#28a745";
    QString senderDisplay = (fromUser == "我") ? "我" : fromUser;
    QString currentTime;
    if(!time.isEmpty())
    {
        currentTime = time;
    }
    else
    {
        currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    }
    QString display = QString("<span style=\"color: %1; font-weight: bold;\">%2</span>"
                              " <span style=\"color: #888; font-size: 10pt;\">%3</span>"
                              "<br/><span style=\"color: #333;\">%4</span>").arg(userColor, senderDisplay, currentTime, msg);
    ui->textEdit->append(display);
}

void ChatWin::localHistory()
{
    QList<ChatRecord> records = ChatDatabase::instance()->loadChatHistory(peerName);
    for(int i = 0; i < records.size(); i++)
    {
        ChatRecord& record = records[i];
        QString displayName = (record.sender == zh) ? "我" : record.sender;
        QString userColor = (record.sender == zh) ? "#2a82da" : "#28a745";
        QString display = QString("<span style=\"color: %1; font-weight: bold;\">%2</span>"
                                  " <span style=\"color: #888; font-size: 10pt;\">%3</span>"
                                  "<br/><span style=\"color: #333;\">%4</span>")
                          .arg(userColor, displayName, record.time, record.message);
        ui->textEdit->append(display);
    }

    if(!records.isEmpty())
    {
        qDebug() << "已加载" << records.size() << "条历史消息";
    }
}

void ChatWin::on_pushButton_clicked()
{
    QString msg = ui->lineEdit->text();
    if(msg.isEmpty())
    {
        return;
    }

    appendMessage("我", msg, "");
    ui->lineEdit->clear();
    ChatDatabase::instance()->saveMessage(zh, peerName, msg);
    QJsonObject chatMsg;
    chatMsg.insert("type", "CHAT");
    chatMsg.insert("to", peerName);
    chatMsg.insert("msg", msg);
    QJsonDocument doc(chatMsg);
    qint64 sent = sock->write(doc.toJson() + "\n");
    qDebug() << "发送消息给：" << peerName << "：" << msg << " 字节数：" << sent;
    sock->flush();
}
