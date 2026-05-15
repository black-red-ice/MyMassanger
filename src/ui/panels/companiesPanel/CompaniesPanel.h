#pragma once
#include "../SidePanel.h"
#include <QList>
#include <QTableWidget>

struct Company {
    QString id, name, inn, kpp, address, phone, email, director, contactPerson;
};

class CompaniesPanel : public SidePanel
{
    Q_OBJECT

public:
    explicit CompaniesPanel(QWidget *parent = nullptr);
    QJsonArray getCompaniesAsJson() const;
    void setCompaniesFromJson(const QJsonArray &arr);

protected:
    void onAddClicked() override;

private slots:
    void onCompanyClicked(int row);
    void filterCompanies(const QString &query);

private:
    QList<Company> m_companies;
    QTableWidget *m_table;

    void render();
};
