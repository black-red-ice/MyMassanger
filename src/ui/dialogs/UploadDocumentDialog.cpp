#include "UploadDocumentDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QFileDialog>
#include <QPixmap>
#include <QFileInfo>

UploadDocumentDialog::UploadDocumentDialog(QWidget *parent)
    : OverlayDialog(parent)
{
    setFixedSize(520, 620);
    setStyleSheet("background: transparent;");
    setupUI();
}

void UploadDocumentDialog::setupUI()
{
    // Контейнер
    QWidget *container = new QWidget(this);
    container->setObjectName("uploadContainer");
    container->setStyleSheet(
        "#uploadContainer {"
        "  background: #1e293b;"
        "  border-radius: 18px;"
        "  border: 1px solid #334155;"
        "}"
        );

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(container);

    QVBoxLayout *containerLayout = new QVBoxLayout(container);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setSpacing(0);

    // Заголовок с оранжевым фоном
    QWidget *header = new QWidget(container);
    header->setStyleSheet(
        "background: #f59e0b;"
        "border-top-left-radius: 18px;"
        "border-top-right-radius: 18px;"
        );
    header->setFixedHeight(60);
    QHBoxLayout *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(24, 0, 24, 0);

    // Иконка cloud-arrow-up-light.svg
    QLabel *iconLabel = new QLabel();
    QPixmap cloudIcon(":/icons/general/images/general/cloud-arrow-up-light.svg");
    iconLabel->setPixmap(cloudIcon.scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    iconLabel->setStyleSheet("background: transparent;");
    headerLayout->addWidget(iconLabel);

    QLabel *titleLabel = new QLabel("Загрузить документ");
    titleLabel->setStyleSheet("color: white; font-weight: 600; font-size: 18px; background: transparent; margin-left: 8px;");
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();

    QPushButton *closeBtn = new QPushButton("✕");
    closeBtn->setFixedSize(36, 36);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(
        "QPushButton { background: rgba(255,255,255,0.1); border: none; border-radius: 18px; color: white; font-size: 16px; }"
        "QPushButton:hover { background: rgba(255,255,255,0.2); }"
        );
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);
    headerLayout->addWidget(closeBtn);

    containerLayout->addWidget(header);

    // Контент
    QWidget *content = new QWidget(container);
    content->setStyleSheet(
        "background: #1e293b;"
        "border-bottom-left-radius: 18px;"
        "border-bottom-right-radius: 18px;"
        );
    QVBoxLayout *contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(24, 24, 24, 24);
    contentLayout->setSpacing(16);

    // Метка
    QLabel *fileLabel = new QLabel("Выберите файл *");
    fileLabel->setStyleSheet("color: #f8fafc; font-size: 14px; font-weight: 600; background: transparent;");
    contentLayout->addWidget(fileLabel);

    // Область выбора файла
    m_dropZoneBtn = new QPushButton(this);
    m_dropZoneBtn->setMinimumHeight(140);
    m_dropZoneBtn->setCursor(Qt::PointingHandCursor);
    m_dropZoneBtn->setStyleSheet(
        "QPushButton {"
        "  background: #1e293b;"
        "  border: 2px dashed #334155;"
        "  border-radius: 12px;"
        "  text-align: center;"
        "}"
        "QPushButton:hover { border-color: #f59e0b; }"
        );

    QVBoxLayout *dropLayout = new QVBoxLayout(m_dropZoneBtn);
    dropLayout->setAlignment(Qt::AlignCenter);
    dropLayout->setSpacing(8);

    QLabel *cloudIconBig = new QLabel();
    QPixmap cloudBigPixmap(":/icons/general/images/general/cloud-arrow-up.svg");
    cloudIconBig->setPixmap(cloudBigPixmap.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    cloudIconBig->setAlignment(Qt::AlignCenter);
    cloudIconBig->setStyleSheet("background: transparent;");
    dropLayout->addWidget(cloudIconBig);

    QLabel *dropText = new QLabel("Перетащите файлы сюда или кликните для выбора");
    dropText->setStyleSheet("color: #94A3B8; font-size: 15px; font-weight: bold; background: transparent;");
    dropText->setAlignment(Qt::AlignCenter);
    dropLayout->addWidget(dropText);

    m_fileListLabel = new QLabel("Файлы не выбраны");
    m_fileListLabel->setStyleSheet("color: #64748B; font-size: 12px; background: transparent;");
    m_fileListLabel->setAlignment(Qt::AlignCenter);
    dropLayout->addWidget(m_fileListLabel);

    connect(m_dropZoneBtn, &QPushButton::clicked, this, &UploadDocumentDialog::onSelectFile);
    contentLayout->addWidget(m_dropZoneBtn);

    // Категория — ✅ фон #1e293b
    QLabel *catLabel = new QLabel("Категория");
    catLabel->setStyleSheet("color: #f8fafc; font-size: 14px; font-weight: 600; background: transparent;");
    contentLayout->addWidget(catLabel);

    m_categoryCombo = new QComboBox();
    m_categoryCombo->addItems({"Общие документы", "Отчёты", "Договоры", "Презентации", "Техническая документация"});
    m_categoryCombo->setStyleSheet(
        "QComboBox { background: #334155; border: 1px solid #334155; border-radius: 10px; color: #f1f5f9; padding: 12px 16px; font-size: 14px; }"
        "QComboBox::drop-down { border: none; width: 30px; }"
        "QComboBox QAbstractItemView { background: #334155; color: #f1f5f9; border: 1px solid #334155; border-radius: 8px; selection-background-color: #1d4ed8; }"
        );
    m_categoryCombo->setFixedHeight(44);
    contentLayout->addWidget(m_categoryCombo);

    // Описание
    QLabel *descLabel = new QLabel("Описание (опционально)");
    descLabel->setStyleSheet("color: #f8fafc; font-size: 14px; font-weight: 600; background: transparent;");
    contentLayout->addWidget(descLabel);

    m_descriptionEdit = new QTextEdit();
    m_descriptionEdit->setPlaceholderText("Добавьте описание документа...");
    m_descriptionEdit->setStyleSheet(
        "QTextEdit { background: #334155; border: 1px solid #334155; border-radius: 10px; color: #f1f5f9; padding: 12px 16px; font-size: 14px; }"
        "QTextEdit:focus { border-color: #f59e0b; }"
        );
    m_descriptionEdit->setFixedHeight(88);
    m_descriptionEdit->setAlignment(Qt::AlignLeft | Qt::AlignTop);  // ✅ слева сверху
    contentLayout->addWidget(m_descriptionEdit);

    contentLayout->addStretch();

    // Кнопки
    QWidget *buttonRow = new QWidget();
    QHBoxLayout *buttonLayout = new QHBoxLayout(buttonRow);
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(12);

    QPushButton *cancelBtn = new QPushButton("Отмена");
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setFixedHeight(44);
    cancelBtn->setStyleSheet(
        "QPushButton { background: #334155; border: none; border-radius: 10px; color: #cbd5e1; font-size: 14px; font-weight: 500; }"
        "QPushButton:hover { background: #475569; }"
        );
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    QPushButton *uploadBtn = new QPushButton(" Загрузить");
    uploadBtn->setCursor(Qt::PointingHandCursor);
    uploadBtn->setFixedHeight(44);
    uploadBtn->setStyleSheet(
        "QPushButton { background: #f59e0b; border: none; border-radius: 10px; color: white; font-size: 14px; font-weight: 600; }"
        "QPushButton:hover { background: #d97706; }"
        );
    QPixmap uploadIcon(":/icons/general/images/general/upload-light.svg");
    uploadBtn->setIcon(QIcon(uploadIcon));
    uploadBtn->setIconSize(QSize(18, 18));
    connect(uploadBtn, &QPushButton::clicked, this, [this]() {
        if (m_filePaths.isEmpty()) {
            return;
        }
        accept();
    });

    buttonLayout->addWidget(cancelBtn, 1);
    buttonLayout->addWidget(uploadBtn, 1);
    contentLayout->addWidget(buttonRow);

    containerLayout->addWidget(content);
}

void UploadDocumentDialog::onSelectFile()
{
    QStringList files = QFileDialog::getOpenFileNames(this, "Выберите документы", QString(),
                                                      "Документы (*.pdf *.docx *.xlsx *.pptx *.txt *.zip *.jpg *.png);;Все файлы (*.*)");
    if (!files.isEmpty()) {
        m_filePaths = files;
        updateFileList();
    }
}

void UploadDocumentDialog::updateFileList()
{
    if (m_filePaths.isEmpty()) {
        m_fileListLabel->setText("Файлы не выбраны");
    } else if (m_filePaths.size() == 1) {
        m_fileListLabel->setText(QString("Выбран: %1").arg(QFileInfo(m_filePaths.first()).fileName()));
    } else {
        m_fileListLabel->setText(QString("Выбрано файлов: %1").arg(m_filePaths.size()));
    }
}

QStringList UploadDocumentDialog::selectedFiles() const { return m_filePaths; }
QString UploadDocumentDialog::category() const { return m_categoryCombo->currentText(); }
QString UploadDocumentDialog::description() const { return m_descriptionEdit->toPlainText(); }
bool UploadDocumentDialog::notifyTeam() const { return m_notifyCheck->isChecked(); }

bool UploadDocumentDialog::eventFilter(QObject *obj, QEvent *event)
{
    // Можно оставить пустым или добавить обработку drop позже
    return OverlayDialog::eventFilter(obj, event);
}
