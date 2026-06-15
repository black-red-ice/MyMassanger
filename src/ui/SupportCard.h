#ifndef SUPPORTCARD_H
#define SUPPORTCARD_H

#include <QWidget>
#include <QLabel>

class SupportCard : public QWidget
{
    Q_OBJECT

public:
    explicit SupportCard(const QString &iconPath,
                         const QString &title,
                         const QString &desc,
                         const QString &color,
                         const QString &categoryKey,
                         QWidget *parent = nullptr);

signals:
    void clicked(const QString &categoryKey);

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    QString m_categoryKey;
};

#endif // SUPPORTCARD_H
