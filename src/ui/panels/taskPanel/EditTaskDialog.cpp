#include "EditTaskDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QDateTimeEdit>
#include <QComboBox>
#include <QPushButton>
#include <QKeyEvent>

EditTaskDialog::EditTaskDialog(const Task &task, QWidget *parent)
    : OverlayDialog(parent)
{
    setFixedSize(520, 620);
    setStyleSheet("background-color: transparent;");

    QWidget *container = new QWidget(this);
    container->setObjectName("editTaskContainer");
    container->setStyleSheet(
        "#editTaskContainer { background-color: #111827; border-radius: 18px; border: 1px solid #334155; }"
        );

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(container);

    QVBoxLayout *containerLayout = new QVBoxLayout(container);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setSpacing(0);

    // Заголовок
    QWidget *header = new QWidget(container);
    header->setStyleSheet("background-color: #1e293b; border-top-left-radius: 18px; border-top-right-radius: 18px; border-bottom: 1px solid #334155;");
    header->setFixedHeight(68);
    QHBoxLayout *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(20, 0, 20, 0);

    QLabel *titleLabel = new QLabel("Редактировать задачу");
    titleLabel->setStyleSheet("color: white; font-weight: 600; font-size: 16px;");

    QPushButton *closeBtn = new QPushButton("✕");
    closeBtn->setFixedSize(34, 34);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(
        "QPushButton { background: rgba(255,255,255,0.14); border: none; border-radius: 17px; color: white; font-size: 16px; }"
        "QPushButton:hover { background: rgba(255,255,255,0.22); }"
        );
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);

    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(closeBtn);
    containerLayout->addWidget(header);

    // Контент
    QWidget *content = new QWidget(container);
    content->setStyleSheet("background-color: #1e293b;");
    QVBoxLayout *contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(24, 20, 24, 20);
    contentLayout->setSpacing(20);

    // Название
    QLabel *nameLabel = new QLabel("Название задачи");
    nameLabel->setStyleSheet("color: #f8fafc; font-size: 13px; font-weight: 600;");
    contentLayout->addWidget(nameLabel);

    m_title = new QLineEdit();
    m_title->setText(task.title);
    m_title->setStyleSheet("QLineEdit { background-color: #111827; border: 1px solid #334155; border-radius: 12px; color: #f8fafc; padding: 12px; } QLineEdit:focus { border-color: #0ea5e9; }");
    contentLayout->addWidget(m_title);

    // Описание
    QLabel *descLabel = new QLabel("Описание");
    descLabel->setStyleSheet("color: #f8fafc; font-size: 13px; font-weight: 600;");
    contentLayout->addWidget(descLabel);

    m_description = new QTextEdit();
    m_description->setPlainText(task.description);
    m_description->setStyleSheet("QTextEdit { background-color: #111827; border: 1px solid #334155; border-radius: 12px; color: #f8fafc; padding: 12px; }");
    m_description->setFixedHeight(120);
    contentLayout->addWidget(m_description);

    // Срок + Приоритет
    QWidget *row = new QWidget();
    QHBoxLayout *rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setSpacing(12);

    QWidget *deadCol = new QWidget();
    QVBoxLayout *deadLayout = new QVBoxLayout(deadCol);
    deadLayout->setContentsMargins(0, 0, 0, 0);
    deadLayout->setSpacing(8);
    QLabel *deadLabel = new QLabel("Срок выполнения");
    deadLabel->setStyleSheet("color: #f8fafc; font-size: 13px; font-weight: 600;");
    deadLayout->addWidget(deadLabel);
    m_deadline = new QDateTimeEdit(task.deadline);
    m_deadline->setCalendarPopup(true);
    m_deadline->setStyleSheet("QDateTimeEdit { background-color: #111827; border: 1px solid #334155; border-radius: 12px; color: #f8fafc; padding: 10px; }");
    m_deadline->setFixedHeight(44);
    deadLayout->addWidget(m_deadline);
    rowLayout->addWidget(deadCol, 1);

    QWidget *prioCol = new QWidget();
    QVBoxLayout *prioLayout = new QVBoxLayout(prioCol);
    prioLayout->setContentsMargins(0, 0, 0, 0);
    prioLayout->setSpacing(8);
    QLabel *prioLabel = new QLabel("Приоритет");
    prioLabel->setStyleSheet("color: #f8fafc; font-size: 13px; font-weight: 600;");
    prioLayout->addWidget(prioLabel);
    m_priority = new QComboBox();
    m_priority->addItems({"Низкий", "Средний", "Высокий"});
    m_priority->setCurrentIndex(static_cast<int>(task.priority));
    m_priority->setStyleSheet("QComboBox { background-color: #111827; border: 1px solid #334155; border-radius: 12px; color: #f8fafc; padding: 10px; }");
    m_priority->setFixedHeight(44);
    prioLayout->addWidget(m_priority);
    rowLayout->addWidget(prioCol, 1);

    contentLayout->addWidget(row);

    // Теги
    QLabel *tagsLabel = new QLabel("Теги (через запятую)");
    tagsLabel->setStyleSheet("color: #f8fafc; font-size: 13px; font-weight: 600;");
    contentLayout->addWidget(tagsLabel);

    m_tags = new QLineEdit();
    m_tags->setText(task.tags);
    m_tags->setStyleSheet("QLineEdit { background-color: #111827; border: 1px solid #334155; border-radius: 12px; color: #f8fafc; padding: 12px; }");
    contentLayout->addWidget(m_tags);

    // Кнопки
    QWidget *buttonRow = new QWidget();
    QHBoxLayout *buttonLayout = new QHBoxLayout(buttonRow);
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(12);

    QPushButton *cancelBtn = new QPushButton("Отмена");
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setStyleSheet("QPushButton { background-color: #334155; border: none; border-radius: 12px; color: #cbd5e1; padding: 14px; font-size: 14px; } QPushButton:hover { background-color: #475569; }");
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    QPushButton *saveBtn = new QPushButton("Сохранить");
    saveBtn->setCursor(Qt::PointingHandCursor);
    saveBtn->setStyleSheet("QPushButton { background-color: #0EA5E9; border: none; border-radius: 12px; color: white; padding: 14px; font-size: 14px; font-weight: 600; } QPushButton:hover { background-color: #1d4ed8; }");

    auto saveTask = [this]() {
        if (m_title->text().trimmed().isEmpty()) {
            m_title->setStyleSheet("QLineEdit { background-color: #111827; border: 1px solid #EF4444; border-radius: 12px; color: #f8fafc; padding: 12px; }");
            m_title->setFocus();
            return;
        }
        accept();
    };

    connect(saveBtn, &QPushButton::clicked, saveTask);
    saveBtn->setDefault(true);

    buttonLayout->addWidget(cancelBtn);
    buttonLayout->addWidget(saveBtn);
    contentLayout->addWidget(buttonRow);
    contentLayout->addStretch();
    containerLayout->addWidget(content);
}

Task EditTaskDialog::getTask() const
{
    Task t;
    t.title = m_title->text().trimmed();
    t.description = m_description->toPlainText().trimmed();
    t.tags = m_tags->text().trimmed();
    t.deadline = m_deadline->dateTime();

    switch (m_priority->currentIndex()) {
    case 0: t.priority = TaskPriority::Low; break;
    case 1: t.priority = TaskPriority::Medium; break;
    case 2: t.priority = TaskPriority::High; break;
    }

    return t;
}

void EditTaskDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        if (m_title->text().trimmed().isEmpty()) {
            m_title->setStyleSheet("QLineEdit { background-color: #111827; border: 1px solid #EF4444; border-radius: 12px; color: #f8fafc; padding: 12px; }");
            m_title->setFocus();
            return;
        }
        accept();
        return;
    }
    if (event->key() == Qt::Key_Escape) {
        reject();
        return;
    }
    QDialog::keyPressEvent(event);
}
