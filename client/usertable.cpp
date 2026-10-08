#include "usertable.h"

UserTable::UserTable(QWidget* parent) : QWidget(parent), isHovered(false), isUnread(false)
{
    label = new QLabel(this);
    label->setGeometry(0, 0, 340, 50);
    label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    label->setStyleSheet("QLabel {padding-left: 10px; font-size: 14px; color: #333333;}");

    label_2 = new QLabel(this);
    label_2->setGeometry(360, 15, 20, 20);
    label_2->setStyleSheet("QLabel {background-color: transparent; border-radius: 10px;}");

    // 设置鼠标追踪
    setMouseTracking(true);

    // 设置默认背景色
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor("#ffffff"));
    setPalette(pal);
}

UserTable::~UserTable()
{

}

void UserTable::setUnread(bool isUnread)
{
    this->isUnread = isUnread;
    if(label_2)
    {
        if(isUnread)
        {
            label_2->setStyleSheet("QLabel {background-color: #ff3b30; border-radius: 10px;}");
        }
        else
        {
            label_2->setStyleSheet("QLabel {background-color: transparent; border-radius: 10px;}");
        }
    }
}

void UserTable::setUser(QString text)
{
    if(label)
    {
        label->setText(text);
    }
}

void UserTable::setUserColor(QString color)
{
    if(label)
    {
        label->setStyleSheet(QString("QLabel {padding-left: 10px; font-size: 14px; color: %1; font-weight: bold;}").arg(color));
    }
}

QString UserTable::getText()
{
    if(label)
    {
        return label->text();
    }
    return QString();
}

void UserTable::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QColor bgColor;
    if(isHovered)
    {
        bgColor = QColor("#e3f2fd");  // 鼠标悬停时的浅蓝色
    }
    else
    {
        bgColor = QColor("#ffffff");  // 正常白色背景
    }
    painter.fillRect(rect(), bgColor);

    // 绘制底部分割线
    painter.setPen(QPen(QColor("#e0e0e0"), 1));
    painter.drawLine(0, height() - 1, width(), height() - 1);
}

void UserTable::enterEvent(QEvent* event)
{
    Q_UNUSED(event);
    isHovered = true;
    update();  // 触发重绘
}

void UserTable::leaveEvent(QEvent* event)
{
    Q_UNUSED(event);
    isHovered = false;
    update();  // 触发重绘
}
