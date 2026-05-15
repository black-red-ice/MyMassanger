#include "DealDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QFrame>
#include <QSpinBox>
#include <QKeyEvent>
#include <QCloseEvent>

DealDialog::DealDialog(QWidget *parent)
    : OverlayDialog(parent)
{
    // Центрируем окно относительно родителя (экрана)
    if (parent) {
        QRect parentRect = parent->geometry();
        int x = parentRect.center().x() - width() / 2;
        int y = parentRect.center().y() - height() / 2;
        move(x, y);
    }

    setFixedSize(520, 640);
    setStyleSheet("background: transparent;");

    // Контейнер
    QWidget *container = new QWidget(this);
    container->setObjectName("dealDialogContainer");
    container->setStyleSheet(
        "#dealDialogContainer {"
        "  background: #1e293b;"
        "  border-radius: 18px;"
        "  border: 1px solid #334155;"
        "}"
        );

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(container);

    QVBoxLayout *containerLayout = new QVBoxLayout(container);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setSpacing(0);

    // Заголовок
    QWidget *header = new QWidget(container);
    header->setStyleSheet(
        "background: #8146f0;"
        "border-top-left-radius: 18px;"
        "border-top-right-radius: 18px;"
        );
    header->setFixedHeight(60);
    QHBoxLayout *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(24, 0, 24, 0);

    QLabel *titleLabel = new QLabel("Новый клиент");
    titleLabel->setStyleSheet("color: white; font-weight: 600; font-size: 18px; background: transparent;");

    QPushButton *closeBtn = new QPushButton("✕");
    closeBtn->setFixedSize(36, 36);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(
        "QPushButton { background: rgba(255,255,255,0.1); border: none; border-radius: 18px; color: white; font-size: 16px; }"
        "QPushButton:hover { background: rgba(255,255,255,0.2); }"
        );
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);

    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(closeBtn);
    containerLayout->addWidget(header);

    // Контент
    QWidget *content = new QWidget(container);
    containerLayout->addWidget(content, 1);
    content->setStyleSheet(
        "background: #1e293b;"
        "border-bottom-left-radius: 18px;"   // ✅ закругление снизу
        "border-bottom-right-radius: 18px;"  // ✅ закругление снизу
        );
    QVBoxLayout *contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(24, 24, 24, 24);
    contentLayout->setSpacing(16);

    // ФИО / Компания *
    QLabel *nameLabel = new QLabel("ФИО / Компания *");
    nameLabel->setStyleSheet("color: #f8fafc; font-size: 14px; font-weight: 600; background: transparent;");
    contentLayout->addWidget(nameLabel);

    m_name = new QLineEdit();
    m_name->setPlaceholderText("Введите название...");
    m_name->setStyleSheet(
        "QLineEdit {"
        "  background: #0f172a;"
        "  border: 1px solid #334155;"
        "  border-radius: 10px;"
        "  color: #f1f5f9;"
        "  padding: 12px 16px;"
        "  font-size: 14px;"
        "}"
        "QLineEdit:focus { border-color: #1d4ed8; }"
        );
    m_name->setFixedHeight(44);
    contentLayout->addWidget(m_name);

    // Телефон + Email в ряд
    QWidget *contactRow = new QWidget();
    QHBoxLayout *contactLayout = new QHBoxLayout(contactRow);
    contactLayout->setContentsMargins(0, 0, 0, 0);
    contactLayout->setSpacing(16);

    QWidget *phoneCol = new QWidget();
    QVBoxLayout *phoneLayout = new QVBoxLayout(phoneCol);
    phoneLayout->setContentsMargins(0, 0, 0, 0);
    phoneLayout->setSpacing(6);
    QLabel *phoneLabel = new QLabel("Телефон");
    phoneLabel->setStyleSheet("color: #f8fafc; font-size: 14px; font-weight: 600; background: transparent;");
    phoneLayout->addWidget(phoneLabel);
    m_phone = new QLineEdit();
    m_phone->setPlaceholderText("+7 (999) 123-45-67");
    m_phone->setStyleSheet(
        "QLineEdit { background: #0f172a; border: 1px solid #334155; border-radius: 10px; color: #f1f5f9; padding: 12px 16px; font-size: 14px; }"
        "QLineEdit:focus { border-color: #1d4ed8; }"
        );
    m_phone->setFixedHeight(44);
    phoneLayout->addWidget(m_phone);
    contactLayout->addWidget(phoneCol, 1);

    // Форматирование телефона
    connect(m_phone, &QLineEdit::textChanged, this, [this](const QString &text) {
        m_phone->blockSignals(true);

        QString digits;
        for (const QChar &ch : text) {
            if (ch.isDigit() || ch == '+') {
                digits += ch;
            }
        }

        QString formatted;
        if (digits.startsWith("+7") || digits.startsWith("7") || digits.startsWith("8")) {
            if (digits.startsWith("8")) digits = "+7" + digits.mid(1);
            if (digits.startsWith("7")) digits = "+7" + digits.mid(1);

            formatted = digits.left(2);
            if (digits.length() > 2) formatted += " (" + digits.mid(2, 3);
            if (digits.length() > 5) formatted += ") " + digits.mid(5, 3);
            if (digits.length() > 8) formatted += "-" + digits.mid(8, 2);
            if (digits.length() > 10) formatted += "-" + digits.mid(10, 2);
        } else {
            formatted = digits;
        }

        m_phone->setText(formatted);
        m_phone->setCursorPosition(formatted.length());
        m_phone->blockSignals(false);
    });

    QWidget *emailCol = new QWidget();
    QVBoxLayout *emailLayout = new QVBoxLayout(emailCol);
    emailLayout->setContentsMargins(0, 0, 0, 0);
    emailLayout->setSpacing(6);
    QLabel *emailLabel = new QLabel("Email");
    emailLabel->setStyleSheet("color: #f8fafc; font-size: 14px; font-weight: 600; background: transparent;");
    emailLayout->addWidget(emailLabel);
    m_email = new QLineEdit();
    m_email->setPlaceholderText("example@mail.ru");
    m_email->setStyleSheet(
        "QLineEdit { background: #0f172a; border: 1px solid #334155; border-radius: 10px; color: #f1f5f9; padding: 12px 16px; font-size: 14px; }"
        "QLineEdit:focus { border-color: #1d4ed8; }"
        );
    m_email->setFixedHeight(44);
    emailLayout->addWidget(m_email);
    contactLayout->addWidget(emailCol, 1);

    contentLayout->addWidget(contactRow);

    // ИНН + Отсрочка в ряд
    QWidget *docRow = new QWidget();
    QHBoxLayout *docLayout = new QHBoxLayout(docRow);
    docLayout->setContentsMargins(0, 0, 0, 0);
    docLayout->setSpacing(16);

    QWidget *innCol = new QWidget();
    QVBoxLayout *innLayout = new QVBoxLayout(innCol);
    innLayout->setContentsMargins(0, 0, 0, 0);
    innLayout->setSpacing(6);
    QLabel *innLabel = new QLabel("ИНН");
    innLabel->setStyleSheet("color: #f8fafc; font-size: 14px; font-weight: 600; background: transparent;");
    innLayout->addWidget(innLabel);
    m_inn = new QLineEdit();
    m_inn->setPlaceholderText("7701234567");
    m_inn->setStyleSheet(
        "QLineEdit { background: #0f172a; border: 1px solid #334155; border-radius: 10px; color: #f1f5f9; padding: 12px 16px; font-size: 14px; }"
        "QLineEdit:focus { border-color: #1d4ed8; }"
        );
    m_inn->setFixedHeight(44);
    innLayout->addWidget(m_inn);
    docLayout->addWidget(innCol, 1);

    QWidget *delayCol = new QWidget();
    QVBoxLayout *delayLayout = new QVBoxLayout(delayCol);
    delayLayout->setContentsMargins(0, 0, 0, 0);
    delayLayout->setSpacing(6);
    QLabel *delayLabel = new QLabel("Отсрочка (дней)");
    delayLabel->setStyleSheet("color: #f8fafc; font-size: 14px; font-weight: 600; background: transparent;");
    delayLayout->addWidget(delayLabel);
    m_delayDays = new QSpinBox();
    m_delayDays->setValue(0);
    m_delayDays->setStyleSheet(
        "QSpinBox {"
        "  background: #0f172a;"
        "  border: 1px solid #334155;"
        "  border-radius: 10px;"
        "  color: #f1f5f9;"
        "  padding: 12px 16px;"
        "  font-size: 14px;"
        "}"
        "QSpinBox:focus { border-color: #1d4ed8; }"
        "QSpinBox::up-button {"
        "  subcontrol-origin: border;"
        "  subcontrol-position: top right;"
        "  width: 24px;"
        "  background: #334155;"
        "  border-top-right-radius: 10px;"
        "  border-left: 1px solid #1e293b;"
        "}"
        "QSpinBox::down-button {"
        "  subcontrol-origin: border;"
        "  subcontrol-position: bottom right;"
        "  width: 24px;"
        "  background: #334155;"
        "  border-bottom-right-radius: 10px;"
        "  border-left: 1px solid #1e293b;"
        "}"
        "QSpinBox::up-arrow, QSpinBox::down-arrow { width: 8px; height: 8px; }"
        );
    m_delayDays->setFixedHeight(44);
    delayLayout->addWidget(m_delayDays);
    docLayout->addWidget(delayCol, 1);

    contentLayout->addWidget(docRow);

    // Гараж / Доп. поле
    QLabel *extraLabel = new QLabel("Гараж / Доп. поле");
    extraLabel->setStyleSheet("color: #f8fafc; font-size: 14px; font-weight: 600; background: transparent;");
    contentLayout->addWidget(extraLabel);

    m_extra = new QLineEdit();
    m_extra->setPlaceholderText("Любые доп. данные");
    m_extra->setStyleSheet(
        "QLineEdit { background: #0f172a; border: 1px solid #334155; border-radius: 10px; color: #f1f5f9; padding: 12px 16px; font-size: 14px; }"
        "QLineEdit:focus { border-color: #1d4ed8; }"
        );
    m_extra->setFixedHeight(44);
    contentLayout->addWidget(m_extra);

    // Стадия сделки
    QLabel *stageLabel = new QLabel("Стадия сделки");
    stageLabel->setStyleSheet("color: #f8fafc; font-size: 14px; font-weight: 600; background: transparent;");
    contentLayout->addWidget(stageLabel);

    m_stage = new QComboBox();
    m_stage->addItems({"📞 Связаться", "🤝 Встреча", "✍️ Подписание", "💰 Продажа", "❌ Отказ"});
    m_stage->setStyleSheet(
        "QComboBox {"
        "  background: #0f172a;"
        "  border: 1px solid #334155;"
        "  border-radius: 10px;"
        "  color: #f1f5f9;"
        "  padding: 12px 16px;"
        "  font-size: 14px;"
        "}"
        "QComboBox:focus { border-color: #1d4ed8; }"
        "QComboBox::drop-down { border: none; width: 30px; }"
        "QComboBox QAbstractItemView {"
        "  background: #0f172a;"
        "  color: #f1f5f9;"
        "  border: 1px solid #334155;"
        "  border-radius: 8px;"
        "  selection-background-color: #1d4ed8;"
        "  padding: 4px;"
        "}"
        );
    m_stage->setFixedHeight(44);
    contentLayout->addWidget(m_stage);

    contentLayout->addSpacing(8);

    // Кнопки
    QWidget *buttonRow = new QWidget();
    QHBoxLayout *buttonLayout = new QHBoxLayout(buttonRow);
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(12);

    QPushButton *cancelBtn = new QPushButton("Отмена");
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setFixedHeight(44);
    cancelBtn->setStyleSheet(
        "QPushButton {"
        "  background: #334155;"
        "  border: none;"
        "  border-radius: 10px;"
        "  color: #cbd5e1;"
        "  font-size: 14px;"
        "  font-weight: 500;"
        "}"
        "QPushButton:hover { background: #475569; }"
        );
    connect(cancelBtn, &QPushButton::clicked, this, [this]() {
        m_reopenDetail = true;
        reject();
    });

    QPushButton *saveBtn = new QPushButton("Сохранить");
    saveBtn->setCursor(Qt::PointingHandCursor);
    saveBtn->setFixedHeight(44);
    saveBtn->setStyleSheet(
        "QPushButton {"
        "  background: #1d4ed8;"
        "  border: none;"
        "  border-radius: 10px;"
        "  color: white;"
        "  font-size: 14px;"
        "  font-weight: 600;"
        "}"
        "QPushButton:hover { background: #1e40af; }"
        );
    connect(saveBtn, &QPushButton::clicked, this, [this]() {
        m_reopenDetail = true;
        accept();
    });

    buttonLayout->addWidget(cancelBtn, 1);
    buttonLayout->addWidget(saveBtn, 1);
    contentLayout->addWidget(buttonRow);

    saveBtn->setDefault(true);
    saveBtn->setAutoDefault(true);
    cancelBtn->setAutoDefault(false);
}

Deal DealDialog::getDeal() const
{
    Deal d;
    d.name = m_name->text();
    d.phone = m_phone->text();
    d.email = m_email->text();
    d.inn = m_inn->text();
    d.extra = m_extra->text();
    d.delayDays = m_delayDays->value();

    switch (m_stage->currentIndex()) {
    case 0: d.stage = DealStage::Contact; break;
    case 1: d.stage = DealStage::Meeting; break;
    case 2: d.stage = DealStage::Contract; break;
    case 3: d.stage = DealStage::Won; break;
    case 4: d.stage = DealStage::Lost; break;
    }

    return d;
}

void DealDialog::setDeal(const Deal &deal)
{
    m_name->setText(deal.name);
    m_phone->setText(deal.phone);
    m_email->setText(deal.email);
    m_inn->setText(deal.inn);
    m_extra->setText(deal.extra);
    m_delayDays->setValue(deal.delayDays);
    m_stage->setCurrentIndex(static_cast<int>(deal.stage));
}

void DealDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        m_reopenDetail = true;
        accept();
        return;
    }
    if (event->key() == Qt::Key_Escape) {
        m_reopenDetail = false;
        reject();  // вместо close()
        return;
    }
    QDialog::keyPressEvent(event);
}

bool DealDialog::shouldReopenDetail() const
{
    return m_reopenDetail;
}
