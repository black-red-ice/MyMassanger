#ifndef SUPPORTDIALOG_H
#define SUPPORTDIALOG_H

#include "overlaydialog.h"
#include <QVector>
#include <QMap>

struct SupportTicket {
    QString id;
    QString subject;
    QString message;
    QString category;
    QString status;
    QString priority;
    QString createdAt;
    QString operatorName;
    QVector<QPair<QString, QString>> messages;
};

class SupportDialog : public OverlayDialog
{
    Q_OBJECT

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void reject() override;
    void closeEvent(QCloseEvent *event) override;

signals:
    void supportClosed();

public:
    explicit SupportDialog(QWidget *parent = nullptr);
    void setDimWidget(QWidget *dim) { m_dimWidget = dim; }
    ~SupportDialog();

private slots:
    void onCreateTicket(const QString &category);
    void onOpenTicketsWorkspace();

private:
    void setupUI();
    void loadTickets();
    void saveTickets();
    QWidget* createSupportCard(const QString &iconPath, const QString &title,
                               const QString &desc, const QString &color,
                               const QString &categoryKey);
    QWidget* createTicketItem(const SupportTicket &ticket);
    void openTicketDetail(const SupportTicket &ticket,
                          QWidget *chatContentLayout,
                          QWidget *infoContentLayout,
                          QWidget *inputWidget);

    QVector<SupportTicket> m_tickets;

    QMap<QString, QString> m_categoryNames;
    QMap<QString, QString> m_categoryColors;
    QMap<QString, QString> m_categoryIcons;
    QMap<QString, QString> m_categoryDescriptions;
    QWidget *m_dimWidget = nullptr;
    void recreateCategoryCards();
};

#endif // SUPPORTDIALOG_H
