#include "websocketserver.h"

WebSocketServer::WebSocketServer(QObject *parent)
    : QObject{parent}
{
    server = new QWebSocketServer(QStringLiteral("TACTICAL COMMS SERVER"), QWebSocketServer::NonSecureMode, nullptr);

    if (server->listen(QHostAddress::Any, 12345)) {
        qDebug() << "[SYSTEM] WebSocket server active on port 12345";
        connect(server, &QWebSocketServer::newConnection, this, &WebSocketServer::onNewConnection);
    } else {
        qDebug() << "[ERROR] WebSocket server failed to start";
    }
}

WebSocketServer::~WebSocketServer()
{
    for (QWebSocket *client : clients_server) {
        client->close();
        client->deleteLater();
    }
    server->close();
    server->deleteLater();
}

void WebSocketServer::onNewConnection()
{
    QWebSocket *client = server->nextPendingConnection();
    clients_server.append(client);

    connect(client, &QWebSocket::textMessageReceived, this, &WebSocketServer::onUpdate);
    connect(client, &QWebSocket::disconnected, this, &WebSocketServer::onClientDisconnected);

    qDebug() << "[SYSTEM] New operator connected - ID:" << client->peerAddress().toString();
}

void WebSocketServer::onUpdate(const QString &message)
{
    QWebSocket *client = qobject_cast<QWebSocket *>(sender());
    if (client) {
        qDebug() << "[COMMS] Message received from operator:" << message;

        QJsonObject statusUpdate;
        statusUpdate["action"] = "status_update";
        statusUpdate["operator_count"] = clients_server.count();

        QJsonArray operatorArray;
        for (int i = 0; i < clients_server.count(); ++i) {
            QJsonObject operatorInfo;
            operatorInfo["id"] = i + 1;
            operatorInfo["status"] = "ACTIVE";
            operatorInfo["callsign"] = QString("OPERATOR_%1").arg(i + 1);
            operatorArray.append(operatorInfo);
        }
        statusUpdate["operators"] = operatorArray;

        QJsonDocument doc(statusUpdate);
        QString updateMessage = doc.toJson(QJsonDocument::Compact);

        for (QWebSocket *socket : std::as_const(clients_server)) {
            socket->sendTextMessage(updateMessage);
        }
    }
}

void WebSocketServer::onClientDisconnected()
{
    QWebSocket *client = qobject_cast<QWebSocket *>(sender());
    if (client) {
        qDebug() << "[SYSTEM] Operator disconnected - ID:" << client->peerAddress().toString();
        clients_server.removeAll(client);
        client->deleteLater();
    }
}
