#pragma once
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>

class SidePanel : public QWidget
{
    Q_OBJECT

public:
    explicit SidePanel(QWidget *parent = nullptr,
                       const QString &title = "",
                       const QString &iconPath = "",
                       const QString &headerColor = "#8146f0",
                       const QString &buttonText = "Добавить",
                       int panelWidth = 400,
                       const QString &buttonColor = "#0EA5E9",
                       const QString &buttonIconPath = "");  // новый параметр

    QPushButton* getAddButton() const { return m_addButton; }
    QLineEdit* getSearchInput() const { return m_searchInput; }
    QVBoxLayout* getContentLayout() const { return m_contentLayout; }
    void setSearchVisible(bool visible);

    // Добавляем публичный метод для установки иконки заголовка
    void setHeaderIcon(const QString &iconPath);

signals:
    void addClicked();
    void closeRequested();  // Сигнал для закрытия панели

protected:
    virtual void onAddClicked() {}

private:
    void setupHeader(const QString &title, const QString &iconPath, const QString &headerColor);
    void setupSearch();
    void setupContentArea();
    void setupFooter(const QString &buttonText);

    QVBoxLayout *m_mainLayout;
    QVBoxLayout *m_contentLayout;
    QLineEdit *m_searchInput;
    QPushButton *m_addButton;
    QLabel *m_headerIconLabel;  // Добавляем для иконки
    int m_panelWidth;
    QString m_buttonColor;
    QWidget *m_separator;
    QString m_buttonIconPath;
};
