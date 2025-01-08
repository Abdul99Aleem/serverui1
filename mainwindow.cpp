#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QNetworkInterface>
#include <QHostAddress>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("TACTICAL COMMUNICATIONS CONTROL");
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumSize(800, 600);

    // Initialize network socket
    stm32Socket = new QTcpSocket(this);
    connect(stm32Socket, &QTcpSocket::readyRead, this, &MainWindow::handleSTM32Data);
    connectToSTM32();

    // Initialize layouts
    topBox = new QHBoxLayout(this);
    clientBox = new QHBoxLayout(this);
    bottomBox = new QHBoxLayout(this);
    mainLayout = new QVBoxLayout(this);

    // Top box setup
    serverStatusCircle = new QLabel(this);
    serverStatusCircle->setFixedSize(25, 25);
    serverStatusCircle->setStyleSheet("border-radius: 12px; background-color: #00ff00; border: 2px solid #004400;");

    ipAddr = new QLabel("NETWORK: " + getLocalIPAddress(), this);
    ipAddr->setStyleSheet("color: #00ff00; font-weight: bold;");

    themeBtn = new QPushButton("SWITCH VIEW", this);
    exitBtn = new QPushButton("TERMINATE", this);

    QString buttonStyle = "QPushButton { background-color: #2b2b2b; color: #00ff00; "
                          "border: 1px solid #00ff00; padding: 8px; }"
                          "QPushButton:hover { background-color: #00ff00; color: #000000; }";

    themeBtn->setStyleSheet(buttonStyle);
    exitBtn->setStyleSheet(buttonStyle);

    topBox->addWidget(serverStatusCircle);
    topBox->addWidget(ipAddr);
    topBox->addWidget(themeBtn);
    topBox->addWidget(exitBtn);
    topBox->setSpacing(20);
    topBox->setContentsMargins(10, 10, 10, 10);

    // Client list setup
    clientList = new QListWidget(this);
    clientList->setStyleSheet("QListWidget { background-color: #1a1a1a; border: 2px solid #00ff00; }");
    clientBox->addWidget(clientList);

    // Bottom box setup
    addClientBtn = new QPushButton("ADD OPERATOR", this);
    removeClientBtn = new QPushButton("REMOVE OPERATOR", this);

    addClientBtn->setStyleSheet(buttonStyle);
    removeClientBtn->setStyleSheet(buttonStyle);

    connect(addClientBtn, &QPushButton::clicked, this, &MainWindow::on_addClientBtn_clicked);
    connect(exitBtn, &QPushButton::clicked, this, &QApplication::quit);
    connect(removeClientBtn, &QPushButton::clicked, this, [this]() {
        QListWidgetItem *currentItem = clientList->currentItem();
        if (currentItem) {
            delete currentItem;
        }
    });

    bottomBox->addWidget(addClientBtn);
    bottomBox->addWidget(removeClientBtn);
    bottomBox->setSpacing(20);
    bottomBox->setContentsMargins(10, 10, 10, 10);

    // Main layout setup
    mainLayout->addLayout(topBox);
    mainLayout->addLayout(clientBox);
    mainLayout->addLayout(bottomBox);
    mainLayout->setSpacing(15);

    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(mainLayout);
    centralWidget->setStyleSheet("background-color: #1a1a1a;");
    setCentralWidget(centralWidget);

    // Initialize timer for updates
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateClientStatusesAndUI);
    timer->start(5000);

    // Initial client list population
    clients = readClientsFromConfigFiles();
    populateClientList(clients);
}

void MainWindow::monitorPJSIPConfig() {
    // Create timer for monitoring pjsip.conf
    pjsipMonitorTimer = new QTimer(this);
    connect(pjsipMonitorTimer, &QTimer::timeout, this, [this]() {
        QStringList currentClients = parseClientsFromPJSIP();

        // Convert QStringList to QList<ClientData>
        QList<ClientData> clients;
        for (const QString &client : currentClients) {
            clients.append(ClientData{client, "", "", "OFFLINE"});
        }

        populateClientList(clients);
        broadcastClientList();
    });
    pjsipMonitorTimer->start(2000); // Check every 2 seconds
}

QStringList MainWindow::parseClientsFromPJSIP() {
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

void MainWindow::broadcastClientList() {
    QStringList clients = parseClientsFromPJSIP();
    QJsonArray clientArray;
    for (const QString &client : clients) {
        clientArray.append(client);
    }
    QJsonDocument doc(clientArray);
    QString message = doc.toJson();

    for (QTcpSocket* client : clientSockets) {
        if (client->state() == QAbstractSocket::ConnectedState) {
            client->write(message.toUtf8());
        }
    }
}


QString MainWindow::getLocalIPAddress() {
    QString ipAddress;
    const QHostAddress &localhost = QHostAddress(QHostAddress::LocalHost);

    for (const QNetworkInterface &interface : QNetworkInterface::allInterfaces()) {
        if (interface.flags().testFlag(QNetworkInterface::IsRunning) &&
            !interface.flags().testFlag(QNetworkInterface::IsLoopBack)) {
            for (const QNetworkAddressEntry &entry : interface.addressEntries()) {
                const QHostAddress &address = entry.ip();
                if (address.protocol() == QAbstractSocket::IPv4Protocol) {
                    ipAddress = address.toString();
                    return ipAddress;
                }
            }
        }
    }
    return "NO NETWORK";
}


void MainWindow::on_addClientBtn_clicked()
{
    CustomDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString username = dialog.getUsername();
        QString password = dialog.getPassword();
        QString dialplan = dialog.getDialplanNumber();

        if (addClientToPjsipConf(username, password, dialplan)) {
            qDebug() << "[SYSTEM] Operator successfully added.";

            QProcess process;
            process.start("asterisk -rx 'pjsip reload'");
            if (process.waitForFinished()) {
                qDebug() << "[SYSTEM] PJSIP configuration reloaded.";
                qDebug() << process.readAllStandardOutput();
            }

            QProcess process2;
            process2.start("asterisk -rx 'dialplan reload'");
            if (process2.waitForFinished()) {
                qDebug() << "[SYSTEM] Dialplan reloaded.";
                qDebug() << process2.readAllStandardOutput();
            }

            clients = readClientsFromConfigFiles();
            populateClientList(clients);
        } else {
            qDebug() << "[ERROR] Operator addition failed.";
        }
    }
}

bool MainWindow::addClientToPjsipConf(const QString &username, const QString &password, const QString &dialplan)
{
    QFile configFile("/etc/asterisk/pjsip.conf");
    if (!configFile.open(QIODevice::Append | QIODevice::Text)) {
        qDebug() << "[ERROR] Cannot access PJSIP configuration.";
        return false;
    }

    QTextStream out(&configFile);
    out << "\n[" << username << "]\n"
        << "type=endpoint\n"
        << "context=default\n"
        << "disallow=all\n"
        << "allow=ulaw,alaw\n"
        << "aors=" << username << "\n"
        << "auth=" << username << "\n\n"
        << "[" << username << "]\n"
        << "type=auth\n"
        << "auth_type=userpass\n"
        << "username=" << username << "\n"
        << "password=" << password << "\n\n"
        << "[" << username << "]\n"
        << "type=aor\n"
        << "max_contacts=1\n";
    configFile.close();

    QFile extensionsFile("/etc/asterisk/extensions.conf");
    if (!extensionsFile.open(QIODevice::Append | QIODevice::Text)) {
        qDebug() << "[ERROR] Cannot access extensions configuration.";
        return false;
    }

    QTextStream extOut(&extensionsFile);
    extOut << "exten => " << dialplan << ",1,Dial(PJSIP/" << username << ")\n";
    extensionsFile.close();

    return true;
}
QList<ClientData> MainWindow::readClientsFromConfigFiles()
{
    QList<ClientData> clients;
    auto findClientByUsername = [&](const QString &username) -> ClientData* {
        for (ClientData &client : clients) {
            if (client.username == username) return &client;
        }
        return nullptr;
    };

    QFile pjsipFile("/etc/asterisk/pjsip.conf");
    if (pjsipFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&pjsipFile);
        QString currentSection;

        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            QRegularExpression sectionRegex(R"(\[(\w+)\])");
            QRegularExpressionMatch sectionMatch = sectionRegex.match(line);

            if (sectionMatch.hasMatch()) {
                currentSection = sectionMatch.captured(1);
                if (currentSection != "global" && currentSection != "transport-udp") {
                    if (!findClientByUsername(currentSection)) {
                        clients.append(ClientData{currentSection, "", "", "OFFLINE"});
                    }
                }
                continue;
            }

            if (!currentSection.isEmpty()) {
                QRegularExpression passwordRegex(R"(password\s*=\s*(\S+))");
                QRegularExpressionMatch passwordMatch = passwordRegex.match(line);
                if (passwordMatch.hasMatch()) {
                    if (ClientData *client = findClientByUsername(currentSection)) {
                        client->password = passwordMatch.captured(1);
                    }
                }
            }
        }
        pjsipFile.close();
    }

    // Update status via asterisk CLI
    QProcess process;
    process.start("asterisk", QStringList() << "-rx" << "pjsip show contacts");
    if (process.waitForFinished()) {
        QString output = process.readAllStandardOutput();
        QRegularExpression statusRegex(R"((\w+)/(\w+)\s+.*\s+Avail)");
        QRegularExpressionMatchIterator it = statusRegex.globalMatch(output);

        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            QString username = match.captured(2);
            if (ClientData *client = findClientByUsername(username)) {
                client->status = "ONLINE";
            }
        }
    }

    return clients;
}

void MainWindow::populateClientList(const QList<ClientData> &clients)
{
    clientList->clear();
    for (const ClientData &client : clients) {
        ClientWidget *widget = new ClientWidget(
            client.username,
            client.dialplan,
            client.password,
            client.status,
            this
            );
        QListWidgetItem *item = new QListWidgetItem(clientList);
        item->setSizeHint(widget->sizeHint());
        clientList->addItem(item);
        clientList->setItemWidget(item, widget);
    }
}

void MainWindow::updateClientStatuses(QList<ClientData> &clients)
{
    QProcess process;
    process.start("asterisk", QStringList() << "-rx" << "pjsip show contacts");
    if (process.waitForFinished()) {
        QString output = process.readAllStandardOutput();
        for (ClientData &client : clients) {
            client.status = output.contains(client.username) ? "ONLINE" : "OFFLINE";
        }
    }
}

void MainWindow::startPeriodicUpdates()
{
    timer->start(5000);
}

void MainWindow::updateClientStatusesAndUI()
{
    updateClientStatuses(clients);
    populateClientList(clients);
}

void MainWindow::connectToSTM32() {
    stm32Socket = new QTcpSocket(this);
    connect(stm32Socket, &QTcpSocket::readyRead, this, &MainWindow::handleSTM32Data);
    // Replace with your STM32's IP and port
    stm32Socket->connectToHost("192.168.1.x", 8080);
}

void MainWindow::handleSTM32Data() {
    QByteArray data = stm32Socket->readAll();
    QString stm32IP = QString::fromUtf8(data);
    ipAddr->setText("NETWORK: " + stm32IP);
}


void MainWindow::updateSTM32IP() {
    if (stm32Socket->state() == QAbstractSocket::ConnectedState) {
        QString stm32IP = stm32Socket->peerAddress().toString();
        ipAddr->setText("NETWORK: " + stm32IP);
    } else {
        ipAddr->setText("NETWORK: " + getLocalIPAddress());
    }
}

MainWindow::~MainWindow()
{
    delete ui;
    delete timer;
}
