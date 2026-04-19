#ifndef TOGGLEBUTTON_H
#define TOGGLEBUTTON_H

#include <QWidget>
#include <QPropertyAnimation>

class ToggleButton : public QWidget
{
    Q_OBJECT
public:
    explicit ToggleButton(bool checked = false, QWidget *parent = nullptr);
    bool isChecked() const { return m_checked; }

signals:
    void toggled(bool checked);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void updateVisuals();

    bool m_checked;
    QWidget *m_track;
    QWidget *m_knob;
    QPropertyAnimation *m_animation;
};

#endif // TOGGLEBUTTON_H
