#include "rightpanel.h"
#include "PhotoViewer.h"
#include <QHBoxLayout>
#include <QListWidgetItem>
#include <QKeyEvent>
#include <QScrollBar>
#include <QScrollArea>
#include <QTimer>
#include <QFile>
#include <QPainter>
#include <QPixmap>
#include <QPainterPath>
#include <QIcon>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStandardPaths>
#include <QDir>

RightPanel::RightPanel(QWidget *parent)
    : QWidget(parent),
    typingTimer(nullptr),
    typingDisplayTimer(nullptr),
    m_typingLabel(nullptr)
{
    qDebug() << "RightPanel ctor START";
    setupUI();
}

void RightPanel::setupUI()
{
    setStyleSheet("background-color: #0f172a;");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    setupChatHeader();
    mainLayout->addWidget(m_chatHeader);

    m_messagesList = new QListWidget();
    m_messagesList->setStyleSheet(
        "QListWidget { background-color: #0f172a; border: none; outline: none; }"
        "QListWidget::item { padding: 4px; }"
        );
    m_messagesList->setWordWrap(true);
    m_messagesList->setSelectionMode(QAbstractItemView::NoSelection);
    connect(m_messagesList->verticalScrollBar(), &QScrollBar::valueChanged, this, &RightPanel::onScrollChanged);
    mainLayout->addWidget(m_messagesList);

    m_typingLabel = new QLabel();
    m_typingLabel->setStyleSheet("color: #94a3b8; font-size: 12px; padding: 4px 16px;");
    m_typingLabel->hide();
    mainLayout->addWidget(m_typingLabel);

    QWidget *inputArea = new QWidget();
    inputArea->setStyleSheet("background-color: #1e293b; border-top: 1px solid #334155;");
    inputArea->setFixedHeight(80);

    QHBoxLayout *inputLayout = new QHBoxLayout(inputArea);
    inputLayout->setContentsMargins(12, 10, 12, 10);
    inputLayout->setSpacing(8);

    m_attachBtn = new QPushButton();
    m_attachBtn->setFixedSize(40, 40);
    m_attachBtn->setCursor(Qt::PointingHandCursor);
    m_attachBtn->setIcon(QIcon(":/icons/darkTheme/images/darkTheme/paperclip.svg"));
    m_attachBtn->setIconSize(QSize(20, 20));
    m_attachBtn->setStyleSheet(
        "QPushButton { background-color: #334155; border: none; border-radius: 20px; }"
        "QPushButton:hover { background-color: #475569; }"
        );
    connect(m_attachBtn, &QPushButton::clicked, this, [this]() {
        QString filePath = QFileDialog::getOpenFileName(this, "Выберите файл", "",
                                                        "Все файлы (*.*);;Изображения (*.png *.jpg *.jpeg *.bmp *.gif)");
        if (!filePath.isEmpty()) {
            emit fileAttached(filePath);
        }
    });

    inputLayout->addWidget(m_attachBtn);

    m_messageInput = new QTextEdit();
    m_messageInput->setPlaceholderText("Напишите сообщение...");
    m_messageInput->setStyleSheet(
        "QTextEdit { background-color: #334155; border: 1px solid #475569; border-radius: 20px; padding: 10px; color: #f1f5f9; font-family: 'Segoe UI', system-ui, -apple-system, sans-serif; font-size: 14px; }"
        );
    m_messageInput->setMaximumHeight(60);
    m_messageInput->installEventFilter(this);
    inputLayout->addWidget(m_messageInput);

    m_sendBtn = new QPushButton("→");
    m_sendBtn->setFixedSize(40, 40);
    m_sendBtn->setCursor(Qt::PointingHandCursor);
    m_sendBtn->setStyleSheet(
        "QPushButton { background-color: #1d4ed8; border: none; border-radius: 20px; color: white; font-size: 20px; font-weight: bold; }"
        "QPushButton:hover { background-color: #2563eb; }"
        );
    connect(m_sendBtn, &QPushButton::clicked, this, &RightPanel::onSendClicked);
    connect(m_messageInput, &QTextEdit::textChanged, this, &RightPanel::onTyping);
    inputLayout->addWidget(m_sendBtn);

    mainLayout->addWidget(inputArea);
}

void RightPanel::onSendClicked()
{
    QString message = m_messageInput->toPlainText().trimmed();
    qDebug() << "=== onSendClicked ===";
    qDebug() << "Message:" << message;

    if (!message.isEmpty()) {
        emit sendMessageRequested(message);
        m_messageInput->clear();
    } else {
        qDebug() << "Message is empty";
    }
}

void RightPanel::setChatTitle(const QString &title)
{
    m_chatTitle->setText(title);
}

void RightPanel::addMessage(const QString &text, bool isOutgoing, int status)
{
    qDebug() << "➕ addMessage:" << text.left(30) << "outgoing:" << isOutgoing << "status:" << status;

    bool isImageMessage = text.startsWith("🖼 ");
    QStringList imageParts;
    if (isImageMessage) {
        imageParts = text.mid(2).split("|");
    }

    QWidget *messageWidget = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(messageWidget);
    mainLayout->setContentsMargins(10, 5, 10, 5);
    mainLayout->setSpacing(2);

    QWidget *bubble = new QWidget();
    bubble->setStyleSheet(
        QString("background-color: %1; border-radius: 12px;")
            .arg(isOutgoing ? "#1d4ed8" : "#334155")
        );

    QVBoxLayout *bubbleLayout = new QVBoxLayout(bubble);
    bubbleLayout->setContentsMargins(4, 4, 4, 4);
    bubbleLayout->setSpacing(4);

    if (isImageMessage && imageParts.size() >= 2) {
        QString fileName = imageParts[0].trimmed();
        QString fileUrl = imageParts[1];

        QString localPath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
                            + "/chat_images/" + fileName;
        QDir().mkpath(QFileInfo(localPath).path());

        if (!QFile::exists(localPath)) {
            QNetworkAccessManager *manager = new QNetworkAccessManager(this);
            QNetworkRequest request(QUrl("http://87.242.118.96:8080" + fileUrl));
            QNetworkReply *reply = manager->get(request);

            QLabel *loadingLabel = new QLabel("Загрузка...");
            loadingLabel->setStyleSheet("color: #cbd5e1; font-size: 12px; background: transparent;");
            loadingLabel->setFixedSize(250, 200);
            loadingLabel->setAlignment(Qt::AlignCenter);
            bubbleLayout->addWidget(loadingLabel);

            connect(reply, &QNetworkReply::finished, this, [reply, loadingLabel, localPath, fileName, isOutgoing, this]() {
                if (reply->error() == QNetworkReply::NoError) {
                    QByteArray data = reply->readAll();
                    QFile file(localPath);
                    if (file.open(QIODevice::WriteOnly)) {
                        file.write(data);
                        file.close();

                        QPixmap pixmap(localPath);
                        if (!pixmap.isNull()) {
                            QPixmap scaled = pixmap.scaled(250, 250, Qt::KeepAspectRatio, Qt::SmoothTransformation);

                            QPixmap rounded(scaled.size());
                            rounded.fill(Qt::transparent);
                            QPainter painter(&rounded);
                            painter.setRenderHint(QPainter::Antialiasing, true);
                            QPainterPath path;
                            path.addRoundedRect(0, 0, scaled.width(), scaled.height(), 12, 12);
                            painter.setClipPath(path);
                            painter.drawPixmap(0, 0, scaled);
                            painter.end();

                            loadingLabel->setPixmap(rounded);
                            loadingLabel->setScaledContents(false);
                            loadingLabel->setFixedSize(scaled.width(), scaled.height());
                            loadingLabel->setCursor(Qt::PointingHandCursor);
                            loadingLabel->setProperty("imagePath", localPath);
                            loadingLabel->installEventFilter(this);
                            loadingLabel->setStyleSheet("background: transparent;");
                        }
                    }
                } else {
                    loadingLabel->setText("Ошибка загрузки");
                }
                reply->deleteLater();
            });
        } else {
            QLabel *imageLabel = new QLabel();
            QPixmap pixmap(localPath);
            if (!pixmap.isNull()) {
                QPixmap scaled = pixmap.scaled(250, 250, Qt::KeepAspectRatio, Qt::SmoothTransformation);

                QPixmap rounded(scaled.size());
                rounded.fill(Qt::transparent);
                QPainter painter(&rounded);
                painter.setRenderHint(QPainter::Antialiasing, true);
                QPainterPath path;
                path.addRoundedRect(0, 0, scaled.width(), scaled.height(), 12, 12);
                painter.setClipPath(path);
                painter.drawPixmap(0, 0, scaled);
                painter.end();

                imageLabel->setPixmap(rounded);
                imageLabel->setCursor(Qt::PointingHandCursor);
                imageLabel->setProperty("imagePath", localPath);
                imageLabel->installEventFilter(this);
                imageLabel->setStyleSheet("background: transparent;");
                imageLabel->setFixedSize(scaled.width(), scaled.height());
                bubbleLayout->addWidget(imageLabel);
            }
        }

        if (!isOutgoing) {
            QLabel *fileNameLabel = new QLabel(fileName);
            fileNameLabel->setStyleSheet("color: #cbd5e1; font-size: 11px; background: transparent;");
            fileNameLabel->setMaximumWidth(250);
            fileNameLabel->setWordWrap(true);
            bubbleLayout->addWidget(fileNameLabel);
        }
    } else {
        QLabel *messageLabel = new QLabel(text);
        messageLabel->setWordWrap(true);
        messageLabel->setStyleSheet("color: #f1f5f9; font-family: 'Segoe UI', system-ui, -apple-system, sans-serif; font-size: 16px;");
        messageLabel->setMaximumWidth(400);

        if (text.startsWith("📎 ")) {
            QStringList parts = text.mid(2).split("|");
            if (parts.size() >= 2) {
                messageLabel->setCursor(Qt::PointingHandCursor);
                messageLabel->setProperty("fileUrl", parts[1]);
                messageLabel->setProperty("fileName", parts[0]);
                messageLabel->installEventFilter(this);
            }
        }

        bubbleLayout->addWidget(messageLabel);
    }

    QHBoxLayout *alignLayout = new QHBoxLayout();
    alignLayout->setContentsMargins(0, 0, 0, 0);
    alignLayout->setSpacing(0);

    if (isOutgoing) {
        alignLayout->addStretch();
        alignLayout->addWidget(bubble);
    } else {
        alignLayout->addWidget(bubble);
        alignLayout->addStretch();
    }

    mainLayout->addLayout(alignLayout);

    if (isOutgoing) {
        QLabel *statusIcon = new QLabel();
        statusIcon->setFixedSize(15, 15);
        statusIcon->setStyleSheet("background: transparent; padding: 0px; margin: 0px;");

        QString iconPath;
        switch (status) {
        case 0: iconPath = ":/icons/darkTheme/images/darkTheme/clock-light.svg"; break;
        case 1: iconPath = ":/icons/darkTheme/images/darkTheme/check-light.svg"; break;
        case 2: iconPath = ":/icons/darkTheme/images/darkTheme/check-double-light.svg"; break;
        case 3: iconPath = ":/icons/general/images/general/check-double-green.svg"; break;
        }

        if (!iconPath.isEmpty()) {
            QPixmap pixmap(iconPath);
            if (!pixmap.isNull()) {
                statusIcon->setPixmap(pixmap.scaled(15, 15, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            }
        }

        QHBoxLayout *statusLayout = new QHBoxLayout();
        statusLayout->setContentsMargins(0, 0, 10, 0);
        statusLayout->setSpacing(0);
        statusLayout->addStretch();
        statusLayout->addWidget(statusIcon);
        mainLayout->addLayout(statusLayout);
    }

    QListWidgetItem *item = new QListWidgetItem(m_messagesList);
    item->setSizeHint(messageWidget->sizeHint());
    m_messagesList->setItemWidget(item, messageWidget);
    m_messagesList->scrollToBottom();
}

void RightPanel::clearMessages()
{
    qDebug() << "Clearing messages, count before:" << m_messagesList->count();
    m_messagesList->clear();
    qDebug() << "Messages cleared, count after:" << m_messagesList->count();
}

bool RightPanel::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_messageInput && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if ((keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) &&
            !(keyEvent->modifiers() & Qt::ShiftModifier)) {
            onSendClicked();
            return true;
        }
    }

    if (obj == m_chatAvatar && event->type() == QEvent::MouseButtonPress) {
        emit avatarClicked(m_currentContactId);
        return true;
    }

    if (event->type() == QEvent::MouseButtonPress) {
        QLabel *label = qobject_cast<QLabel*>(obj);
        if (label) {
            if (label->property("imagePath").isValid()) {
                QString imagePath = label->property("imagePath").toString();
                if (!QFile::exists(imagePath)) return true;

                PhotoViewer *viewer = new PhotoViewer(imagePath, nullptr);
                viewer->setAttribute(Qt::WA_DeleteOnClose);
                viewer->exec();
                return true;
            }

            if (label->property("isFile").toBool()) {
                QString fileName = label->property("fileName").toString();
                QString savePath = QFileDialog::getSaveFileName(this, "Сохранить файл", fileName);
                if (!savePath.isEmpty()) {
                    emit downloadFileRequested(fileName, savePath);
                }
                return true;
            }

            if (label->property("fileUrl").isValid()) {
                QString fileUrl = label->property("fileUrl").toString();
                QString fileName = label->property("fileName").toString();
                QString savePath = QFileDialog::getSaveFileName(this, "Сохранить файл", fileName);
                if (!savePath.isEmpty()) {
                    emit downloadFileRequested(fileUrl, savePath);
                }
                return true;
            }
        }
    }

    return QWidget::eventFilter(obj, event);
}

void RightPanel::onTyping()
{
    if (!typingTimer) {
        typingTimer = new QTimer(this);
        typingTimer->setSingleShot(true);
        connect(typingTimer, &QTimer::timeout, this, [this]() {
            emit typingStop();
        });
    }
    emit typing();
    typingTimer->start(1500);
}

void RightPanel::showTyping(const QString &username)
{
    if (!m_typingLabel) return;
    m_typingLabel->setText(username + " печатает...");
    m_typingLabel->show();

    if (!typingDisplayTimer) {
        typingDisplayTimer = new QTimer(this);
        typingDisplayTimer->setSingleShot(true);
        connect(typingDisplayTimer, &QTimer::timeout, this, [this]() {
            m_typingLabel->hide();
        });
    }
    typingDisplayTimer->start(2000);
}

void RightPanel::setHasMore(bool hasMore) { m_hasMore = hasMore; }
void RightPanel::setLoadingOlder(bool loading) { m_loadingOlder = loading; }

void RightPanel::onScrollChanged(int value)
{
    if (value == 0 && m_hasMore && !m_loadingOlder) {
        emit needLoadOlder();
    }
}

void RightPanel::setupChatHeader()
{
    m_chatHeader = new QWidget();
    m_chatHeader->setStyleSheet("background-color: #1e293b;");
    m_chatHeader->setFixedHeight(70);

    QHBoxLayout *headerLayout = new QHBoxLayout(m_chatHeader);
    headerLayout->setContentsMargins(16, 8, 16, 8);
    headerLayout->setSpacing(12);

    m_chatAvatar = new QLabel();
    m_chatAvatar->setFixedSize(44, 44);
    m_chatAvatar->setStyleSheet(
        "background: #1d4ed8; border-radius: 12px; color: white; font-size: 18px; font-weight: bold;"
        );
    m_chatAvatar->setScaledContents(true);
    m_chatAvatar->setAlignment(Qt::AlignCenter);
    m_chatAvatar->setText("?");
    m_chatAvatar->setCursor(Qt::PointingHandCursor);
    m_chatAvatar->installEventFilter(this);
    m_chatAvatar->setProperty("isChatAvatar", true);
    headerLayout->addWidget(m_chatAvatar);

    QVBoxLayout *infoLayout = new QVBoxLayout();
    infoLayout->setSpacing(2);

    m_chatTitle = new QLabel("Чат");
    m_chatTitle->setStyleSheet("color: #f1f5f9; font-size: 16px; font-weight: bold; background: transparent;");

    m_lastSeenLabel = new QLabel("");
    m_lastSeenLabel->setStyleSheet("color: #94A3B8; font-size: 12px; background: transparent;");

    infoLayout->addWidget(m_chatTitle);
    infoLayout->addWidget(m_lastSeenLabel);
    headerLayout->addLayout(infoLayout, 1);

    m_pinBtn = new QPushButton();
    m_pinBtn->setFixedSize(36, 36);
    m_pinBtn->setCursor(Qt::PointingHandCursor);
    m_pinBtn->setIcon(QIcon(":/icons/darkTheme/images/darkTheme/thumbtack.svg"));
    m_pinBtn->setIconSize(QSize(22, 22));
    m_pinBtn->setStyleSheet(
        "QPushButton { background: transparent; border: none; }"
        "QPushButton:hover { background: #334155; border-radius: 8px; }"
        );
    connect(m_pinBtn, &QPushButton::clicked, this, [this]() {
        m_isPinned = !m_isPinned;
        m_pinBtn->setIcon(QIcon(m_isPinned ?
                                    ":/icons/darkTheme/images/darkTheme/thumbtack-slash.svg" :
                                    ":/icons/darkTheme/images/darkTheme/thumbtack.svg"));
        m_pinBtn->setIconSize(QSize(18, 18));
        emit pinToggled(m_isPinned);
    });
    headerLayout->addWidget(m_pinBtn);

    m_muteBtn = new QPushButton();
    m_muteBtn->setFixedSize(36, 36);
    m_muteBtn->setCursor(Qt::PointingHandCursor);
    m_muteBtn->setIcon(QIcon(":/icons/darkTheme/images/darkTheme/bell.svg"));
    m_muteBtn->setIconSize(QSize(22, 22));
    m_muteBtn->setStyleSheet(
        "QPushButton { background: transparent; border: none; }"
        "QPushButton:hover { background: #334155; border-radius: 8px; }"
        );
    connect(m_muteBtn, &QPushButton::clicked, this, [this]() {
        m_isMuted = !m_isMuted;
        m_muteBtn->setIcon(QIcon(m_isMuted ?
                                     ":/icons/darkTheme/images/darkTheme/bell-slash.svg" :
                                     ":/icons/darkTheme/images/darkTheme/bell.svg"));
    });
    headerLayout->addWidget(m_muteBtn);
}

void RightPanel::setContactInfo(int userId, const QString &name, bool online, const QString &lastSeen)
{
    m_currentContactId = userId;
    m_chatTitle->setText(name);

    if (m_chatAvatar->pixmap().isNull() && m_chatAvatar->text().isEmpty()) {
        m_chatAvatar->setText(name.left(1).toUpper());
    }

    if (online) {
        m_lastSeenLabel->setText("🟢 В сети");
        m_lastSeenLabel->setStyleSheet("color: #10B981; font-size: 12px; background: transparent;");
    } else {
        m_lastSeenLabel->setText("⚫ Не в сети");
        m_lastSeenLabel->setStyleSheet("color: #94A3B8; font-size: 12px; background: transparent;");
    }
}

void RightPanel::setChatAvatar(const QString &avatarPath)
{
    if (!m_chatAvatar) return;

    qDebug() << "setChatAvatar called with path:" << avatarPath;

    if (!avatarPath.isEmpty()) {
        QPixmap pixmap;

        // Проверяем локальный путь или кеш
        if (QFile::exists(avatarPath)) {
            pixmap.load(avatarPath);
            qDebug() << "Loaded avatar directly from:" << avatarPath;
        } else if (avatarPath.startsWith("/files/")) {
            // Пробуем загрузить из кеша
            QString cachePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
                                + "/avatars/" + QString::number(m_currentContactId) + ".jpg";
            if (QFile::exists(cachePath)) {
                pixmap.load(cachePath);
                qDebug() << "Loaded avatar from cache:" << cachePath;
            }
        }

        if (!pixmap.isNull()) {
            QPixmap rounded(44, 44);
            rounded.fill(Qt::transparent);
            QPainter painter(&rounded);
            painter.setRenderHint(QPainter::Antialiasing, true);
            QPainterPath path;
            path.addRoundedRect(0, 0, 44, 44, 12, 12);
            painter.setClipPath(path);
            painter.drawPixmap(0, 0, pixmap.scaled(44, 44, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
            painter.end();

            m_chatAvatar->setPixmap(rounded);
            m_chatAvatar->setText("");
            m_chatAvatar->setStyleSheet("border-radius: 12px; background: transparent;");
            qDebug() << "Chat avatar updated successfully";
            return;
        }
    }

    // Если не загрузили - показываем первую букву
    QString title = m_chatTitle->text();
    if (!title.isEmpty()) {
        m_chatAvatar->setText(title.left(1).toUpper());
    } else {
        m_chatAvatar->setText("?");
    }
    m_chatAvatar->setStyleSheet(
        "background: #1d4ed8; border-radius: 12px; color: white; font-size: 18px; font-weight: bold;"
        );
}

void RightPanel::setPinned(bool pinned)
{
    m_isPinned = pinned;
    m_pinBtn->setIcon(QIcon(pinned ?
                                ":/icons/darkTheme/images/darkTheme/thumbtack-slash.svg" :
                                ":/icons/darkTheme/images/darkTheme/thumbtack.svg"));
    m_pinBtn->setIconSize(QSize(18, 18));
}

void RightPanel::addImageMessage(const QString &filePath, bool isOutgoing, int status)
{
    QWidget *messageWidget = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(messageWidget);
    mainLayout->setContentsMargins(10, 5, 10, 5);
    mainLayout->setSpacing(2);

    QWidget *bubble = new QWidget();
    bubble->setStyleSheet(
        QString("background-color: %1; border-radius: 12px; padding: 4px;")
            .arg(isOutgoing ? "#1d4ed8" : "#334155")
        );

    QVBoxLayout *bubbleLayout = new QVBoxLayout(bubble);
    bubbleLayout->setContentsMargins(4, 4, 4, 4);
    bubbleLayout->setSpacing(4);

    QPixmap pixmap(filePath);
    if (!pixmap.isNull()) {
        QPixmap scaled = pixmap.scaled(250, 250, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        QPixmap rounded(scaled.size());
        rounded.fill(Qt::transparent);
        QPainter painter(&rounded);
        painter.setRenderHint(QPainter::Antialiasing, true);
        QPainterPath path;
        path.addRoundedRect(0, 0, scaled.width(), scaled.height(), 12, 12);
        painter.setClipPath(path);
        painter.drawPixmap(0, 0, scaled);
        painter.end();

        QLabel *imageLabel = new QLabel();
        imageLabel->setPixmap(rounded);
        imageLabel->setCursor(Qt::PointingHandCursor);
        imageLabel->setProperty("imagePath", filePath);
        imageLabel->installEventFilter(this);
        imageLabel->setStyleSheet("background: transparent;");
        imageLabel->setFixedSize(scaled.width(), scaled.height());
        bubbleLayout->addWidget(imageLabel);
    }

    QHBoxLayout *alignLayout = new QHBoxLayout();
    alignLayout->setContentsMargins(0, 0, 0, 0);
    alignLayout->setSpacing(0);

    if (isOutgoing) {
        alignLayout->addStretch();
        alignLayout->addWidget(bubble);
    } else {
        alignLayout->addWidget(bubble);
        alignLayout->addStretch();
    }

    mainLayout->addLayout(alignLayout);

    if (isOutgoing) {
        QLabel *statusIcon = new QLabel();
        statusIcon->setFixedSize(15, 15);
        statusIcon->setStyleSheet("background: transparent;");

        QString iconPath;
        switch (status) {
        case 0: iconPath = ":/icons/darkTheme/images/darkTheme/clock-light.svg"; break;
        case 1: iconPath = ":/icons/darkTheme/images/darkTheme/check-light.svg"; break;
        case 2: iconPath = ":/icons/darkTheme/images/darkTheme/check-double-light.svg"; break;
        case 3: iconPath = ":/icons/general/images/general/check-double-green.svg"; break;
        }

        if (!iconPath.isEmpty()) {
            QPixmap p(iconPath);
            if (!p.isNull()) {
                statusIcon->setPixmap(p.scaled(15, 15, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            }
        }

        QHBoxLayout *statusLayout = new QHBoxLayout();
        statusLayout->setContentsMargins(0, 0, 10, 0);
        statusLayout->addStretch();
        statusLayout->addWidget(statusIcon);
        mainLayout->addLayout(statusLayout);
    }

    QListWidgetItem *item = new QListWidgetItem(m_messagesList);
    item->setSizeHint(messageWidget->sizeHint());
    m_messagesList->setItemWidget(item, messageWidget);
    m_messagesList->scrollToBottom();
}
