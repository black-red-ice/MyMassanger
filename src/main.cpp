#include "ui/mainwindow.h"
#include "ui/dialogs/logindialog.h"
#include <QApplication>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication Aura(argc, argv);

    // Подключение к PostgreSQL
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");
    QString connectionString =
        "DRIVER={PostgreSQL Unicode(x64)};"
        "SERVER=87.242.118.96;"
        "PORT=5432;"
        "DATABASE=aura_messenger;"
        "UID=aura_user;"
        "PWD=AuraSecurePass2024!;"
        "SSLmode=disable;";
    db.setDatabaseName(connectionString);

    if (db.open()) {
        qDebug() << "Connected to PostgreSQL via ODBC!";
    } else {
        qDebug() << "Connection failed:" << db.lastError().text();
    }

    // Показываем диалог входа
    LoginDialog loginDialog;

    if (loginDialog.exec() == QDialog::Accepted) {
        // Вход успешен, открываем главное окно
        MainWindow *mainWindow = new MainWindow();  // ← СОЗДАЁМ В КУЧЕ (new)

        mainWindow->setCurrentUser(loginDialog.getCurrentUserId(), loginDialog.getCurrentUsername());

        NetworkManager *netManager = loginDialog.getNetworkManager();
        loginDialog.getNetworkManager()->setParent(nullptr);
        mainWindow->setNetworkManager(netManager);

        mainWindow->setWindowTitle("Aura - " + loginDialog.getCurrentUsername());
        mainWindow->showMaximized();

        return Aura.exec();
    }

    return 0;
}
