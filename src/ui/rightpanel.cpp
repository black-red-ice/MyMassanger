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
#include <QRegularExpression>
#include <QTextBrowser>

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

    // Создаем ScrollArea для сообщений
    m_scrollArea = new QScrollArea();
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setStyleSheet(
        "QScrollArea { background-color: #0f172a; border: none; }"
        "QScrollBar:vertical { background: transparent; width: 6px; margin: 0px; }"
        "QScrollBar::handle:vertical { background: #334155; border-radius: 3px; min-height: 30px; }"
        );

    // Контейнер для сообщений
    m_messagesContainer = new QWidget();
    m_messagesContainer->setStyleSheet("background: #0f172a;");
    m_messagesLayout = new QVBoxLayout(m_messagesContainer);
    m_messagesLayout->setContentsMargins(8, 8, 8, 8);
    m_messagesLayout->setSpacing(4);
    m_messagesLayout->addStretch(); // Stretch внизу

    // 🔥 ВАЖНО: контейнер не должен растягиваться по горизонтали
    m_messagesContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_scrollArea->setWidget(m_messagesContainer);
    mainLayout->addWidget(m_scrollArea);

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
    bool isFileMessage = text.startsWith("📎 ");
    QStringList parts;
    if (isImageMessage) {
        parts = text.mid(2).split("|");
    } else if (isFileMessage) {
        parts = text.mid(2).split("|");
    }

    QWidget *messageWidget = new QWidget();
    messageWidget->setAttribute(Qt::WA_StyledBackground, true);
    messageWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    QHBoxLayout *mainLayout = new QHBoxLayout(messageWidget);
    mainLayout->setContentsMargins(8, 2, 8, 2);
    mainLayout->setSpacing(0);

    QWidget *bubble = new QWidget();
    bubble->setAttribute(Qt::WA_StyledBackground, true);
    bubble->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    bubble->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::MinimumExpanding);

    int maxBubbleWidth = qMax(220, (int)(this->width() * 0.33));
    bubble->setMaximumWidth(maxBubbleWidth);

    QString bubbleStyle;
    if (isOutgoing) {
        bubbleStyle =
            "background-color: #1d4ed8;"
            "border-radius: 14px;"
            "border-bottom-right-radius: 3px;"
            "padding: 8px 12px;";
    } else {
        bubbleStyle =
            "background-color: #334155;"
            "border-radius: 14px;"
            "border-bottom-left-radius: 3px;"
            "padding: 8px 12px;";
    }
    bubble->setStyleSheet(bubbleStyle);

    QVBoxLayout *bubbleLayout = new QVBoxLayout(bubble);
    bubbleLayout->setContentsMargins(0, 0, 0, 0);
    bubbleLayout->setSpacing(4);
    bubbleLayout->setContentsMargins(12, 8, 12, 8);

    // === ИЗОБРАЖЕНИЕ ===
    if (isImageMessage && parts.size() >= 2) {
        QString fileName = parts[0].trimmed();
        QString fileUrl = parts[1];

        auto addImageToBubble = [this, bubbleLayout, maxBubbleWidth](const QString &localPath) {
            QPixmap pixmap(localPath);
            if (!pixmap.isNull()) {
                int maxWidth = qMin(180, maxBubbleWidth - 24);
                int maxHeight = 180;
                QPixmap scaled = pixmap.scaled(maxWidth, maxHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);

                QLabel *imageLabel = new QLabel();
                imageLabel->setPixmap(scaled);
                imageLabel->setCursor(Qt::PointingHandCursor);
                imageLabel->setProperty("imagePath", localPath);
                imageLabel->installEventFilter(this);
                imageLabel->setStyleSheet("background: transparent; border-radius: 8px;");
                imageLabel->setAlignment(Qt::AlignCenter);
                bubbleLayout->addWidget(imageLabel);
            }
        };

        QString localPath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
                            + "/chat_images/" + fileName;
        QDir().mkpath(QFileInfo(localPath).path());

        if (QFile::exists(localPath)) {
            addImageToBubble(localPath);
        } else {
            QLabel *loadingLabel = new QLabel("📷 Загрузка...");
            loadingLabel->setStyleSheet("color: #94a3b8; font-size: 11px; background: transparent;");
            loadingLabel->setAlignment(Qt::AlignCenter);
            bubbleLayout->addWidget(loadingLabel);

            QNetworkAccessManager *manager = new QNetworkAccessManager(this);
            QNetworkRequest request(QUrl("http://87.242.118.96:8080" + fileUrl));
            QNetworkReply *reply = manager->get(request);

            connect(reply, &QNetworkReply::finished, this, [reply, loadingLabel, localPath, addImageToBubble]() {
                if (reply->error() == QNetworkReply::NoError) {
                    QByteArray data = reply->readAll();
                    QFile file(localPath);
                    if (file.open(QIODevice::WriteOnly)) {
                        file.write(data);
                        file.close();
                        loadingLabel->deleteLater();
                        addImageToBubble(localPath);
                    }
                } else {
                    loadingLabel->setText("❌ Ошибка");
                }
                reply->deleteLater();
            });
        }
    }
    // === ФАЙЛ ===
    else if (isFileMessage && parts.size() >= 2) {
        QString fileName = parts[0].trimmed();
        QString fileUrl = parts[1];
        QString fileSize = parts.size() >= 3 ? parts[2] : "";

        QHBoxLayout *fileLayout = new QHBoxLayout();
        fileLayout->setSpacing(8);

        QLabel *fileIcon = new QLabel();
        QString ext = QFileInfo(fileName).suffix().toLower();
        QString iconPath = ":/icons/general/images/general/";
        if (ext == "pdf") iconPath += "file-pdf.svg";
        else if (ext == "doc" || ext == "docx") iconPath += "file-word.svg";
        else if (ext == "xls" || ext == "xlsx") iconPath += "file-excel.svg";
        else if (ext == "jpg" || ext == "png" || ext == "gif") iconPath += "file-image.svg";
        else iconPath += "file.svg";

        QPixmap pixmap(iconPath);
        fileIcon->setPixmap(pixmap.scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        fileIcon->setStyleSheet("background: transparent;");
        fileLayout->addWidget(fileIcon);

        QVBoxLayout *fileInfoLayout = new QVBoxLayout();
        fileInfoLayout->setSpacing(2);

        QLabel *nameLabel = new QLabel(fileName);
        nameLabel->setStyleSheet("color: #f1f5f9; font-size: 12px; font-weight: 500; background: transparent;");
        nameLabel->setWordWrap(true);
        nameLabel->setMaximumWidth(maxBubbleWidth - 50);
        nameLabel->setCursor(Qt::PointingHandCursor);
        nameLabel->setProperty("fileUrl", fileUrl);
        nameLabel->setProperty("fileName", fileName);
        nameLabel->installEventFilter(this);
        fileInfoLayout->addWidget(nameLabel);

        if (!fileSize.isEmpty()) {
            QLabel *sizeLabel = new QLabel(fileSize);
            sizeLabel->setStyleSheet("color: #94a3b8; font-size: 10px; background: transparent;");
            fileInfoLayout->addWidget(sizeLabel);
        }

        fileLayout->addLayout(fileInfoLayout, 1);
        bubbleLayout->addLayout(fileLayout);
    }
    // === ТЕКСТОВОЕ СООБЩЕНИЕ ===
    else {
        QString wrappedText = text;
        wrappedText.replace(
            QRegularExpression("(\\S{25})"),
            "\\1\u200B"
            );
        QLabel *messageLabel = new QLabel(wrappedText);
        messageLabel->setWordWrap(true);
        messageLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
        messageLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        messageLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
        messageLabel->setMaximumWidth(maxBubbleWidth - 24);
        messageLabel->setStyleSheet(
            "color: #f1f5f9;"
            "font-family: 'Segoe UI', system-ui, sans-serif;"
            "font-size: 13px;"
            "background: transparent;"
            );

        messageLabel->setMinimumHeight(1);
        messageLabel->adjustSize();
        bubbleLayout->addWidget(messageLabel);
    }

    // === ВРЕМЯ И СТАТУС ===
    QHBoxLayout *metaLayout = new QHBoxLayout();
    metaLayout->setContentsMargins(0, 4, 0, 0);
    metaLayout->setSpacing(4);
    metaLayout->addStretch();

    QLabel *timeLabel = new QLabel(QDateTime::currentDateTime().toString("HH:mm"));
    timeLabel->setStyleSheet("color: #94a3b8; font-size: 10px; background: transparent;");
    metaLayout->addWidget(timeLabel);

    if (isOutgoing) {
        QString statusIcon;
        QString statusColor;
        switch (status) {
        case 0: statusIcon = "⏳"; statusColor = "#94a3b8"; break;
        case 1: statusIcon = "✓"; statusColor = "#94a3b8"; break;
        case 2: statusIcon = "✓✓"; statusColor = "#94a3b8"; break;
        case 3: statusIcon = "✓✓"; statusColor = "#10b981"; break;
        default: statusIcon = ""; statusColor = "#94a3b8";
        }

        QLabel *statusLabel = new QLabel(statusIcon);
        statusLabel->setStyleSheet(QString("color: %1; font-size: 11px; background: transparent;").arg(statusColor));
        statusLabel->setFixedWidth(20);
        metaLayout->addWidget(statusLabel);
    }

    bubbleLayout->addLayout(metaLayout);

    // Выравнивание пузырька
    if (isOutgoing) {
        mainLayout->addStretch();
        mainLayout->addWidget(bubble);
    } else {
        mainLayout->addWidget(bubble);
        mainLayout->addStretch();
    }

    // Вставляем сообщение перед stretch
    int insertIndex = m_messagesLayout->count() - 1;
    m_messagesLayout->insertWidget(insertIndex, messageWidget);

    // Прокрутка вниз
    QTimer::singleShot(50, this, [this]() {
        QScrollBar *scrollBar = m_scrollArea->verticalScrollBar();
        if (scrollBar) {
            scrollBar->setValue(scrollBar->maximum());
        }
    });
}

void RightPanel::clearMessages()
{
    qDebug() << "Clearing messages";
    while (m_messagesLayout->count() > 1) {
        QLayoutItem *item = m_messagesLayout->takeAt(0);
        if (item->widget()) delete item->widget();
        delete item;
    }
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
    qDebug() << "🖼 addImageMessage:" << filePath << "outgoing:" << isOutgoing << "status:" << status;

    QWidget *messageWidget = new QWidget();
    messageWidget->setAttribute(Qt::WA_StyledBackground, true);
    messageWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    QHBoxLayout *mainLayout = new QHBoxLayout(messageWidget);
    mainLayout->setContentsMargins(8, 2, 8, 2);
    mainLayout->setSpacing(0);

    QWidget *bubble = new QWidget();
    bubble->setAttribute(Qt::WA_StyledBackground, true);
    bubble->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

    int maxBubbleWidth = qMax(220, (int)(this->width() * 0.33));
    bubble->setMaximumWidth(maxBubbleWidth);

    QString bubbleStyle;
    if (isOutgoing) {
        bubbleStyle =
            "background-color: #1d4ed8;"
            "border-radius: 14px;"
            "border-bottom-right-radius: 3px;";
    } else {
        bubbleStyle =
            "background-color: #334155;"
            "border-radius: 14px;"
            "border-bottom-left-radius: 3px;";
    }
    bubble->setStyleSheet(bubbleStyle);

    QVBoxLayout *bubbleLayout = new QVBoxLayout(bubble);
    bubbleLayout->setContentsMargins(0, 0, 0, 0);
    bubbleLayout->setSpacing(4);

    QPixmap pixmap(filePath);
    if (!pixmap.isNull()) {
        int maxWidth = qMin(200, maxBubbleWidth - 24);
        int maxHeight = 200;
        QPixmap scaled = pixmap.scaled(maxWidth, maxHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);

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

    // Время и статус
    QHBoxLayout *metaLayout = new QHBoxLayout();
    metaLayout->setContentsMargins(0, 4, 0, 0);
    metaLayout->setSpacing(4);
    metaLayout->addStretch();

    QLabel *timeLabel = new QLabel(QDateTime::currentDateTime().toString("HH:mm"));
    timeLabel->setStyleSheet("color: #94a3b8; font-size: 10px; background: transparent;");
    metaLayout->addWidget(timeLabel);

    if (isOutgoing) {
        QString statusIcon;
        QString statusColor;
        switch (status) {
        case 0: statusIcon = "⏳"; statusColor = "#94a3b8"; break;
        case 1: statusIcon = "✓"; statusColor = "#94a3b8"; break;
        case 2: statusIcon = "✓✓"; statusColor = "#94a3b8"; break;
        case 3: statusIcon = "✓✓"; statusColor = "#10b981"; break;
        default: statusIcon = ""; statusColor = "#94a3b8";
        }

        QLabel *statusLabel = new QLabel(statusIcon);
        statusLabel->setStyleSheet(QString("color: %1; font-size: 11px; background: transparent;").arg(statusColor));
        statusLabel->setFixedWidth(20);
        metaLayout->addWidget(statusLabel);
    }

    bubbleLayout->addLayout(metaLayout);

    // Выравнивание пузырька
    if (isOutgoing) {
        mainLayout->addStretch();
        mainLayout->addWidget(bubble);
    } else {
        mainLayout->addWidget(bubble);
        mainLayout->addStretch();
    }

    // Вставляем сообщение перед stretch
    int insertIndex = m_messagesLayout->count() - 1;
    m_messagesLayout->insertWidget(insertIndex, messageWidget);

    // Прокрутка вниз
    QTimer::singleShot(50, this, [this]() {
        QScrollBar *scrollBar = m_scrollArea->verticalScrollBar();
        if (scrollBar) {
            scrollBar->setValue(scrollBar->maximum());
        }
    });
}
