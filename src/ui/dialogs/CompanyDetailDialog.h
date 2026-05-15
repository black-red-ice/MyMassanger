#pragma once
#include "overlaydialog.h"
#include "../panels/companiesPanel/CompaniesPanel.h"

class CompanyDetailDialog : public OverlayDialog
{
    Q_OBJECT

public:
    explicit CompanyDetailDialog(const Company &company, QWidget *parent = nullptr);
    bool editWasRequested() const { return m_editRequested; }

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    Company m_company;
    bool m_editRequested = false;
};
