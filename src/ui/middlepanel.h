#ifndef MIDDLEPANEL_H
#define MIDDLEPANEL_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLineEdit>

class MiddlePanel : public QWidget
{
    Q_OBJECT

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

public:
    explicit MiddlePanel(QWidget *parent = nullptr);

    void setContacts(const QList<QVariantMap> &contacts);
    void setChats(const QList<QVariantMap> &chats);

signals:
    void chatSelected(const QString &chatId);
    void contactSelected(qint64 contactId);

private:
    void setupUI();
    void createDummyChats();

    QLineEdit *m_searchBox;
    QVBoxLayout *m_listLayout;
    QWidget *m_listContainer;
};

#endif // MIDDLEPANEL_H
