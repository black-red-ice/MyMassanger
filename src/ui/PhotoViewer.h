#pragma once
#include <QDialog>
#include <QLabel>
#include <QPushButton>

class PhotoViewer : public QDialog
{
    Q_OBJECT

public:
    explicit PhotoViewer(const QString &imagePath, QWidget *parent = nullptr);
    ~PhotoViewer();

signals:
    void closed();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    QWidget *m_photoContainer;
};
