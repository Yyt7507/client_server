#ifndef CHATWIN_H
#define CHATWIN_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonDocument>
#include <QDateTime>
#include "chatdatabase.h"

namespace Ui
{
class ChatWin;
}

class ChatWin : public QMainWindow
{
    Q_OBJECT

public:
    explicit ChatWin(const QString& peerName, QTcpSocket* sock, const QString& zh, QWidget* parent = nullptr);
    ~ChatWin();
    void appendMessage(const QString& fromUser, const QString& msg, const QString& time);
    void localHistory();
private slots:
    void on_pushButton_clicked();

private:
    Ui::ChatWin* ui;
    QString peerName;
    QTcpSocket* sock;
    QString zh;
};

#endif // CHATWIN_H
