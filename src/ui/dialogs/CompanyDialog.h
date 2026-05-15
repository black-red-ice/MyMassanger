#pragma once
#include "overlaydialog.h"
#include "../panels/companiesPanel/CompaniesPanel.h"

class QLineEdit;
class QPushButton;

class CompanyDialog : public OverlayDialog
{
    Q_OBJECT

public:
    explicit CompanyDialog(QWidget *parent = nullptr);
    Company getCompany() const;
    void setCompany(const Company &company);
    bool shouldReopenDetail() const { return m_reopenDetail; }

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    QLineEdit *m_name;
    QLineEdit *m_inn;
    QLineEdit *m_kpp;
    QLineEdit *m_address;
    QLineEdit *m_phone;
    QLineEdit *m_email;
    QLineEdit *m_contactPerson;
    QLabel *m_titleLabel = nullptr;
    bool m_reopenDetail = false;
};
