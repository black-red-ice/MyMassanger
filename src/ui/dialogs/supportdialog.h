#ifndef SUPPORTDIALOG_H
#define SUPPORTDIALOG_H

#include "overlaydialog.h"  // вместо QDialog

class SupportDialog : public OverlayDialog  // изменено
{
    Q_OBJECT
public:
    explicit SupportDialog(QWidget *parent = nullptr);
    ~SupportDialog();

private:
    void setupUI();
    QWidget* createSupportCard(const QString &iconPath, const QString &title, const QString &desc, const QString &color);
    QWidget* createTicketItem(const QString &id, const QString &title, const QString &status, const QString &statusColor);
};

#endif // SUPPORTDIALOG_H
