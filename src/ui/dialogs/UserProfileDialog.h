#pragma once
#include "overlaydialog.h"
#include <QLabel>
#include <QPushButton>

class UserProfileDialog : public OverlayDialog
{
    Q_OBJECT

public:
    explicit UserProfileDialog(int userId, const QString &username,
                               const QString &avatarPath, bool online,
                               QJsonObject profileData,  // без const
                               QWidget *parent = nullptr);

private:
    QPushButton *m_avatarButton;
    QLabel *m_nameLabel;
    QLabel *m_statusLabel;
    QLabel *m_deptValue;
    QLabel *m_tabValue;
    QLabel *m_emailValue;
    QLabel *m_phoneValue;
    void loadAvatar();
    int m_userId;
    QString m_avatarPath;

    QPixmap makeRoundedPixmap(const QPixmap &source, int size);
    void showFullPhoto(const QString &imagePath);
};
