#include "CompanyDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QFrame>
#include <QKeyEvent>

CompanyDialog::CompanyDialog(QWidget *parent)
    : OverlayDialog(parent)
{
    setFixedSize(520, 520);
    setStyleSheet("background: transparent;");

    // Контейнер
    QWidget *container = new QWidget(this);
    container->setObjectName("companyDialogContainer");
    container->setStyleSheet(
        "#companyDialogContainer {"
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
        "background: #3B82F6;"
        "border-top-left-radius: 18px;"
        "border-top-right-radius: 18px;"
        );
    header->setFixedHeight(60);
    QHBoxLayout *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(24, 0, 24, 0);

    m_titleLabel = new QLabel("Добавить компанию");
    m_titleLabel->setStyleSheet("color: white; font-weight: 600; font-size: 18px; background: transparent;");

    QPushButton *closeBtn = new QPushButton("✕");
    closeBtn->setFixedSize(36, 36);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(
        "QPushButton { background: rgba(255,255,255,0.1); border: none; border-radius: 18px; color: white; font-size: 16px; }"
        "QPushButton:hover { background: rgba(255,255,255,0.2); }"
        );
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);

    headerLayout->addWidget(m_titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(closeBtn);
    containerLayout->addWidget(header);

    // Контент
    QWidget *content = new QWidget(container);
    content->setStyleSheet(
        "background: #1e293b;"
        "border-bottom-left-radius: 18px;"
        "border-bottom-right-radius: 18px;"
        );
    QVBoxLayout *contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(24, 20, 24, 20);
    contentLayout->setSpacing(0);

    // Название *
    QLabel *nameLabel = new QLabel("Название *");
    nameLabel->setStyleSheet("color: #f8fafc; font-size: 14px; font-weight: 600; background: transparent; margin-bottom: 0px;");
    contentLayout->addWidget(nameLabel);

    m_name = new QLineEdit();
    m_name->setPlaceholderText("Введите название компании...");
    m_name->setStyleSheet(
        "QLineEdit { background: #0f172a; border: 1px solid #334155; border-radius: 10px; color: #f1f5f9; padding: 12px 16px; font-size: 14px; }"
        "QLineEdit:focus { border-color: #3B82F6; }"
        );
    m_name->setFixedHeight(44);
    contentLayout->addWidget(m_name);
    contentLayout->addSpacing(10);

    // ИНН + КПП в ряд
    QHBoxLayout *docRow = new QHBoxLayout();
    docRow->setSpacing(16);

    QVBoxLayout *innCol = new QVBoxLayout();
    innCol->setSpacing(0);
    QLabel *innLabel = new QLabel("ИНН");
    innLabel->setStyleSheet("color: #f8fafc; font-size: 14px; font-weight: 600; background: transparent; margin-bottom: 0px;");
    innCol->addWidget(innLabel);
    m_inn = new QLineEdit();
    m_inn->setPlaceholderText("7701234567");
    m_inn->setStyleSheet(
        "QLineEdit { background: #0f172a; border: 1px solid #334155; border-radius: 10px; color: #f1f5f9; padding: 12px 16px; font-size: 14px; }"
        "QLineEdit:focus { border-color: #3B82F6; }"
        );
    m_inn->setFixedHeight(44);
    innCol->addWidget(m_inn);
    docRow->addLayout(innCol, 1);

    QVBoxLayout *kppCol = new QVBoxLayout();
    kppCol->setSpacing(0);
    QLabel *kppLabel = new QLabel("КПП");
    kppLabel->setStyleSheet("color: #f8fafc; font-size: 14px; font-weight: 600; background: transparent; margin-bottom: 0px;");
    kppCol->addWidget(kppLabel);
    m_kpp = new QLineEdit();
    m_kpp->setPlaceholderText("770101001");
    m_kpp->setStyleSheet(
        "QLineEdit { background: #0f172a; border: 1px solid #334155; border-radius: 10px; color: #f1f5f9; padding: 12px 16px; font-size: 14px; }"
        "QLineEdit:focus { border-color: #3B82F6; }"
        );
    m_kpp->setFixedHeight(44);
    kppCol->addWidget(m_kpp);
    docRow->addLayout(kppCol, 1);

    contentLayout->addLayout(docRow);
    contentLayout->addSpacing(10);

    // Юридический адрес
    QLabel *addrLabel = new QLabel("Юридический адрес");
    addrLabel->setStyleSheet("color: #f8fafc; font-size: 14px; font-weight: 600; background: transparent; margin-bottom: 0px;");
    contentLayout->addWidget(addrLabel);

    m_address = new QLineEdit();
    m_address->setPlaceholderText("Введите адрес...");
    m_address->setStyleSheet(
        "QLineEdit { background: #0f172a; border: 1px solid #334155; border-radius: 10px; color: #f1f5f9; padding: 12px 16px; font-size: 14px; }"
        "QLineEdit:focus { border-color: #3B82F6; }"
        );
    m_address->setFixedHeight(44);
    contentLayout->addWidget(m_address);
    contentLayout->addSpacing(10);

    // Контактное лицо (перемещено выше)
    QLabel *contactLabel = new QLabel("Контактное лицо");
    contactLabel->setStyleSheet("color: #f8fafc; font-size: 14px; font-weight: 600; background: transparent; margin-bottom: 0px;");
    contentLayout->addWidget(contactLabel);

    m_contactPerson = new QLineEdit();
    m_contactPerson->setPlaceholderText("ФИО контактного лица...");
    m_contactPerson->setStyleSheet(
        "QLineEdit { background: #0f172a; border: 1px solid #334155; border-radius: 10px; color: #f1f5f9; padding: 12px 16px; font-size: 14px; }"
        "QLineEdit:focus { border-color: #3B82F6; }"
        );
    m_contactPerson->setFixedHeight(44);
    contentLayout->addWidget(m_contactPerson);
    contentLayout->addSpacing(10);

    // Телефон + Email в ряд
    QHBoxLayout *contactRow = new QHBoxLayout();
    contactRow->setSpacing(16);

    QVBoxLayout *phoneCol = new QVBoxLayout();
    phoneCol->setSpacing(0);
    QLabel *phoneLabel = new QLabel("Телефон");
    phoneLabel->setStyleSheet("color: #f8fafc; font-size: 14px; font-weight: 600; background: transparent; margin-bottom: 0px;");
    phoneCol->addWidget(phoneLabel);
    m_phone = new QLineEdit();
    m_phone->setPlaceholderText("+7(999)123-45-67");
    m_phone->setStyleSheet(
        "QLineEdit { background: #0f172a; border: 1px solid #334155; border-radius: 10px; color: #f1f5f9; padding: 12px 16px; font-size: 14px; }"
        "QLineEdit:focus { border-color: #3B82F6; }"
        );
    m_phone->setFixedHeight(44);
    phoneCol->addWidget(m_phone);
    contactRow->addLayout(phoneCol, 1);

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

    QVBoxLayout *emailCol = new QVBoxLayout();
    emailCol->setSpacing(0);
    QLabel *emailLabel = new QLabel("Email");
    emailLabel->setStyleSheet("color: #f8fafc; font-size: 14px; font-weight: 600; background: transparent; margin-bottom: 0px;");
    emailCol->addWidget(emailLabel);
    m_email = new QLineEdit();
    m_email->setPlaceholderText("example@mail.ru");
    m_email->setStyleSheet(
        "QLineEdit { background: #0f172a; border: 1px solid #334155; border-radius: 10px; color: #f1f5f9; padding: 12px 16px; font-size: 14px; }"
        "QLineEdit:focus { border-color: #3B82F6; }"
        );
    m_email->setFixedHeight(44);
    emailCol->addWidget(m_email);
    contactRow->addLayout(emailCol, 1);

    contentLayout->addLayout(contactRow);
    contentLayout->addSpacing(16);

    // Кнопки
    QWidget *buttonRow = new QWidget();
    QHBoxLayout *buttonLayout = new QHBoxLayout(buttonRow);
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(12);

    QPushButton *cancelBtn = new QPushButton("Отмена");
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setFixedHeight(44);
    cancelBtn->setStyleSheet(
        "QPushButton { background: #334155; border: none; border-radius: 10px; color: #cbd5e1; font-size: 14px; font-weight: 500; }"
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
        "QPushButton { background: #3B82F6; border: none; border-radius: 10px; color: white; font-size: 14px; font-weight: 600; }"
        "QPushButton:hover { background: #2563EB; }"
        );
    connect(saveBtn, &QPushButton::clicked, this, [this]() {
        if (m_name->text().trimmed().isEmpty()) {
            m_name->setStyleSheet("...красная рамка...");
            m_name->setFocus();
            return;
        }
        m_reopenDetail = true;
        accept();
    });

    buttonLayout->addWidget(cancelBtn, 1);
    buttonLayout->addWidget(saveBtn, 1);
    contentLayout->addWidget(buttonRow);

    saveBtn->setDefault(true);
    saveBtn->setAutoDefault(true);
    cancelBtn->setAutoDefault(false);

    containerLayout->addWidget(content);
}

Company CompanyDialog::getCompany() const
{
    Company c;
    c.name = m_name->text();
    c.inn = m_inn->text();
    c.kpp = m_kpp->text();
    c.address = m_address->text();
    c.phone = m_phone->text();
    c.email = m_email->text();
    c.contactPerson = m_contactPerson->text();
    return c;
}

void CompanyDialog::setCompany(const Company &company)
{
    qDebug() << "=== setCompany START ===";
    qDebug() << "m_titleLabel:" << m_titleLabel;
    qDebug() << "m_contactPerson:" << m_contactPerson;

    m_titleLabel->setText("Редактирование компании");
    m_name->setText(company.name);
    m_inn->setText(company.inn);
    m_kpp->setText(company.kpp);
    m_address->setText(company.address);
    m_phone->setText(company.phone);
    m_email->setText(company.email);
    m_contactPerson->setText(company.contactPerson);
    qDebug() << "=== setCompany END ===";
}

void CompanyDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        if (m_name->text().trimmed().isEmpty()) {
            m_name->setStyleSheet("...красная рамка...");
            m_name->setFocus();
            return;
        }
        m_reopenDetail = true;
        accept();
        return;
    }
    if (event->key() == Qt::Key_Escape) {
        m_reopenDetail = false;
        reject();
        return;
    }
    QDialog::keyPressEvent(event);
}
