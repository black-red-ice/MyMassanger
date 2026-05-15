#pragma once
#include "overlaydialog.h"
#include <QLabel>
#include <QPushButton>

class ProfileDialog : public OverlayDialog
{
    Q_OBJECT

protected:
    void keyPressEvent(QKeyEvent *event) override;
    bool event(QEvent *event) override;

public:
    explicit ProfileDialog(QWidget *parent = nullptr);

signals:
    void logoutRequested();

private:
    QLabel *m_nameLabel;
    QLabel *m_positionLabel;
    QLabel *m_deptValue;
    QLabel *m_tabValue;
    QLabel *m_emailValue;
    QLabel *m_phoneValue;
    QPushButton *m_avatarButton;

    QPixmap makeRoundedPixmap(const QPixmap &source, int size);
};
