#include <QApplication>
#include "MainWindow.h"

// 程序入口：创建并显示主窗口
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
