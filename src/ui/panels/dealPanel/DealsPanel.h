#pragma once
#include "../SidePanel.h"
#include "Deal.h"

class DealsPanel : public SidePanel
{
    Q_OBJECT

signals:
    void openChatWithClient(const QString &clientName, const QString &clientPhone);

public:
    explicit DealsPanel(QWidget *parent = nullptr);
    QJsonArray getDealsAsJson() const;
    void setDealsFromJson(const QJsonArray &arr);

protected:
    void onAddClicked() override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void onStageChanged(int dealIndex, DealStage stage);
    void filterDeals(const QString &query);

private:
    QList<Deal> m_deals;
    QVBoxLayout *m_dealsLayout;
    QTimer *m_clickTimer = nullptr;
    int m_pendingClickIndex = -1;

    void render();
    QWidget* createDealCard(const Deal &deal, int index);
    void onCardClicked(int index);
};
