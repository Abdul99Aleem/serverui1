#include "dialogbox.h"

CustomDialog::CustomDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Military Secure Access");
    setStyleSheet("QDialog { background-color: #1a1a1a; color: #00ff00; }");

    // Create input fields
    QLabel *usernameLabel = new QLabel("CALLSIGN:", this);
    usernameInput = new QLineEdit(this);

    QLabel *passwordLabel = new QLabel("ACCESS CODE:", this);
    passwordInput = new QLineEdit(this);
    passwordInput->setEchoMode(QLineEdit::Password);

    QLabel *dialplanLabel = new QLabel("TACTICAL NUMBER:", this);
    dialplanInput = new QLineEdit(this);

    // Error label
    errorLabel = new QLabel(this);
    errorLabel->setStyleSheet("color: #ff0000;");
    errorLabel->setVisible(false);

    // Buttons
    okButton = new QPushButton("CONFIRM", this);
    cancelButton = new QPushButton("ABORT", this);

    // Styling
    QString inputStyle = "QLineEdit { background-color: #2b2b2b; color: #00ff00; border: 1px solid #00ff00; padding: 5px; }";
    QString buttonStyle = "QPushButton { background-color: #2b2b2b; color: #00ff00; border: 1px solid #00ff00; padding: 10px; }"
                          "QPushButton:hover { background-color: #00ff00; color: #000000; }";
    QString labelStyle = "QLabel { color: #00ff00; font-weight: bold; }";

    usernameInput->setStyleSheet(inputStyle);
    passwordInput->setStyleSheet(inputStyle);
    dialplanInput->setStyleSheet(inputStyle);
    okButton->setStyleSheet(buttonStyle);
    cancelButton->setStyleSheet(buttonStyle);
    usernameLabel->setStyleSheet(labelStyle);
    passwordLabel->setStyleSheet(labelStyle);
    dialplanLabel->setStyleSheet(labelStyle);

    // Layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(usernameLabel);
    mainLayout->addWidget(usernameInput);
    mainLayout->addWidget(passwordLabel);
    mainLayout->addWidget(passwordInput);
    mainLayout->addWidget(dialplanLabel);
    mainLayout->addWidget(dialplanInput);
    mainLayout->addWidget(errorLabel);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(okButton);
    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);

    // Connect signals
    connect(okButton, &QPushButton::clicked, this, &CustomDialog::validateInputs);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

QString CustomDialog::getUsername() const
{
    return usernameInput->text();
}

QString CustomDialog::getPassword() const
{
    return passwordInput->text();
}

QString CustomDialog::getDialplanNumber() const
{
    return dialplanInput->text();
}

void CustomDialog::validateInputs()
{
    if (usernameInput->text().isEmpty()) {
        errorLabel->setText("CALLSIGN REQUIRED");
        errorLabel->setVisible(true);
        return;
    }

    if (passwordInput->text().isEmpty()) {
        errorLabel->setText("ACCESS CODE REQUIRED");
        errorLabel->setVisible(true);
        return;
    }

    QString dialplan = dialplanInput->text();
    if (dialplan.isEmpty()) {
        errorLabel->setText("TACTICAL NUMBER REQUIRED");
        errorLabel->setVisible(true);
        return;
    }

    if (!dialplan.toInt()) {
        errorLabel->setText("INVALID TACTICAL NUMBER FORMAT");
        errorLabel->setVisible(true);
        return;
    }

    accept();
}
