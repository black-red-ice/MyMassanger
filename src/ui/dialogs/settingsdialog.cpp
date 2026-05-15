#include "settingsdialog.h"
#include "ui/widgets/togglebutton.h"
#include "core/utils/stylehelper.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QScrollArea>
#include <QPixmap>
#include <QIcon>
#include <QPainter>
#include <QTimer>

SettingsDialog::SettingsDialog(QWidget *parent) : OverlayDialog(parent)
{
    setupUI();
}

SettingsDialog::~SettingsDialog() = default;

void SettingsDialog::setupUI()
{
    setFixedSize(560, 650);
    setStyleSheet("background-color: transparent;");

    // Контейнер с фоном и скруглением
    QWidget *container = new QWidget(this);
    container->setObjectName("container");
    container->setAutoFillBackground(true);
    container->setStyleSheet(
        "#container {"
        "  background-color: #1e293b;"
        "  border-radius: 16px;"
        "}"
        );

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(container);

    QVBoxLayout *containerLayout = new QVBoxLayout(container);
    containerLayout->setSpacing(0);
    containerLayout->setContentsMargins(0, 0, 0, 0);

    // ========== Заголовок ==========
    QWidget *header = new QWidget();
    header->setStyleSheet(
        "background-color: #1e293b;"
        "border-top-left-radius: 16px;"
        "border-top-right-radius: 16px;"
        "border-bottom: 1px solid #334155;"
        );
    header->setFixedHeight(70);

    QHBoxLayout *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(24, 0, 24, 0);

    QLabel *iconLabel = new QLabel();
    iconLabel->setPixmap(QPixmap(":/icons/darkTheme/images/darkTheme/gear.svg").scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    QLabel *titleLabel = new QLabel("Настройки");
    titleLabel->setStyleSheet("color: #f1f5f9; font-size: 18px; font-weight: 600; margin-left: 8px;");

    QPushButton *closeBtn = new QPushButton("✕");
    closeBtn->setFixedSize(36, 36);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(
        "QPushButton { background-color: #334155; border: none; border-radius: 18px; color: #94a3b8; font-size: 16px; }"
        "QPushButton:hover { background-color: #1d4ed8; color: white; }"
        );
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);

    QHBoxLayout *titleLayout = new QHBoxLayout();
    titleLayout->setSpacing(0);
    titleLayout->addWidget(iconLabel);
    titleLayout->addWidget(titleLabel);

    headerLayout->addLayout(titleLayout);
    headerLayout->addStretch();
    headerLayout->addWidget(closeBtn);

    containerLayout->addWidget(header);

    // ========== Скроллируемая область ==========
    QScrollArea *scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet(
        "QScrollArea {"
        "  background-color: transparent;"
        "  border: none;"
        "}"
        "QScrollBar:vertical {"
        "  background-color: #1e293b;"
        "  width: 8px;"
        "  border-radius: 4px;"
        "  margin: 0px;"
        "}"
        "QScrollBar::handle:vertical {"
        "  background-color: #475569;"
        "  border-radius: 4px;"
        "  min-height: 30px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "  background-color: #1d4ed8;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "  height: 0px;"
        "}"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
        "  background-color: #1e293b;"
        "}"
        );

    QWidget *content = new QWidget();
    content->setStyleSheet("background-color: #1e293b;");
    QVBoxLayout *contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(24, 24, 24, 24);
    contentLayout->setSpacing(20);

    // ========== Группа: Внешний вид ==========
    QWidget *appearanceGroup = createSettingsGroup("Внешний вид", ":/icons/darkTheme/images/darkTheme/palette.svg");
    QVBoxLayout *appearanceLayout = qobject_cast<QVBoxLayout*>(appearanceGroup->layout());

    QWidget *themeItem = createSettingsItem("Тема", "Выбор цветовой схемы");
    m_themeCombo = StyleHelper::createComboBox({"Тёмная", "Светлая", "Авто (системная)"});
    qobject_cast<QHBoxLayout*>(themeItem->layout())->addWidget(m_themeCombo);
    appearanceLayout->addWidget(themeItem);

    QWidget *accentItem = createSettingsItem("Акцентный цвет", "Цвет кнопок и выделений");
    m_accentCombo = StyleHelper::createComboBox({"Синий (Aura)", "Фиолетовый", "Зелёный", "Красный"});
    qobject_cast<QHBoxLayout*>(accentItem->layout())->addWidget(m_accentCombo);
    appearanceLayout->addWidget(accentItem);

    QWidget *densityItem = createSettingsItem("Плотность интерфейса", "Размер элементов");
    m_densityCombo = StyleHelper::createComboBox({"Компактная", "Обычная", "Комфортная"});
    m_densityCombo->setCurrentIndex(1);
    qobject_cast<QHBoxLayout*>(densityItem->layout())->addWidget(m_densityCombo);
    appearanceLayout->addWidget(densityItem);

    contentLayout->addWidget(appearanceGroup);

    // ========== Группа: Уведомления ==========
    QWidget *notifyGroup = createSettingsGroup("Уведомления", ":/icons/darkTheme/images/darkTheme/bell.svg");
    QVBoxLayout *notifyLayout = qobject_cast<QVBoxLayout*>(notifyGroup->layout());
    notifyLayout->addWidget(createToggleItem("Включить уведомления", "Показывать уведомления о новых сообщениях", true));
    notifyLayout->addWidget(createToggleItem("Звуковые уведомления", "Проигрывать звук при новом сообщении", true));
    notifyLayout->addWidget(createToggleItem("Уведомления в трее", "Показывать уведомления в системном трее", true));
    contentLayout->addWidget(notifyGroup);

    // ========== Группа: Конфиденциальность ==========
    QWidget *privacyGroup = createSettingsGroup("Конфиденциальность", ":/icons/darkTheme/images/darkTheme/shield.svg");
    QVBoxLayout *privacyLayout = qobject_cast<QVBoxLayout*>(privacyGroup->layout());

    QWidget *statusItem = createSettingsItem("Статус \"В сети\"", "Кто может видеть ваш статус");
    QComboBox *statusCombo = StyleHelper::createComboBox({"Все", "Только контакты", "Никто"});
    statusCombo->setCurrentIndex(1);
    qobject_cast<QHBoxLayout*>(statusItem->layout())->addWidget(statusCombo);
    privacyLayout->addWidget(statusItem);

    QWidget *photoItem = createSettingsItem("Фотография профиля", "Кто может видеть вашу фотографию");
    QComboBox *photoCombo = StyleHelper::createComboBox({"Все", "Только контакты", "Никто"});
    photoCombo->setCurrentIndex(1);
    qobject_cast<QHBoxLayout*>(photoItem->layout())->addWidget(photoCombo);
    privacyLayout->addWidget(photoItem);

    privacyLayout->addWidget(createToggleItem("Двухфакторная аутентификация", "Дополнительная защита аккаунта", false));
    contentLayout->addWidget(privacyGroup);

    // ========== Кнопки ==========
    QWidget *buttonWidget = new QWidget();
    buttonWidget->setStyleSheet("background-color: transparent;");
    QHBoxLayout *buttonLayout = new QHBoxLayout(buttonWidget);
    buttonLayout->setContentsMargins(0, 20, 0, 20);
    buttonLayout->setSpacing(12);

    QPushButton *cancelBtn = new QPushButton("Отмена");
    cancelBtn->setFixedHeight(48);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setStyleSheet(
        "QPushButton { background-color: #334155; border: 1px solid #475569; border-radius: 8px; color: #f1f5f9; font-size: 14px; font-weight: 500; }"
        "QPushButton:hover { background-color: #475569; border-color: #1d4ed8; }"
        );
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    QPushButton *saveBtn = new QPushButton("Сохранить настройки");
    saveBtn->setFixedHeight(48);
    saveBtn->setCursor(Qt::PointingHandCursor);
    saveBtn->setStyleSheet(
        "QPushButton { background-color: #1d4ed8; border: none; border-radius: 8px; color: white; font-size: 14px; font-weight: 500; }"
        "QPushButton:hover { background-color: #2563eb; }"
        );
    connect(saveBtn, &QPushButton::clicked, this, &SettingsDialog::saveSettings);

    buttonLayout->addWidget(cancelBtn, 1);
    buttonLayout->addWidget(saveBtn, 1);
    contentLayout->addWidget(buttonWidget);

    // ========== Версия приложения ==========
    QLabel *versionLabel = new QLabel("Aura v2.4.1 (build 2026.05.12)");
    versionLabel->setAlignment(Qt::AlignCenter);
    versionLabel->setStyleSheet(
        "color: #64748B;"
        "font-size: 11px;"
        "background: transparent;"
        "padding: 0px;"
        "margin-top: 8px;"
        );
    contentLayout->addWidget(versionLabel);

    scroll->setWidget(content);
    containerLayout->addWidget(scroll, 1);
    // ✅ Виджет для закругления снизу
    QWidget *bottomRound = new QWidget();
    bottomRound->setFixedHeight(16);
    bottomRound->setStyleSheet(
        "background-color: #1e293b;"
        "border-bottom-left-radius: 16px;"
        "border-bottom-right-radius: 16px;"
        );
    containerLayout->addWidget(bottomRound);
}

QWidget* SettingsDialog::createSettingsGroup(const QString &title, const QString &iconPath)
{
    QWidget *group = new QWidget();
    group->setStyleSheet("QWidget { background-color: #334155; border-radius: 12px; }");
    QVBoxLayout *layout = new QVBoxLayout(group);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    // Заголовок группы с иконкой
    QWidget *titleWidget = new QWidget();
    QHBoxLayout *titleLayout = new QHBoxLayout(titleWidget);
    titleLayout->setContentsMargins(16, 16, 16, 12);
    titleLayout->setSpacing(10);

    // Иконка группы
    if (!iconPath.isEmpty()) {
        QLabel *iconLabel = new QLabel();
        QIcon icon(iconPath);
        QPixmap pixmap = icon.pixmap(20, 20);
        if (!pixmap.isNull()) {
            iconLabel->setPixmap(pixmap);
        }
        titleLayout->addWidget(iconLabel);
    }

    QLabel *titleLabel = new QLabel(title);
    titleLabel->setStyleSheet("color: #f1f5f9; font-size: 16px; font-weight: 600; background-color: transparent;");
    titleLayout->addWidget(titleLabel);
    titleLayout->addStretch();

    layout->addWidget(titleWidget);

    return group;
}

QWidget* SettingsDialog::createSettingsItem(const QString &title, const QString &description)
{
    QWidget *item = new QWidget();
    item->setStyleSheet("background-color: transparent;");

    QHBoxLayout *layout = new QHBoxLayout(item);
    layout->setContentsMargins(16, 12, 16, 12);
    layout->setSpacing(16);

    QWidget *textWidget = new QWidget();
    QVBoxLayout *textLayout = new QVBoxLayout(textWidget);
    textLayout->setSpacing(4);
    textLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *titleLabel = new QLabel(title);
    titleLabel->setStyleSheet("color: #f1f5f9; font-size: 14px; font-weight: 500;");
    QLabel *descLabel = new QLabel(description);
    descLabel->setStyleSheet("color: #94a3b8; font-size: 13px;");
    descLabel->setWordWrap(true);

    textLayout->addWidget(titleLabel);
    textLayout->addWidget(descLabel);

    layout->addWidget(textWidget, 1);
    layout->addStretch();

    return item;
}

QWidget* SettingsDialog::createToggleItem(const QString &title, const QString &description, bool checked)
{
    QWidget *item = createSettingsItem(title, description);
    ToggleButton *toggle = new ToggleButton(checked, item);
    qobject_cast<QHBoxLayout*>(item->layout())->addWidget(toggle);
    return item;
}

void SettingsDialog::saveSettings()
{
    // Здесь будет сохранение настроек через SettingsManager
    accept();
}

void SettingsDialog::reject()
{
    QDialog::reject();
}
