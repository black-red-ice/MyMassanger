#pragma once
#include "overlaydialog.h"
#include <QPushButton>
#include <QPainter>
#include <QPainterPath>

class QLineEdit;
class QComboBox;
class MainWindow;

class EditProfileDialog : public OverlayDialog
{
    Q_OBJECT

protected:
    void keyPressEvent(QKeyEvent *event) override;

public:
    explicit EditProfileDialog(QWidget *parent = nullptr);

    QString getName() const;
    QString getPosition() const;
    QString getDepartment() const;
    QString getEmail() const;
    QString getPhone() const;
    QString getTabNumber() const;
    QString getAvatarPath() const;

    void setName(const QString &name);
    void setPosition(const QString &position);
    void setDepartment(const QString &department);
    void setEmail(const QString &email);
    void setPhone(const QString &phone);
    void setTabNumber(const QString &tabNumber);
    void setAvatarPath(const QString &path);
    void setAvatarKey(const QString &key);
    void setMainWindow(MainWindow *mw) { m_mainWindow = mw; }


private:
    QPixmap makeRoundedPixmap(const QPixmap &source, int size);

    QLineEdit *m_name;
    QLineEdit *m_position;
    QComboBox *m_department;
    QLineEdit *m_email;
    QLineEdit *m_phone;
    QLineEdit *m_tabNumber;
    QPushButton *m_avatarButton;
    QString m_avatarPath;
    QString m_avatarKey;
    MainWindow *m_mainWindow = nullptr;
};
