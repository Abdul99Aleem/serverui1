#include "websocketserver.h"
#include <QWebSocket>
#include <QWebSocketServer>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QFile>
#include <QTextStream>


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

    // Start monitoring client list
    monitorTimer = new QTimer(this);
    connect(monitorTimer, &QTimer::timeout, this, &WebSocketServer::monitorClientList);
    monitorTimer->start(2000); // Check every 2 seconds
}

void WebSocketServer::broadcastClientList() {
    QStringList clients = parseClientsFromPJSIP();
    QJsonArray clientArray;
    for (const QString &client : clients) {
        clientArray.append(client);
    }
    QJsonDocument doc(clientArray);
    QString message = doc.toJson();

    for (QWebSocket *client : clients_server) {
        if (client->state() == QAbstractSocket::ConnectedState) {
            client->sendTextMessage(message);
        } else {
            clients_server.removeAll(client);
            client->deleteLater();
        }
    }
}

void WebSocketServer::monitorClientList() {
    QStringList currentClients = parseClientsFromPJSIP();
    if (currentClients != lastClientList) {
        lastClientList = currentClients;
        broadcastClientList();
    }
}

QStringList WebSocketServer::parseClientsFromPJSIP() {
    QStringList clients;
    QFile file("/etc/asterisk/pjsip.conf");

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString currentSection;

        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();

            if (line.startsWith("[") && line.endsWith("]")) {
                currentSection = line.mid(1, line.length() - 2);
                if (!currentSection.startsWith("transport") &&
                    !currentSection.startsWith("global")) {
                    clients.append(currentSection);
                }
            }
        }
        file.close();
    }
    return clients;
}

void WebSocketServer::onNewConnection() {
    QWebSocket *client = server->nextPendingConnection();
    clients_server.append(client);

    connect(client, &QWebSocket::textMessageReceived, this, &WebSocketServer::onUpdate);
    connect(client, &QWebSocket::disconnected, this, &WebSocketServer::onClientDisconnected);

    qDebug() << "[SYSTEM] New operator connected - ID:" << client->peerAddress().toString();
}

void WebSocketServer::onUpdate(const QString &message) {
    QWebSocket *client = qobject_cast<QWebSocket *>(sender());
    if (client) {
        qDebug() << "[COMMS] Message received from operator:" << message;

        // Broadcast the message to all clients
        for (QWebSocket *socket : clients_server) {
            if (socket != client) {
                socket->sendTextMessage(message);
            }
        }
    }
}

void WebSocketServer::onClientDisconnected() {
    QWebSocket *client = qobject_cast<QWebSocket *>(sender());
    if (client) {
        qDebug() << "[SYSTEM] Operator disconnected - ID:" << client->peerAddress().toString();
        clients_server.removeAll(client);
        client->deleteLater();
    }
}



WebSocketServer::~WebSocketServer() {
    for (QWebSocket *client : clients_server) {
        client->close();
        client->deleteLater();
    }
    server->close();
    server->deleteLater();
}
