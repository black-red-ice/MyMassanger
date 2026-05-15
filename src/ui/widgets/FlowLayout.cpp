#include "FlowLayout.h"

#include <QWidget>
#include <QLayoutItem>

FlowLayout::FlowLayout(QWidget *parent, int margin, int hSpacing, int vSpacing)
    : QLayout(parent), m_hSpace(hSpacing), m_vSpace(vSpacing)
{
    setContentsMargins(margin, margin, margin, margin);
}

FlowLayout::~FlowLayout()
{
    QLayoutItem *item;
    while ((item = takeAt(0)) != nullptr) {
        delete item;
    }
}

void FlowLayout::addItem(QLayoutItem *item)
{
    m_itemList.append(item);
}

int FlowLayout::count() const
{
    return m_itemList.size();
}

QLayoutItem *FlowLayout::itemAt(int index) const
{
    return m_itemList.value(index);
}

QLayoutItem *FlowLayout::takeAt(int index)
{
    if (index < 0 || index >= m_itemList.size())
        return nullptr;
    return m_itemList.takeAt(index);
}

Qt::Orientations FlowLayout::expandingDirections() const
{
    return {};
}

bool FlowLayout::hasHeightForWidth() const
{
    return true;
}

int FlowLayout::heightForWidth(int width) const
{
    return doLayout(QRect(0, 0, width, 0), true);
}

QSize FlowLayout::sizeHint() const
{
    return minimumSize();
}

QSize FlowLayout::minimumSize() const
{
    QSize size;
    for (QLayoutItem *item : m_itemList) {
        size = size.expandedTo(item->minimumSize());
    }

    const QMargins m = contentsMargins();
    size += QSize(m.left() + m.right(), m.top() + m.bottom());
    return size;
}

void FlowLayout::setGeometry(const QRect &rect)
{
    QLayout::setGeometry(rect);
    doLayout(rect, false);
}

int FlowLayout::horizontalSpacing() const
{
    if (m_hSpace >= 0)
        return m_hSpace;
    return smartSpacing(QStyle::PM_LayoutHorizontalSpacing);
}

int FlowLayout::verticalSpacing() const
{
    if (m_vSpace >= 0)
        return m_vSpace;
    return smartSpacing(QStyle::PM_LayoutVerticalSpacing);
}

int FlowLayout::doLayout(const QRect &rect, bool testOnly) const
{
    int left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    QRect effectiveRect = rect.adjusted(+left, +top, -right, -bottom);

    int x = effectiveRect.x();
    int y = effectiveRect.y();
    int lineHeight = 0;

    const int spaceX = horizontalSpacing();
    const int spaceY = verticalSpacing();

    for (QLayoutItem *item : m_itemList) {
        const int nextX = x + item->sizeHint().width() + spaceX;

        if (nextX - spaceX > effectiveRect.right() && lineHeight > 0) {
            x = effectiveRect.x();
            y = y + lineHeight + spaceY;
            lineHeight = 0;
        }

        if (!testOnly) {
            item->setGeometry(QRect(QPoint(x, y), item->sizeHint()));
        }

        x = x + item->sizeHint().width() + spaceX;
        lineHeight = qMax(lineHeight, item->sizeHint().height());
    }

    // Return height used inside rect (incl. margins)
    return (y - effectiveRect.y()) + lineHeight + bottom + top;
}

int FlowLayout::smartSpacing(QStyle::PixelMetric pm) const
{
    QObject *parentObj = parent();
    if (!parentObj) {
        return -1;
    } else if (parentObj->isWidgetType()) {
        QWidget *pw = static_cast<QWidget *>(parentObj);
        return pw->style()->pixelMetric(pm, nullptr, pw);
    }
    return static_cast<QLayout *>(parentObj)->spacing();
}

