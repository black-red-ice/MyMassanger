#include "EmployeesPanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>

EmployeesPanel::EmployeesPanel(QWidget *parent)
    : SidePanel(parent, "Сотрудники и отделы",
                ":/icons/darkTheme/images/darkTheme/users.svg",
                "#1d4ed8",
                "",
                360,
                "#1d4ed8",
                "")
{
    // Убираем все элементы из contentLayout
    while (getContentLayout()->count() > 0) {
        QLayoutItem *item = getContentLayout()->takeAt(0);
        delete item;
    }

    // Фильтры отделов
    createDepartmentFilters();

    // Список сотрудников
    m_employeesLayout = new QVBoxLayout();
    m_employeesLayout->setSpacing(8);
    getContentLayout()->addLayout(m_employeesLayout);
    getContentLayout()->addStretch();

    // Данные
    m_employees.append({"Иван Петров", "Директор по IT", "IT", true});
    m_employees.append({"Анна Сидорова", "HR менеджер", "HR", true});
    m_employees.append({"Дмитрий Козлов", "Финансовый директор", "Финансы", false});
    m_employees.append({"Мария Волкова", "Руководитель отдела продаж", "Продажи", true});
    m_employees.append({"Сергей Иванов", "Системный администратор", "IT", false});

    getSearchInput()->setPlaceholderText("Поиск сотрудников...");
    getAddButton()->parentWidget()->hide();

    render();
}

void EmployeesPanel::createDepartmentFilters()
{
    m_filterWidget = new QWidget();
    m_filterWidget->setStyleSheet("background: transparent;");
    QHBoxLayout *filterLayout = new QHBoxLayout(m_filterWidget);
    filterLayout->setContentsMargins(0, 4, 0, 8);
    filterLayout->setSpacing(6);

    struct DeptInfo { QString name; QString color; };
    QList<DeptInfo> deps = {
        {"Все", "#94A3B8"},
        {"IT", "#3B82F6"},
        {"Продажи", "#10B981"},
        {"HR", "#8B5CF6"},
        {"Финансы", "#F59E0B"}
    };

    for (const DeptInfo &dep : deps) {
        QPushButton *btn = new QPushButton(dep.name);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFixedHeight(28);
        btn->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);

        QString bg, fg;
        if (dep.name == "Все") {
            bg = "#334155"; fg = "#cbd5e1";
        } else if (dep.name == "IT") {
            bg = "rgba(59,130,246,0.15)"; fg = "#3B82F6";
        } else if (dep.name == "Продажи") {
            bg = "rgba(16,185,129,0.15)"; fg = "#10B981";
        } else if (dep.name == "HR") {
            bg = "rgba(139,92,246,0.15)"; fg = "#8B5CF6";
        } else {
            bg = "rgba(245,158,11,0.15)"; fg = "#F59E0B";
        }

        btn->setStyleSheet(
            QString("QPushButton { background: %1; border: none; border-radius: 14px; "
                    "color: %2; font-size: 12px; font-weight: 500; padding: 0 12px; }"
                    "QPushButton:hover { opacity: 0.85; }").arg(bg, fg)
            );
        connect(btn, &QPushButton::clicked, this, [this, name = dep.name]() { filterByDepartment(name); });
        filterLayout->addWidget(btn);
    }
    filterLayout->addStretch();
    getContentLayout()->addWidget(m_filterWidget);
}

void EmployeesPanel::filterByDepartment(const QString &department)
{
    while (m_employeesLayout->count() > 0) {
        QLayoutItem *item = m_employeesLayout->takeAt(0);
        if (item->widget()) delete item->widget();
        delete item;
    }
    for (const Employee &emp : m_employees) {
        if (department == "Все" || emp.department == department) {
            QWidget *card = createEmployeeCard(emp);
            m_employeesLayout->addWidget(card);
        }
    }
    m_employeesLayout->addStretch();
}

void EmployeesPanel::filterEmployees(const QString &query)
{
    // Реализуй при необходимости
}

QWidget* EmployeesPanel::createEmployeeCard(const Employee &emp)
{
    QWidget *card = new QWidget();
    card->setStyleSheet(
        "QWidget#empCard { background: #1e293b; border-radius: 12px; border: 1px solid #334155; }"
        "QWidget#empCard:hover { background: #263244; }"
        );
    card->setObjectName("empCard");
    card->setCursor(Qt::PointingHandCursor);

    QHBoxLayout *cardLayout = new QHBoxLayout(card);
    cardLayout->setContentsMargins(12, 10, 16, 10);
    cardLayout->setSpacing(12);

    // Аватарка
    QWidget *avatar = new QWidget();
    avatar->setFixedSize(44, 44);
    avatar->setStyleSheet("background: #1d4ed8; border-radius: 12px;");
    QLabel *avatarText = new QLabel(avatar);
    QString init = emp.name.split(" ").first().left(1) + emp.name.split(" ").last().left(1);
    avatarText->setText(init.toUpper());
    avatarText->setAlignment(Qt::AlignCenter);
    avatarText->setStyleSheet("color: white; font-weight: bold; font-size: 14px; background: transparent;");
    avatarText->setGeometry(0, 0, 44, 44);

    QWidget *statusDot = new QWidget(avatar);
    statusDot->setFixedSize(10, 10);
    statusDot->move(32, 32);
    statusDot->setStyleSheet(
        QString("background: %1; border-radius: 5px; border: 2px solid #1e293b;")
            .arg(emp.online ? "#10B981" : "#6B7280")
        );
    cardLayout->addWidget(avatar);

    // Информация
    QVBoxLayout *infoLayout = new QVBoxLayout();
    infoLayout->setSpacing(2);

    QLabel *nameLabel = new QLabel(emp.name);
    nameLabel->setFixedHeight(20);
    nameLabel->setStyleSheet("color: #f1f5f9; font-size: 14px; font-weight: 600; background: transparent;");
    infoLayout->addWidget(nameLabel);

    QLabel *posLabel = new QLabel(emp.position);
    posLabel->setFixedHeight(18);
    posLabel->setStyleSheet("color: #94A3B8; font-size: 12px; background: transparent;");
    infoLayout->addWidget(posLabel);
    infoLayout->addSpacing(6);

    // Цветной бейдж отдела
    QString depBg, depColor;
    if (emp.department == "IT") { depBg = "rgba(59, 130, 246, 0.2)"; depColor = "#3B82F6"; }
    else if (emp.department == "Продажи") { depBg = "rgba(16, 185, 129, 0.2)"; depColor = "#10B981"; }
    else if (emp.department == "HR") { depBg = "rgba(139, 92, 246, 0.2)"; depColor = "#8B5CF6"; }
    else if (emp.department == "Финансы") { depBg = "rgba(245, 158, 11, 0.2)"; depColor = "#F59E0B"; }
    else { depBg = "rgba(59, 130, 246, 0.2)"; depColor = "#3B82F6"; }

    QLabel *depLabel = new QLabel(emp.department + " отдел");
    depLabel->setStyleSheet(
        QString("background: %1; color: %2; padding: 4px 10px; "
                "border-radius: 10px; font-size: 11px;").arg(depBg, depColor)
        );
    depLabel->adjustSize();
    depLabel->setAlignment(Qt::AlignLeft);
    infoLayout->addWidget(depLabel);
    infoLayout->setAlignment(depLabel, Qt::AlignLeft);

    cardLayout->addLayout(infoLayout,0);
    return card;
}

void EmployeesPanel::render()
{
    while (m_employeesLayout->count() > 0) {
        QLayoutItem *item = m_employeesLayout->takeAt(0);
        if (item->widget()) delete item->widget();
        delete item;
    }
    for (const Employee &emp : m_employees) {
        QWidget *card = createEmployeeCard(emp);
        m_employeesLayout->addWidget(card);
    }
    m_employeesLayout->addStretch();
}
