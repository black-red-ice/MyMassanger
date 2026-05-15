#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include "overlaydialog.h"  // вместо QDialog

class QPushButton;
class QComboBox;

class SettingsDialog : public OverlayDialog  // изменено
{
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

private slots:
    void saveSettings();
    void reject() override;

private:
    void setupUI();
    QWidget* createSettingsGroup(const QString &title, const QString &iconPath = QString());
    QWidget* createSettingsItem(const QString &title, const QString &description);
    QWidget* createToggleItem(const QString &title, const QString &description, bool checked);

    QPushButton* m_saveBtn;
    QPushButton* m_cancelBtn;
    QComboBox* m_themeCombo;
    QComboBox* m_accentCombo;
    QComboBox* m_densityCombo;
};

#endif // SETTINGSDIALOG_H
