#include "newticketdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QPushButton>
#include <QFileDialog>
#include <QDateTime>

NewTicketDialog::NewTicketDialog(const QString &category, const QString &categoryName,
                                 const QString &categoryColor, QWidget *parent)
    : OverlayDialog(parent), m_category(category), m_categoryName(categoryName),
    m_categoryColor(categoryColor)
{
    setupUI();
}

void NewTicketDialog::setupUI()
{
    setFixedSize(500, 620);

    QWidget *container = new QWidget(this);
    container->setObjectName("container");
    container->setStyleSheet("#container { background-color: #1e293b; border-radius: 16px; }");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(container);

    QVBoxLayout *containerLayout = new QVBoxLayout(container);
    containerLayout->setSpacing(0);
    containerLayout->setContentsMargins(0, 0, 0, 0);

    // Заголовок с цветом категории
    QWidget *header = new QWidget();
    header->setStyleSheet(QString("background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 %1, stop:1 %2); border-top-left-radius: 16px; border-top-right-radius: 16px;")
                              .arg(m_categoryColor).arg(m_categoryColor + "80"));
    header->setFixedHeight(80);

    QHBoxLayout *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(24, 0, 24, 0);

    QLabel *titleLabel = new QLabel("Новое обращение - " + m_categoryName);
    titleLabel->setStyleSheet("color: white; font-size: 18px; font-weight: 600;");

    QPushButton *closeBtn = new QPushButton("✕");
    closeBtn->setFixedSize(36, 36);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(
        "QPushButton { background-color: rgba(255,255,255,0.1); border: none; border-radius: 18px; color: white; font-size: 16px; }"
        "QPushButton:hover { background-color: rgba(255,255,255,0.2); }"
        );
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);

    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(closeBtn);
    containerLayout->addWidget(header);

    // Контент
    QWidget *content = new QWidget();
    QVBoxLayout *contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(24, 24, 24, 24);
    contentLayout->setSpacing(20);

    // Поле "Тема"
    QLabel *subjectLabel = new QLabel("Тема обращения");
    subjectLabel->setStyleSheet("color: #f1f5f9; font-size: 14px; font-weight: 500;");
    m_subjectEdit = new QLineEdit();
    m_subjectEdit->setPlaceholderText("Например: Не работает VPN, Ошибка в программе");
    m_subjectEdit->setStyleSheet("background-color: #334155; border: 1px solid #475569; border-radius: 12px; padding: 12px; color: white;");

    // Поле "Описание"
    QLabel *messageLabel = new QLabel("Описание проблемы");
    messageLabel->setStyleSheet("color: #f1f5f9; font-size: 14px; font-weight: 500;");
    m_messageEdit = new QTextEdit();
    m_messageEdit->setPlaceholderText("Опишите проблему максимально подробно...\n• Что именно происходит?\n• Когда возникла проблема?\n• Какие действия привели к ошибке?");
    m_messageEdit->setStyleSheet("background-color: #334155; border: 1px solid #475569; border-radius: 12px; padding: 12px; color: white;");
    m_messageEdit->setMinimumHeight(150);

    // Приоритет
    QLabel *priorityLabel = new QLabel("Приоритет");
    priorityLabel->setStyleSheet("color: #f1f5f9; font-size: 14px; font-weight: 500;");
    m_priorityCombo = new QComboBox();
    m_priorityCombo->addItem("🟢 Низкий", "low");
    m_priorityCombo->addItem("🟡 Средний", "medium");
    m_priorityCombo->addItem("🔴 Высокий", "high");
    m_priorityCombo->setCurrentIndex(1);
    m_priorityCombo->setStyleSheet("background-color: #334155; border: 1px solid #475569; border-radius: 12px; padding: 10px; color: white;");

    // Кнопка прикрепления файлов
    m_attachBtn = new QPushButton("📎 Прикрепить файлы");
    m_attachBtn->setStyleSheet("background-color: #334155; border: 1px solid #475569; border-radius: 12px; padding: 10px; color: #94a3b8;");
    connect(m_attachBtn, &QPushButton::clicked, this, &NewTicketDialog::onAttachFile);

    m_attachedFilesLabel = new QLabel();
    m_attachedFilesLabel->setStyleSheet("color: #64748b; font-size: 12px;");
    m_attachedFilesLabel->setVisible(false);

    // Кнопки действий
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(12);

    QPushButton *cancelBtn = new QPushButton("Отмена");
    cancelBtn->setStyleSheet("background-color: #334155; border: none; border-radius: 12px; padding: 12px; color: #94a3b8; font-weight: 500;");
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    m_sendBtn = new QPushButton("Отправить обращение");
    m_sendBtn->setStyleSheet(QString("background-color: %1; border: none; border-radius: 12px; padding: 12px; color: white; font-weight: 500;").arg(m_categoryColor));
    connect(m_sendBtn, &QPushButton::clicked, this, &NewTicketDialog::onSendClicked);

    buttonLayout->addWidget(cancelBtn);
    buttonLayout->addWidget(m_sendBtn);

    contentLayout->addWidget(subjectLabel);
    contentLayout->addWidget(m_subjectEdit);
    contentLayout->addWidget(messageLabel);
    contentLayout->addWidget(m_messageEdit);
    contentLayout->addWidget(priorityLabel);
    contentLayout->addWidget(m_priorityCombo);
    contentLayout->addWidget(m_attachBtn);
    contentLayout->addWidget(m_attachedFilesLabel);
    contentLayout->addLayout(buttonLayout);

    containerLayout->addWidget(content);
}

void NewTicketDialog::onAttachFile()
{
    QStringList files = QFileDialog::getOpenFileNames(this, "Выберите файлы");
    if (!files.isEmpty()) {
        m_attachedFiles = files;
        m_attachedFilesLabel->setText(QString("Прикреплено файлов: %1").arg(files.size()));
        m_attachedFilesLabel->setVisible(true);
    }
}

void NewTicketDialog::onSendClicked()
{
    if (m_subjectEdit->text().trimmed().isEmpty()) {
        // Показать предупреждение
        return;
    }
    if (m_messageEdit->toPlainText().trimmed().isEmpty()) {
        return;
    }
    accept();
}

NewTicketDialog::TicketData NewTicketDialog::getTicketData() const
{
    TicketData data;
    data.subject = m_subjectEdit->text().trimmed();
    data.message = m_messageEdit->toPlainText().trimmed();
    data.priority = m_priorityCombo->currentData().toString();
    data.category = m_category;
    return data;
}
