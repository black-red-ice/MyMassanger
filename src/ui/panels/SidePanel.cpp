#include "SidePanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QScrollArea>
#include <QIcon>
#include <QPixmap>
#include <QDebug>

SidePanel::SidePanel(QWidget *parent,
                     const QString &title,
                     const QString &iconPath,
                     const QString &headerColor,
                     const QString &buttonText,
                     int panelWidth,
                     const QString &buttonColor,
                     const QString &buttonIconPath)
    : QWidget(parent), m_panelWidth(panelWidth), m_buttonColor(buttonColor),
    m_buttonIconPath(buttonIconPath), m_headerIconLabel(nullptr)
{
    setFixedWidth(panelWidth);
    setStyleSheet("background-color: #1e293b;");

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    setupHeader(title, iconPath, headerColor);
    setupSearch();
    setupContentArea();
    setupFooter(buttonText);
}

void SidePanel::setupHeader(const QString &title, const QString &iconPath, const QString &headerColor)
{
    QWidget *header = new QWidget();
    header->setFixedHeight(77);
    header->setStyleSheet(QString("background-color: %1;").arg(headerColor));
    QHBoxLayout *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(20, 0, 20, 0);

    // Иконка заголовка
    m_headerIconLabel = new QLabel();
    m_headerIconLabel->setStyleSheet("background: transparent;");
    m_headerIconLabel->setFixedSize(24, 24);

    if (!iconPath.isEmpty()) {
        // Пробуем загрузить SVG иконку
        QPixmap pixmap(iconPath);
        if (!pixmap.isNull()) {
            m_headerIconLabel->setPixmap(pixmap.scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            qDebug() << "SidePanel: Icon loaded from" << iconPath;
        } else {
            // Если не загрузилась, показываем текстовый индикатор
            m_headerIconLabel->setText("●");
            m_headerIconLabel->setStyleSheet("color: white; background: transparent; font-size: 16px;");
            qDebug() << "SidePanel: Failed to load icon from" << iconPath << ", using fallback";
        }
        headerLayout->addWidget(m_headerIconLabel);
    }

    // Заголовок
    QLabel *titleLabel = new QLabel(title);
    titleLabel->setStyleSheet(
        "color: white;"
        "font-size: 18px;"
        "font-weight: 600;"
        "background: transparent;"
        "margin-left: 8px;"
        );
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();

    // Крестик закрытия
    QPushButton *closeBtn = new QPushButton("✕");
    closeBtn->setFixedSize(36, 36);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(
        "QPushButton {"
        "  background: rgba(255, 255, 255, 0.1);"
        "  border: none;"
        "  border-radius: 18px;"
        "  color: white;"
        "  font-size: 16px;"
        "}"
        "QPushButton:hover {"
        "  background: rgba(255, 255, 255, 0.2);"
        "}"
        );

    // ВАЖНО: Испускаем сигнал closeRequested вместо простого hide
    connect(closeBtn, &QPushButton::clicked, this, [this]() {
        qDebug() << "SidePanel: Close button clicked, emitting closeRequested";
        emit closeRequested();
    });

    headerLayout->addWidget(closeBtn);

    m_mainLayout->addWidget(header);
}

void SidePanel::setHeaderIcon(const QString &iconPath)
{
    if (m_headerIconLabel && !iconPath.isEmpty()) {
        QPixmap pixmap(iconPath);
        if (!pixmap.isNull()) {
            m_headerIconLabel->setPixmap(pixmap.scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    }
}

void SidePanel::setupSearch()
{
    QWidget *searchWidget = new QWidget();
    searchWidget->setStyleSheet("background: #1e293b;");
    QHBoxLayout *searchLayout = new QHBoxLayout(searchWidget);
    searchLayout->setContentsMargins(16, 12, 16, 12);

    m_searchInput = new QLineEdit();
    m_searchInput->setPlaceholderText("Поиск...");
    m_searchInput->setStyleSheet(
        "QLineEdit {"
        "  background: #334155;"
        "  border: 1px solid #475569;"
        "  border-radius: 20px;"
        "  color: #f1f5f9;"
        "  font-size: 14px;"
        "  padding: 12px 16px 12px 44px;"
        "}"
        "QLineEdit:focus {"
        "  border-color: #1d4ed8;"
        "}"
        );
    m_searchInput->setFixedHeight(44);

    // Иконка поиска
    QLabel *searchIcon = new QLabel(m_searchInput);
    QPixmap searchPixmap(":/icons/general/images/general/magnifying-glass.svg");
    if (!searchPixmap.isNull()) {
        searchIcon->setPixmap(searchPixmap.scaled(16, 16, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        searchIcon->setText("🔍");
    }
    searchIcon->setStyleSheet("background: transparent;");
    searchIcon->move(14, 14);

    searchLayout->addWidget(m_searchInput);
    m_mainLayout->addWidget(searchWidget);

    // Разделитель
    QFrame *separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    separator->setStyleSheet("color: #334155; max-height: 1px;");
    m_mainLayout->addWidget(separator);
    m_separator = separator;
}

void SidePanel::setupContentArea()
{
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    scrollArea->setStyleSheet(
        "QScrollArea { background: #1e293b; border: none; }"
        "QScrollBar:vertical { background: transparent; width: 6px; }"
        "QScrollBar::handle:vertical { background: #334155; border-radius: 3px; min-height: 30px; }"
        "QScrollBar::handle:vertical:hover { background: #475569; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }"
        );

    QWidget *scrollContent = new QWidget();
    scrollContent->setStyleSheet("background: #1e293b;");
    m_contentLayout = new QVBoxLayout(scrollContent);
    m_contentLayout->setContentsMargins(16, 12, 16, 12);
    m_contentLayout->setSpacing(12);
    m_contentLayout->addStretch();

    scrollArea->setWidget(scrollContent);
    m_mainLayout->addWidget(scrollArea, 1);
}

void SidePanel::setupFooter(const QString &buttonText)
{
    QWidget *footer = new QWidget();
    footer->setStyleSheet("background: #1e293b; border-top: 1px solid #334155;");
    QVBoxLayout *footerLayout = new QVBoxLayout(footer);
    footerLayout->setContentsMargins(16, 12, 16, 16);

    m_addButton = new QPushButton(buttonText);
    m_addButton->setCursor(Qt::PointingHandCursor);
    m_addButton->setFixedHeight(44);

    // Загружаем иконку если путь указан
    if (!m_buttonIconPath.isEmpty()) {
        QPixmap pixmap(m_buttonIconPath);
        if (!pixmap.isNull()) {
            m_addButton->setIcon(QIcon(pixmap));
            m_addButton->setIconSize(QSize(20, 20));
        }
    }

    m_addButton->setStyleSheet(
        QString(
            "QPushButton {"
            "  background-color: %1;"
            "  border: none;"
            "  border-radius: 10px;"
            "  color: white;"
            "  font-size: 14px;"
            "  font-weight: 600;"
            "}"
            "QPushButton:hover { background-color: #1d4ed8; }"
            ).arg(m_buttonColor)
        );

    connect(m_addButton, &QPushButton::clicked, this, [this]() {
        onAddClicked();
        emit addClicked();
    });

    footerLayout->addWidget(m_addButton);
    m_mainLayout->addWidget(footer);
}

void SidePanel::setSearchVisible(bool visible)
{
    m_searchInput->setVisible(visible);
    if (m_separator) {
        m_separator->setVisible(visible);
    }
}
