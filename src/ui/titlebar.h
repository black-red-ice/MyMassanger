#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <QWidget>

class TitleBar : public QWidget
{
    Q_OBJECT
public:
    explicit TitleBar(QWidget *parent = nullptr);

signals:
    void minimizeClicked();
    void maximizeClicked();
    void closeClicked();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    QPoint m_dragPosition;
    bool m_dragging = false;
};

#endif // TITLEBAR_H
