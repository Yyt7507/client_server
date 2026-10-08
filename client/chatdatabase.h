#ifndef CHATDATABASE_H
#define CHATDATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QList>
#include <QDebug>
#include <QDir>
#include <QCoreApplication>

struct ChatRecord
{
    QString sender;
    QString receiver;
    QString message;
    QString time;
};

class ChatDatabase: public QObject
{
    Q_OBJECT
private:
    explicit ChatDatabase(QObject* parent = nullptr);
    ~ChatDatabase();

    static ChatDatabase* m_instance;
    QSqlDatabase m_db;
    QString m_currentUser;
    QString m_connectionName;
public:
    static ChatDatabase* instance();
    bool initDatabase(const QString& currentUser);
    void closeDatabase();
    void saveMessage(const QString& sender, const QString& receiver, const QString& message, const QString& timestamp = "");
    QList<ChatRecord> loadChatHistory(const QString& peerUser);
    QString currentUser() const
    {
        return m_currentUser;
    }
    QString getTableName(const QString& peerUser);
    void createTable(const QString& peerUser);
};

#endif // CHATDATABASE_H
