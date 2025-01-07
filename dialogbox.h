#ifndef DIALOGBOX_H
#define DIALOGBOX_H

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

class CustomDialog : public QDialog
{
    Q_OBJECT
public:
    explicit CustomDialog(QWidget *parent = nullptr);
    QString getUsername() const;
    QString getPassword() const;
    QString getDialplanNumber() const;

private:
    QLineEdit *usernameInput;
    QLineEdit *passwordInput;
    QLineEdit *dialplanInput;
    QLabel *errorLabel;
    QPushButton *okButton;
    QPushButton *cancelButton;

private slots:
    void validateInputs();
};

#endif // DIALOGBOX_H
