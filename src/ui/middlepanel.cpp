#include "middlepanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QEvent>

MiddlePanel::MiddlePanel(QWidget *parent) : QWidget(parent)
{
    setupUI();
}

void MiddlePanel::setupUI()
{
    setFixedWidth(320);
    setStyleSheet("background-color: #0f172a; border-right: 1px solid #1e293b;");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Заголовок с поиском
    QWidget *searchWidget = new QWidget();
    searchWidget->setStyleSheet("background-color: #1e293b;");
    searchWidget->setFixedHeight(70);

    QHBoxLayout *searchLayout = new QHBoxLayout(searchWidget);
    m_searchBox = new QLineEdit();
    m_searchBox->setPlaceholderText("Поиск...");
    m_searchBox->setStyleSheet(
        "QLineEdit { background-color: #334155; border: 1px solid #475569; border-radius: 20px; padding: 10px 15px; color: #f1f5f9; }"
        );
    searchLayout->addWidget(m_searchBox);

    mainLayout->addWidget(searchWidget);

    // Контейнер для списка
    m_listContainer = new QWidget();
    m_listContainer->setStyleSheet("background-color: #0f172a;");
    m_listLayout = new QVBoxLayout(m_listContainer);
    m_listLayout->setContentsMargins(5, 5, 5, 5);
    m_listLayout->setSpacing(5);

    //createDummyChats();
    m_listLayout->addStretch();

    mainLayout->addWidget(m_listContainer);
}

void MiddlePanel::createDummyChats()
{
    // Временные элементы для демонстрации (заглушки)
    for (int i = 1; i <= 5; ++i) {
        QPushButton *item = new QPushButton(QString("Элемент списка %1").arg(i));
        item->setStyleSheet(
            "QPushButton { background-color: #1e293b; border: none; border-radius: 8px; padding: 15px; color: #f1f5f9; text-align: left; margin: 5px; }"
            "QPushButton:hover { background-color: #334155; }"
            );
        m_listLayout->addWidget(item);
        connect(item, &QPushButton::clicked, [this, i]() {
            emit chatSelected(QString::number(i));
        });
    }
}

void MiddlePanel::setContacts(const QList<QVariantMap> &contacts)
{
    qDebug() << "MiddlePanel::setContacts called with" << contacts.size() << "contacts";

    // Очищаем текущий список
    QLayoutItem *item;
    while ((item = m_listLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }

    // Если контактов нет, показываем заглушку
    if (contacts.isEmpty()) {
        QLabel *emptyLabel = new QLabel("Нет контактов");
        emptyLabel->setStyleSheet("color: #64748b; font-size: 14px; padding: 20px;");
        emptyLabel->setAlignment(Qt::AlignCenter);
        m_listLayout->addWidget(emptyLabel);
        m_listLayout->addStretch();
        return;
    }

    // Добавляем контакты
    for (const QVariantMap &contact : contacts) {
        QString displayName = contact["full_name"].toString();
        if (displayName.isEmpty()) {
            displayName = contact["username"].toString();
        }

        // Определяем статус
        int status = contact["status"].toInt();
        QString statusText;
        QString statusColor;
        switch (status) {
        case 0: statusText = "🟢"; statusColor = "#22c55e"; break;
        case 1: statusText = "🟡"; statusColor = "#f59e0b"; break;
        case 2: statusText = "🔴"; statusColor = "#ef4444"; break;
        default: statusText = "⚫"; statusColor = "#64748b"; break;
        }

        // Создаем виджет контакта
        QWidget *contactWidget = new QWidget();
        contactWidget->setStyleSheet("background-color: transparent;");

        QHBoxLayout *contactLayout = new QHBoxLayout(contactWidget);
        contactLayout->setContentsMargins(10, 8, 10, 8);
        contactLayout->setSpacing(10);

        // Статус
        QLabel *statusLabel = new QLabel(statusText);
        statusLabel->setStyleSheet(QString("color: %1; font-size: 10px;").arg(statusColor));
        statusLabel->setFixedWidth(16);

        // Имя
        QLabel *nameLabel = new QLabel(displayName);
        nameLabel->setStyleSheet("color: #f1f5f9; font-size: 14px; font-weight: 500;");

        contactLayout->addWidget(statusLabel);
        contactLayout->addWidget(nameLabel, 1);

        // Делаем виджет кликабельным
        contactWidget->setCursor(Qt::PointingHandCursor);
        contactWidget->installEventFilter(this);
        contactWidget->setProperty("contactId", contact["id"].toLongLong());

        m_listLayout->addWidget(contactWidget);
    }

    m_listLayout->addStretch();
}

bool MiddlePanel::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QWidget *widget = qobject_cast<QWidget*>(obj);
        if (widget && widget->property("contactId").isValid()) {
            qint64 contactId = widget->property("contactId").toLongLong();
            emit contactSelected(contactId);
            return true;  // ← ВАЖНО: возвращаем true, чтобы событие не шло дальше
        }
    }
    return QWidget::eventFilter(obj, event);
}
