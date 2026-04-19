#include "supportdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QFrame>

static QString getHeaderGradientColor() {
    return "qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #0EA5E9, stop:1 #0284C7)";
}

SupportDialog::SupportDialog(QWidget *parent) : QDialog(parent)
{
    setupUI();
}

SupportDialog::~SupportDialog() = default;

void SupportDialog::setupUI()
{
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
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

    QLabel *titleLabel = new QLabel("🎧 Служба поддержки");
    titleLabel->setStyleSheet("color: white; font-size: 18px; font-weight: 600; background-color: transparent;");

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

    // Заголовок с иконкой
    QWidget *info = new QWidget();
    QVBoxLayout *infoLayout = new QVBoxLayout(info);
    infoLayout->setSpacing(8);

    QLabel *headsetIcon = new QLabel("🎧");
    headsetIcon->setAlignment(Qt::AlignCenter);
    headsetIcon->setStyleSheet("font-size: 48px; background-color: transparent;");
    QLabel *helpTitle = new QLabel("Нужна помощь?");
    helpTitle->setAlignment(Qt::AlignCenter);
    helpTitle->setStyleSheet("color: #f1f5f9; font-size: 18px; font-weight: bold;");
    QLabel *helpDesc = new QLabel("Выберите категорию обращения");
    helpDesc->setAlignment(Qt::AlignCenter);
    helpDesc->setStyleSheet("color: #94a3b8; font-size: 14px;");

    infoLayout->addWidget(headsetIcon);
    infoLayout->addWidget(helpTitle);
    infoLayout->addWidget(helpDesc);

    contentLayout->addWidget(info);

    // Карточки категорий
    QWidget *categories = new QWidget();
    QGridLayout *grid = new QGridLayout(categories);
    grid->setSpacing(12);
    grid->setContentsMargins(0, 0, 0, 0);

    grid->addWidget(createSupportCard("💻", "IT поддержка", "Проблемы с техникой, ПО", "#3B82F6"), 0, 0);
    grid->addWidget(createSupportCard("👔", "HR вопросы", "Отпуска, документы", "#8B5CF6"), 0, 1);
    grid->addWidget(createSupportCard("💰", "Финансы", "Зарплата, расходы", "#F59E0B"), 1, 0);
    grid->addWidget(createSupportCard("❓", "Другое", "Иные вопросы", "#64748B"), 1, 1);

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

QWidget* SupportDialog::createSupportCard(const QString &emoji, const QString &title, const QString &desc, const QString &color)
{
    QWidget *card = new QWidget();
    card->setCursor(Qt::PointingHandCursor);

    // Единый цвет для всех карточек при наведении (фиолетовый, как у HR)
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

    QLabel *emojiLabel = new QLabel(emoji);
    emojiLabel->setStyleSheet(QString("font-size: 28px; color: %1; background-color: transparent; border: none;").arg(color));
    emojiLabel->setAlignment(Qt::AlignCenter);

    QLabel *titleLabel = new QLabel(title);
    titleLabel->setStyleSheet("color: #f1f5f9; font-size: 14px; font-weight: 600; background-color: transparent; border: none;");
    titleLabel->setAlignment(Qt::AlignCenter);

    QLabel *descLabel = new QLabel(desc);
    descLabel->setStyleSheet("color: #94a3b8; font-size: 12px; background-color: transparent; border: none;");
    descLabel->setAlignment(Qt::AlignCenter);
    descLabel->setWordWrap(true);

    layout->addWidget(emojiLabel);
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
