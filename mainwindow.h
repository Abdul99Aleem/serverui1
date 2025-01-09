#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLayout>
#include <QPushButton>
#include <QListWidget>
#include <QListWidgetItem>
#include <QLabel>
#include <QComboBox>
#include <QDebug>
#include <QInputDialog>
#include <QFile>
#include <QRegularExpression>
#include <QProcess>
#include <QTimer>
#include "dialogbox.h"
#include "clientwidget.h"
#include <QTcpSocket>
#include <QNetworkInterface>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

struct ClientData {
    QString username;
    QString password;
    QString dialplan;
    QString status;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QString getLocalIPAddress();
    void on_addClientBtn_clicked();
    bool addClientToPjsipConf(const QString &username, const QString &password, const QString &dialplan);
    QList<ClientData> readClientsFromConfigFiles();
    void populateClientList(const QList<ClientData> &clients);
    void updateClientStatuses(QList<ClientData> &clients);
    void startPeriodicUpdates();

private slots:
    void updateSTM32IP();
    void updateClientStatusesAndUI();
    void showNotificationDialog();
    void broadcastNotification(const QString& message);

private:
    Ui::MainWindow *ui;
    QHBoxLayout *topBox;
    QHBoxLayout *clientBox;
    QHBoxLayout *bottomBox;
    QVBoxLayout *mainLayout;
    QLabel *serverStatusCircle;
    QLabel *ipAddr;
    QPushButton* notifyBtn;
    QPushButton *exitBtn;
    QListWidget *clientList;
    QList<ClientData> clients;
    QPushButton *addClientBtn;
    QPushButton *removeClientBtn;
    QTimer *timer;
    QTcpSocket *stm32Socket;
    void connectToSTM32();
    void handleSTM32Data();

    QList<QTcpSocket*> clientSockets;
    void monitorPJSIPConfig();
    void broadcastClientList();
    QStringList parseClientsFromPJSIP();
    QTimer *pjsipMonitorTimer;
};

#endif // MAINWINDOW_H
