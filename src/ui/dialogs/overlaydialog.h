#ifndef OVERLAYDIALOG_H
#define OVERLAYDIALOG_H

#include <QDialog>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

class OverlayDialog : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(double opacity READ opacity WRITE setOpacity)

public:
    explicit OverlayDialog(QWidget *parent = nullptr);
    ~OverlayDialog();

    void setOpacity(double opacity);
    double opacity() const { return m_opacity; }

protected:
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private:
    void startFadeIn();
    void startFadeOut();

    QGraphicsOpacityEffect *m_effect;
    QPropertyAnimation *m_animation;
    double m_opacity = 0.0;
};

#endif // OVERLAYDIALOG_H
