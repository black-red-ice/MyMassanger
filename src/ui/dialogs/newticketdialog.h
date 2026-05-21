#ifndef NEWTICKETDIALOG_H
#define NEWTICKETDIALOG_H

#include "overlaydialog.h"
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QPushButton>
#include <QListWidget>
#include <QLabel>
#include <QStringList>
#include <QKeyEvent>

struct TicketDialogConfig {
    QString categoryKey;
    QString categoryName;
    QString categoryColor;
    QString title;
    QString subtitle;
    QString subjectPlaceholder;
    QStringList descriptionHints;
    bool showSubtitleAsDescription = false;
};

struct TicketData {
    QString subject;
    QString message;
    QString priority;
    QString category;
    QStringList attachments;
};

class NewTicketDialog : public OverlayDialog
{
    Q_OBJECT

public:
    explicit NewTicketDialog(const TicketDialogConfig &config, QWidget *parent = nullptr);
    ~NewTicketDialog() override;

    TicketData getTicketData() const;
    bool reopenSupportWindow() const { return m_reopenSupport; }

protected:
    void reject() override;
    void accept() override;
    void keyPressEvent(QKeyEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void onPriorityChanged(int index);
    void onAttachFile();
    void onSendClicked();
    void removeAttachment(int index);

private:
    void setupUI();
    void updateAttachmentsDisplay();
    void updatePlaceholderGeometry();

    TicketDialogConfig m_config;
    bool m_reopenSupport = true;

    // UI элементы
    QLineEdit *m_subjectEdit;
    QTextEdit *m_messageEdit;
    QComboBox *m_priorityCombo;
    QPushButton *m_attachBtn;
    QPushButton *m_cancelBtn;
    QPushButton *m_sendBtn;
    QListWidget *m_attachmentsList;
    QLabel *m_placeholderLabel;

    // Данные
    QStringList m_attachedFiles;
};

#endif // NEWTICKETDIALOG_H
