#include "CompaniesPanel.h"
#include "mainwindow.h"
#include "CompanyDialog.h"
#include "CompanyDetailDialog.h"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QUuid>
#include <QTimer>
#include <QJsonArray>
#include <QJsonObject>

CompaniesPanel::CompaniesPanel(QWidget *parent)
    : SidePanel(parent,
                "Компании (справочник)",
                ":/icons/general/images/general/building-light.svg",
                "#3B82F6",
                "Добавить компанию",
                400,  // ← Изменили с 420 на 400
                "#3B82F6",
                ":/icons/general/images/general/plus.svg")
{
    getSearchInput()->setPlaceholderText("Поиск по ИНН или названию...");
    connect(getSearchInput(), &QLineEdit::textChanged, this, &CompaniesPanel::filterCompanies);

    QLayoutItem *stretch = getContentLayout()->takeAt(getContentLayout()->count() - 1);
    delete stretch;

    // Заголовки колонок
    QWidget *headerRow = new QWidget();
    headerRow->setStyleSheet("background: transparent;");
    QHBoxLayout *headerLayout = new QHBoxLayout(headerRow);
    headerLayout->setContentsMargins(16, 8, 16, 8);
    headerLayout->setSpacing(8);

    QLabel *nameHeader = new QLabel("Название");
    nameHeader->setStyleSheet("color: #64748B; font-size: 12px; font-weight: 600; background: transparent;");
    nameHeader->setMinimumWidth(120);

    QLabel *innHeader = new QLabel("ИНН");
    innHeader->setStyleSheet("color: #64748B; font-size: 12px; font-weight: 600; background: transparent;");
    innHeader->setMinimumWidth(100);

    QLabel *phoneHeader = new QLabel("Телефон");
    phoneHeader->setStyleSheet("color: #64748B; font-size: 12px; font-weight: 600; background: transparent;");

    headerLayout->addWidget(nameHeader);
    headerLayout->addWidget(innHeader);
    headerLayout->addWidget(phoneHeader, 1);

    getContentLayout()->addWidget(headerRow);

    // Разделитель
    QFrame *headerSep = new QFrame();
    headerSep->setFrameShape(QFrame::HLine);
    headerSep->setStyleSheet("color: #334155; max-height: 1px;");
    getContentLayout()->addWidget(headerSep);

    // Таблица
    m_table = new QTableWidget();
    m_table->setColumnCount(3);
    m_table->setHorizontalHeaderLabels({"Название", "ИНН", "Телефон"});
    m_table->horizontalHeader()->setVisible(false);  // Скрываем встроенные заголовки
    m_table->verticalHeader()->setVisible(false);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setShowGrid(false);
    m_table->setStyleSheet(
        "QTableWidget { background: #1e293b; border: none; color: #f1f5f9; }"
        "QTableWidget::item { padding: 10px 8px; border-bottom: 1px solid #334155; }"
        "QTableWidget::item:selected { background: rgba(59, 130, 246, 0.2); }"
        );
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_table->setColumnWidth(0, 140);
    m_table->setColumnWidth(1, 110);

    connect(m_table, &QTableWidget::cellClicked, this, [this](int row, int) { onCompanyClicked(row); });

    getContentLayout()->addWidget(m_table);
    getContentLayout()->addStretch();

    render();
}

void CompaniesPanel::onAddClicked()
{
    QWidget *dimWidget = new QWidget(this->window());
    dimWidget->setObjectName("companyDim");
    dimWidget->setStyleSheet("#companyDim { background-color: rgba(0, 0, 0, 180); }");
    dimWidget->setGeometry(this->window()->rect());
    dimWidget->raise();
    dimWidget->show();

    CompanyDialog dialog(dimWidget);
    if (dialog.exec() == QDialog::Accepted) {
        Company c = dialog.getCompany();
        c.id = QUuid::createUuid().toString();
        m_companies.append(c);
        render();

        // Сохраняем на сервер
        MainWindow *mw = qobject_cast<MainWindow*>(this->window());
        if (mw) {
            mw->saveCompaniesToServer();
        }
    }
    dimWidget->deleteLater();
}

void CompaniesPanel::onCompanyClicked(int row)
{
    if (row < 0 || row >= m_companies.size()) return;

    QWidget *dim = new QWidget(this->window());
    dim->setStyleSheet("background-color: rgba(0, 0, 0, 180);");
    dim->setGeometry(this->window()->rect());
    dim->show();

    bool reopenDetail = true;

    while (reopenDetail) {
        reopenDetail = false;

        CompanyDetailDialog detailDialog(m_companies[row], dim);
        detailDialog.exec();

        if (detailDialog.editWasRequested()) {
            CompanyDialog editDialog(dim);
            editDialog.setCompany(m_companies[row]);

            if (editDialog.exec() == QDialog::Accepted) {
                Company updated = editDialog.getCompany();
                updated.id = m_companies[row].id;
                m_companies[row] = updated;
                render();

                // Сохраняем на сервер
                MainWindow *mw = qobject_cast<MainWindow*>(this->window());
                if (mw) {
                    mw->saveCompaniesToServer();
                }
            }

            reopenDetail = editDialog.shouldReopenDetail();
        }
    }

    dim->deleteLater();
}

void CompaniesPanel::filterCompanies(const QString &query)
{
    for (int i = 0; i < m_table->rowCount(); ++i) {
        bool match = query.isEmpty();
        if (!match) {
            for (int j = 0; j < m_table->columnCount(); ++j) {
                QTableWidgetItem *item = m_table->item(i, j);
                if (item && item->text().contains(query, Qt::CaseInsensitive)) {
                    match = true;
                    break;
                }
            }
        }
        m_table->setRowHidden(i, !match);
    }
}

void CompaniesPanel::render()
{
    m_table->setRowCount(m_companies.size());
    for (int i = 0; i < m_companies.size(); ++i) {
        const Company &c = m_companies[i];
        m_table->setItem(i, 0, new QTableWidgetItem(c.name));
        m_table->setItem(i, 1, new QTableWidgetItem(c.inn));
        m_table->setItem(i, 2, new QTableWidgetItem(c.phone));
        m_table->setRowHeight(i, 44);
    }
}

QJsonArray CompaniesPanel::getCompaniesAsJson() const
{
    QJsonArray arr;
    for (const Company &c : m_companies) {
        QJsonObject obj;
        obj["id"] = c.id;
        obj["name"] = c.name;
        obj["inn"] = c.inn;
        obj["kpp"] = c.kpp;
        obj["address"] = c.address;
        obj["phone"] = c.phone;
        obj["email"] = c.email;
        obj["director"] = c.director;
        obj["contactPerson"] = c.contactPerson;
        arr.append(obj);
    }
    return arr;
}

void CompaniesPanel::setCompaniesFromJson(const QJsonArray &arr)
{
    m_companies.clear();
    for (const QJsonValue &v : arr) {
        QJsonObject obj = v.toObject();
        Company c;
        c.id = obj["id"].toString();
        c.name = obj["name"].toString();
        c.inn = obj["inn"].toString();
        c.kpp = obj["kpp"].toString();
        c.address = obj["address"].toString();
        c.phone = obj["phone"].toString();
        c.email = obj["email"].toString();
        c.director = obj["director"].toString();
        c.contactPerson = obj["contactPerson"].toString();
        m_companies.append(c);
    }
    render();
}
