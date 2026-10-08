#ifndef SYSTEMMSG_H
#define SYSTEMMSG_H
#include <QString>

class SystemMsg
{
public:
    SystemMsg();
    SystemMsg(QString lineIP, int linePort, QString lineZH);
    QString lineIP = "";
    int linePort = 0;
    QString lineZH = "";
};

#endif // SYSTEMMSG_H
