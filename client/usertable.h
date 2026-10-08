#ifndef USERTABLE_H
#define USERTABLE_H

#include <QWidget>
#include <QLabel>
#include <QPainter>
#include <QHBoxLayout>
#include <QEvent>
#include <QDebug>

class UserTable : public QWidget
{
    Q_OBJECT

public:
    explicit UserTable(QWidget* parent = nullptr);
    ~UserTable();
    void setUnread(bool isUnread);
    void setUser(QString text);
    void setUserColor(QString color);
    QString getText();

    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    QLabel* label;
    QLabel* label_2;
    bool isHovered;
    bool isUnread;
};

#endif // USERTABLE_H
