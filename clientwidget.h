#ifndef CLIENTWIDGET_H
#define CLIENTWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <QHBoxLayout>
#include <QGraphicsEllipseItem>
#include <QGraphicsScene>
#include <QGraphicsView>

class ClientWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ClientWidget(const QString &clientName, const QString &dialPlan,
                          const QString &password, const QString &status,
                          QWidget *parent = nullptr);
    void updateStatus(bool available);

private:
    QLabel *clientNameLabel;
    QLabel *statusCircle;
    QComboBox *detailsDropdown;
    void setStatusCircle(bool available);
};

#endif // CLIENTWIDGET_H
