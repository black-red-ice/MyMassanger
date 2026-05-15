#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QStackedWidget>
#include <QCheckBox>

class NetworkManager;
class MainWindow;

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

    qint64 getCurrentUserId() const { return m_currentUserId; }
    QString getCurrentUsername() const { return m_currentUsername; }
    NetworkManager* getNetworkManager() const { return m_networkManager; }
    void setNetworkManager(NetworkManager *manager);
    void setMainWindow(MainWindow *mw);

signals:
    void loginSuccess(qint64 userId, const QString &username);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onLoginClicked();
    void onRegisterClicked();
    void onRegisterNextStep();
    void onBackToFirstStep();
    void onSwitchToRegister();
    void onSwitchToLogin();
    void onForgotPassword();
    void onAlternativeLogin();
    void onNetworkConnected();
    void onNetworkDisconnected();
    void onJsonResponse(const QJsonObject &obj);
    void onAlternativeGoogle();
    void onAlternativeGitHub();
    void onAlternativeApple();
    void onAlternativeQR();
    void onBackFromAlternative();
    void onConfirmationSent(bool success);
    void onConfirmationFailed(const QString &error);
    void onRegistrationCompleted();


private:
    void setupUI();
    void setupLoginForm();
    void setupRegisterForm();
    void setupPasswordStep();
    void resizeToLoginSize();
    void resizeToRegisterFirstStep();
    void resizeToRegisterSecondStep();
    void centerWindow();
    //QString hashPassword(const QString &password);

    bool m_passwordStepCreated = false;
    QStackedWidget *m_stackedWidget = nullptr;
    QPushButton *m_loginTab;
    QPushButton *m_registerTab;
    QWidget *m_dragArea;

    // Форма входа
    QWidget *m_loginWidget;
    QLineEdit *m_loginUsername;
    QLineEdit *m_loginPassword;
    QPushButton *m_loginBtn;
    QCheckBox *m_rememberMe;

    // Форма регистрации (шаг 1)
    QWidget *m_registerWidget;
    QWidget *m_passwordWidget;

    // Поля первого шага регистрации
    QLineEdit *m_regFullName;
    QLineEdit *m_regEmail;
    QLineEdit *m_regPhone;
    QLineEdit *m_regBirthDate;
    QPushButton *m_registerNextBtn;

    // Поля второго шага регистрации
    QLineEdit *m_regPassword = nullptr;
    QLineEdit *m_regConfirmPassword = nullptr;
    QPushButton *m_registerSubmitBtn = nullptr;
    QCheckBox *m_termsCheckbox = nullptr;

    QLabel *m_statusLabel;

    qint64 m_currentUserId = -1;
    QString m_currentUsername;

    NetworkManager* m_networkManager = nullptr;
    bool m_waitingForResponse = false;
    QString m_pendingAction;

    bool m_dragging = false;
    QPoint m_dragPosition;

    QString m_tempFullName;
    QString m_tempEmail;
    QString m_tempPhone;
    QString m_tempBirthDate;
    QString m_tempUsername;
    QString m_tempPassword;

    QString m_lastLogin;
    QString m_lastPassword;

    void setupAlternativeLoginForm();
    QWidget *m_alternativeWidget;
    QWidget *m_qrWidget = nullptr;
    void showEmailConfirmationDialog();

    QLineEdit *m_tempEmailEdit;
    QLineEdit *m_tempUsernameEdit;
    QLineEdit *m_tempPasswordEdit;
    QLineEdit *m_confirmationCodeEdit;
    QDialog *m_confirmationDialog;
    MainWindow *m_mainWindow = nullptr;

    QString m_registeredUsername;
    QString m_registeredPassword;
};

#endif // LOGINDIALOG_H
