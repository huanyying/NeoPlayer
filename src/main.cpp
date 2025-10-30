#include <QApplication>      // Qt 应用程序核心类
#include "MainWindow.h"

int main(int argc, char* argv[]) {
    // qt 主程序
    QApplication a(argc, argv);
    std::setlocale(LC_NUMERIC, "C");
    // qt窗口
    MainWindow w;
    w.show();
    int ret = a.exec();
    w.m_mpv->mpvEventThread->join();
    return ret;
}

// #include "main.moc"
