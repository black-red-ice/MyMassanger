#include "DealsPanel.h"
#include "DealDialog.h"
#include "mainwindow.h"
#include "../../dialogs/ClientDetailDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QUuid>
#include <QEvent>
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>
#include <QTimer>
#include <QPushButton>
#include <QApplication>
#include <QJsonArray>

DealsPanel::DealsPanel(QWidget *parent)
    : SidePanel(parent,
                "Клиенты и сделки",
                ":/icons/darkTheme/images/darkTheme/user-tie-light.svg",
                "#8146f0",
                "Добавить клиента",
                400,
                "#8146f0",
                ":/icons/general/images/general/plus.svg")  // ← иконка кнопки
{
    getSearchInput()->setPlaceholderText("Поиск клиентов...");
    connect(getSearchInput(), &QLineEdit::textChanged, this, [this](const QString &text) { filterDeals(text); });

    // Убираем стандартный stretch
    QLayoutItem *stretch = getContentLayout()->takeAt(getContentLayout()->count() - 1);
    delete stretch;

    m_dealsLayout = new QVBoxLayout();
    m_dealsLayout->setSpacing(12);
    getContentLayout()->addLayout(m_dealsLayout);
    getContentLayout()->addStretch();

    render();
}

void DealsPanel::onAddClicked()
{
    QWidget *dimWidget = new QWidget(this->window());
    dimWidget->setObjectName("dealDim");
    dimWidget->setStyleSheet("#dealDim { background-color: rgba(0, 0, 0, 180); }");
    dimWidget->setGeometry(this->window()->rect());
    dimWidget->raise();
    dimWidget->show();

    DealDialog dialog(dimWidget);
    if (dialog.exec() == QDialog::Accepted) {
        Deal d = dialog.getDeal();
        d.id = QUuid::createUuid().toString();
        m_deals.append(d);
        render();

        // Сохраняем на сервер
        MainWindow *mw = qobject_cast<MainWindow*>(this->window());
        if (mw) {
            mw->saveClientsToServer();
        }
    }
    dimWidget->deleteLater();
}

void DealsPanel::render()
{
    while (m_dealsLayout->count() > 0) {
        QLayoutItem *item = m_dealsLayout->takeAt(0);
        if (item->widget()) delete item->widget();
        delete item;
    }
    for (int i = 0; i < m_deals.size(); ++i) {
        QWidget *card = createDealCard(m_deals[i], i);
        m_dealsLayout->addWidget(card);
    }
}

bool DealsPanel::eventFilter(QObject *obj, QEvent *event)
{
    QWidget *widget = qobject_cast<QWidget*>(obj);

    if (widget && widget->objectName() == "dealCard") {
        if (event->type() == QEvent::Enter) {
            widget->setStyleSheet(
                "QWidget#dealCard {"
                "  background: #1e293b;"
                "  border-radius: 12px;"
                "  border: 1px solid #1d4ed8;"    // ✅ синий при наведении
                "}"
                );

            // Анимация приподнятия через pos
            QPropertyAnimation *anim = new QPropertyAnimation(widget, "pos");
            anim->setDuration(150);
            anim->setStartValue(widget->pos());
            anim->setEndValue(widget->pos() - QPoint(0, 4));
            anim->setEasingCurve(QEasingCurve::OutCubic);
            anim->start(QAbstractAnimation::DeleteWhenStopped);

            // Тень
            widget->setGraphicsEffect(nullptr);
            QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect();
            shadow->setBlurRadius(12);
            shadow->setColor(QColor(29, 78, 216, 80));
            shadow->setOffset(0, 6);
            widget->setGraphicsEffect(shadow);

            return true;  // ✅ поглощаем событие
        }
        else if (event->type() == QEvent::Leave) {
            widget->setStyleSheet(
                "QWidget#dealCard {"
                "  background: #1e293b;"
                "  border-radius: 12px;"
                "  border: 1px solid #1e315a;"    // ✅ тёмно-синий
                "}"
                );

            // Анимация возврата
            QPropertyAnimation *anim = new QPropertyAnimation(widget, "pos");
            anim->setDuration(150);
            anim->setStartValue(widget->pos());
            anim->setEndValue(widget->pos() + QPoint(0, 4));
            anim->setEasingCurve(QEasingCurve::OutCubic);
            anim->start(QAbstractAnimation::DeleteWhenStopped);

            widget->setGraphicsEffect(nullptr);

            return true;
        }
        else if (event->type() == QEvent::MouseButtonDblClick) {
            // Двойной клик — открываем чат
            if (m_clickTimer) {
                m_clickTimer->stop();
            }
            int idx = widget->property("dealIndex").toInt();
            if (idx >= 0 && idx < m_deals.size()) {
                const Deal &deal = m_deals[idx];
                emit openChatWithClient(deal.name, deal.phone);
                return true;
            }
        }
        else if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                int idx = widget->property("dealIndex").toInt();
                if (idx >= 0 && idx < m_deals.size()) {
                    // Запускаем таймер — если за 250ms не будет второго клика, откроем карточку
                    if (!m_clickTimer) {
                        m_clickTimer = new QTimer(this);
                        m_clickTimer->setSingleShot(true);
                        connect(m_clickTimer, &QTimer::timeout, this, [this]() {
                            if (m_pendingClickIndex >= 0) {
                                onCardClicked(m_pendingClickIndex);
                                m_pendingClickIndex = -1;
                            }
                        });
                    }
                    m_pendingClickIndex = idx;
                    m_clickTimer->start(250);
                    return true;
                }
            }
        }
    }

    return QWidget::eventFilter(obj, event);
}

void DealsPanel::onCardClicked(int index)
{
    qDebug() << "=== onCardClicked called with index:" << index;

    if (index < 0 || index >= m_deals.size()) {
        qDebug() << "Invalid index, returning";
        return;
    }

    qDebug() << "Creating dim widget";
    QWidget *dim = new QWidget(this->window());
    dim->setStyleSheet("background-color: rgba(0, 0, 0, 180);");
    dim->setGeometry(this->window()->rect());
    dim->show();

    bool reopenDetail = true;
    qDebug() << "Starting detail loop";

    while (reopenDetail) {
        reopenDetail = false;
        qDebug() << "Creating ClientDetailDialog";

        ClientDetailDialog detailDialog(m_deals[index], dim);
        qDebug() << "ClientDetailDialog created, calling exec()";

        detailDialog.exec();
        qDebug() << "ClientDetailDialog exec() returned, editWasRequested:" << detailDialog.editWasRequested();

        if (detailDialog.editWasRequested()) {
            DealDialog editDialog(dim);
            editDialog.setDeal(m_deals[index]);

            if (editDialog.exec() == QDialog::Accepted) {
                Deal updated = editDialog.getDeal();
                updated.id = m_deals[index].id;
                m_deals[index] = updated;
                render();

                // Сохраняем на сервер
                MainWindow *mw = qobject_cast<MainWindow*>(this->window());
                if (mw) {
                    mw->saveClientsToServer();
                }
            }

            reopenDetail = editDialog.shouldReopenDetail();
        }
    }

    qDebug() << "Closing dim widget";
    dim->close();
    dim->deleteLater();
    qDebug() << "onCardClicked finished";
}

void DealsPanel::filterDeals(const QString &query)
{
    while (m_dealsLayout->count() > 0) {
        QLayoutItem *item = m_dealsLayout->takeAt(0);
        if (item->widget()) delete item->widget();
        delete item;
    }
    for (int i = 0; i < m_deals.size(); ++i) {
        const Deal &deal = m_deals[i];
        if (query.isEmpty() ||
            deal.name.contains(query, Qt::CaseInsensitive) ||
            deal.phone.contains(query, Qt::CaseInsensitive) ||
            deal.inn.contains(query, Qt::CaseInsensitive)) {
            QWidget *card = createDealCard(deal, i);
            m_dealsLayout->addWidget(card);
        }
    }
}

void DealsPanel::onStageChanged(int dealIndex, DealStage stage)
{
    if (dealIndex >= 0 && dealIndex < m_deals.size()) {
        m_deals[dealIndex].stage = stage;

        // Обновляем визуал
        QLayoutItem *item = m_dealsLayout->itemAt(dealIndex);
        if (item && item->widget()) {
            QWidget *card = item->widget();
            QList<QPushButton*> buttons = card->findChildren<QPushButton*>();
            for (QPushButton *btn : buttons) {
                QString text = btn->text();
                DealStage btnStage;
                if (text.contains("Связаться")) btnStage = DealStage::Contact;
                else if (text.contains("Встреча")) btnStage = DealStage::Meeting;
                else if (text.contains("Подписание")) btnStage = DealStage::Contract;
                else if (text.contains("Продажа")) btnStage = DealStage::Won;
                else if (text.contains("Отказ")) btnStage = DealStage::Lost;
                else continue;
                bool isActive = (stage == btnStage);
                btn->setStyleSheet(QString(
                                       "QPushButton {"
                                       "  background: %1; border: none; border-radius: 8px;"
                                       "  color: %2; font-size: 11px; font-weight: %3; padding: 0 8px;"
                                       "}"
                                       "QPushButton:hover { background: %4; }"
                                       ).arg(isActive ? "#1d4ed8" : "#334155",
                                            isActive ? "white" : "#94A3B8",
                                            isActive ? "600" : "400",
                                            isActive ? "#1e40af" : "#475569"));
            }
        }
        MainWindow *mw = qobject_cast<MainWindow*>(this->window());
        if (mw) {
            mw->saveClientsToServer();
        }
    }
}

QWidget* DealsPanel::createDealCard(const Deal &deal, int index)
{
    QWidget *card = new QWidget();
    card->setStyleSheet(
        "QWidget#dealCard { background: #1e293b; border-radius: 12px; border: 1px solid #1e315a; }"
        );
    card->setProperty("dealIndex", index);
    card->setObjectName("dealCard");
    card->setMinimumWidth(0);
    card->setMaximumWidth(10000);
    card->setCursor(Qt::PointingHandCursor);
    card->installEventFilter(this);

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(16, 14, 16, 14);
    cardLayout->setSpacing(8);

    QLabel *nameLabel = new QLabel(deal.name);
    nameLabel->setStyleSheet("color: white; font-size: 15px; font-weight: 600; background: transparent;");
    nameLabel->setWordWrap(true);
    cardLayout->addWidget(nameLabel);

    QHBoxLayout *detailsRow = new QHBoxLayout();
    detailsRow->setSpacing(16);
    detailsRow->setContentsMargins(0, 0, 0, 0);
    QLabel *phoneLabel = new QLabel("📞 " + deal.phone);
    phoneLabel->setStyleSheet("color: #94A3B8; font-size: 12px; background: transparent;");
    detailsRow->addWidget(phoneLabel);
    if (!deal.email.isEmpty()) {
        QLabel *emailLabel = new QLabel("📧 " + deal.email);
        emailLabel->setStyleSheet("color: #94A3B8; font-size: 12px; background: transparent;");
        detailsRow->addWidget(emailLabel);
    }
    detailsRow->addStretch();
    cardLayout->addLayout(detailsRow);

    QHBoxLayout *detailsRow2 = new QHBoxLayout();
    detailsRow2->setSpacing(16);
    detailsRow2->setContentsMargins(0, 0, 0, 0);
    QLabel *innLabel = new QLabel("ИНН: " + deal.inn);
    innLabel->setStyleSheet("color: #94A3B8; font-size: 12px; background: transparent;");
    detailsRow2->addWidget(innLabel);
    if (!deal.extra.isEmpty()) {
        QLabel *extraLabel = new QLabel(deal.extra);
        extraLabel->setStyleSheet("color: #94A3B8; font-size: 12px; background: transparent;");
        detailsRow2->addWidget(extraLabel);
    }
    detailsRow2->addStretch();
    cardLayout->addLayout(detailsRow2);

    QVBoxLayout *stagesContainer = new QVBoxLayout();
    stagesContainer->setSpacing(6);
    stagesContainer->setContentsMargins(0, 4, 0, 0);

    struct StageInfo { DealStage stage; QString icon; QString label; };
    QList<StageInfo> stages = {
        { DealStage::Contact, "📞", "Связаться" },
        { DealStage::Meeting, "🤝", "Встреча" },
        { DealStage::Contract, "✍️", "Подписание" },
        { DealStage::Won, "💰", "Продажа" },
        { DealStage::Lost, "❌", "Отказ" }
    };

    QHBoxLayout *stagesRow1 = new QHBoxLayout();
    stagesRow1->setSpacing(6);
    stagesRow1->setContentsMargins(0, 0, 0, 0);
    for (int i = 0; i < 3; ++i) {
        const auto &si = stages[i];
        QPushButton *btn = new QPushButton(si.icon + " " + si.label);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFixedHeight(32);
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        bool isActive = (deal.stage == si.stage);
        btn->setStyleSheet(QString(
                               "QPushButton { background: %1; border: none; border-radius: 8px;"
                               " color: %2; font-size: 11px; font-weight: %3; padding: 0 8px; }"
                               "QPushButton:hover { background: %4; }"
                               ).arg(isActive ? "#1d4ed8" : "#334155",
                                    isActive ? "white" : "#94A3B8",
                                    isActive ? "600" : "400",
                                    isActive ? "#1e40af" : "#475569"));
        int dlIndex = index;
        DealStage st = si.stage;
        connect(btn, &QPushButton::clicked, this, [this, dlIndex, st]() { onStageChanged(dlIndex, st); });
        stagesRow1->addWidget(btn, 1);
    }
    stagesContainer->addLayout(stagesRow1);

    QHBoxLayout *stagesRow2 = new QHBoxLayout();
    stagesRow2->setSpacing(6);
    stagesRow2->setContentsMargins(0, 0, 0, 0);
    for (int i = 3; i < 5; ++i) {
        const auto &si = stages[i];
        QPushButton *btn = new QPushButton(si.icon + " " + si.label);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFixedHeight(32);
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        bool isActive = (deal.stage == si.stage);
        btn->setStyleSheet(QString(
                               "QPushButton { background: %1; border: none; border-radius: 8px;"
                               " color: %2; font-size: 11px; font-weight: %3; padding: 0 8px; }"
                               "QPushButton:hover { background: %4; }"
                               ).arg(isActive ? "#1d4ed8" : "#334155",
                                    isActive ? "white" : "#94A3B8",
                                    isActive ? "600" : "400",
                                    isActive ? "#1e40af" : "#475569"));
        int dlIndex = index;
        DealStage st = si.stage;
        connect(btn, &QPushButton::clicked, this, [this, dlIndex, st]() { onStageChanged(dlIndex, st); });
        stagesRow2->addWidget(btn, 1);
    }
    stagesContainer->addLayout(stagesRow2);
    cardLayout->addLayout(stagesContainer);

    return card;
}

QJsonArray DealsPanel::getDealsAsJson() const
{
    QJsonArray arr;
    for (const Deal &deal : m_deals) {
        arr.append(deal.toJson());
    }
    return arr;
}

void DealsPanel::setDealsFromJson(const QJsonArray &arr)
{
    m_deals.clear();
    for (const QJsonValue &v : arr) {
        m_deals.append(Deal::fromJson(v.toObject()));
    }
    render();
}
