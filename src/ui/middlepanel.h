#ifndef MIDDLEPANEL_H
#define MIDDLEPANEL_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QStackedWidget>
#include <QScrollArea>
#include <QPushButton>

class MiddlePanel : public QWidget
{
    Q_OBJECT

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

public:
    explicit MiddlePanel(QWidget *parent = nullptr);

    void setContacts(const QList<QVariantMap> &contacts);
    void setChats(const QList<QVariantMap>& chats);
    void setUserOnline(int userId, bool online);
    void updateUserAvatar(int userId, const QString &avatarPath);
    void setAvatarPixmap(QPushButton *avatar, const QPixmap &pixmap);
    void updateChatAvatar(int userId, const QString &avatarPath);
    QMap<int, QPushButton*> m_chatAvatarButtons;
    void showChats() { if (m_stack) m_stack->setCurrentWidget(m_chatsScrollArea); }
    void showContacts() { if (m_stack) m_stack->setCurrentWidget(m_contactsList); }

signals:
    void chatSelected(const QString &chatId);
    void contactSelected(qint64 contactId);
    void createGroupRequested();
    void createChannelRequested();
    void searchUsersByNickname(const QString &nickname);
    void searchContacts(const QString &query);
    void clearSearch();

private slots:
    void onCreateGroupClicked();
    void onCreateChannelClicked();
    void onSearch();
    void onSearchTextChanged(const QString &text);

private:
    void setupUI();
    QPixmap makeRoundedPixmap(const QPixmap &source, int size);  // Добавить в private

    QLineEdit *m_searchBox;
    QVBoxLayout *m_listLayout;
    QWidget *m_listContainer;

    QListWidget* m_contactsList;
    QListWidget* m_chatsList;
    QStackedWidget *m_stack = nullptr;
    QScrollArea *m_chatsScrollArea;
    QWidget *m_chatsContainer;
    QVBoxLayout *m_chatsLayout;
    QMap<int, QPushButton*> m_avatarButtons;
};

#endif // MIDDLEPANEL_H
