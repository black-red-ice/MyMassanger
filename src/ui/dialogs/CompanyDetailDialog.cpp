#include "CompanyDetailDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QKeyEvent>
#include <QTimer>
#include <QKeyEvent>

CompanyDetailDialog::CompanyDetailDialog(const Company &company, QWidget *parent)
    : OverlayDialog(parent), m_company(company)
{
    setFixedSize(520, 520);
    setStyleSheet("background: transparent;");

    QWidget *container = new QWidget(this);
    container->setObjectName("companyDetailContainer");
    container->setStyleSheet(
        "#companyDetailContainer {"
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

    QLabel *titleLabel = new QLabel("Карточка компании");
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
    content->setStyleSheet(
        "background: #1e293b;"
        "border-bottom-left-radius: 18px;"
        "border-bottom-right-radius: 18px;"
        );
    QVBoxLayout *contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(24, 20, 24, 20);
    contentLayout->setSpacing(0);

    // Название
    QLabel *nameLabel = new QLabel("Название");
    nameLabel->setStyleSheet("color: #94A3B8; font-size: 14px; font-weight: 500; background: transparent;");
    contentLayout->addWidget(nameLabel);
    QLabel *nameValue = new QLabel(m_company.name);
    nameValue->setStyleSheet("color: #f1f5f9; font-size: 14px; font-weight: 600; background: transparent; padding: 8px 0;");
    contentLayout->addWidget(nameValue);
    contentLayout->addSpacing(16);

    // ИНН + КПП в ряд
    QHBoxLayout *docRow = new QHBoxLayout();
    docRow->setSpacing(16);

    QVBoxLayout *innCol = new QVBoxLayout();
    innCol->setSpacing(4);
    QLabel *innLabel = new QLabel("ИНН");
    innLabel->setStyleSheet("color: #94A3B8; font-size: 14px; font-weight: 500; background: transparent;");
    innCol->addWidget(innLabel);
    QLabel *innValue = new QLabel(m_company.inn);
    innValue->setStyleSheet("color: #f1f5f9; font-size: 14px; font-weight: 600; background: transparent; padding: 8px 0;");
    innCol->addWidget(innValue);
    docRow->addLayout(innCol, 1);

    QVBoxLayout *kppCol = new QVBoxLayout();
    kppCol->setSpacing(4);
    QLabel *kppLabel = new QLabel("КПП");
    kppLabel->setStyleSheet("color: #94A3B8; font-size: 14px; font-weight: 500; background: transparent;");
    kppCol->addWidget(kppLabel);
    QLabel *kppValue = new QLabel(m_company.kpp.isEmpty() ? "—" : m_company.kpp);
    kppValue->setStyleSheet("color: #f1f5f9; font-size: 14px; font-weight: 600; background: transparent; padding: 8px 0;");
    kppCol->addWidget(kppValue);
    docRow->addLayout(kppCol, 1);

    contentLayout->addLayout(docRow);
    contentLayout->addSpacing(16);

    // Юридический адрес
    QLabel *addrLabel = new QLabel("Юридический адрес");
    addrLabel->setStyleSheet("color: #94A3B8; font-size: 14px; font-weight: 500; background: transparent;");
    contentLayout->addWidget(addrLabel);
    QLabel *addrValue = new QLabel(m_company.address.isEmpty() ? "—" : m_company.address);
    addrValue->setStyleSheet("color: #f1f5f9; font-size: 14px; font-weight: 600; background: transparent; padding: 8px 0;");
    addrValue->setWordWrap(true);
    contentLayout->addWidget(addrValue);
    contentLayout->addSpacing(16);

    // Контактное лицо
    QLabel *contactLabel = new QLabel("Контактное лицо");
    contactLabel->setStyleSheet("color: #94A3B8; font-size: 14px; font-weight: 500; background: transparent;");
    contentLayout->addWidget(contactLabel);
    QLabel *contactValue = new QLabel(m_company.contactPerson.isEmpty() ? "—" : m_company.contactPerson);
    contactValue->setStyleSheet("color: #f1f5f9; font-size: 14px; font-weight: 600; background: transparent; padding: 8px 0;");
    contentLayout->addWidget(contactValue);
    contentLayout->addSpacing(16);

    // Телефон + Email в ряд
    QHBoxLayout *contactRow = new QHBoxLayout();
    contactRow->setSpacing(16);

    QVBoxLayout *phoneCol = new QVBoxLayout();
    phoneCol->setSpacing(4);
    QLabel *phoneLabel = new QLabel("Телефон");
    phoneLabel->setStyleSheet("color: #94A3B8; font-size: 14px; font-weight: 500; background: transparent;");
    phoneCol->addWidget(phoneLabel);
    QLabel *phoneValue = new QLabel(m_company.phone.isEmpty() ? "—" : m_company.phone);
    phoneValue->setStyleSheet("color: #f1f5f9; font-size: 14px; font-weight: 600; background: transparent; padding: 8px 0;");
    phoneCol->addWidget(phoneValue);
    contactRow->addLayout(phoneCol, 1);

    QVBoxLayout *emailCol = new QVBoxLayout();
    emailCol->setSpacing(4);
    QLabel *emailLabel = new QLabel("Email");
    emailLabel->setStyleSheet("color: #94A3B8; font-size: 14px; font-weight: 500; background: transparent;");
    emailCol->addWidget(emailLabel);
    QLabel *emailValue = new QLabel(m_company.email.isEmpty() ? "—" : m_company.email);
    emailValue->setStyleSheet("color: #f1f5f9; font-size: 14px; font-weight: 600; background: transparent; padding: 8px 0;");
    emailCol->addWidget(emailValue);
    contactRow->addLayout(emailCol, 1);

    contentLayout->addLayout(contactRow);
    contentLayout->addSpacing(16);

    contentLayout->addStretch();

    // Кнопки внизу
    QWidget *buttonRow = new QWidget();
    QHBoxLayout *buttonLayout = new QHBoxLayout(buttonRow);
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(12);

    QPushButton *closeButton = new QPushButton("Закрыть");
    closeButton->setCursor(Qt::PointingHandCursor);
    closeButton->setFixedHeight(44);
    closeButton->setStyleSheet(
        "QPushButton { background: #334155; border: none; border-radius: 10px; color: #cbd5e1; font-size: 14px; font-weight: 500; }"
        "QPushButton:hover { background: #475569; }"
        );
    closeButton->setAutoDefault(false);
    closeButton->setDefault(false);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::reject);

    QPushButton *editButton = new QPushButton("Редактировать");
    editButton->setCursor(Qt::PointingHandCursor);
    editButton->setFixedHeight(44);
    editButton->setStyleSheet(
        "QPushButton { background: #0EA5E9; border: none; border-radius: 10px; color: white; font-size: 14px; font-weight: 600; }"
        "QPushButton:hover { background: #1d4ed8; }"
        );
    editButton->setAutoDefault(false);
    editButton->setDefault(false);
    connect(editButton, &QPushButton::clicked, this, [this]() {
        m_editRequested = true;
        reject();
    });

    buttonLayout->addWidget(closeButton, 1);
    buttonLayout->addWidget(editButton, 1);
    contentLayout->addWidget(buttonRow);

    QTimer::singleShot(0, this, [this]() {
        QList<QPushButton*> buttons = findChildren<QPushButton*>();
        for (QPushButton* btn : buttons) {
            btn->setAutoDefault(false);
            btn->setDefault(false);
            btn->setFocusPolicy(Qt::NoFocus);
        }
        setFocusPolicy(Qt::NoFocus);
        if (!children().isEmpty()) {
            QWidget* firstChild = qobject_cast<QWidget*>(children().first());
            if (firstChild) firstChild->setFocus();
        }
    });

    containerLayout->addWidget(content);
}

void CompanyDetailDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        return;  // Игнорируем Enter
    }
    if (event->key() == Qt::Key_Escape) {
        reject();
        return;
    }
    QDialog::keyPressEvent(event);
}
