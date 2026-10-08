#-------------------------------------------------
#
# Project created by QtCreator 2026-06-17T00:56:43
#
#-------------------------------------------------

QT       += core gui network sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = server
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
## 添加头文件路径  spdlog
#INCLUDEPATH += /mnt/hgfs/share/spdlog-1.x/include

## 添加静态库
#LIBS +=/mnt/hgfs/share/spdlog-1.x/build/libspdlog.a

## 如果需要 pthread 支持（spdlog 需要）
#LIBS += -pthread


SOURCES += \
        main.cpp \
        serverwin.cpp \
    systemmsg.cpp \
    userdata.cpp \
    udpsender.cpp

HEADERS += \
        serverwin.h \
    systemmsg.h \
    userdata.h \
    udpsender.h

FORMS += \
        serverwin.ui
