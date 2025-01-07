#include "clientwidget.h"

ClientWidget::ClientWidget(const QString &clientName, const QString &dialPlan, const QString &password, const QString &status, QWidget *parent)
    : QWidget(parent)
{
    // Client name
    clientNameLabel = new QLabel(clientName, this);
    clientNameLabel->setStyleSheet("QLabel { color: #00ff00; font-weight: bold; }");

    // Status circle
    statusCircle = new QLabel(this);
    setStatusCircle(status == "available");

    // Dropdown for details
    detailsDropdown = new QComboBox(this);
    detailsDropdown->addItem("Dial Plan: " + dialPlan);
    detailsDropdown->addItem("Password: " + password);
    detailsDropdown->addItem("Call Status: " + status);
    detailsDropdown->setStyleSheet("QComboBox { background-color: #2b2b2b; color: #00ff00; border: 1px solid #00ff00; }");

    // Layout
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(statusCircle);
    layout->addWidget(clientNameLabel);
    layout->addWidget(detailsDropdown);
    setLayout(layout);

    // Widget styling
    setStyleSheet("background-color: #1a1a1a; border: 1px solid #00ff00; border-radius: 5px;");
}

void ClientWidget::updateStatus(bool available)
{
    setStatusCircle(available);
}

void ClientWidget::setStatusCircle(bool available)
{
    QString style = available ?
                        "background-color: #00ff00; border: 2px solid #004400; border-radius: 10px;" :
                        "background-color: #ff0000; border: 2px solid #440000; border-radius: 10px;";
    statusCircle->setStyleSheet(style);
    statusCircle->setFixedSize(20, 20);
}
