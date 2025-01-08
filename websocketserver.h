#ifndef WEBSOCKETSERVER_H
#define WEBSOCKETSERVER_H

#include <QObject>
#include <QWebSocketServer>
#include <QWebSocket>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QDebug>

class WebSocketServer : public QObject {
    Q_OBJECT

public:
    explicit WebSocketServer(QObject *parent = nullptr);
    ~WebSocketServer();

    void broadcastClientList();
    void broadcastCallEvent(const QString &caller, const QString &callee);
    void broadcastMessageEvent(const QString &sender, const QString &receiver, const QString &message);

private slots:
    void onNewConnection();
    void onUpdate(const QString &message);
    void onClientDisconnected();
    void monitorClientList();

private:
    QWebSocketServer *server;
    QList<QWebSocket *> clients_server;
    QStringList lastClientList;
    QTimer *monitorTimer;

    QStringList parseClientsFromPJSIP();
};

#endif // WEBSOCKETSERVER_H
