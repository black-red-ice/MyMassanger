#include "mainwindow.h"
#include "middlepanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QEvent>
#include <QDebug>
#include <QListWidgetItem>
#include <QDateTime>
#include <QPainter>
#include <QPainterPath>
#include <QFile>
#include <QStandardPaths>
#include <QSettings>

MiddlePanel::MiddlePanel(QWidget *parent) : QWidget(parent)
{
    setupUI();
}

void MiddlePanel::setupUI()
{
    setFixedWidth(320);
    setStyleSheet("background-color: #1e293a;");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Заголовок с поиском
    QWidget *searchWidget = new QWidget();
    searchWidget->setStyleSheet("background-color: #1e293b;");
    searchWidget->setFixedHeight(70);

    QHBoxLayout *searchLayout = new QHBoxLayout(searchWidget);
    m_searchBox = new QLineEdit();
    m_searchBox->setPlaceholderText("Поиск...");
    m_searchBox->setStyleSheet(
        "QLineEdit { background-color: #334155; border: 1px solid #475569; border-radius: 20px; padding: 10px 15px; color: #f1f5f9; }"
        );
    searchLayout->addWidget(m_searchBox);

    mainLayout->addWidget(searchWidget);

    // Контакты (QListWidget остается для обратной совместимости)
    m_contactsList = new QListWidget();
    m_contactsList->setStyleSheet(
        "QListWidget { border: none; outline: none; background-color: #1e293a; }"
        "QListWidget::item { border: none; padding: 8px; }"
        "QListWidget::item:selected { background-color: #1e293b; }"
        );
    m_contactsList->setSpacing(2);
    m_contactsList->setSelectionRectVisible(false);
    m_contactsList->setFocusPolicy(Qt::NoFocus);
    m_contactsList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_contactsList->setSelectionMode(QAbstractItemView::NoSelection);

    // Чаты (ScrollArea с кастомными виджетами)
    m_chatsScrollArea = new QScrollArea();
    m_chatsScrollArea->setWidgetResizable(true);
    m_chatsScrollArea->setFrameShape(QFrame::NoFrame);
    m_chatsScrollArea->setStyleSheet(
        "QScrollArea { background: #1e293a; border: none; }"
        "QScrollBar:vertical { background: transparent; width: 6px; }"
        "QScrollBar::handle:vertical { background: #334155; border-radius: 3px; }"
        );

    m_chatsContainer = new QWidget();
    m_chatsContainer->setStyleSheet("background: transparent;");
    m_chatsLayout = new QVBoxLayout(m_chatsContainer);
    m_chatsLayout->setContentsMargins(0, 0, 0, 0);
    m_chatsLayout->setSpacing(2);
    m_chatsLayout->addStretch();

    m_chatsScrollArea->setWidget(m_chatsContainer);

    // Стек для переключения между контактами и чатами
    m_stack = new QStackedWidget();
    m_stack->addWidget(m_contactsList);    // индекс 0
    m_stack->addWidget(m_chatsScrollArea); // индекс 1

    mainLayout->addWidget(m_stack, 1);

    // Нижние кнопки
    QWidget *bottomWidget = new QWidget();
    bottomWidget->setStyleSheet("background-color: #1e293a;");
    bottomWidget->setFixedHeight(60);

    QHBoxLayout *bottomLayout = new QHBoxLayout(bottomWidget);
    bottomLayout->setContentsMargins(12, 10, 12, 10);
    bottomLayout->setSpacing(12);

    QPushButton *createGroupBtn = new QPushButton("➕ Создать группу");
    createGroupBtn->setCursor(Qt::PointingHandCursor);
    createGroupBtn->setStyleSheet(
        "QPushButton { background-color: #334155; border: none; border-radius: 8px; padding: 10px; color: #f1f5f9; font-size: 13px; font-weight: 500; }"
        "QPushButton:hover { background-color: #1d4ed8; }"
        );
    connect(createGroupBtn, &QPushButton::clicked, this, &MiddlePanel::onCreateGroupClicked);

    QPushButton *createChannelBtn = new QPushButton("📢 Создать канал");
    createChannelBtn->setCursor(Qt::PointingHandCursor);
    createChannelBtn->setStyleSheet(
        "QPushButton { background-color: #334155; border: none; border-radius: 8px; padding: 10px; color: #f1f5f9; font-size: 13px; font-weight: 500; }"
        "QPushButton:hover { background-color: #1d4ed8; }"
        );
    connect(createChannelBtn, &QPushButton::clicked, this, &MiddlePanel::onCreateChannelClicked);

    connect(m_searchBox, &QLineEdit::returnPressed, this, &MiddlePanel::onSearch);
    connect(m_searchBox, &QLineEdit::textChanged, this, &MiddlePanel::onSearchTextChanged);

    bottomLayout->addWidget(createGroupBtn);
    bottomLayout->addWidget(createChannelBtn);

    mainLayout->addWidget(bottomWidget);

    connect(m_contactsList, &QListWidget::itemClicked, this, [this](QListWidgetItem* item){
        QVariantMap c = item->data(Qt::UserRole).toMap();
        if (!c.contains("id")) return;
        qDebug() << "EMIT contactSelected FROM MiddlePanel";
        emit contactSelected(c["id"].toLongLong());
    });
}

void MiddlePanel::setContacts(const QList<QVariantMap> &contacts)
{
    qDebug() << "=== setContacts called with" << contacts.size() << "contacts ===";

    showContacts();
    m_contactsList->clear();
    m_avatarButtons.clear();

    MainWindow *mw = qobject_cast<MainWindow*>(this->window());

    for (const auto &c : contacts) {
        QString name = c["full_name"].toString();
        if (name.isEmpty()) name = c["username"].toString();
        int userId = c["id"].toInt();

        qDebug() << "Creating contact item for userId:" << userId << "name:" << name;

        QWidget *contactWidget = new QWidget();
        contactWidget->setCursor(Qt::PointingHandCursor);
        contactWidget->setMinimumHeight(60);

        QHBoxLayout *layout = new QHBoxLayout(contactWidget);
        layout->setContentsMargins(12, 8, 12, 8);
        layout->setSpacing(12);

        QPushButton *avatar = new QPushButton();
        avatar->setObjectName(QString("avatar_%1").arg(userId));
        avatar->setFixedSize(44, 44);
        avatar->setText(name.left(1).toUpper());
        avatar->setProperty("userId", userId);
        avatar->setProperty("isAvatar", true);
        avatar->setStyleSheet(
            "QPushButton {"
            "  background: #1d4ed8;"
            "  border-radius: 12px;"
            "  border: none;"
            "  color: white;"
            "  font-size: 18px;"
            "  font-weight: bold;"
            "}"
            );

        layout->addWidget(avatar);

        QLabel *nameLabel = new QLabel(name);
        nameLabel->setStyleSheet("color: #f1f5f9; font-size: 14px; font-weight: 500; background: transparent;");
        nameLabel->setMinimumHeight(44);
        layout->addWidget(nameLabel, 1, Qt::AlignVCenter);

        QListWidgetItem *item = new QListWidgetItem();
        item->setData(Qt::UserRole, c);
        item->setSizeHint(QSize(0, 60));

        m_contactsList->addItem(item);
        m_contactsList->setItemWidget(item, contactWidget);

        // Сохраняем кнопку
        m_avatarButtons[userId] = avatar;
        qDebug() << "Saved avatar button for userId:" << userId << "total buttons:" << m_avatarButtons.size();

        // Проверяем кеш
        if (mw) {
            QString cachedAvatar = mw->getCachedAvatar(userId);
            qDebug() << "Cached avatar for userId:" << userId << ":" << cachedAvatar;

            if (!cachedAvatar.isEmpty() && QFile::exists(cachedAvatar)) {
                QPixmap pixmap(cachedAvatar);
                if (!pixmap.isNull()) {
                    setAvatarPixmap(avatar, pixmap);
                    qDebug() << "✅ Loaded cached avatar for" << userId;
                }
            }
        }
    }

    qDebug() << "=== setContacts finished, total buttons:" << m_avatarButtons.size() << "===";
}

void MiddlePanel::onCreateGroupClicked()
{
    emit createGroupRequested();
}

void MiddlePanel::onCreateChannelClicked()
{
    emit createChannelRequested();
}

void MiddlePanel::onSearch()
{
    QString query = m_searchBox->text().trimmed();

    if (query.startsWith('@')) {
        emit searchUsersByNickname(query);

        // очищаем список контактов
        m_contactsList->clear();

        // добавляем "поиск..."
        QListWidgetItem *item = new QListWidgetItem("🔍 Поиск пользователей...");
        item->setFlags(Qt::NoItemFlags); // нельзя кликнуть
        //item->setForeground(QColor("#94a3b8"));

        m_contactsList->addItem(item);
    }
}

void MiddlePanel::setUserOnline(int userId, bool online)
{
    for (int i = 0; i < m_contactsList->count(); i++) {
        QListWidgetItem* item = m_contactsList->item(i);
        QVariantMap data = item->data(Qt::UserRole).toMap();

        if (data["id"].toInt() == userId) {
            QString name = data["username"].toString();

            item->setText(online ? name + " 🟢" : name);
            break;
        }
    }
}

void MiddlePanel::setChats(const QList<QVariantMap>& chats)
{
    showChats();
    m_chatAvatarButtons.clear();
    m_stack->setCurrentWidget(m_chatsScrollArea);
    m_chatAvatarButtons.clear();

    while (m_chatsLayout->count() > 1) {
        QLayoutItem *item = m_chatsLayout->takeAt(0);
        if (item->widget()) delete item->widget();
        delete item;
    }

    MainWindow *mw = qobject_cast<MainWindow*>(this->window());

    for (const auto& c : chats) {
        QString title = c.value("name").toString();
        if (title.isEmpty()) title = c.value("title").toString();
        if (title.isEmpty()) title = "Чат";

        int userId = c["user_id"].toInt();
        QString lastMessage = c["last_message"].toString();
        int unread = c["unread"].toInt();
        QString lastTime = c["last_time"].toString();

        // Форматируем время
        QString timeStr;
        if (!lastTime.isEmpty()) {
            QDateTime dt = QDateTime::fromString(lastTime, Qt::ISODate);
            if (!dt.isValid()) {
                dt = QDateTime::fromString(lastTime, "yyyy-MM-dd hh:mm:ss");
            }
            if (dt.isValid()) {
                QDate today = QDate::currentDate();
                if (dt.date() == today) {
                    timeStr = dt.toString("HH:mm");
                } else if (dt.date().year() == today.year()) {
                    timeStr = dt.toString("dd.MM");
                } else {
                    timeStr = dt.toString("dd.MM.yy");
                }
            }
        }

        // Создаем виджет чата
        QWidget *chatWidget = new QWidget();
        chatWidget->setCursor(Qt::PointingHandCursor);
        chatWidget->setFixedHeight(72);
        chatWidget->setStyleSheet(
            "QWidget#chatCard { background: transparent; border-radius: 8px; }"
            "QWidget#chatCard:hover { background: #263244; }"
            );
        chatWidget->setObjectName("chatCard");

        QHBoxLayout *layout = new QHBoxLayout(chatWidget);
        layout->setContentsMargins(10, 8, 10, 8);
        layout->setSpacing(10);

        // Аватар
        QPushButton *avatar = new QPushButton();
        avatar->setObjectName(QString("chatAvatar_%1").arg(userId));
        avatar->setFixedSize(44, 44);

        // Показываем первую букву
        QString firstLetter = title.isEmpty() ? "?" : title.left(1).toUpper();
        avatar->setText(firstLetter);

        avatar->setProperty("userId", userId);
        avatar->setProperty("isChatAvatar", true);
        avatar->setCursor(Qt::PointingHandCursor);
        avatar->setStyleSheet(
            "QPushButton {"
            "  background: #1d4ed8;"
            "  border-radius: 12px;"
            "  border: none;"
            "  color: white;"
            "  font-size: 18px;"
            "  font-weight: bold;"
            "}"
            );

        layout->addWidget(avatar);

        // 🔥 СОХРАНЯЕМ КНОПКУ В MAP ДЛЯ ЧАТОВ
        if (userId > 0) {
            m_chatAvatarButtons[userId] = avatar;
        }

        // Информация
        QVBoxLayout *infoLayout = new QVBoxLayout();
        infoLayout->setSpacing(3);
        infoLayout->setContentsMargins(0, 2, 0, 2);

        // Имя и время
        QHBoxLayout *topRow = new QHBoxLayout();
        topRow->setSpacing(8);
        QLabel *nameLabel = new QLabel(title);
        nameLabel->setStyleSheet("color: #f1f5f9; font-size: 14px; font-weight: 600; background: transparent;");
        nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

        QLabel *timeLabel = new QLabel(timeStr);
        timeLabel->setStyleSheet("color: #64748B; font-size: 11px; background: transparent;");
        timeLabel->setFixedWidth(45);
        timeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

        topRow->addWidget(nameLabel);
        topRow->addWidget(timeLabel);
        infoLayout->addLayout(topRow);

        // Последнее сообщение и счетчик
        QHBoxLayout *bottomRow = new QHBoxLayout();
        bottomRow->setSpacing(8);
        QLabel *msgLabel = new QLabel(lastMessage.isEmpty() ? "" : lastMessage);
        msgLabel->setStyleSheet("color: #94A3B8; font-size: 12px; background: transparent;");
        msgLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        msgLabel->setMaximumWidth(200);

        bottomRow->addWidget(msgLabel);
        bottomRow->addStretch();

        if (unread > 0) {
            QLabel *badge = new QLabel(QString::number(unread));
            badge->setAlignment(Qt::AlignCenter);
            badge->setFixedSize(20, 20);
            badge->setStyleSheet(
                "background: #1d4ed8; color: white; border-radius: 10px; font-size: 11px; font-weight: bold;"
                );
            bottomRow->addWidget(badge);
        }

        infoLayout->addLayout(bottomRow);
        layout->addLayout(infoLayout, 1);

        chatWidget->setProperty("chatId", c["id"].toString());
        chatWidget->setProperty("chatTitle", title);
        chatWidget->setProperty("userId", userId);
        chatWidget->installEventFilter(this);

        m_chatsLayout->insertWidget(m_chatsLayout->count() - 1, chatWidget);

        // 🔥 ПРОВЕРЯЕМ КЕШ И СРАЗУ ПОКАЗЫВАЕМ АВАТАР
        if (mw && userId > 0) {
            QSettings settings("Aura", "Messenger");
            QString avatarKey = "userAvatar_" + QString::number(userId);
            QString savedAvatarPath = settings.value(avatarKey).toString();

            if (!savedAvatarPath.isEmpty()) {
                // Проверяем кеш
                QString cachePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
                                    + "/avatars/" + QString::number(userId) + ".jpg";

                QString finalPath = savedAvatarPath;
                if (savedAvatarPath.startsWith("/files/") && QFile::exists(cachePath)) {
                    finalPath = cachePath;
                }

                if (QFile::exists(finalPath)) {
                    QPixmap pixmap(finalPath);
                    if (!pixmap.isNull()) {
                        setAvatarPixmap(avatar, pixmap);
                        qDebug() << "✅ Loaded saved avatar for chat user:" << userId << "from:" << finalPath;
                        continue;
                    }
                }
            }

            // Если нет сохраненного аватара - запрашиваем с сервера
            qDebug() << "🔄 No saved avatar for user:" << userId << ", requesting from server";
            mw->requestUserAvatar(userId);
        }
    }
}

void MiddlePanel::onSearchTextChanged(const QString &text)
{
    qDebug() << "onSearchTextChanged:" << text;

    if (text.isEmpty()) {
        // 🔥 Очищаем поиск и показываем чаты или контакты
        emit clearSearch();
    } else if (text.startsWith('@')) {
        // Поиск по никнейму
        emit searchUsersByNickname(text);
    } else {
        // Поиск по имени в контактах
        emit searchContacts(text);
    }
}

bool MiddlePanel::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QWidget *widget = qobject_cast<QWidget*>(obj);
        qDebug() << "eventFilter clicked, object:" << obj << "widget:" << widget;
        if (widget) {
            qDebug() << "chatId property:" << widget->property("chatId");
        }
        if (widget && widget->property("chatId").isValid()) {
            QString chatId = widget->property("chatId").toString();
            qDebug() << "Chat clicked:" << chatId;
            emit chatSelected(chatId);
            return true;
        }
    }
    return QWidget::eventFilter(obj, event);
}

void MiddlePanel::updateUserAvatar(int userId, const QString &avatarPath)
{
    qDebug() << "updateUserAvatar - userId:" << userId << "avatarPath:" << avatarPath;

    if (m_avatarButtons.contains(userId)) {
        QPushButton *avatar = m_avatarButtons[userId];

        if (!avatarPath.isEmpty() && QFile::exists(avatarPath)) {
            QPixmap pixmap(avatarPath);
            if (!pixmap.isNull()) {
                setAvatarPixmap(avatar, pixmap);
                return;
            }
        }

        // 🔥 НЕТ АВАТАРА - ПОКАЗЫВАЕМ ПЕРВУЮ БУКВУ
        // Ищем имя пользователя
        QString name = "";
        for (int i = 0; i < m_contactsList->count(); ++i) {
            QListWidgetItem *item = m_contactsList->item(i);
            if (item) {
                QVariantMap data = item->data(Qt::UserRole).toMap();
                if (data["id"].toInt() == userId) {
                    name = data["username"].toString();
                    break;
                }
            }
        }

        if (name.isEmpty()) {
            name = QString::number(userId);
        }

        avatar->setText(name.left(1).toUpper());
        avatar->setIcon(QIcon());
        avatar->setStyleSheet(
            "QPushButton {"
            "  background: #1d4ed8;"
            "  border-radius: 12px;"
            "  border: none;"
            "  color: white;"
            "  font-size: 18px;"
            "  font-weight: bold;"
            "}"
            );
    }

    updateChatAvatar(userId, avatarPath);
}

void MiddlePanel::setAvatarPixmap(QPushButton *avatar, const QPixmap &pixmap)
{
    if (!avatar || pixmap.isNull()) {
        qDebug() << "setAvatarPixmap: invalid parameters";
        return;
    }

    qDebug() << "setAvatarPixmap: setting avatar";

    QPixmap rounded(44, 44);
    rounded.fill(Qt::transparent);
    QPainter painter(&rounded);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QPainterPath path;
    path.addRoundedRect(0, 0, 44, 44, 12, 12);
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, pixmap.scaled(44, 44, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    painter.end();

    avatar->setIcon(QIcon(rounded));
    avatar->setIconSize(QSize(44, 44));
    avatar->setText("");  // Убираем букву
    avatar->setStyleSheet("QPushButton { background: transparent; border: none; border-radius: 12px; }");

    // Принудительное обновление
    avatar->update();
    avatar->repaint();

    qDebug() << "setAvatarPixmap completed, icon null:" << avatar->icon().isNull();
}

void MiddlePanel::updateChatAvatar(int userId, const QString &avatarPath)
{
    qDebug() << "updateChatAvatar called - userId:" << userId << "avatarPath:" << avatarPath;

    // Проверяем map для чатов
    if (m_chatAvatarButtons.contains(userId)) {
        QPushButton *avatar = m_chatAvatarButtons[userId];

        if (!avatarPath.isEmpty()) {
            QPixmap pixmap;

            if (QFile::exists(avatarPath)) {
                pixmap.load(avatarPath);
                qDebug() << "Loaded chat avatar from path:" << avatarPath;
            } else if (avatarPath.startsWith("/files/")) {
                QString cachePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
                + "/avatars/" + QString::number(userId) + ".jpg";
                if (QFile::exists(cachePath)) {
                    pixmap.load(cachePath);
                    qDebug() << "Loaded chat avatar from cache:" << cachePath;
                }
            }

            if (!pixmap.isNull()) {
                setAvatarPixmap(avatar, pixmap);
            }
        }
    } else {
        qDebug() << "No chat avatar button found for userId:" << userId;
    }
}

QPixmap MiddlePanel::makeRoundedPixmap(const QPixmap &source, int size)
{
    QPixmap scaled = source.scaled(size, size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

    int x = (scaled.width() - size) / 2;
    int y = (scaled.height() - size) / 2;
    QPixmap square = scaled.copy(x, y, size, size);

    QPixmap rounded(size, size);
    rounded.fill(Qt::transparent);

    QPainter painter(&rounded);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QPainterPath path;
    path.addRoundedRect(0, 0, size, size, 12, 12);
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, square);
    painter.end();

    return rounded;
}
