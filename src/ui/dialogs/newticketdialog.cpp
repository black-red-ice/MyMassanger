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
#include <QListWidget>
#include <QMessageBox>
#include <QFrame>
#include <QApplication>
#include <QPixmap>
#include <QIcon>
#include <QTimer>
#include <QFileInfo>
#include <QTextCursor>
#include <QDebug>

NewTicketDialog::NewTicketDialog(
    const TicketDialogConfig &config,
    QWidget *parent
    )
    : OverlayDialog(parent),
    m_config(config),
    m_reopenSupport(true)
{
    qDebug() << "🟢 NewTicketDialog создан, m_reopenSupport = true";
    setupUI();
}


NewTicketDialog::~NewTicketDialog()
{
    qDebug() << "🔴 NewTicketDialog уничтожен";
}

void NewTicketDialog::setupUI()
{
    setFixedSize(560, 620);

    setStyleSheet(
        "QDialog { "
        "   background: transparent; "
        "}"
        );

    QWidget *container =
        new QWidget(this);

    container->setObjectName("container");

    container->setStyleSheet(
        "#container { "
        "   background-color: #0F172A; "
        "   border-radius: 28px; "
        "   border: 1px solid rgba(255,255,255,0.06); "
        "}"
        );

    QVBoxLayout *mainLayout =
        new QVBoxLayout(this);

    mainLayout->setContentsMargins(
        0,
        0,
        0,
        0
        );

    mainLayout->addWidget(container);

    QVBoxLayout *containerLayout =
        new QVBoxLayout(container);

    containerLayout->setSpacing(0);

    containerLayout->setContentsMargins(
        0,
        0,
        0,
        0
        );

    // ================= HEADER =================

    QWidget *header = new QWidget();

    header->setFixedHeight(110);

    header->setStyleSheet(QString(
                              "background-color: %1;"
                              "border-top-left-radius: 28px;"
                              "border-top-right-radius: 28px;"
                              ).arg(m_config.categoryColor));

    QHBoxLayout *headerLayout =
        new QHBoxLayout(header);

    headerLayout->setContentsMargins(
        32,
        20,
        32,
        20
        );

    headerLayout->setSpacing(14);

    QWidget *iconContainer =
        new QWidget();

    iconContainer->setFixedSize(56, 56);

    iconContainer->setStyleSheet(
        "background-color: rgba(255,255,255,0.15);"
        "border-radius: 20px;"
        );

    QHBoxLayout *iconLayout =
        new QHBoxLayout(iconContainer);

    iconLayout->setAlignment(
        Qt::AlignCenter
        );

    QLabel *categoryIconLabel =
        new QLabel();

    QString categoryIconPath;

    if (m_config.categoryKey == "it")
        categoryIconPath =
            ":/icons/general/images/general/laptop-light.svg";
    else if (m_config.categoryKey == "hr")
        categoryIconPath =
            ":/icons/general/images/general/user-tie-light.svg";
    else if (m_config.categoryKey == "finance")
        categoryIconPath =
            ":/icons/general/images/general/coins-light.svg";
    else
        categoryIconPath =
            ":/icons/general/images/general/circle-question-light.svg";

    QPixmap categoryIcon(categoryIconPath);

    if (!categoryIcon.isNull()) {

        categoryIconLabel->setPixmap(
            categoryIcon.scaled(
                28,
                28,
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation
                )
            );
    }

    iconLayout->addWidget(categoryIconLabel);

    QWidget *textContainer =
        new QWidget();

    QVBoxLayout *textLayout =
        new QVBoxLayout(textContainer);

    textLayout->setContentsMargins(
        0,
        0,
        0,
        0
        );

    textLayout->setSpacing(4);

    QLabel *titleLabel =
        new QLabel(m_config.title);

    titleLabel->setStyleSheet(
        "color: white;"
        "font-size: 24px;"
        "font-weight: 700;"
        );

    QString categoryDisplayText;

    if (m_config.categoryKey == "it")
        categoryDisplayText = "IT поддержка";
    else if (m_config.categoryKey == "hr")
        categoryDisplayText = "HR вопросы";
    else if (m_config.categoryKey == "finance")
        categoryDisplayText = "Финансы";
    else
        categoryDisplayText = "Другое";

    QLabel *categoryLabel =
        new QLabel(categoryDisplayText);

    categoryLabel->setStyleSheet(
        "color: rgba(255,255,255,0.8);"
        "font-size: 14px;"
        );

    textLayout->addWidget(titleLabel);
    textLayout->addWidget(categoryLabel);

    QPushButton *closeBtn =
        new QPushButton("✕");

    closeBtn->setFixedSize(40, 40);

    closeBtn->setCursor(
        Qt::PointingHandCursor
        );

    closeBtn->setStyleSheet(
        "QPushButton { "
        "   background-color: rgba(255,255,255,0.1); "
        "   border: none; "
        "   border-radius: 14px; "
        "   color: white; "
        "   font-size: 18px; "
        "}"
        "QPushButton:hover { "
        "   background-color: rgba(255,255,255,0.2); "
        "}"
        );

    connect(
        closeBtn,
        &QPushButton::clicked,
        this,
        &NewTicketDialog::reject  // Было &QDialog::reject
        );

    headerLayout->addWidget(iconContainer);
    headerLayout->addWidget(textContainer);
    headerLayout->addStretch();
    headerLayout->addWidget(closeBtn);

    containerLayout->addWidget(header);

    // ================= CONTENT =================

    QWidget *content =
        new QWidget();

    content->setStyleSheet(
        "background: transparent;"
        );

    QVBoxLayout *contentLayout =
        new QVBoxLayout(content);

    contentLayout->setContentsMargins(
        32,
        20,
        32,
        24
        );

    contentLayout->setSpacing(16);

    // ================= SUBJECT =================

    QHBoxLayout *subjectHeaderLayout =
        new QHBoxLayout();

    subjectHeaderLayout->setSpacing(8);

    QLabel *subjectIcon =
        new QLabel();

    QPixmap subjectIconPixmap(
        ":/icons/general/images/general/heading-grey.svg"
        );

    if (!subjectIconPixmap.isNull()) {

        subjectIcon->setPixmap(
            subjectIconPixmap.scaled(
                18,
                18,
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation
                )
            );
    }

    QLabel *subjectLabel =
        new QLabel("Тема обращения");

    subjectLabel->setStyleSheet(
        "color: white;"
        "font-size: 14px;"
        "font-weight: 600;"
        );

    subjectHeaderLayout->addWidget(subjectIcon);
    subjectHeaderLayout->addWidget(subjectLabel);
    subjectHeaderLayout->addStretch();

    m_subjectEdit =
        new QLineEdit();

    m_subjectEdit->setPlaceholderText(
        m_config.subjectPlaceholder
        );

    m_subjectEdit->setFixedHeight(48);

    m_subjectEdit->setStyleSheet(
        "QLineEdit { "
        "   background-color: #1E293B; "
        "   border: 1px solid #334155; "
        "   border-radius: 16px; "
        "   padding: 12px 18px; "
        "   color: white; "
        "   font-size: 14px; "
        "}"
        "QLineEdit:focus { "
        "   border-color: #3B82F6; "
        "}"
        );

    // ================= MESSAGE =================

    QHBoxLayout *messageHeaderLayout =
        new QHBoxLayout();

    messageHeaderLayout->setSpacing(8);

    QLabel *messageIcon =
        new QLabel();

    QPixmap messageIconPixmap(
        ":/icons/general/images/general/align-left.svg"
        );

    if (!messageIconPixmap.isNull()) {

        messageIcon->setPixmap(
            messageIconPixmap.scaled(
                18,
                18,
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation
                )
            );
    }

    QLabel *messageLabel =
        new QLabel("Описание проблемы");

    messageLabel->setStyleSheet(
        "color: white;"
        "font-size: 14px;"
        "font-weight: 600;"
        );

    messageHeaderLayout->addWidget(messageIcon);
    messageHeaderLayout->addWidget(messageLabel);
    messageHeaderLayout->addStretch();

    QWidget *textEditContainer =
        new QWidget();

    textEditContainer->setFixedHeight(150);

    QVBoxLayout *containerEditLayout =
        new QVBoxLayout(textEditContainer);

    containerEditLayout->setContentsMargins(
        0,
        0,
        0,
        0
        );

    containerEditLayout->setSpacing(0);

    m_messageEdit =
        new QTextEdit();

    m_messageEdit->viewport()->installEventFilter(this);

    m_messageEdit->setFixedHeight(150);

    m_messageEdit->setStyleSheet(
        "QTextEdit { "
        "   background-color: #1E293B; "
        "   border: 1px solid #334155; "
        "   border-radius: 16px; "
        "   padding: 12px 18px; "
        "   color: white; "
        "   font-size: 13px; "
        "   font-family: 'Segoe UI', 'Arial', sans-serif; "
        "}"
        "QTextEdit:focus { "
        "   border-color: #3B82F6; "
        "}"
        );

    containerEditLayout->addWidget(
        m_messageEdit
        );

    // ================= PLACEHOLDER =================

    m_placeholderLabel =
        new QLabel(textEditContainer);

    QString placeholderHtml =
        "<html><body style='"
        "color:#64748B;"
        "font-size:13px;"
        "font-family:\"Segoe UI\", Arial, sans-serif;"
        "line-height:1.6;"
        "'>";

    for (int i = 0;
         i < m_config.descriptionHints.size();
         ++i)
    {
        const QString &hint =
            m_config.descriptionHints[i];

        if (i == 0)
            placeholderHtml +=
                hint + "<br>";
        else
            placeholderHtml +=
                "• " + hint + "<br>";
    }

    placeholderHtml +=
        "</body></html>";

    m_placeholderLabel->setText(
        placeholderHtml
        );

    m_placeholderLabel->setStyleSheet(
        "background: transparent;"
        );

    m_placeholderLabel->setWordWrap(true);

    m_placeholderLabel->setAlignment(
        Qt::AlignTop | Qt::AlignLeft
        );

    m_placeholderLabel->setFocusPolicy(
        Qt::NoFocus
        );

    m_placeholderLabel->setAttribute(
        Qt::WA_TransparentForMouseEvents
        );

    m_placeholderLabel->raise();

    connect(
        m_messageEdit,
        &QTextEdit::textChanged,
        this,
        [this]()
        {
            m_placeholderLabel->setVisible(
                m_messageEdit
                    ->toPlainText()
                    .trimmed()
                    .isEmpty()
                );
        }
        );

    QTextCursor cursor =
        m_messageEdit->textCursor();

    cursor.movePosition(
        QTextCursor::Start
        );

    m_messageEdit->setTextCursor(
        cursor
        );

    // ================= PRIORITY + ATTACHMENTS =================

    QHBoxLayout *priorityAttachmentsLayout = new QHBoxLayout();
    priorityAttachmentsLayout->setSpacing(12);
    priorityAttachmentsLayout->setContentsMargins(0, 0, 0, 0);

    // ===== PRIORITY =====

    QVBoxLayout *priorityLayout = new QVBoxLayout();
    priorityLayout->setSpacing(8);
    priorityLayout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *priorityHeaderLayout = new QHBoxLayout();
    priorityHeaderLayout->setSpacing(8);

    QLabel *priorityIcon = new QLabel();

    QPixmap priorityIconPixmap(":/icons/general/images/general/flag-grey.svg");
    if (!priorityIconPixmap.isNull()) {
        priorityIcon->setPixmap(priorityIconPixmap.scaled(18, 18, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    priorityIcon->setStyleSheet("background: transparent;");

    QLabel *priorityLabel = new QLabel("Приоритет");
    priorityLabel->setStyleSheet("color: white; font-size: 14px; font-weight: 600;");

    priorityHeaderLayout->addWidget(priorityIcon);
    priorityHeaderLayout->addWidget(priorityLabel);
    priorityHeaderLayout->addStretch();

    m_priorityCombo = new QComboBox();
    m_priorityCombo->setFixedHeight(48);
    m_priorityCombo->addItem("🟢 Низкий - Не срочно", "low");
    m_priorityCombo->addItem("🟡 Средний - Желательно побыстрее", "medium");
    m_priorityCombo->addItem("🔴 Высокий - Срочно!", "high");
    m_priorityCombo->setCurrentIndex(1);

    m_priorityCombo->setStyleSheet(
        "QComboBox { "
        "background-color:#1E293B;"
        "border:1px solid #334155;"
        "border-radius:16px;"
        "padding:12px 14px;"
        "color:white;"
        "font-size:13px;"
        "}"
        );

    m_priorityCombo->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);

    priorityLayout->addLayout(priorityHeaderLayout);
    priorityLayout->addWidget(m_priorityCombo);

    // ===== ATTACHMENTS =====

    QVBoxLayout *attachmentsLayout = new QVBoxLayout();
    attachmentsLayout->setSpacing(8);
    attachmentsLayout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *attachmentsHeaderLayout = new QHBoxLayout();
    attachmentsHeaderLayout->setSpacing(8);

    QLabel *attachmentsIcon = new QLabel();

    QPixmap attachmentsIconPixmap(":/icons/general/images/general/paperclip.svg");
    if (!attachmentsIconPixmap.isNull()) {
        attachmentsIcon->setPixmap(attachmentsIconPixmap.scaled(18, 18, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    attachmentsIcon->setStyleSheet("background: transparent;");

    QLabel *attachmentsTitle = new QLabel("Вложения");
    attachmentsTitle->setStyleSheet("color: white; font-size: 14px; font-weight: 600;");

    attachmentsHeaderLayout->addWidget(attachmentsIcon);
    attachmentsHeaderLayout->addWidget(attachmentsTitle);
    attachmentsHeaderLayout->addStretch();

    m_attachBtn = new QPushButton(" Нажмите для выбора файлов");
    m_attachBtn->setFixedHeight(48);
    m_attachBtn->setStyleSheet(
        "QPushButton {"
        "background-color:#1E293B;"
        "border:1px dashed #475569;"
        "border-radius:16px;"
        "color:#94A3B8;"
        "font-size:13px;"
        "}"
        );

    m_attachBtn->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);

    connect(
        m_attachBtn,
        &QPushButton::clicked,
        this,
        &NewTicketDialog::onAttachFile
        );

    attachmentsLayout->addLayout(attachmentsHeaderLayout);
    attachmentsLayout->addWidget(m_attachBtn);

    QWidget *priorityWrapper = new QWidget();
    priorityWrapper->setLayout(priorityLayout);

    QWidget *attachmentsWrapper = new QWidget();
    attachmentsWrapper->setLayout(attachmentsLayout);

    priorityAttachmentsLayout->addWidget(priorityWrapper, 1);
    priorityAttachmentsLayout->addWidget(attachmentsWrapper, 1);

    QWidget *priorityAttachmentsWidget = new QWidget();
    priorityAttachmentsWidget->setLayout(priorityAttachmentsLayout);

    // ================= ATTACHMENTS LIST =================

    m_attachmentsList =
        new QListWidget();

    m_attachmentsList->setVisible(false);

    m_attachmentsList->setMaximumHeight(80);

    m_attachmentsList->setStyleSheet(
        "QListWidget { "
        "   background-color: #1E293B; "
        "   border: 1px solid #334155; "
        "   border-radius: 12px; "
        "   padding: 6px; "
        "}"
        );

    // ================= BUTTONS =================

    QHBoxLayout *buttonLayout =
        new QHBoxLayout();

    buttonLayout->setSpacing(12);

    m_cancelBtn =
        new QPushButton("Отмена");

    m_cancelBtn->setFixedHeight(48);

    m_cancelBtn->setCursor(
        Qt::PointingHandCursor
        );

    m_cancelBtn->setStyleSheet(
        "QPushButton { "
        "   background-color: #334155; "
        "   border: none; "
        "   border-radius: 16px; "
        "   color: #94A3B8; "
        "   font-weight: 600; "
        "   font-size: 14px; "
        "}"
        "QPushButton:hover { "
        "   background-color: #475569; "
        "   color: #CBD5E1; "
        "}"
        );

    connect(
        m_cancelBtn,
        &QPushButton::clicked,
        this,
        &NewTicketDialog::reject  // Было &QDialog::reject
        );

    m_sendBtn =
        new QPushButton(
            "Отправить обращение"
            );

    m_sendBtn->setFixedHeight(48);

    m_sendBtn->setCursor(
        Qt::PointingHandCursor
        );

    m_sendBtn->setStyleSheet(QString(
                                 "QPushButton { "
                                 "   background-color: %1; "
                                 "   border: none; "
                                 "   border-radius: 16px; "
                                 "   color: white; "
                                 "   font-weight: 600; "
                                 "   font-size: 14px; "
                                 "}"
                                 "QPushButton:hover { "
                                 "   background-color: %2; "
                                 "}"
                                 ).arg(m_config.categoryColor)
                                 .arg(m_config.categoryColor + "cc"));

    connect(
        m_sendBtn,
        &QPushButton::clicked,
        this,
        &NewTicketDialog::onSendClicked
        );

    buttonLayout->addWidget(m_cancelBtn);
    buttonLayout->addWidget(m_sendBtn);

    // ================= BUILD =================

    contentLayout->addLayout(subjectHeaderLayout);
    contentLayout->addWidget(m_subjectEdit);

    contentLayout->addLayout(messageHeaderLayout);
    contentLayout->addWidget(textEditContainer);

    contentLayout->addWidget(
        priorityAttachmentsWidget
        );

    contentLayout->addWidget(
        m_attachmentsList
        );

    contentLayout->addLayout(buttonLayout);

    containerLayout->addWidget(content);

    onPriorityChanged(1);

    updatePlaceholderGeometry();
}

void NewTicketDialog::onPriorityChanged(
    int index
    )
{
    Q_UNUSED(index)

    QString priority =
        m_priorityCombo
            ->currentData()
            .toString();

    QString color;

    if (priority == "high")
        color = "#EF4444";
    else if (priority == "medium")
        color = "#F59E0B";
    else
        color = "#10B981";

    m_priorityCombo->setStyleSheet(
        "QComboBox { "
        "   background-color: #1E293B; "
        "   border: 1px solid " + color + "; "
                  "border-radius: 16px; "
                  "padding: 12px 14px; "
                  "color: white; "
                  "font-size: 13px; "
                  "}"
                  "QComboBox::drop-down { "
                  "   border: none; "
                  "   width: 0px; "
                  "}"
        );
}

void NewTicketDialog::onAttachFile()
{
    QStringList files =
        QFileDialog::getOpenFileNames(
            this,
            "Выберите файлы для прикрепления"
            );

    if (!files.isEmpty()) {

        m_attachedFiles.append(files);

        updateAttachmentsDisplay();
    }
}

void NewTicketDialog::updateAttachmentsDisplay()
{
    if (m_attachedFiles.isEmpty()) {

        m_attachmentsList->setVisible(false);

        m_attachBtn->setText(
            " Нажмите для выбора файлов"
            );

        return;
    }

    m_attachmentsList->clear();

    m_attachmentsList->setVisible(true);

    for (int i = 0;
         i < m_attachedFiles.size();
         ++i)
    {
        QFileInfo fi(
            m_attachedFiles[i]
            );

        QListWidgetItem *item =
            new QListWidgetItem(
                "📄 " + fi.fileName()
                );

        item->setData(
            Qt::UserRole,
            i
            );

        m_attachmentsList->addItem(item);
    }

    m_attachBtn->setText(
        QString(" Выбрано файлов: %1")
            .arg(m_attachedFiles.size())
        );
}

void NewTicketDialog::removeAttachment(
    int index
    )
{
    if (index >= 0 &&
        index < m_attachedFiles.size())
    {
        m_attachedFiles.removeAt(index);

        updateAttachmentsDisplay();
    }
}

void NewTicketDialog::onSendClicked()
{
    if (m_subjectEdit
            ->text()
            .trimmed()
            .isEmpty())
    {
        QMessageBox::warning(
            this,
            "Ошибка",
            "Пожалуйста, заполните тему обращения"
            );

        m_subjectEdit->setFocus();

        return;
    }

    if (m_messageEdit
            ->toPlainText()
            .trimmed()
            .isEmpty())
    {
        QMessageBox::warning(
            this,
            "Ошибка",
            "Пожалуйста, опишите проблему"
            );

        m_messageEdit->setFocus();

        return;
    }

    accept();
}

TicketData NewTicketDialog::getTicketData() const
{
    TicketData data;

    data.subject =
        m_subjectEdit
            ->text()
            .trimmed();

    data.message =
        m_messageEdit
            ->toPlainText()
            .trimmed();

    data.priority =
        m_priorityCombo
            ->currentData()
            .toString();

    data.category =
        m_config.categoryKey;

    data.attachments =
        m_attachedFiles;

    return data;
}

void NewTicketDialog::updatePlaceholderGeometry()
{
    if (!m_placeholderLabel || !m_messageEdit)
        return;

    QRect r = m_messageEdit->viewport()->geometry();

    m_placeholderLabel->setGeometry(
        r.x() + 6,
        r.y() + 2,   // 👈 было 6, стало 2 (подняли вверх)
        r.width() - 12,
        r.height() - 10
        );

    m_placeholderLabel->adjustSize();
}

bool NewTicketDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_messageEdit->viewport() &&
        (event->type() == QEvent::Resize ||
         event->type() == QEvent::Move))
    {
        updatePlaceholderGeometry();
    }

    return OverlayDialog::eventFilter(obj, event);
}

void NewTicketDialog::reject()
{
    qDebug() << "🟡 reject() вызван, устанавливаю m_reopenSupport = true";
    m_reopenSupport = true;
    OverlayDialog::reject();
}

void NewTicketDialog::accept()
{
    qDebug() << "🟢 accept() вызван, устанавливаю m_reopenSupport = false";
    m_reopenSupport = false;
    OverlayDialog::accept();
}

void NewTicketDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        qDebug() << "⌨️ Enter нажат — игнорирую";
        return;
    }
    if (event->key() == Qt::Key_Escape) {
        qDebug() << "⌨️ Escape нажат, вызываю reject()";
        m_reopenSupport = true;
        reject();
        return;
    }
    OverlayDialog::keyPressEvent(event);
}
