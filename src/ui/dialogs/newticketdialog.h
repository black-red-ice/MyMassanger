#ifndef NEWTICKETDIALOG_H
#define NEWTICKETDIALOG_H

#include <QDialog>
#include "overlaydialog.h"

class QLineEdit;
class QTextEdit;
class QComboBox;
class QPushButton;
class QLabel;

class NewTicketDialog : public OverlayDialog
{
    Q_OBJECT
public:
    explicit NewTicketDialog(const QString &category, const QString &categoryName,
                             const QString &categoryColor, QWidget *parent = nullptr);

    struct TicketData {
        QString subject;
        QString message;
        QString priority;
        QString category;
    };

    TicketData getTicketData() const;

private slots:
    void onSendClicked();
    void onAttachFile();

private:
    void setupUI();

    QLineEdit *m_subjectEdit;
    QTextEdit *m_messageEdit;
    QComboBox *m_priorityCombo;
    QPushButton *m_sendBtn;
    QPushButton *m_attachBtn;
    QLabel *m_attachedFilesLabel;

    QString m_category;
    QString m_categoryName;
    QString m_categoryColor;
    QStringList m_attachedFiles;
};

#endif // NEWTICKETDIALOG_H
