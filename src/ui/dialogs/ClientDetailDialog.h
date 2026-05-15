#pragma once
#include "overlaydialog.h"
#include "../panels/dealPanel/Deal.h"

class ClientDetailDialog : public OverlayDialog
{
    Q_OBJECT

public:
    explicit ClientDetailDialog(const Deal &deal, QWidget *parent = nullptr);
    bool editWasRequested() const { return m_editRequested; }

protected:
    bool event(QEvent *event) override;

private:
    Deal m_deal;
    bool m_editRequested = false;
};
