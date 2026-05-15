#ifndef LEFTNAVIGATION_H
#define LEFTNAVIGATION_H

#include <QWidget>
#include "Navigation.h"

class QPushButton;
class QVBoxLayout;

class LeftNavigation : public QWidget
{
    Q_OBJECT
public:
    explicit LeftNavigation(QWidget *parent = nullptr);
    void updateProfileAvatar();

signals:
    void currentIndexChanged(NavItem item);
    void toggleSidePanel(int panelType);   // 1-сотрудники, 2-проекты, 3-задачи, 4-календарь, 5-документы, 6-настройки, 7-поддержка, 8-CRM-Клиенты, 9-CRM-Компании

private slots:
    void onChatClicked();
    void onEmployeesClicked();
    void onProjectsClicked();
    void onTasksClicked();
    void onCalendarClicked();
    void onDocsClicked();
    void onSettingsClicked();
    void onSupportClicked();
    void onCrmClientsClicked();
    void onCrmCompaniesClicked();
    void onProfileClicked();

private:
    void setupUI();
    void addNavButton(QPushButton *btn, const QString &iconPath);
    void resetOtherButtons(QPushButton *activeBtn);

    QVBoxLayout* m_mainLayout;
    QPushButton* m_btnChat;
    QPushButton* m_btnEmployees;
    QPushButton* m_btnProjects;
    QPushButton* m_btnTasks;
    QPushButton* m_btnCalendar;
    QPushButton* m_btnDocs;
    QPushButton* m_btnSettings;
    QPushButton* m_btnSupport;
    QPushButton* m_btnCrmClients;
    QPushButton* m_btnCrmCompanies;
    QPushButton* m_btnProfile;
    void setDefaultAvatar();

    int m_currentIndex = 0;
};

#endif // LEFTNAVIGATION_H
