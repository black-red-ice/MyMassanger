#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QStackedWidget>
#include <QCheckBox>

class NetworkManager;

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

    qint64 getCurrentUserId() const { return m_currentUserId; }
    QString getCurrentUsername() const { return m_currentUsername; }
    NetworkManager* getNetworkManager() const { return m_networkManager; }

signals:
    void loginSuccess(qint64 userId, const QString &username);

private slots:
    void onLoginClicked();
    void onRegisterClicked();
    void onSwitchToRegister();
    void onSwitchToLogin();
    void onNetworkConnected();
    void onNetworkDisconnected();
    void onCommandResponse(const QString &response);

private:
    void setupUI();
    void setupLoginForm();
    void setupRegisterForm();
    QString hashPassword(const QString &password);

    QStackedWidget *m_stackedWidget;

    // Форма входа
    QWidget *m_loginWidget;
    QLineEdit *m_loginUsername;
    QLineEdit *m_loginPassword;
    QPushButton *m_loginBtn;
    QPushButton *m_switchToRegisterBtn;
    QCheckBox *m_rememberMe;

    // Форма регистрации
    QWidget *m_registerWidget;
    QLineEdit *m_regUsername;
    QLineEdit *m_regEmail;
    QLineEdit *m_regFullName;
    QLineEdit *m_regPassword;
    QLineEdit *m_regConfirmPassword;
    QPushButton *m_registerBtn;
    QPushButton *m_switchToLoginBtn;

    QLabel *m_statusLabel;

    qint64 m_currentUserId = -1;
    QString m_currentUsername;

    NetworkManager *m_networkManager;
    bool m_waitingForResponse = false;
    QString m_pendingAction;
};

#endif // LOGINDIALOG_H
