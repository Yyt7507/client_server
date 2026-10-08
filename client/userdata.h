#ifndef USERDATA_H
#define USERDATA_H
#include <QString>

class UserData
{
public:
    UserData();
    UserData(QString ip, int port, QString zh, bool isOnline = true);
    QString ip = "";
    int port = 0;
    QString zh = "";
    bool isOnline;
};

#endif // USERDATA_H
