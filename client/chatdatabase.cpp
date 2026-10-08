#include "chatdatabase.h"

ChatDatabase* ChatDatabase::m_instance = nullptr;

ChatDatabase::ChatDatabase(QObject* parent): QObject(parent)
{

}

ChatDatabase::~ChatDatabase()
{
    closeDatabase();
}

ChatDatabase* ChatDatabase::instance()
{
    if(m_instance == nullptr)
    {
        m_instance = new ChatDatabase;
    }
    return m_instance;
}

bool ChatDatabase::initDatabase(const QString& currentUser)
{
    if(m_db.isOpen())
    {
        closeDatabase();
    }
    m_currentUser = currentUser;
    m_connectionName = "ChatConnection_" + currentUser;
    if(QSqlDatabase::contains(m_connectionName))
    {
        m_db = QSqlDatabase::database(m_connectionName);
    }
    else
    {
        m_db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
    }

    QString Path = QCoreApplication::applicationDirPath() + "/chatdata";
    QDir dir(Path);
    qDebug() << "数据库地址：" << Path;
    QString userDBPath = Path + "/" + currentUser;
    dir.mkpath(userDBPath);
    QString dbFile = userDBPath + "/ChatHistory.db";
    m_db.setDatabaseName(dbFile);

    if(!m_db.open())
    {
        qDebug() << currentUser << "的聊天记录数据库打开失败";
        return false;
    }
    qDebug() << currentUser << "的聊天记录数据库成功打开";
    return true;
}

void ChatDatabase::closeDatabase()
{
    if(m_db.isOpen())
    {
        m_db.close();
    }
    m_currentUser.clear();
    m_connectionName.clear();
}

void ChatDatabase::saveMessage(const QString& sender, const QString& receiver, const QString& message, const QString& timestamp)
{
    if(!m_db.isOpen())
    {
        qDebug() << "数据库未打开，无法保存消息";
        return;
    }
    QString peerUser = (sender == m_currentUser) ? receiver : sender;
    createTable(peerUser);
    QString tableName = getTableName(peerUser);

    QSqlQuery query(m_db);

    if(timestamp.isEmpty())
    {
        query.prepare(QString("INSERT INTO %1 (sender, message) VALUES (?, ?)").arg(tableName));
        query.addBindValue(sender);
        query.addBindValue(message);
    }
    else
    {
        query.prepare(QString("INSERT INTO %1 (sender, message, timestamp) VALUES (?, ?, ?)").arg(tableName));
        query.addBindValue(sender);
        query.addBindValue(message);
        query.addBindValue(timestamp);
    }

    if(!query.exec())
    {
        qDebug() << "保存消息失败";
    }
    else
    {
        qDebug() << "消息已保存，用户：" << m_currentUser << "，表：" << tableName;
    }
}

QList<ChatRecord> ChatDatabase::loadChatHistory(const QString& peerUser)
{
    QList<ChatRecord> records;
    if(!m_db.isOpen())
    {
        qDebug() << "数据库未打开，无法加载历史记录";
        return records;
    }

    createTable(peerUser);
    QString tableName = getTableName(peerUser);

    QSqlQuery query(m_db);
    QString selectSQL = QString("SELECT sender, message, timestamp FROM %1 ORDER BY id ASC").arg(tableName);
    if(!query.exec(selectSQL))
    {
        qDebug() << "加载聊天记录失败";
        return records;
    }

    while(query.next())
    {
        ChatRecord record;
        record.sender = query.value(0).toString();
        record.message = query.value(1).toString();
        record.time = query.value(2).toString();
        records.append(record);
    }
    qDebug() << "用户" << m_currentUser << "加载了与" << peerUser << "的" << records.size() << "条聊天记录";
    return records;
}

QString ChatDatabase::getTableName(const QString& peerUser)
{
    return "ChatWith_" + peerUser;
}

void ChatDatabase::createTable(const QString& peerUser)
{
    QString tableName = getTableName(peerUser);
    QSqlQuery query(m_db);
    QString createSQL = QString("CREATE TABLE IF NOT EXISTS %1 ("
                                "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                "sender TEXT NOT NULL,"
                                "message TEXT NOT NULL,"
                                "timestamp DATETIME DEFAULT (datetime('now','localtime')))").arg(tableName);

    if(!query.exec(createSQL))
    {
        qDebug() << "创建聊天记录表失败";
    }
    else
    {
        qDebug() << "聊天记录表已创建-" << tableName;
    }
}
