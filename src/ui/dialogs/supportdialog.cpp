#include "supportdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QFrame>
#include <QPixmap>
#include <QIcon>
#include <QPainter>
#include <QDialog>

SupportDialog::SupportDialog(QWidget *parent) : OverlayDialog(parent)
{
    setupUI();
}

SupportDialog::~SupportDialog() = default;

void SupportDialog::setupUI()
{
    //setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    //setAttribute(Qt::WA_TranslucentBackground);
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

    // Иконка наушников вместо эмодзи
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

    // Карточки категорий с SVG-иконками
    QWidget *categories = new QWidget();
    QGridLayout *grid = new QGridLayout(categories);
    grid->setSpacing(12);
    grid->setContentsMargins(0, 0, 0, 0);

    grid->addWidget(createSupportCard(":/icons/darkTheme/images/darkTheme/laptop.svg", "IT поддержка", "Проблемы с техникой, ПО", "#3B82F6"), 0, 0);
    grid->addWidget(createSupportCard(":/icons/darkTheme/images/darkTheme/user-tie-p.svg", "HR вопросы", "Отпуска, документы", "#8B5CF6"), 0, 1);
    grid->addWidget(createSupportCard(":/icons/darkTheme/images/darkTheme/coins.svg", "Финансы", "Зарплата, расходы", "#F59E0B"), 1, 0);
    grid->addWidget(createSupportCard(":/icons/darkTheme/images/darkTheme/circle-question.svg", "Другое", "Иные вопросы", "#64748B"), 1, 1);

    contentLayout->addWidget(categories);

    // Мои обращения
    QWidget *tickets = new QWidget();
    QVBoxLayout *ticketsLayout = new QVBoxLayout(tickets);
    ticketsLayout->setSpacing(8);

    QLabel *ticketsTitle = new QLabel("Мои обращения");
    ticketsTitle->setStyleSheet("color: #f1f5f9; font-size: 15px; font-weight: 600;");

    QWidget *list = new QWidget();
    list->setStyleSheet("background-color: #1e293b; border-radius: 12px; padding: 4px;");
    QVBoxLayout *listLayout = new QVBoxLayout(list);
    listLayout->setSpacing(0);
    listLayout->setContentsMargins(16, 8, 16, 8);

    listLayout->addWidget(createTicketItem("#IT-245", "Проблема с принтером", "В работе", "#F59E0B"));

    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("background-color: #334155; max-height: 1px; margin: 8px 0;");
    listLayout->addWidget(line);

    listLayout->addWidget(createTicketItem("#HR-189", "Запрос на отпуск", "Завершён", "#10B981"));

    ticketsLayout->addWidget(ticketsTitle);
    ticketsLayout->addWidget(list);

    contentLayout->addWidget(tickets);

    containerLayout->addWidget(content);
}

QWidget* SupportDialog::createSupportCard(const QString &iconPath, const QString &title, const QString &desc, const QString &color)
{
    QWidget *card = new QWidget();
    card->setCursor(Qt::PointingHandCursor);

    QString hoverBorderColor = "#8B5CF6";

    QString styleSheet = QString(
                             "QWidget { "
                             "   background-color: #1e293b; "
                             "   border-radius: 12px; "
                             "   border: 1px solid rgba(29, 78, 216, 0.2); "
                             "}"
                             "QWidget:hover { "
                             "   border: 1px solid %1; "
                             "}"
                             ).arg(hoverBorderColor);

    card->setStyleSheet(styleSheet);

    QVBoxLayout *layout = new QVBoxLayout(card);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(8);

    // Иконка из SVG
    QLabel *iconLabel = new QLabel();
    QPixmap pixmap(iconPath);
    if (!pixmap.isNull()) {
        // Окрашиваем иконку в цвет категории
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
    connect(btn, &QPushButton::clicked, [title]() {
        // Действие
    });

    return card;
}

QWidget* SupportDialog::createTicketItem(const QString &id, const QString &title, const QString &status, const QString &statusColor)
{
    QWidget *item = new QWidget();
    QHBoxLayout *layout = new QHBoxLayout(item);
    layout->setContentsMargins(0, 8, 0, 8);
    layout->setSpacing(12);

    QLabel *info = new QLabel(id + " - " + title);
    info->setStyleSheet("color: #f1f5f9; font-size: 13px; font-weight: 500;");

    QLabel *statusLabel = new QLabel(status);
    statusLabel->setStyleSheet(QString("color: %1; font-size: 13px; font-weight: 500;").arg(statusColor));

    layout->addWidget(info, 1);
    layout->addWidget(statusLabel);

    return item;
}
