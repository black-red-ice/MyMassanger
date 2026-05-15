#ifndef RIGHTPANEL_H
#define RIGHTPANEL_H

#include <QWidget>
#include <QListWidget>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFileDialog>

class RightPanel : public QWidget
{
    Q_OBJECT
public:
    explicit RightPanel(QWidget *parent = nullptr);
    void showTyping(const QString &username);
    void setHasMore(bool hasMore);
    void setLoadingOlder(bool loading);
    void setContactInfo(int userId, const QString &name, bool online, const QString &lastSeen = "");
    void setChatAvatar(const QString &avatarPath);
    QString getChatTitle() const { return m_chatTitle->text(); }
    void setPinned(bool pinned);
    void addImageMessage(const QString &filePath, bool isOutgoing, int status = 0);

public slots:
    void setChatTitle(const QString &title);
    void addMessage(const QString &text, bool isOutgoing = false, int status = 0);
    void clearMessages();
    QListWidget* getMessagesList() const { return m_messagesList; }

signals:
    void sendMessageRequested(const QString &message);
    void typing();
    void typingStop();
    void needLoadOlder();
    void avatarClicked(int userId);
    void pinToggled(bool pinned);
    void fileAttached(const QString &filePath);
    void downloadFileRequested(const QString &fileName, const QString &savePath);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void onSendClicked();
    void onTyping();
    void onScrollChanged(int value);

private:
    void setupUI();
    void setupChatHeader();  // новый метод

    QLabel *m_chatTitle;
    QLabel *m_chatStatus;
    QLabel *m_chatAvatar;
    QPushButton *m_pinBtn;
    QPushButton *m_muteBtn;
    QLabel *m_lastSeenLabel;

    bool m_isPinned = false;
    bool m_isMuted = false;

    QListWidget* m_messagesList;
    QTextEdit* m_messageInput;
    QPushButton* m_sendBtn;
    QLabel *m_typingLabel;
    QTimer *typingTimer = nullptr;
    QTimer *typingDisplayTimer = nullptr;
    bool m_hasMore = true;
    bool m_loadingOlder = false;

    int m_currentContactId = -1;
    QWidget *m_chatHeader;
    QPushButton *m_attachBtn;
};


#endif // RIGHTPANEL_H
