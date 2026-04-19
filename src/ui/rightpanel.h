#ifndef RIGHTPANEL_H
#define RIGHTPANEL_H

#include <QWidget>
#include <QListWidget>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

class RightPanel : public QWidget
{
    Q_OBJECT
public:
    explicit RightPanel(QWidget *parent = nullptr);

public slots:
    void setChatTitle(const QString &title);
    void addMessage(const QString &text, bool isOutgoing = false);
    void clearMessages();

signals:
    void sendMessageRequested(const QString &message);

private slots:
    void onSendClicked();

private:
    void setupUI();

    QLabel* m_chatTitle;
    QListWidget* m_messagesList;  // Используем QListWidget вместо ручного управления
    QTextEdit* m_messageInput;
    QPushButton* m_sendBtn;
};

#endif // RIGHTPANEL_H
