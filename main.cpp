#include "mainwindow.h"
#include "websocketserver.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Set application-wide style
    QApplication::setStyle("Fusion");
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(27, 27, 27));
    darkPalette.setColor(QPalette::WindowText, QColor(0, 255, 0));
    darkPalette.setColor(QPalette::Base, QColor(43, 43, 43));
    darkPalette.setColor(QPalette::AlternateBase, QColor(27, 27, 27));
    darkPalette.setColor(QPalette::Text, QColor(0, 255, 0));
    darkPalette.setColor(QPalette::Button, QColor(27, 27, 27));
    darkPalette.setColor(QPalette::ButtonText, QColor(0, 255, 0));
    QApplication::setPalette(darkPalette);

    MainWindow w;
    w.show();
    WebSocketServer server;

    return a.exec();
}
