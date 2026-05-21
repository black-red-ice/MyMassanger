#include "supportdialog.h"
#include "newticketdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QFrame>
#include <QPixmap>
#include <QIcon>
#include <QPainter>
#include <QScrollArea>
#include <QListWidget>
#include <QTextEdit>
#include <QComboBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QLineEdit>
#include <QScreen>
#include <QApplication>
#include <QLineEdit>
#include <QTimer>
#include <QPainterPath>
#include <QObject>

SupportDialog::SupportDialog(QWidget *parent) : OverlayDialog(parent)
{
    m_categoryNames = {
        {"it", "IT поддержка"},
        {"hr", "HR вопросы"},
        {"finance", "Финансы"},
        {"other", "Другое"}
    };

    m_categoryColors = {
        {"it", "#3B82F6"},
        {"hr", "#8B5CF6"},
        {"finance", "#F59E0B"},
        {"other", "#06B6D4"}
    };

    m_categoryIcons = {
        {"it", ":/icons/darkTheme/images/darkTheme/laptop.svg"},
        {"hr", ":/icons/darkTheme/images/darkTheme/user-tie-p.svg"},
        {"finance", ":/icons/darkTheme/images/darkTheme/coins.svg"},
        {"other", ":/icons/darkTheme/images/darkTheme/circle-question.svg"}
    };

    m_categoryDescriptions = {
        {"it", "Проблемы с доступом, программным обеспечением, техникой"},
        {"hr", "Кадровые вопросы, отпуска, документы, адаптация"},
        {"finance", "Зарплата, отчёты, расходы, финансовая документация"},
        {"other", "Любые другие вопросы, не вошедшие в другие категории"}
    };

    loadTickets();
    setupUI();
}

SupportDialog::~SupportDialog()
{
    saveTickets();
}

void SupportDialog::setupUI()
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

    // Заголовок
    QWidget *header = new QWidget();
    header->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #0EA5E9, stop:1 #0284C7); border-top-left-radius: 16px; border-top-right-radius: 16px;");
    header->setFixedHeight(70);

    QHBoxLayout *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(24, 0, 24, 0);

    QLabel *headsetIcon = new QLabel();
    QPixmap headsetPixmap(":/icons/darkTheme/images/darkTheme/headset-w.svg");
    if (!headsetPixmap.isNull()) {
        headsetIcon->setPixmap(headsetPixmap.scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    headsetIcon->setStyleSheet("background-color: transparent;");

    QLabel *titleLabel = new QLabel("Служба поддержки");
    titleLabel->setStyleSheet("color: white; font-size: 18px; font-weight: 600; background-color: transparent; margin-left: 8px;");

    QPushButton *closeBtn = new QPushButton("✕");
    closeBtn->setFixedSize(36, 36);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(
        "QPushButton { background-color: rgba(255,255,255,0.1); border: none; border-radius: 18px; color: white; font-size: 16px; }"
        "QPushButton:hover { background-color: rgba(255,255,255,0.2); }"
        );
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);

    QHBoxLayout *titleLayout = new QHBoxLayout();
    titleLayout->setSpacing(0);
    titleLayout->addWidget(headsetIcon);
    titleLayout->addWidget(titleLabel);

    headerLayout->addLayout(titleLayout);
    headerLayout->addStretch();
    headerLayout->addWidget(closeBtn);

    containerLayout->addWidget(header);

    // Контент
    QWidget *content = new QWidget();
    QVBoxLayout *contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(24, 24, 24, 24);
    contentLayout->setSpacing(20);

    // Заголовок с большой иконкой
    QWidget *info = new QWidget();
    QVBoxLayout *infoLayout = new QVBoxLayout(info);
    infoLayout->setSpacing(8);

    QLabel *bigHeadsetIcon = new QLabel();
    QPixmap bigHeadsetPixmap(":/icons/darkTheme/images/darkTheme/headset-b.svg");
    if (!bigHeadsetPixmap.isNull()) {
        bigHeadsetIcon->setPixmap(bigHeadsetPixmap.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    bigHeadsetIcon->setAlignment(Qt::AlignCenter);
    bigHeadsetIcon->setStyleSheet("background-color: transparent;");

    QLabel *helpTitle = new QLabel("Нужна помощь?");
    helpTitle->setAlignment(Qt::AlignCenter);
    helpTitle->setStyleSheet("color: #f1f5f9; font-size: 18px; font-weight: bold;");
    QLabel *helpDesc = new QLabel("Выберите категорию обращения");
    helpDesc->setAlignment(Qt::AlignCenter);
    helpDesc->setStyleSheet("color: #94a3b8; font-size: 14px;");

    infoLayout->addWidget(bigHeadsetIcon);
    infoLayout->addWidget(helpTitle);
    infoLayout->addWidget(helpDesc);

    contentLayout->addWidget(info);

    // Карточки категорий
    QWidget *categories = new QWidget();
    QGridLayout *grid = new QGridLayout(categories);
    grid->setSpacing(12);
    grid->setContentsMargins(0, 0, 0, 0);

    grid->addWidget(createSupportCard(m_categoryIcons["it"], m_categoryNames["it"],
                                      m_categoryDescriptions["it"], m_categoryColors["it"], "it"), 0, 0);
    grid->addWidget(createSupportCard(m_categoryIcons["hr"], m_categoryNames["hr"],
                                      m_categoryDescriptions["hr"], m_categoryColors["hr"], "hr"), 0, 1);
    grid->addWidget(createSupportCard(m_categoryIcons["finance"], m_categoryNames["finance"],
                                      m_categoryDescriptions["finance"], m_categoryColors["finance"], "finance"), 1, 0);
    grid->addWidget(createSupportCard(m_categoryIcons["other"], m_categoryNames["other"],
                                      m_categoryDescriptions["other"], m_categoryColors["other"], "other"), 1, 1);

    contentLayout->addWidget(categories);

    // Мои обращения
    QWidget *tickets = new QWidget();
    QVBoxLayout *ticketsLayout = new QVBoxLayout(tickets);
    ticketsLayout->setSpacing(8);

    QLabel *ticketsTitle = new QLabel("Мои обращения");
    ticketsTitle->setStyleSheet("color: #f1f5f9; font-size: 15px; font-weight: 600;");
    ticketsLayout->addWidget(ticketsTitle);

    QPushButton *openTicketsBtn = new QPushButton("📂 Открыть мои обращения");
    openTicketsBtn->setStyleSheet(
        "QPushButton { background-color: #0EA5E9; border: none; border-radius: 12px; padding: 12px; color: white; font-weight: 500; }"
        "QPushButton:hover { background-color: #0284C7; }"
        );
    connect(openTicketsBtn, &QPushButton::clicked, this, &SupportDialog::onOpenTicketsWorkspace);
    ticketsLayout->addWidget(openTicketsBtn);

    QLabel *infoLabel = new QLabel("Все ваши обращения сохраняются и доступны здесь");
    infoLabel->setStyleSheet("color: #64748b; font-size: 12px; text-align: center;");
    infoLabel->setAlignment(Qt::AlignCenter);
    ticketsLayout->addWidget(infoLabel);

    contentLayout->addWidget(tickets);

    containerLayout->addWidget(content);
}

QWidget* SupportDialog::createSupportCard(const QString &iconPath, const QString &title,
                                          const QString &desc, const QString &color,
                                          const QString &categoryKey)
{
    QWidget *card = new QWidget();
    card->setCursor(Qt::PointingHandCursor);

    card->setStyleSheet(QString(
                            "QWidget { "
                            "   background-color: #1e293b; "
                            "   border-radius: 12px; "
                            "   border: 1px solid rgba(29, 78, 216, 0.2); "
                            "}"
                            "QWidget:hover { "
                            "   border: 1px solid %1; "
                            "}"
                            ).arg(color));

    QVBoxLayout *layout = new QVBoxLayout(card);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(8);

    QLabel *iconLabel = new QLabel();
    QPixmap pixmap(iconPath);
    if (!pixmap.isNull()) {
        QPixmap coloredPixmap(pixmap.size());
        coloredPixmap.fill(Qt::transparent);
        QPainter painter(&coloredPixmap);
        painter.setCompositionMode(QPainter::CompositionMode_Source);
        painter.drawPixmap(0, 0, pixmap);
        painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        painter.fillRect(coloredPixmap.rect(), QColor(color));
        painter.end();
        iconLabel->setPixmap(coloredPixmap.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setStyleSheet("background-color: transparent; border: none;");

    QLabel *titleLabel = new QLabel(title);
    titleLabel->setStyleSheet("color: #f1f5f9; font-size: 14px; font-weight: 600; background-color: transparent; border: none;");
    titleLabel->setAlignment(Qt::AlignCenter);

    QLabel *descLabel = new QLabel(desc);
    descLabel->setStyleSheet("color: #94a3b8; font-size: 12px; background-color: transparent; border: none;");
    descLabel->setAlignment(Qt::AlignCenter);
    descLabel->setWordWrap(true);

    layout->addWidget(iconLabel);
    layout->addWidget(titleLabel);
    layout->addWidget(descLabel);

    // Прозрачная кнопка для клика
    QPushButton *btn = new QPushButton(card);
    btn->setGeometry(card->rect());
    btn->setStyleSheet("background-color: transparent; border: none;");
    connect(btn, &QPushButton::clicked, [this, categoryKey]() {
        onCreateTicket(categoryKey);
    });

    return card;
}

void SupportDialog::onCreateTicket(const QString &category)
{
    NewTicketDialog dialog(category, m_categoryNames[category], m_categoryColors[category], this);
    if (dialog.exec() == QDialog::Accepted) {
        auto ticketData = dialog.getTicketData();

        SupportTicket newTicket;
        newTicket.id = "TKT-" + QString::number(QDateTime::currentDateTime().toSecsSinceEpoch());
        newTicket.subject = ticketData.subject;
        newTicket.message = ticketData.message;
        newTicket.category = m_categoryNames[category];
        newTicket.status = "Новая";
        newTicket.priority = ticketData.priority;
        newTicket.createdAt = QDateTime::currentDateTime().toString("dd.MM.yyyy HH:mm");
        newTicket.operatorName = "Назначится";

        newTicket.messages.append(QPair<QString, QString>("user", ticketData.message));
        newTicket.messages.append(QPair<QString, QString>("support", "Здравствуйте! Ваше обращение принято в работу. Мы свяжемся с вами в ближайшее время."));

        m_tickets.prepend(newTicket);
        saveTickets();

        QMessageBox::information(this, "Успех",
                                 QString("Обращение #%1 успешно создано!").arg(newTicket.id));
    }
}

void SupportDialog::onOpenTicketsWorkspace()
{
    // Создаём окно без рамки с прозрачным фоном
    QWidget *workspaceWidget = new QWidget(this);
    workspaceWidget->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    workspaceWidget->setAttribute(Qt::WA_TranslucentBackground);
    workspaceWidget->setStyleSheet("background: transparent;");

    // Размеры экрана
    QRect screenGeometry = QApplication::primaryScreen()->geometry();
    int margin = 30;

    workspaceWidget->setGeometry(
        screenGeometry.x() + margin,
        screenGeometry.y() + margin,
        screenGeometry.width() - margin * 2,
        screenGeometry.height() - margin * 2
        );

    // Применяем маску для скругления углов окна
    QPainterPath path;
    path.addRoundedRect(workspaceWidget->rect(), 20, 20);
    workspaceWidget->setMask(QRegion(path.toFillPolygon().toPolygon()));

    // Центральный виджет (без скругления, так как маска уже есть)
    QWidget *centralWidget = new QWidget(workspaceWidget);
    centralWidget->setGeometry(workspaceWidget->rect());
    centralWidget->setObjectName("centralWidget");
    centralWidget->setAttribute(Qt::WA_StyledBackground);
    centralWidget->setAutoFillBackground(true);
    centralWidget->setStyleSheet(
        "#centralWidget { "
        "   background-color: #0F172A; "
        "   border: 1px solid rgba(255,255,255,0.08); "
        "}"
        );

    // Layout для центрального виджета
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Верхняя панель
    QWidget *topBar = new QWidget();
    topBar->setFixedHeight(60);
    topBar->setStyleSheet(
        "background-color: #0F172A; "
        "border-bottom: 1px solid #334155; "
        "border-top-left-radius: 20px; "
        "border-top-right-radius: 20px; "
        "border-bottom-left-radius: 0px; "
        "border-bottom-right-radius: 0px;"
        );
    QHBoxLayout *topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(24, 0, 24, 0);

    QLabel *folderIcon = new QLabel();
    QPixmap folderPixmap(":/icons/general/images/general/folder-open-blue.svg");
    if (!folderPixmap.isNull()) {
        folderIcon->setPixmap(folderPixmap.scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    folderIcon->setStyleSheet("background: transparent; border: none; border-radius: 0px;");

    QLabel *titleLabel = new QLabel("Управление обращениями");
    titleLabel->setStyleSheet("color: white; font-size: 18px; font-weight: 600; background: transparent; border: none; border-radius: 0px;");

    QPushButton *closeBtn = new QPushButton("✕");
    closeBtn->setFixedSize(36, 36);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(
        "QPushButton { "
        "   background-color: #334155; "
        "   border: none; "
        "   border-radius: 12px; "
        "   color: white; "
        "   font-size: 16px; "
        "}"
        "QPushButton:hover { background-color: #475569; }"
        );
    connect(closeBtn, &QPushButton::clicked, workspaceWidget, &QWidget::close);

    topLayout->addWidget(folderIcon);
    topLayout->addSpacing(8);
    topLayout->addWidget(titleLabel);
    topLayout->addStretch();
    topLayout->addWidget(closeBtn);
    mainLayout->addWidget(topBar);

    // === КОНТЕЙНЕР ДЛЯ ТРЁХ КОЛОНОК ===
    QWidget *columnsContainer = new QWidget();
    columnsContainer->setObjectName("columnsContainer");
    columnsContainer->setStyleSheet(
        "#columnsContainer { "
        "   background-color: #0F172A; "
        "   border-right: 1px solid #334155; "
        "   border-left: 1px solid #334155; "
        "   border-radius: 0px; "
        "}"
        );

    QHBoxLayout *columnsLayout = new QHBoxLayout(columnsContainer);
    columnsLayout->setContentsMargins(0, 0, 0, 0);
    columnsLayout->setSpacing(0);

    // === КОЛОНКА 1: Список обращений ===
    QWidget *listColumn = new QWidget();
    listColumn->setFixedWidth(380);
    listColumn->setStyleSheet("background-color: #111827; border-radius: 0px;");
    QVBoxLayout *listLayout = new QVBoxLayout(listColumn);
    listLayout->setContentsMargins(0, 0, 0, 0);
    listLayout->setSpacing(0);

    // Заголовок колонки
    QWidget *listHeader = new QWidget();
    listHeader->setFixedHeight(70);
    listHeader->setStyleSheet("border-bottom: 1px solid #334155; background: #111827; border-radius: 0px;");
    QVBoxLayout *listHeaderLayout = new QVBoxLayout(listHeader);
    listHeaderLayout->setContentsMargins(20, 15, 20, 15);

    QLabel *listTitle = new QLabel("Мои обращения");
    listTitle->setStyleSheet("color: white; font-size: 16px; font-weight: 600; background: transparent; border: none; border-radius: 0px;");
    QLabel *listSubtitle = new QLabel("Список всех ваших обращений");
    listSubtitle->setStyleSheet("color: #94A3B8; font-size: 12px; background: transparent; border: none; border-radius: 0px;");

    listHeaderLayout->addWidget(listTitle);
    listHeaderLayout->addWidget(listSubtitle);
    listLayout->addWidget(listHeader);

    // Поиск
    QLineEdit *searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText("Поиск обращений...");
    searchEdit->setStyleSheet(
        "QLineEdit { "
        "   background-color: #1E293B; "
        "   border: 1px solid #334155; "
        "   border-radius: 12px; "
        "   padding: 10px 16px; "
        "   color: white; "
        "   margin: 12px; "
        "}"
        "QLineEdit:focus { border-color: #3B82F6; }"
        );
    listLayout->addWidget(searchEdit);

    // Контейнер для карточек обращений
    QWidget *ticketsContainer = new QWidget();
    ticketsContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ticketsContainer->setStyleSheet("background: transparent; border-radius: 0px;");
    QVBoxLayout *ticketsListLayout = new QVBoxLayout(ticketsContainer);
    ticketsListLayout->setContentsMargins(8, 8, 8, 8);
    ticketsListLayout->setSpacing(8);
    ticketsListLayout->addStretch();
    listLayout->addWidget(ticketsContainer, 1);

    // Вектор для хранения виджетов обращений
    QVector<QWidget*> ticketWidgets;
    QVector<SupportTicket> ticketData;

    // Заполняем список обращений
    for (const auto &ticket : m_tickets) {
        QWidget *cardWidget = new QWidget();
        cardWidget->setCursor(Qt::PointingHandCursor);
        cardWidget->setStyleSheet(
            "QWidget { "
            "   background-color: #1E293B; "
            "   border-radius: 12px; "
            "   border: 1px solid #334155; "
            "}"
            "QWidget:hover { background-color: #2D3A4E; }"
            );

        QVBoxLayout *cardLayout = new QVBoxLayout(cardWidget);
        cardLayout->setContentsMargins(16, 14, 16, 14);
        cardLayout->setSpacing(6);

        QHBoxLayout *headerLayout = new QHBoxLayout();
        headerLayout->setSpacing(8);

        QLabel *idLabel = new QLabel(ticket.id);
        idLabel->setStyleSheet("color: white; font-size: 14px; font-weight: 600; background: transparent; border: none; border-radius: 0px;");

        QLabel *statusLabel = new QLabel(ticket.status);
        statusLabel->setAlignment(Qt::AlignCenter);
        statusLabel->setFixedWidth(80);
        QString statusColor = (ticket.status == "Новая") ? "#F59E0B" :
                                  (ticket.status == "В работе") ? "#3B82F6" : "#10B981";
        statusLabel->setStyleSheet(QString("color: %1; font-size: 12px; font-weight: 500; background: rgba(0,0,0,0.2); border-radius: 12px; padding: 4px 8px;").arg(statusColor));

        headerLayout->addWidget(idLabel);
        headerLayout->addStretch();
        headerLayout->addWidget(statusLabel);
        cardLayout->addLayout(headerLayout);

        QLabel *subjectLabel = new QLabel(ticket.subject);
        subjectLabel->setStyleSheet("color: white; font-size: 14px; font-weight: 500; background: transparent; border: none; border-radius: 0px;");
        subjectLabel->setWordWrap(true);
        cardLayout->addWidget(subjectLabel);

        QHBoxLayout *footerLayout = new QHBoxLayout();
        footerLayout->setSpacing(8);

        QLabel *categoryLabel = new QLabel(ticket.category);
        categoryLabel->setStyleSheet("color: #94A3B8; font-size: 12px; background: transparent; border: none; border-radius: 0px;");

        QLabel *dateLabel = new QLabel(ticket.createdAt);
        dateLabel->setStyleSheet("color: #64748B; font-size: 12px; background: transparent; border: none; border-radius: 0px;");

        footerLayout->addWidget(categoryLabel);
        footerLayout->addStretch();
        footerLayout->addWidget(dateLabel);
        cardLayout->addLayout(footerLayout);

        ticketWidgets.append(cardWidget);
        ticketData.append(ticket);
        ticketsListLayout->insertWidget(ticketsListLayout->count() - 1, cardWidget);

        cardWidget->setProperty("ticketId", ticket.id);
        cardWidget->installEventFilter(this);
    }

    columnsLayout->addWidget(listColumn);

    // === КОЛОНКА 2: Чат (с сообщением по умолчанию) ===
    QWidget *chatColumn = new QWidget();
    chatColumn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    chatColumn->setStyleSheet("background-color: #0F172A; border-radius: 0px;");
    QVBoxLayout *chatLayout = new QVBoxLayout(chatColumn);
    chatLayout->setContentsMargins(0, 0, 0, 0);
    chatLayout->setSpacing(0);

    QWidget *chatHeader = new QWidget();
    chatHeader->setFixedHeight(70);
    chatHeader->setStyleSheet("border-bottom: 1px solid #334155; background: #0F172A; border-radius: 0px;");
    QVBoxLayout *chatHeaderLayout = new QVBoxLayout(chatHeader);
    chatHeaderLayout->setContentsMargins(20, 15, 20, 15);

    QLabel *chatTitle = new QLabel("Переписка");
    chatTitle->setStyleSheet("color: white; font-size: 16px; font-weight: 600; background: transparent; border: none; border-radius: 0px;");
    QLabel *chatSubtitle = new QLabel("История сообщений по обращению");
    chatSubtitle->setStyleSheet("color: #94A3B8; font-size: 12px; background: transparent; border: none; border-radius: 0px;");

    chatHeaderLayout->addWidget(chatTitle);
    chatHeaderLayout->addWidget(chatSubtitle);
    chatLayout->addWidget(chatHeader);

    // Область сообщений с заглушкой
    QWidget *chatContent = new QWidget();
    chatContent->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    chatContent->setStyleSheet("background: transparent; border-radius: 0px;");
    QVBoxLayout *chatContentLayout = new QVBoxLayout(chatContent);
    chatContentLayout->setContentsMargins(20, 20, 20, 20);
    chatContentLayout->setSpacing(0);

    // Заглушка для центральной панели (по центру)
    QWidget *emptyChatWidget = new QWidget();
    emptyChatWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QVBoxLayout *emptyChatLayout = new QVBoxLayout(emptyChatWidget);
    emptyChatLayout->setAlignment(Qt::AlignCenter);

    QLabel *inboxIcon = new QLabel();
    QPixmap inboxPixmap(":/icons/general/images/general/inbox.svg");
    if (!inboxPixmap.isNull()) {
        inboxIcon->setPixmap(inboxPixmap.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    inboxIcon->setAlignment(Qt::AlignCenter);
    inboxIcon->setStyleSheet("background: transparent; border: none; opacity: 0.5;");

    QLabel *emptyChatLabel = new QLabel("Выберите обращение для просмотра");
    emptyChatLabel->setAlignment(Qt::AlignCenter);
    emptyChatLabel->setStyleSheet("color: #64748B; font-size: 16px; background: transparent; border: none;");

    emptyChatLayout->addWidget(inboxIcon);
    emptyChatLayout->addSpacing(16);
    emptyChatLayout->addWidget(emptyChatLabel);

    chatContentLayout->addWidget(emptyChatWidget);
    chatLayout->addWidget(chatContent, 1);

    // Поле ввода сообщения (скрыто по умолчанию)
    QWidget *inputWidget = new QWidget();
    inputWidget->setFixedHeight(70);
    inputWidget->setStyleSheet("border-top: 1px solid #334155; background: #0F172A; border-radius: 0px;");
    QHBoxLayout *inputLayout = new QHBoxLayout(inputWidget);
    inputLayout->setContentsMargins(20, 12, 20, 12);
    inputLayout->setSpacing(12);
    inputWidget->setVisible(false); // скрываем, пока не выбран тикет

    QPushButton *attachBtn = new QPushButton();
    attachBtn->setFixedSize(44, 44);
    attachBtn->setCursor(Qt::PointingHandCursor);
    attachBtn->setStyleSheet(
        "QPushButton { background-color: #1E293B; border: 1px solid #334155; border-radius: 12px; }"
        "QPushButton:hover { background-color: #334155; }"
        );
    QPixmap paperclipPixmap(":/icons/general/images/general/paperclip.svg");
    if (!paperclipPixmap.isNull()) {
        attachBtn->setIcon(QIcon(paperclipPixmap.scaled(18, 18, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
        attachBtn->setIconSize(QSize(18, 18));
    }

    QTextEdit *replyEdit = new QTextEdit();
    replyEdit->setPlaceholderText("Введите сообщение...");
    replyEdit->setFixedHeight(44);
    replyEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    replyEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    replyEdit->setStyleSheet(
        "QTextEdit { background-color: #1E293B; border: 1px solid #334155; border-radius: 12px; padding: 10px; color: white; }"
        "QTextEdit:focus { border-color: #3B82F6; }"
        );

    QPushButton *sendBtn = new QPushButton();
    sendBtn->setFixedSize(44, 44);
    sendBtn->setCursor(Qt::PointingHandCursor);
    sendBtn->setStyleSheet(
        "QPushButton { background-color: #2563EB; border: none; border-radius: 12px; }"
        "QPushButton:hover { background-color: #3B82F6; }"
        );
    QPixmap planePixmap(":/icons/general/images/general/paper-plane.svg");
    if (!planePixmap.isNull()) {
        sendBtn->setIcon(QIcon(planePixmap.scaled(18, 18, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
        sendBtn->setIconSize(QSize(18, 18));
    }

    inputLayout->addWidget(attachBtn);
    inputLayout->addWidget(replyEdit, 1);
    inputLayout->addWidget(sendBtn);
    chatLayout->addWidget(inputWidget);

    columnsLayout->addWidget(chatColumn, 1);

    // === КОЛОНКА 3: Информация (с заглушкой) ===
    QWidget *infoColumn = new QWidget();
    infoColumn->setFixedWidth(320);
    infoColumn->setStyleSheet("background-color: #111827; border-radius: 0px;");
    QVBoxLayout *infoLayout = new QVBoxLayout(infoColumn);
    infoLayout->setContentsMargins(0, 0, 0, 0);
    infoLayout->setSpacing(0);

    QWidget *infoHeader = new QWidget();
    infoHeader->setFixedHeight(70);
    infoHeader->setStyleSheet("border-bottom: 1px solid #334155; background: #111827; border-radius: 0px;");
    QVBoxLayout *infoHeaderLayout = new QVBoxLayout(infoHeader);
    infoHeaderLayout->setContentsMargins(20, 15, 20, 15);

    QLabel *infoTitle = new QLabel("Информация");
    infoTitle->setStyleSheet("color: white; font-size: 16px; font-weight: 600; background: transparent; border: none; border-radius: 0px;");
    QLabel *infoSubtitle = new QLabel("Детали обращения");
    infoSubtitle->setStyleSheet("color: #94A3B8; font-size: 12px; background: transparent; border: none; border-radius: 0px;");

    infoHeaderLayout->addWidget(infoTitle);
    infoHeaderLayout->addWidget(infoSubtitle);
    infoLayout->addWidget(infoHeader);

    // Область информации с заглушкой
    QWidget *infoContent = new QWidget();
    infoContent->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    infoContent->setStyleSheet("background: transparent; border-radius: 0px;");
    QVBoxLayout *infoContentLayout = new QVBoxLayout(infoContent);
    infoContentLayout->setContentsMargins(20, 20, 20, 20);
    infoContentLayout->setSpacing(0);

    // Заглушка для правой панели (наверху)
    QWidget *emptyInfoWidget = new QWidget();
    QVBoxLayout *emptyInfoLayout = new QVBoxLayout(emptyInfoWidget);
    emptyInfoLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    QLabel *infoIcon = new QLabel();
    QPixmap infoIconPixmap(":/icons/general/images/general/circle-info.svg");
    if (!infoIconPixmap.isNull()) {
        infoIcon->setPixmap(infoIconPixmap.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    infoIcon->setAlignment(Qt::AlignCenter);
    infoIcon->setStyleSheet("background: transparent; border: none; opacity: 0.5;");

    QLabel *emptyInfoLabel = new QLabel("Выберите обращение");
    emptyInfoLabel->setAlignment(Qt::AlignCenter);
    emptyInfoLabel->setStyleSheet("color: #64748B; font-size: 14px; background: transparent; border: none;");

    emptyInfoLayout->addWidget(infoIcon);
    emptyInfoLayout->addSpacing(12);
    emptyInfoLayout->addWidget(emptyInfoLabel);

    infoContentLayout->addWidget(emptyInfoWidget);
    infoLayout->addWidget(infoContent, 1);

    columnsLayout->addWidget(infoColumn);

    mainLayout->addWidget(columnsContainer, 1);

    // Сохраняем указатели для использования в eventFilter
    workspaceWidget->setProperty("emptyChatWidget", QVariant::fromValue(emptyChatWidget));
    workspaceWidget->setProperty("emptyInfoWidget", QVariant::fromValue(emptyInfoWidget));
    workspaceWidget->setProperty("chatContentLayout", QVariant::fromValue(chatContentLayout));
    workspaceWidget->setProperty("infoContentLayout", QVariant::fromValue(infoContentLayout));
    workspaceWidget->setProperty("inputWidget", QVariant::fromValue(inputWidget));

    // Фильтрация поиска
    connect(searchEdit, &QLineEdit::textChanged, [ticketWidgets, ticketData](const QString &text) {
        for (int i = 0; i < ticketWidgets.size(); ++i) {
            bool visible = text.isEmpty() ||
                           ticketData[i].id.contains(text, Qt::CaseInsensitive) ||
                           ticketData[i].subject.contains(text, Qt::CaseInsensitive);
            ticketWidgets[i]->setVisible(visible);
        }
    });

    // Сохраняем указатели для использования в eventFilter
    workspaceWidget->setProperty("chatContentLayout", QVariant::fromValue(chatContent));
    workspaceWidget->setProperty("infoContentLayout", QVariant::fromValue(infoContent));
    workspaceWidget->setProperty("inputWidget", QVariant::fromValue(inputWidget));
    workspaceWidget->setProperty("ticketsContainer", QVariant::fromValue(ticketsContainer));

    // Показываем окно
    workspaceWidget->show();
}

QWidget* SupportDialog::createTicketItem(const SupportTicket &ticket)
{
    QWidget *widget = new QWidget();
    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->setContentsMargins(12, 8, 12, 8);

    QVBoxLayout *textLayout = new QVBoxLayout();
    textLayout->setSpacing(4);

    QLabel *titleLabel = new QLabel(QString("%1 - %2").arg(ticket.id, ticket.subject));
    titleLabel->setStyleSheet("color: white; font-size: 13px; font-weight: 500;");

    QLabel *infoLabel = new QLabel(QString("%1 • %2").arg(ticket.category, ticket.createdAt));
    infoLabel->setStyleSheet("color: #94A3B8; font-size: 11px;");

    textLayout->addWidget(titleLabel);
    textLayout->addWidget(infoLabel);

    QLabel *statusLabel = new QLabel(ticket.status);
    statusLabel->setFixedWidth(80);
    statusLabel->setAlignment(Qt::AlignCenter);

    QString statusColor = (ticket.status == "Новая") ? "#F59E0B" :
                              (ticket.status == "В работе") ? "#3B82F6" : "#10B981";
    statusLabel->setStyleSheet(QString("color: %1; font-size: 12px; font-weight: 500;").arg(statusColor));

    layout->addLayout(textLayout, 1);
    layout->addWidget(statusLabel);

    return widget;
}

void SupportDialog::loadTickets()
{
    QSettings settings("Aura", "Messenger");
    QString jsonStr = settings.value("support_tickets").toString();

    if (jsonStr.isEmpty()) {
        // Добавляем тестовые данные
        SupportTicket testTicket;
        testTicket.id = "IT-2451";
        testTicket.subject = "VPN не подключается";
        testTicket.message = "После обновления Aura VPN перестал подключаться. Ошибка возникает при авторизации.";
        testTicket.category = "IT поддержка";
        testTicket.status = "В работе";
        testTicket.priority = "high";
        testTicket.createdAt = QDateTime::currentDateTime().toString("dd.MM.yyyy HH:mm");
        testTicket.operatorName = "Алексей Смирнов";
        testTicket.messages.append(QPair<QString, QString>("user", testTicket.message));
        testTicket.messages.append(QPair<QString, QString>("support", "Здравствуйте! Ваше обращение принято в работу. Мы свяжемся с вами в ближайшее время."));
        m_tickets.append(testTicket);

        SupportTicket testTicket2;
        testTicket2.id = "HR-1932";
        testTicket2.subject = "Запрос отпуска";
        testTicket2.message = "Хотел уточнить статус согласования отпуска на июнь.";
        testTicket2.category = "HR вопросы";
        testTicket2.status = "Ожидание";
        testTicket2.priority = "medium";
        testTicket2.createdAt = QDateTime::currentDateTime().toString("dd.MM.yyyy HH:mm");
        testTicket2.operatorName = "Елена Ковалева";
        testTicket2.messages.append(QPair<QString, QString>("user", testTicket2.message));
        testTicket2.messages.append(QPair<QString, QString>("support", "Здравствуйте! Ваше обращение принято в работу."));
        m_tickets.append(testTicket2);

        saveTickets();
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
    QJsonArray array = doc.array();

    for (const auto &value : array) {
        QJsonObject obj = value.toObject();
        SupportTicket ticket;
        ticket.id = obj["id"].toString();
        ticket.subject = obj["subject"].toString();
        ticket.message = obj["message"].toString();
        ticket.category = obj["category"].toString();
        ticket.status = obj["status"].toString();
        ticket.priority = obj["priority"].toString();
        ticket.createdAt = obj["createdAt"].toString();
        ticket.operatorName = obj["operatorName"].toString();

        QJsonArray messagesArray = obj["messages"].toArray();
        for (const auto &msgVal : messagesArray) {
            QJsonObject msgObj = msgVal.toObject();
            ticket.messages.append(QPair<QString, QString>(msgObj["sender"].toString(), msgObj["text"].toString()));
        }

        m_tickets.append(ticket);
    }
}

void SupportDialog::saveTickets()
{
    QJsonArray array;
    for (const auto &ticket : m_tickets) {
        QJsonObject obj;
        obj["id"] = ticket.id;
        obj["subject"] = ticket.subject;
        obj["message"] = ticket.message;
        obj["category"] = ticket.category;
        obj["status"] = ticket.status;
        obj["priority"] = ticket.priority;
        obj["createdAt"] = ticket.createdAt;
        obj["operatorName"] = ticket.operatorName;

        QJsonArray messagesArray;
        for (const auto &msg : ticket.messages) {
            QJsonObject msgObj;
            msgObj["sender"] = msg.first;
            msgObj["text"] = msg.second;
            messagesArray.append(msgObj);
        }
        obj["messages"] = messagesArray;

        array.append(obj);
    }

    QJsonDocument doc(array);
    QSettings settings("Aura", "Messenger");
    settings.setValue("support_tickets", QString::fromUtf8(doc.toJson()));
}

bool SupportDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QWidget *widget = qobject_cast<QWidget*>(obj);
        if (widget && widget->property("ticketId").isValid()) {
            QString ticketId = widget->property("ticketId").toString();

            // Находим тикет по ID
            SupportTicket selectedTicket;
            for (const auto &ticket : m_tickets) {
                if (ticket.id == ticketId) {
                    selectedTicket = ticket;
                    break;
                }
            }

            if (!selectedTicket.id.isEmpty()) {
                // Получаем сохранённые указатели
                QWidget *workspaceWidget = widget->window();
                QWidget *chatContent = workspaceWidget->property("chatContentLayout").value<QWidget*>();
                QWidget *infoContent = workspaceWidget->property("infoContentLayout").value<QWidget*>();
                QWidget *inputWidget = workspaceWidget->property("inputWidget").value<QWidget*>();

                if (chatContent && infoContent && inputWidget) {
                    // Показываем поле ввода
                    inputWidget->setVisible(true);

                    // Очищаем и заполняем центральную панель (чат)
                    QVBoxLayout *chatLayout = qobject_cast<QVBoxLayout*>(chatContent->layout());
                    if (!chatLayout) {
                        chatLayout = new QVBoxLayout(chatContent);
                        chatLayout->setContentsMargins(20, 20, 20, 20);
                        chatLayout->setSpacing(20);
                    }

                    // Удаляем все существующие виджеты
                    QLayoutItem *item;
                    while ((item = chatLayout->takeAt(0)) != nullptr) {
                        if (item->widget()) delete item->widget();
                        delete item;
                    }

                    // Добавляем заголовок обращения
                    QLabel *subjectLabel = new QLabel(selectedTicket.subject);
                    subjectLabel->setStyleSheet("color: white; font-size: 20px; font-weight: 700; margin-bottom: 20px;");
                    subjectLabel->setWordWrap(true);
                    chatLayout->addWidget(subjectLabel);

                    // Сообщение пользователя (справа)
                    QWidget *userMsgWidget = new QWidget();
                    QHBoxLayout *userMsgLayout = new QHBoxLayout(userMsgWidget);
                    userMsgLayout->setContentsMargins(0, 0, 0, 0);
                    userMsgLayout->setSpacing(0);
                    userMsgLayout->addStretch();

                    QLabel *userBubble = new QLabel(selectedTicket.message);
                    userBubble->setWordWrap(true);
                    userBubble->setMaximumWidth(500);
                    userBubble->setStyleSheet(
                        "background-color: #2563EB; "
                        "border-radius: 16px; "
                        "border-bottom-right-radius: 4px; "
                        "padding: 12px 16px; "
                        "color: white;"
                        );
                    userMsgLayout->addWidget(userBubble);
                    chatLayout->addWidget(userMsgWidget);

                    // Время сообщения пользователя
                    QLabel *userTimeLabel = new QLabel(selectedTicket.createdAt);
                    userTimeLabel->setAlignment(Qt::AlignRight);
                    userTimeLabel->setStyleSheet("color: #64748B; font-size: 11px; margin-top: 4px; margin-bottom: 16px;");
                    userTimeLabel->setContentsMargins(0, 0, 12, 0);
                    chatLayout->addWidget(userTimeLabel);

                    // Сообщение поддержки (слева)
                    QWidget *supportMsgWidget = new QWidget();
                    QHBoxLayout *supportMsgLayout = new QHBoxLayout(supportMsgWidget);
                    supportMsgLayout->setContentsMargins(0, 0, 0, 0);
                    supportMsgLayout->setSpacing(8);

                    QLabel *supportAvatar = new QLabel();
                    QPixmap headsetPixmap(":/icons/darkTheme/images/darkTheme/headset-w.svg");
                    if (!headsetPixmap.isNull()) {
                        supportAvatar->setPixmap(headsetPixmap.scaled(28, 28, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                    }
                    supportAvatar->setFixedSize(32, 32);
                    supportAvatar->setStyleSheet("background-color: #3B82F6; border-radius: 8px; padding: 4px;");
                    supportAvatar->setAlignment(Qt::AlignCenter);
                    supportMsgLayout->addWidget(supportAvatar);

                    QLabel *supportBubble = new QLabel("Здравствуйте! Ваше обращение принято в работу. Мы свяжемся с вами в ближайшее время.");
                    supportBubble->setWordWrap(true);
                    supportBubble->setMaximumWidth(500);
                    supportBubble->setStyleSheet(
                        "background-color: #1E293B; "
                        "border-radius: 16px; "
                        "border-bottom-left-radius: 4px; "
                        "padding: 12px 16px; "
                        "color: white;"
                        );
                    supportMsgLayout->addWidget(supportBubble);
                    supportMsgLayout->addStretch();
                    chatLayout->addWidget(supportMsgWidget);

                    // Время сообщения поддержки
                    QLabel *supportTimeLabel = new QLabel("Оператор ответил сразу");
                    supportTimeLabel->setStyleSheet("color: #64748B; font-size: 11px; margin-top: 4px; margin-bottom: 20px;");
                    supportTimeLabel->setContentsMargins(40, 0, 0, 0);
                    chatLayout->addWidget(supportTimeLabel);

                    chatLayout->addStretch();

                    // === ЗАПОЛНЯЕМ ПРАВУЮ ПАНЕЛЬ (ИНФОРМАЦИЯ) ===
                    QVBoxLayout *infoLayout = qobject_cast<QVBoxLayout*>(infoContent->layout());
                    if (!infoLayout) {
                        infoLayout = new QVBoxLayout(infoContent);
                        infoLayout->setContentsMargins(20, 20, 20, 20);
                        infoLayout->setSpacing(16);
                    }

                    // Удаляем все существующие виджеты
                    while ((item = infoLayout->takeAt(0)) != nullptr) {
                        if (item->widget()) delete item->widget();
                        delete item;
                    }

                    // Функция добавления строки
                    auto addInfoRow = [infoLayout](const QString &label, const QString &value, bool addLine = true) {
                        QWidget *row = new QWidget();
                        QVBoxLayout *rowLayout = new QVBoxLayout(row);
                        rowLayout->setContentsMargins(0, 8, 0, 8);
                        rowLayout->setSpacing(4);

                        QLabel *labelWidget = new QLabel(label);
                        labelWidget->setStyleSheet("color: #64748B; font-size: 12px;");

                        QLabel *valueWidget = new QLabel(value);
                        valueWidget->setStyleSheet("color: white; font-size: 14px; font-weight: 500;");
                        valueWidget->setWordWrap(true);

                        rowLayout->addWidget(labelWidget);
                        rowLayout->addWidget(valueWidget);

                        if (addLine) {
                            QFrame *line = new QFrame();
                            line->setFrameShape(QFrame::HLine);
                            line->setStyleSheet("background-color: #334155; max-height: 1px; margin-top: 8px;");
                            rowLayout->addWidget(line);
                        }

                        infoLayout->addWidget(row);
                    };

                    addInfoRow("НОМЕР", selectedTicket.id);
                    addInfoRow("КАТЕГОРИЯ", selectedTicket.category);

                    // Статус
                    QString statusColor = (selectedTicket.status == "Новая") ? "#F59E0B" :
                                              (selectedTicket.status == "В работе") ? "#3B82F6" : "#10B981";
                    QWidget *statusRow = new QWidget();
                    QVBoxLayout *statusRowLayout = new QVBoxLayout(statusRow);
                    statusRowLayout->setContentsMargins(0, 8, 0, 8);
                    statusRowLayout->setSpacing(4);

                    QLabel *statusLabelWidget = new QLabel("СТАТУС");
                    statusLabelWidget->setStyleSheet("color: #64748B; font-size: 12px;");

                    QLabel *statusValueWidget = new QLabel(selectedTicket.status);
                    statusValueWidget->setStyleSheet(QString("color: %1; font-size: 14px; font-weight: 500;").arg(statusColor));

                    statusRowLayout->addWidget(statusLabelWidget);
                    statusRowLayout->addWidget(statusValueWidget);

                    QFrame *statusLine = new QFrame();
                    statusLine->setFrameShape(QFrame::HLine);
                    statusLine->setStyleSheet("background-color: #334155; max-height: 1px; margin-top: 8px;");
                    statusRowLayout->addWidget(statusLine);
                    infoLayout->addWidget(statusRow);

                    // Приоритет с выбором
                    QWidget *priorityRow = new QWidget();
                    QVBoxLayout *priorityRowLayout = new QVBoxLayout(priorityRow);
                    priorityRowLayout->setContentsMargins(0, 8, 0, 8);
                    priorityRowLayout->setSpacing(4);

                    QLabel *priorityLabelWidget = new QLabel("ПРИОРИТЕТ");
                    priorityLabelWidget->setStyleSheet("color: #64748B; font-size: 12px;");

                    QComboBox *priorityCombo = new QComboBox();
                    priorityCombo->addItem("Низкий", "low");
                    priorityCombo->addItem("Средний", "medium");
                    priorityCombo->addItem("Высокий", "high");
                    priorityCombo->setStyleSheet(
                        "QComboBox { background-color: #1E293B; border: 1px solid #334155; border-radius: 8px; padding: 8px 12px; color: white; }"
                        "QComboBox::drop-down { border: none; width: 24px; }"
                        );

                    int idx = priorityCombo->findData(selectedTicket.priority);
                    if (idx >= 0) priorityCombo->setCurrentIndex(idx);

                    priorityRowLayout->addWidget(priorityLabelWidget);
                    priorityRowLayout->addWidget(priorityCombo);

                    QFrame *priorityLine = new QFrame();
                    priorityLine->setFrameShape(QFrame::HLine);
                    priorityLine->setStyleSheet("background-color: #334155; max-height: 1px; margin-top: 8px;");
                    priorityRowLayout->addWidget(priorityLine);
                    infoLayout->addWidget(priorityRow);

                    addInfoRow("СОЗДАНО", selectedTicket.createdAt);
                    addInfoRow("ОПЕРАТОР", selectedTicket.operatorName, false);

                    infoLayout->addStretch();
                }
            }
            return true;
        }
    }
    return QDialog::eventFilter(obj, event);
}

void SupportDialog::openTicketDetail(const SupportTicket &ticket,
                                     QWidget *chatContentLayout,
                                     QWidget *infoContentLayout,
                                     QWidget *inputWidget)
{
    // 1. Показываем поле ввода
    inputWidget->setVisible(true);

    // 2. Очищаем центральную панель (чат)
    QVBoxLayout *chatLayout = qobject_cast<QVBoxLayout*>(chatContentLayout->layout());
    if (!chatLayout) {
        chatLayout = new QVBoxLayout(chatContentLayout);
        chatLayout->setContentsMargins(20, 20, 20, 20);
        chatLayout->setSpacing(20);
    }

    // Удаляем все существующие виджеты
    QLayoutItem *item;
    while ((item = chatLayout->takeAt(0)) != nullptr) {
        if (item->widget()) delete item->widget();
        delete item;
    }

    // Добавляем заголовок обращения
    QLabel *subjectLabel = new QLabel(ticket.subject);
    subjectLabel->setStyleSheet("color: white; font-size: 20px; font-weight: 700; margin-bottom: 20px;");
    subjectLabel->setWordWrap(true);
    chatLayout->addWidget(subjectLabel);

    // === Сообщение пользователя (справа) ===
    QWidget *userMsgWidget = new QWidget();
    QHBoxLayout *userMsgLayout = new QHBoxLayout(userMsgWidget);
    userMsgLayout->setContentsMargins(0, 0, 0, 0);
    userMsgLayout->setSpacing(0);
    userMsgLayout->addStretch();

    QLabel *userBubble = new QLabel(ticket.message);
    userBubble->setWordWrap(true);
    userBubble->setMaximumWidth(500);
    userBubble->setStyleSheet(
        "background-color: #2563EB; "
        "border-radius: 16px; "
        "border-bottom-right-radius: 4px; "
        "padding: 12px 16px; "
        "color: white; "
        "line-height: 1.5;"
        );
    userMsgLayout->addWidget(userBubble);
    chatLayout->addWidget(userMsgWidget);

    // Время сообщения пользователя
    QLabel *userTimeLabel = new QLabel(ticket.createdAt);
    userTimeLabel->setAlignment(Qt::AlignRight);
    userTimeLabel->setStyleSheet("color: #64748B; font-size: 11px; margin-top: 4px; margin-bottom: 16px;");
    userTimeLabel->setContentsMargins(0, 0, 12, 0);
    chatLayout->addWidget(userTimeLabel);

    // === Сообщение поддержки (слева) ===
    QWidget *supportMsgWidget = new QWidget();
    QHBoxLayout *supportMsgLayout = new QHBoxLayout(supportMsgWidget);
    supportMsgLayout->setContentsMargins(0, 0, 0, 0);
    supportMsgLayout->setSpacing(0);

    // Аватар поддержки
    QLabel *supportAvatar = new QLabel();
    QPixmap headsetPixmap(":/icons/darkTheme/images/darkTheme/headset-w.svg");
    if (!headsetPixmap.isNull()) {
        supportAvatar->setPixmap(headsetPixmap.scaled(28, 28, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    supportAvatar->setFixedSize(32, 32);
    supportAvatar->setStyleSheet("background-color: #3B82F6; border-radius: 8px; padding: 4px;");
    supportAvatar->setAlignment(Qt::AlignCenter);
    supportMsgLayout->addWidget(supportAvatar);
    supportMsgLayout->addSpacing(8);

    QLabel *supportBubble = new QLabel("Здравствуйте! Ваше обращение принято в работу. Мы свяжемся с вами в ближайшее время.");
    supportBubble->setWordWrap(true);
    supportBubble->setMaximumWidth(500);
    supportBubble->setStyleSheet(
        "background-color: #1E293B; "
        "border-radius: 16px; "
        "border-bottom-left-radius: 4px; "
        "padding: 12px 16px; "
        "color: white; "
        "line-height: 1.5;"
        );
    supportMsgLayout->addWidget(supportBubble);
    supportMsgLayout->addStretch();
    chatLayout->addWidget(supportMsgWidget);

    // Время сообщения поддержки
    QLabel *supportTimeLabel = new QLabel("Оператор ответил сразу");
    supportTimeLabel->setStyleSheet("color: #64748B; font-size: 11px; margin-top: 4px; margin-bottom: 20px;");
    supportTimeLabel->setContentsMargins(40, 0, 0, 0);
    chatLayout->addWidget(supportTimeLabel);

    // Добавляем растяжку в конец
    chatLayout->addStretch();

    // 3. Очищаем правую панель (информация)
    QVBoxLayout *infoLayout = qobject_cast<QVBoxLayout*>(infoContentLayout->layout());
    if (!infoLayout) {
        infoLayout = new QVBoxLayout(infoContentLayout);
        infoLayout->setContentsMargins(20, 20, 20, 20);
        infoLayout->setSpacing(16);
    }

    // Удаляем все существующие виджеты
    while ((item = infoLayout->takeAt(0)) != nullptr) {
        if (item->widget()) delete item->widget();
        delete item;
    }

    // Функция добавления строки информации
    auto addInfoRow = [infoLayout](const QString &label, const QString &value, bool isLast = false) {
        QWidget *row = new QWidget();
        QVBoxLayout *rowLayout = new QVBoxLayout(row);
        rowLayout->setContentsMargins(0, 8, 0, 8);
        rowLayout->setSpacing(4);

        QLabel *labelWidget = new QLabel(label);
        labelWidget->setStyleSheet("color: #64748B; font-size: 12px;");

        QLabel *valueWidget = new QLabel(value);
        valueWidget->setStyleSheet("color: white; font-size: 14px; font-weight: 500;");
        valueWidget->setWordWrap(true);

        rowLayout->addWidget(labelWidget);
        rowLayout->addWidget(valueWidget);

        if (!isLast) {
            QFrame *line = new QFrame();
            line->setFrameShape(QFrame::HLine);
            line->setStyleSheet("background-color: #334155; max-height: 1px; margin-top: 8px;");
            rowLayout->addWidget(line);
        }

        infoLayout->addWidget(row);
    };

    // Номер обращения
    addInfoRow("НОМЕР", ticket.id);

    // Категория
    addInfoRow("КАТЕГОРИЯ", ticket.category);

    // Статус (только для просмотра)
    QString statusColor = (ticket.status == "Новая") ? "#F59E0B" :
                              (ticket.status == "В работе") ? "#3B82F6" : "#10B981";
    QWidget *statusRow = new QWidget();
    QVBoxLayout *statusRowLayout = new QVBoxLayout(statusRow);
    statusRowLayout->setContentsMargins(0, 8, 0, 8);
    statusRowLayout->setSpacing(4);

    QLabel *statusLabelWidget = new QLabel("СТАТУС");
    statusLabelWidget->setStyleSheet("color: #64748B; font-size: 12px;");

    QLabel *statusValueWidget = new QLabel(ticket.status);
    statusValueWidget->setStyleSheet(QString("color: %1; font-size: 14px; font-weight: 500;").arg(statusColor));

    statusRowLayout->addWidget(statusLabelWidget);
    statusRowLayout->addWidget(statusValueWidget);

    QFrame *statusLine = new QFrame();
    statusLine->setFrameShape(QFrame::HLine);
    statusLine->setStyleSheet("background-color: #334155; max-height: 1px; margin-top: 8px;");
    statusRowLayout->addWidget(statusLine);

    infoLayout->addWidget(statusRow);

    // Приоритет (с возможностью изменения)
    QWidget *priorityRow = new QWidget();
    QVBoxLayout *priorityRowLayout = new QVBoxLayout(priorityRow);
    priorityRowLayout->setContentsMargins(0, 8, 0, 8);
    priorityRowLayout->setSpacing(4);

    QLabel *priorityLabelWidget = new QLabel("ПРИОРИТЕТ");
    priorityLabelWidget->setStyleSheet("color: #64748B; font-size: 12px;");

    QComboBox *priorityCombo = new QComboBox();
    priorityCombo->addItem("Низкий", "low");
    priorityCombo->addItem("Средний", "medium");
    priorityCombo->addItem("Высокий", "high");
    priorityCombo->setStyleSheet(
        "QComboBox { "
        "   background-color: #1E293B; "
        "   border: 1px solid #334155; "
        "   border-radius: 8px; "
        "   padding: 8px 12px; "
        "   color: white; "
        "   font-size: 13px; "
        "}"
        "QComboBox::drop-down { border: none; width: 24px; }"
        "QComboBox QAbstractItemView { background-color: #1E293B; color: white; border: 1px solid #334155; }"
        );

    int idx = priorityCombo->findData(ticket.priority);
    if (idx >= 0) priorityCombo->setCurrentIndex(idx);

    connect(priorityCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this, &ticket, priorityCombo](int) {
        for (auto &t : m_tickets) {
            if (t.id == ticket.id) {
                t.priority = priorityCombo->currentData().toString();
                saveTickets();
                break;
            }
        }
    });

    priorityRowLayout->addWidget(priorityLabelWidget);
    priorityRowLayout->addWidget(priorityCombo);

    QFrame *priorityLine = new QFrame();
    priorityLine->setFrameShape(QFrame::HLine);
    priorityLine->setStyleSheet("background-color: #334155; max-height: 1px; margin-top: 8px;");
    priorityRowLayout->addWidget(priorityLine);

    infoLayout->addWidget(priorityRow);

    // Дата создания
    addInfoRow("СОЗДАНО", ticket.createdAt, false);

    // Оператор
    addInfoRow("ОПЕРАТОР", ticket.operatorName, true);

    infoLayout->addStretch();
}
