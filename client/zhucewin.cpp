#include "zhucewin.h"
#include "ui_zhucewin.h"
#include <QMessageBox>

zhucewin::zhucewin(QWidget* parent, QTcpSocket* sock) :
    QMainWindow(parent),
    ui(new Ui::zhucewin), socket(sock)
{
    ui->setupUi(this);
}

zhucewin::~zhucewin()
{
    delete ui;
}

void zhucewin::connectReadData()
{
    connect(socket, &QTcpSocket::readyRead, this, &zhucewin::readdata);
}

void zhucewin::on_pushButton_clicked()
{
    //发送注册消息
    QString userzh = ui->lineEdit->text();
    QString usermm = ui->lineEdit_2->text();
    if(userzh == "" || usermm == "")
    {
        return;
    }
    QJsonObject zhucelmsg_obj;
    zhucelmsg_obj.insert("type", "ZC");
    zhucelmsg_obj.insert("userzh", userzh);
    zhucelmsg_obj.insert("usermm", usermm);
    QJsonDocument doc(zhucelmsg_obj);
    socket->write(doc.toJson());
    qDebug() << doc.toJson();
}

void zhucewin::readdata()
{
    QByteArray data =  socket->readAll();
    qDebug() << " zhucewin::readdata   :" << data;
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if(err.error != QJsonParseError::NoError)
    {
        qDebug() << "数据不是json  无法解析";
        return;
    }
    if(doc.object().value("type").toString() == "ZC")
    {
        QString msg = doc.object().value("msg").toString();
        if(msg == "success")
        {
            QMessageBox::information(this, "注册消息", "注册成功");
            this->parentWidget()->show();
            this->hide();
            //解除注册页面的绑定
            disconnect(socket, &QTcpSocket::readyRead, this, &zhucewin::readdata);
            emit sendDLwinMsg();
        }
        else
        {
            QMessageBox::warning(this, "注册消息", "注册失败");
        }

    }
}
