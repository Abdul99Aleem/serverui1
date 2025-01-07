#ifndef WEBSOCKETSERVER_H
#define WEBSOCKETSERVER_H

#include <QObject>
#include <QCoreApplication>
#include <QtWebSockets/QWebSocketServer>
#include <QtWebSockets/QWebSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

class WebSocketServer : public QObject
{
    Q_OBJECT

public:
    explicit WebSocketServer(QObject *parent = nullptr);
    ~WebSocketServer();

private:
    QWebSocketServer *server;
    QList<QWebSocket *> clients_server;

private slots:
    void onNewConnection();
    void onUpdate(const QString &message);
    void onClientDisconnected();
};

#endif // WEBSOCKETSERVER_H
