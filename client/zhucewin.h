#ifndef ZHUCEWIN_H
#define ZHUCEWIN_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonDocument>
namespace Ui {
class zhucewin;
}

class zhucewin : public QMainWindow
{
    Q_OBJECT

public:
    explicit zhucewin(QWidget *parent = nullptr,QTcpSocket *sock = nullptr);
    ~zhucewin();

public slots:
    void connectReadData();
private slots:
    void on_pushButton_clicked();

    void readdata();
signals:
    void sendDLwinMsg();
private:
    Ui::zhucewin *ui;
    QTcpSocket *socket = nullptr;
};

#endif // ZHUCEWIN_H
