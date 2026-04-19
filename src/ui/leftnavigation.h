#ifndef LEFTNAVIGATION_H
#define LEFTNAVIGATION_H

#include <QWidget>

class QPushButton;
class QVBoxLayout;

class LeftNavigation : public QWidget
{
    Q_OBJECT
public:
    explicit LeftNavigation(QWidget *parent = nullptr);

signals:
    void currentIndexChanged(int index);   // индекс выбранного пункта (0-чаты, 1-сотрудники и т.д.)
    void toggleSidePanel(int panelType);   // 1-сотрудники, 2-проекты, 3-задачи и т.д.

private slots:
    void onChatClicked();
    void onEmployeesClicked();
    void onProjectsClicked();
    void onTasksClicked();
    void onCalendarClicked();
    void onDocsClicked();
    void onSettingsClicked();
    void onSupportClicked();

private:
    void setupUI();
    void addNavButton(QPushButton *btn, const QString &iconPath);

    QVBoxLayout* m_mainLayout;
    QPushButton* m_btnChat;
    QPushButton* m_btnEmployees;
    QPushButton* m_btnProjects;
    QPushButton* m_btnTasks;
    QPushButton* m_btnCalendar;
    QPushButton* m_btnDocs;
    QPushButton* m_btnSettings;
    QPushButton* m_btnSupport;

    int m_currentIndex = 0;
};

#endif // LEFTNAVIGATION_H
