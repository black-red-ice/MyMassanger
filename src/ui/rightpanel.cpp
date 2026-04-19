#include "rightpanel.h"
#include <QHBoxLayout>
#include <QListWidgetItem>

RightPanel::RightPanel(QWidget *parent) : QWidget(parent)
{
    setupUI();
}

void RightPanel::setupUI()
{
    setStyleSheet("background-color: #0f172a;");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Заголовок чата
    QWidget *chatHeader = new QWidget();
    chatHeader->setStyleSheet("background-color: #1e293b; border-bottom: 1px solid #334155;");
    chatHeader->setFixedHeight(70);

    QHBoxLayout *headerLayout = new QHBoxLayout(chatHeader);
    m_chatTitle = new QLabel("Чат");
    m_chatTitle->setStyleSheet("color: #f1f5f9; font-size: 16px; font-weight: bold;");
    headerLayout->addWidget(m_chatTitle);
    headerLayout->addStretch();

    mainLayout->addWidget(chatHeader);

    // Список сообщений
    m_messagesList = new QListWidget();
    m_messagesList->setStyleSheet(
        "QListWidget { background-color: #0f172a; border: none; outline: none; }"
        "QListWidget::item { padding: 4px; }"
        );
    m_messagesList->setWordWrap(true);
    m_messagesList->setSelectionMode(QAbstractItemView::NoSelection);

    mainLayout->addWidget(m_messagesList);

    // Поле ввода
    QWidget *inputArea = new QWidget();
    inputArea->setStyleSheet("background-color: #1e293b; border-top: 1px solid #334155;");
    inputArea->setFixedHeight(80);

    QHBoxLayout *inputLayout = new QHBoxLayout(inputArea);
    m_messageInput = new QTextEdit();
    m_messageInput->setPlaceholderText("Напишите сообщение...");
    m_messageInput->setStyleSheet(
        "QTextEdit { background-color: #334155; border: 1px solid #475569; border-radius: 20px; padding: 10px; color: #f1f5f9; }"
        );
    m_messageInput->setMaximumHeight(60);

    m_sendBtn = new QPushButton("→");
    m_sendBtn->setFixedSize(40, 40);
    m_sendBtn->setCursor(Qt::PointingHandCursor);
    m_sendBtn->setStyleSheet(
        "QPushButton { background-color: #1d4ed8; border: none; border-radius: 20px; color: white; font-size: 20px; font-weight: bold; }"
        "QPushButton:hover { background-color: #2563eb; }"
        );

    connect(m_sendBtn, &QPushButton::clicked, this, &RightPanel::onSendClicked);

    inputLayout->addWidget(m_messageInput);
    inputLayout->addWidget(m_sendBtn);

    mainLayout->addWidget(inputArea);
}

void RightPanel::onSendClicked()
{
    QString message = m_messageInput->toPlainText().trimmed();
    if (!message.isEmpty()) {
        emit sendMessageRequested(message);
        m_messageInput->clear();
    }
}

void RightPanel::setChatTitle(const QString &title)
{
    m_chatTitle->setText(title);
}

void RightPanel::addMessage(const QString &text, bool isOutgoing)
{
    // Создаём виджет для сообщения
    QWidget *messageWidget = new QWidget();
    QHBoxLayout *layout = new QHBoxLayout(messageWidget);
    layout->setContentsMargins(10, 5, 10, 5);

    QLabel *messageLabel = new QLabel(text);
    messageLabel->setWordWrap(true);
    messageLabel->setStyleSheet(
        QString("background-color: %1; border-radius: 12px; padding: 8px 12px; color: #f1f5f9;")
            .arg(isOutgoing ? "#1d4ed8" : "#334155")
        );
    messageLabel->setMaximumWidth(400);

    if (isOutgoing) {
        layout->addStretch();
        layout->addWidget(messageLabel);
    } else {
        layout->addWidget(messageLabel);
        layout->addStretch();
    }

    // Добавляем в список
    QListWidgetItem *item = new QListWidgetItem(m_messagesList);
    item->setSizeHint(messageWidget->sizeHint());
    m_messagesList->setItemWidget(item, messageWidget);

    // Прокручиваем к новому сообщению
    m_messagesList->scrollToBottom();
}

void RightPanel::clearMessages()
{
    qDebug() << "Clearing messages, count before:" << m_messagesList->count();
    m_messagesList->clear();
    qDebug() << "Messages cleared, count after:" << m_messagesList->count();
}
