#include "logindialog.h"
#include "../network/NetworkManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QCryptographicHash>
#include <QDebug>
#include <QTimer>
#include <QCryptographicHash>

LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent)
{
    setupUI();
    setWindowTitle("Aura - Вход");
    setFixedSize(400, 500);
    setModal(true);

    // Создаем NetworkManager
    m_networkManager = new NetworkManager(this);
    connect(m_networkManager, &NetworkManager::connected, this, &LoginDialog::onNetworkConnected);
    connect(m_networkManager, &NetworkManager::disconnected, this, &LoginDialog::onNetworkDisconnected);
    connect(m_networkManager, &NetworkManager::commandResponse, this, &LoginDialog::onCommandResponse);

    // Подключаемся к серверу
    m_networkManager->connectToServer("87.242.118.96", 12345);
}

LoginDialog::~LoginDialog() = default;

void LoginDialog::setupUI()
{
    // Основной layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 30, 40, 30);
    mainLayout->setSpacing(20);

    // Логотип
    QLabel *logoLabel = new QLabel("AURA");
    logoLabel->setStyleSheet(
        "font-size: 32px;"
        "font-weight: bold;"
        "color: #1d4ed8;"
        );
    logoLabel->setAlignment(Qt::AlignCenter);

    // Статус
    m_statusLabel = new QLabel("");
    m_statusLabel->setStyleSheet(
        "color: #ef4444;"
        "font-size: 13px;"
        "padding: 8px;"
        "background-color: rgba(239, 68, 68, 0.1);"
        "border-radius: 6px;"
        );
    m_statusLabel->setVisible(false);
    m_statusLabel->setWordWrap(true);
    m_statusLabel->setAlignment(Qt::AlignCenter);

    // Стековый виджет для переключения форм
    m_stackedWidget = new QStackedWidget();
    m_stackedWidget->setStyleSheet("background-color: transparent;");

    setupLoginForm();
    setupRegisterForm();

    m_stackedWidget->addWidget(m_loginWidget);
    m_stackedWidget->addWidget(m_registerWidget);

    // Сборка
    mainLayout->addStretch();
    mainLayout->addWidget(logoLabel);
    mainLayout->addSpacing(20);
    mainLayout->addWidget(m_statusLabel);
    mainLayout->addWidget(m_stackedWidget);
    mainLayout->addStretch();

    // Стилизация диалога
    setStyleSheet(
        "QDialog { background-color: #1e293b; }"
        "QLabel { color: #f1f5f9; background-color: transparent; }"
        );
}

void LoginDialog::setupLoginForm()
{
    m_loginWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(m_loginWidget);
    layout->setSpacing(15);

    QLabel *title = new QLabel("Вход в аккаунт");
    title->setStyleSheet("font-size: 18px; font-weight: 600; margin-bottom: 10px;");
    title->setAlignment(Qt::AlignCenter);

    m_loginUsername = new QLineEdit();
    m_loginUsername->setPlaceholderText("Имя пользователя или Email");
    m_loginUsername->setStyleSheet(
        "QLineEdit { background-color: #334155; border: 1px solid #475569;"
        "border-radius: 8px; padding: 12px; color: #f1f5f9; font-size: 14px; }"
        "QLineEdit:focus { border-color: #1d4ed8; }"
        );

    m_loginPassword = new QLineEdit();
    m_loginPassword->setPlaceholderText("Пароль");
    m_loginPassword->setEchoMode(QLineEdit::Password);
    m_loginPassword->setStyleSheet(m_loginUsername->styleSheet());

    m_rememberMe = new QCheckBox("Запомнить меня");
    m_rememberMe->setStyleSheet(
        "QCheckBox { color: #94a3b8; }"
        "QCheckBox::indicator { width: 18px; height: 18px; }"
        );

    m_loginBtn = new QPushButton("Войти");
    m_loginBtn->setCursor(Qt::PointingHandCursor);
    m_loginBtn->setStyleSheet(
        "QPushButton { background-color: #1d4ed8; border: none; border-radius: 8px;"
        "padding: 12px; color: white; font-size: 15px; font-weight: 600; }"
        "QPushButton:hover { background-color: #2563eb; }"
        "QPushButton:pressed { background-color: #1e40af; }"
        );
    connect(m_loginBtn, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);

    m_switchToRegisterBtn = new QPushButton("Создать аккаунт");
    m_switchToRegisterBtn->setCursor(Qt::PointingHandCursor);
    m_switchToRegisterBtn->setStyleSheet(
        "QPushButton { background-color: transparent; border: 1px solid #475569;"
        "border-radius: 8px; padding: 10px; color: #94a3b8; font-size: 13px; }"
        "QPushButton:hover { background-color: #334155; color: #f1f5f9; }"
        );
    connect(m_switchToRegisterBtn, &QPushButton::clicked, this, &LoginDialog::onSwitchToRegister);

    layout->addWidget(title);
    layout->addWidget(m_loginUsername);
    layout->addWidget(m_loginPassword);
    layout->addWidget(m_rememberMe);
    layout->addSpacing(5);
    layout->addWidget(m_loginBtn);
    layout->addWidget(m_switchToRegisterBtn);
}

void LoginDialog::setupRegisterForm()
{
    m_registerWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(m_registerWidget);
    layout->setSpacing(12);

    QLabel *title = new QLabel("Регистрация");
    title->setStyleSheet("font-size: 18px; font-weight: 600; margin-bottom: 5px;");
    title->setAlignment(Qt::AlignCenter);

    m_regUsername = new QLineEdit();
    m_regUsername->setPlaceholderText("Имя пользователя");
    m_regUsername->setStyleSheet(m_loginUsername->styleSheet());

    m_regEmail = new QLineEdit();
    m_regEmail->setPlaceholderText("Email");
    m_regEmail->setStyleSheet(m_loginUsername->styleSheet());

    m_regFullName = new QLineEdit();
    m_regFullName->setPlaceholderText("Полное имя (необязательно)");
    m_regFullName->setStyleSheet(m_loginUsername->styleSheet());

    m_regPassword = new QLineEdit();
    m_regPassword->setPlaceholderText("Пароль");
    m_regPassword->setEchoMode(QLineEdit::Password);
    m_regPassword->setStyleSheet(m_loginUsername->styleSheet());

    m_regConfirmPassword = new QLineEdit();
    m_regConfirmPassword->setPlaceholderText("Подтвердите пароль");
    m_regConfirmPassword->setEchoMode(QLineEdit::Password);
    m_regConfirmPassword->setStyleSheet(m_loginUsername->styleSheet());

    m_registerBtn = new QPushButton("Зарегистрироваться");
    m_registerBtn->setCursor(Qt::PointingHandCursor);
    m_registerBtn->setStyleSheet(m_loginBtn->styleSheet());
    connect(m_registerBtn, &QPushButton::clicked, this, &LoginDialog::onRegisterClicked);

    m_switchToLoginBtn = new QPushButton("У меня уже есть аккаунт");
    m_switchToLoginBtn->setCursor(Qt::PointingHandCursor);
    m_switchToLoginBtn->setStyleSheet(m_switchToRegisterBtn->styleSheet());
    connect(m_switchToLoginBtn, &QPushButton::clicked, this, &LoginDialog::onSwitchToLogin);

    layout->addWidget(title);
    layout->addWidget(m_regUsername);
    layout->addWidget(m_regEmail);
    layout->addWidget(m_regFullName);
    layout->addWidget(m_regPassword);
    layout->addWidget(m_regConfirmPassword);
    layout->addSpacing(5);
    layout->addWidget(m_registerBtn);
    layout->addWidget(m_switchToLoginBtn);
}

void LoginDialog::onLoginClicked()
{
    QString username = m_loginUsername->text().trimmed();
    QString password = m_loginPassword->text();

    if (username.isEmpty() || password.isEmpty()) {
        m_statusLabel->setText("Заполните все поля");
        m_statusLabel->setVisible(true);
        return;
    }

    if (!m_networkManager->isConnected()) {
        m_statusLabel->setText("Нет подключения к серверу");
        m_statusLabel->setVisible(true);
        return;
    }

    m_statusLabel->setText("Вход...");
    m_statusLabel->setStyleSheet(
        "color: #f59e0b; font-size: 13px; padding: 8px;"
        "background-color: rgba(245, 158, 11, 0.1); border-radius: 6px;"
        );
    m_statusLabel->setVisible(true);
    m_loginBtn->setEnabled(false);

    QString passwordHash = hashPassword(password);
    QString command = QString("LOGIN:%1:%2").arg(username).arg(passwordHash);

    m_pendingAction = "login";
    m_waitingForResponse = true;
    m_networkManager->sendCommand(command);
}

void LoginDialog::onRegisterClicked()
{
    QString username = m_regUsername->text().trimmed();
    QString email = m_regEmail->text().trimmed();
    QString fullName = m_regFullName->text().trimmed();
    QString password = m_regPassword->text();
    QString confirm = m_regConfirmPassword->text();

    if (username.isEmpty() || email.isEmpty() || password.isEmpty()) {
        m_statusLabel->setText("Заполните обязательные поля");
        m_statusLabel->setVisible(true);
        return;
    }

    if (password != confirm) {
        m_statusLabel->setText("Пароли не совпадают");
        m_statusLabel->setVisible(true);
        return;
    }

    if (password.length() < 6) {
        m_statusLabel->setText("Пароль должен быть не менее 6 символов");
        m_statusLabel->setVisible(true);
        return;
    }

    if (!m_networkManager->isConnected()) {
        m_statusLabel->setText("Нет подключения к серверу");
        m_statusLabel->setVisible(true);
        return;
    }

    m_statusLabel->setText("Регистрация...");
    m_statusLabel->setStyleSheet(
        "color: #f59e0b; font-size: 13px; padding: 8px;"
        "background-color: rgba(245, 158, 11, 0.1); border-radius: 6px;"
        );
    m_statusLabel->setVisible(true);
    m_registerBtn->setEnabled(false);

    QString passwordHash = hashPassword(password);
    QString command = QString("REGISTER:%1:%2:%3:%4")
                          .arg(username)
                          .arg(email)
                          .arg(passwordHash)
                          .arg(fullName.isEmpty() ? "" : fullName);

    m_pendingAction = "register";
    m_waitingForResponse = true;
    m_networkManager->sendCommand(command);
}

void LoginDialog::onSwitchToRegister()
{
    m_stackedWidget->setCurrentIndex(1);
    m_statusLabel->setVisible(false);
    setWindowTitle("Aura - Регистрация");
}

void LoginDialog::onSwitchToLogin()
{
    m_stackedWidget->setCurrentIndex(0);
    m_statusLabel->setVisible(false);
    setWindowTitle("Aura - Вход");
}

QString LoginDialog::hashPassword(const QString &password)
{
    QByteArray hash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    return QString(hash.toHex());
}

void LoginDialog::onNetworkConnected()
{
    qDebug() << "Connected to server";
    m_statusLabel->setText("Подключено к серверу");
    m_statusLabel->setStyleSheet(
        "color: #22c55e; font-size: 13px; padding: 8px;"
        "background-color: rgba(34, 197, 94, 0.1); border-radius: 6px;"
        );
    m_statusLabel->setVisible(true);
}

void LoginDialog::onNetworkDisconnected()
{
    qDebug() << "Disconnected from server";
    m_statusLabel->setText("Нет подключения к серверу");
    m_statusLabel->setStyleSheet(
        "color: #ef4444; font-size: 13px; padding: 8px;"
        "background-color: rgba(239, 68, 68, 0.1); border-radius: 6px;"
        );
    m_statusLabel->setVisible(true);
    m_loginBtn->setEnabled(true);
    m_registerBtn->setEnabled(true);
    m_waitingForResponse = false;
}

void LoginDialog::onCommandResponse(const QString &response)
{
    qDebug() << "Command response:" << response;

    if (!m_waitingForResponse) return;

    QStringList parts = response.split(':');
    QString status = parts[0];

    if (m_pendingAction == "login") {
        if (status == "LOGIN_OK" && parts.size() >= 4) {
            m_currentUserId = parts[1].toLongLong();
            m_currentUsername = parts[2];
            emit loginSuccess(m_currentUserId, m_currentUsername);
            accept();
        } else if (status == "LOGIN_ERROR" && parts.size() >= 2) {
            m_statusLabel->setText("Ошибка: " + parts[1]);
            m_statusLabel->setStyleSheet(
                "color: #ef4444; font-size: 13px; padding: 8px;"
                "background-color: rgba(239, 68, 68, 0.1); border-radius: 6px;"
                );
            m_loginBtn->setEnabled(true);
        }
    }
    else if (m_pendingAction == "register") {
        if (status == "REGISTER_OK") {
            m_statusLabel->setText("Регистрация успешна! Теперь войдите.");
            m_statusLabel->setStyleSheet(
                "color: #22c55e; font-size: 13px; padding: 8px;"
                "background-color: rgba(34, 197, 94, 0.1); border-radius: 6px;"
                );
            m_registerBtn->setEnabled(true);
            onSwitchToLogin();
        } else if (status == "REGISTER_ERROR" && parts.size() >= 2) {
            m_statusLabel->setText("Ошибка: " + parts[1]);
            m_statusLabel->setStyleSheet(
                "color: #ef4444; font-size: 13px; padding: 8px;"
                "background-color: rgba(239, 68, 68, 0.1); border-radius: 6px;"
                );
            m_registerBtn->setEnabled(true);
        }
    }

    m_waitingForResponse = false;
}
