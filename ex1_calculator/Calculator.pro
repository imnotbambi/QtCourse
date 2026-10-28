#-------------------------------------------------
# 带键盘事件的计算器 —— Qt 工程文件
# 课程：Qt应用程序开发  实验一
#-------------------------------------------------

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Calculator
TEMPLATE = app

CONFIG += c++11

SOURCES += \
        main.cpp \
        MainWindow.cpp

HEADERS += \
        MainWindow.h

FORMS += \
        mainwindow.ui
