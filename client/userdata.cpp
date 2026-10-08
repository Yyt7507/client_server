#include "userdata.h"

UserData::UserData()
{
    this->isOnline = false;
}

UserData::UserData(QString ip, int port, QString zh, bool isOnline): ip(ip), port(port), zh(zh), isOnline(isOnline)
{

}
