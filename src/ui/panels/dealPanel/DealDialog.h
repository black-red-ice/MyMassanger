#pragma once
#include "overlaydialog.h"
#include "Deal.h"

class QLineEdit;
class QComboBox;
class QSpinBox;

class DealDialog : public OverlayDialog
{
    Q_OBJECT

public:
    explicit DealDialog(QWidget *parent = nullptr);
    Deal getDeal() const;
    void setDeal(const Deal &deal);
    bool shouldReopenDetail() const;

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    QLineEdit *m_name;
    QLineEdit *m_phone;
    QLineEdit *m_email;
    QLineEdit *m_inn;
    QLineEdit *m_extra;
    QSpinBox *m_delayDays;
    QComboBox *m_stage;

    bool m_reopenDetail = false;
};
