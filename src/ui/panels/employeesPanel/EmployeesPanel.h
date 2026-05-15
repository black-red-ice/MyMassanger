#pragma once
#include "../SidePanel.h"
#include <QList>
#include <QMap>

struct Employee {
    QString name;
    QString position;
    QString department;
    bool online;
};

class EmployeesPanel : public SidePanel
{
    Q_OBJECT

public:
    explicit EmployeesPanel(QWidget *parent = nullptr);

protected:
    void onAddClicked() override {}

private slots:
    void filterByDepartment(const QString &department);
    void filterEmployees(const QString &query);

private:
    QList<Employee> m_employees;
    QVBoxLayout *m_employeesLayout;
    QWidget *m_filterWidget;

    void render();
    void createDepartmentFilters();
    QWidget* createEmployeeCard(const Employee &emp);
};
