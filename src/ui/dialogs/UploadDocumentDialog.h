#pragma once
#include "overlaydialog.h"
#include <QStringList>
#include <QTextEdit>

class QLineEdit;
class QComboBox;
class QCheckBox;
class QLabel;

class UploadDocumentDialog : public OverlayDialog
{
    Q_OBJECT
public:
    explicit UploadDocumentDialog(QWidget *parent = nullptr);
    QStringList selectedFiles() const;
    QString category() const;
    QString description() const;
    bool notifyTeam() const;

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;  // ✅

private slots:
    void onSelectFile();

private:
    void setupUI();
    void updateFileList();

    QStringList m_filePaths;
    QLabel *m_fileListLabel;
    QComboBox *m_categoryCombo;
    QTextEdit *m_descriptionEdit;
    QCheckBox *m_notifyCheck;
    QPushButton *m_dropZoneBtn;   // ✅ заменим QWidget dropZone на кнопку
};
