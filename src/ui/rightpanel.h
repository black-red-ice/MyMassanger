#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>
#include <QListWidget>

class PhotoViewer;

class RightPanel : public QWidget
{
    Q_OBJECT

public:
    explicit RightPanel(QWidget *parent = nullptr);

    void setChatTitle(const QString &title);
    void addMessage(const QString &text, bool isOutgoing = false, int status = 0);
    void clearMessages();
    void setContactInfo(int userId, const QString &name, bool online, const QString &lastSeen);
    void setChatAvatar(const QString &avatarPath);
    void setPinned(bool pinned);
    void addImageMessage(const QString &filePath, bool isOutgoing, int status);
    void showTyping(const QString &username);
    void setHasMore(bool hasMore);
    void setLoadingOlder(bool loading);
    QString getChatTitle() const { return m_chatTitle ? m_chatTitle->text() : QString(); }

    // Для совместимости со старым кодом
    QScrollArea* getScrollArea() const { return m_scrollArea; }
    QWidget* getMessagesContainer() const { return m_messagesContainer; }

signals:
    void sendMessageRequested(const QString &message);
    void typing();
    void typingStop();
    void avatarClicked(int userId);
    void needLoadOlder();
    void pinToggled(bool pinned);
    void fileAttached(const QString &filePath);
    void downloadFileRequested(const QString &fileUrl, const QString &savePath);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void onSendClicked();
    void onTyping();
    void onScrollChanged(int value);

private:
    void setupUI();
    void setupChatHeader();

    QWidget *m_chatHeader = nullptr;
    QLabel *m_chatAvatar = nullptr;
    QLabel *m_chatTitle = nullptr;
    QLabel *m_lastSeenLabel = nullptr;
    QPushButton *m_pinBtn = nullptr;
    QPushButton *m_muteBtn = nullptr;

    // Новые элементы для сообщений
    QScrollArea *m_scrollArea = nullptr;
    QWidget *m_messagesContainer = nullptr;
    QVBoxLayout *m_messagesLayout = nullptr;

    QTextEdit *m_messageInput = nullptr;
    QPushButton *m_sendBtn = nullptr;
    QPushButton *m_attachBtn = nullptr;
    QLabel *m_typingLabel = nullptr;

    QTimer *typingTimer = nullptr;
    QTimer *typingDisplayTimer = nullptr;

    int m_currentContactId = 0;
    bool m_isPinned = false;
    bool m_isMuted = false;
    bool m_hasMore = false;
    bool m_loadingOlder = false;
};
