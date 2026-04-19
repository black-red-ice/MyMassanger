#include "stylehelper.h"

QComboBox* StyleHelper::createComboBox(const QStringList &items)
{
    QComboBox *combo = new QComboBox();
    combo->addItems(items);
    combo->setCursor(Qt::PointingHandCursor);
    combo->setStyleSheet(
        "QComboBox { background-color: #475569; border: 1px solid #64748b; border-radius: 8px; padding: 8px 12px; color: #f1f5f9; font-size: 14px; min-width: 160px; }"
        "QComboBox:hover { border-color: #1d4ed8; }"
        "QComboBox::drop-down { border: none; width: 30px; }"
        "QComboBox::down-arrow { image: none; width: 0; height: 0; border-left: 5px solid transparent; border-right: 5px solid transparent; border-top: 6px solid #94a3b8; margin-right: 10px; }"
        "QComboBox QAbstractItemView { background-color: #334155; border: 1px solid #475569; border-radius: 8px; color: #f1f5f9; selection-background-color: #1d4ed8; }"
        );
    return combo;
}
