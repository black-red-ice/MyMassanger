#include "ui/mainwindow.h"
#include "ui/dialogs/logindialog.h"
#include "network/networkmanager.h"

#include <QApplication>
#include <QDebug>
#include <QSettings>
#include <QJsonObject>
#include <QTimer>
#include <QMessageBox>
#include <QEventLoop>

int main(int argc, char *argv[])
{
    qDebug() << "MAIN START";

    QApplication Aura(argc, argv);

    qDebug() << "QApplication created";

    NetworkManager *netManager = new NetworkManager(&Aura);
    netManager->connectToServer("87.242.118.96", 12345);

    // Ждём подключения через сигнал
    QEventLoop loop;
    bool connected = false;

    // Если уже подключён
    if (netManager->isConnected()) {
        connected = true;
    } else {
        QObject::connect(netManager, &NetworkManager::connected, [&]() {
            qDebug() << "Connected to server!";
            connected = true;
            loop.quit();
        });

        QObject::connect(netManager, &NetworkManager::disconnected, [&]() {
            qDebug() << "Disconnected from server";
        });

        // Таймаут 5 секунд
        QTimer timeoutTimer;
        timeoutTimer.setSingleShot(true);
        QObject::connect(&timeoutTimer, &QTimer::timeout, [&]() {
            qDebug() << "Connection timeout!";
            loop.quit();
        });
        timeoutTimer.start(5000);

        loop.exec();
    }

    if (!connected) {
        qDebug() << "Failed to connect to server, but continuing...";
        // Не выходим, просто показываем предупреждение позже
    }

    QSettings settings("Aura", "Messenger");
    bool autoLogin = settings.value("rememberMe", false).toBool();

    if (autoLogin) {
        QString savedUsername = settings.value("savedUsername").toString();
        QString savedPassword = settings.value("savedPassword").toString();

        if (!savedUsername.isEmpty() && !savedPassword.isEmpty()) {
            netManager->connectToServer("87.242.118.96", 12345);

            MainWindow *mainWindow = new MainWindow();
            LoginDialog loginDialog;
            mainWindow->setNetworkManager(netManager);
            loginDialog.setMainWindow(mainWindow);
            qDebug() << "mainWindow pointer in main:" << mainWindow;

            // Таймер для проверки подключения
            QTimer *checkTimer = new QTimer(mainWindow);
            checkTimer->setInterval(100);

            QObject::connect(checkTimer, &QTimer::timeout, mainWindow,
                             [netManager, savedUsername, savedPassword, mainWindow, checkTimer]() {
                                 if (netManager->isConnected()) {
                                     checkTimer->stop();
                                     checkTimer->deleteLater();

                                     qDebug() << "Connected! Sending auto-login...";

                                     QJsonObject data;
                                     data["username"] = savedUsername;
                                     data["password"] = savedPassword;
                                     netManager->sendJson("login", data);

                                     auto *conn = new QMetaObject::Connection();
                                     *conn = QObject::connect(netManager, &NetworkManager::jsonReceived,
                                                              [netManager, savedUsername, mainWindow, conn](const QJsonObject &obj) {
                                                                  if (obj["type"].toString() == "login") {
                                                                      QString status = obj["data"].toObject()["status"].toString();
                                                                      if (status == "ok") {
                                                                          qint64 userId = obj["data"].toObject()["user_id"].toInt();
                                                                          qDebug() << "AUTO LOGIN SUCCESS";
                                                                          mainWindow->setCurrentUser(userId, savedUsername);
                                                                          mainWindow->setWindowTitle("Aura - " + savedUsername);
                                                                          mainWindow->showMaximized();
                                                                      } else {
                                                                          qDebug() << "AUTO LOGIN FAILED, showing login dialog";
                                                                          LoginDialog *loginDialog = new LoginDialog();
                                                                          loginDialog->setNetworkManager(netManager);
                                                                          loginDialog->setMainWindow(mainWindow);
                                                                          loginDialog->setAttribute(Qt::WA_DeleteOnClose);

                                                                          QObject::connect(loginDialog, &QDialog::accepted, mainWindow, [mainWindow, loginDialog]() {
                                                                              mainWindow->setCurrentUser(loginDialog->getCurrentUserId(), loginDialog->getCurrentUsername());
                                                                              mainWindow->setWindowTitle("Aura - " + loginDialog->getCurrentUsername());
                                                                              mainWindow->showMaximized();
                                                                          });
                                                                          QObject::connect(loginDialog, &QDialog::rejected, mainWindow, [mainWindow]() {
                                                                              mainWindow->close();
                                                                          });
                                                                          loginDialog->show();
                                                                      }
                                                                      QObject::disconnect(*conn);
                                                                      delete conn;
                                                                  }
                                                              });
                                 }
                             });

            checkTimer->start();

            return Aura.exec();
        }
    }

    // Обычный вход
    MainWindow *mainWindow = new MainWindow();
    mainWindow->setNetworkManager(netManager);
    LoginDialog loginDialog;
    loginDialog.setNetworkManager(netManager);
    loginDialog.setMainWindow(mainWindow);

    if (loginDialog.exec() == QDialog::Accepted) {
        mainWindow->setCurrentUser(
            loginDialog.getCurrentUserId(),
            loginDialog.getCurrentUsername());
        mainWindow->showMaximized();
        return Aura.exec();
    }

    delete mainWindow;
    return 0;
}
