#include "TaskDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QDateTimeEdit>
#include <QComboBox>
#include <QPushButton>
#include <QKeyEvent>

TaskDialog::TaskDialog(QWidget *parent)
    : OverlayDialog(parent)
{
    setFixedSize(520, 620);
    setStyleSheet("background-color: transparent;");

    QWidget *container = new QWidget(this);
    container->setObjectName("taskDialogContainer");
    container->setStyleSheet(
        "#taskDialogContainer { background-color: #1e293b; border-radius: 18px; border: 1px solid #334155; }"
        );

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(container);

    QVBoxLayout *containerLayout = new QVBoxLayout(container);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setSpacing(0);

    QWidget *header = new QWidget(container);
    header->setStyleSheet("background-color: #1e293b; border-top-left-radius: 18px; border-top-right-radius: 18px; border-bottom: 1px solid #334155;");
    header->setFixedHeight(68);
    QHBoxLayout *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(20, 0, 20, 0);

    QLabel *titleLabel = new QLabel("Новая задача", header);
    titleLabel->setStyleSheet("color: white; font-weight: 600; font-size: 16px;");

    QPushButton *closeBtn = new QPushButton("✕", header);
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

    QWidget *content = new QWidget(container);
    content->setStyleSheet("background-color: #1e293b;");
    QVBoxLayout *contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(24, 20, 24, 20);
    contentLayout->setSpacing(20);

    QLabel *titleLabelField = new QLabel("Название задачи", content);
    titleLabelField->setStyleSheet("color: #f8fafc; font-size: 13px; font-weight: 600;");
    contentLayout->addWidget(titleLabelField);

    m_title = new QLineEdit(content);
    m_title->setPlaceholderText("Введите название задачи...");
    m_title->setStyleSheet("QLineEdit { background-color: #111827; border: 1px solid #334155; border-radius: 12px; color: #f8fafc; padding: 12px; } QLineEdit:focus { border-color: #0ea5e9; }\n");
    contentLayout->addWidget(m_title);

    QLabel *descriptionLabel = new QLabel("Описание", content);
    descriptionLabel->setStyleSheet("color: #f8fafc; font-size: 13px; font-weight: 600;");
    contentLayout->addWidget(descriptionLabel);

    m_description = new QTextEdit(content);
    m_description->setPlaceholderText("Добавьте описание задачи...");
    m_description->setStyleSheet("QTextEdit { background-color: #111827; border: 1px solid #334155; border-radius: 12px; color: #f8fafc; padding: 12px; } QScrollBar:vertical { width: 8px; background: #111827; } QScrollBar::handle:vertical { background: #334155; border-radius: 4px; }\n");
    m_description->setFixedHeight(120);
    contentLayout->addWidget(m_description);

    QWidget *row = new QWidget(content);
    QHBoxLayout *rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setSpacing(12);

    QWidget *deadlineColumn = new QWidget(row);
    QVBoxLayout *deadlineLayout = new QVBoxLayout(deadlineColumn);
    deadlineLayout->setContentsMargins(0, 0, 0, 0);
    deadlineLayout->setSpacing(8);

    QLabel *deadlineLabel = new QLabel("Срок выполнения", deadlineColumn);
    deadlineLabel->setStyleSheet("color: #f8fafc; font-size: 13px; font-weight: 600;");
    deadlineLayout->addWidget(deadlineLabel);

    m_deadline = new QDateTimeEdit(QDateTime::currentDateTime(), deadlineColumn);
    m_deadline->setCalendarPopup(true);
    m_deadline->setStyleSheet(
        "QDateTimeEdit { background-color: #111827; border: 1px solid #334155; border-radius: 12px; color: #f8fafc; padding: 10px 10px 10px 12px; }"
        "QDateTimeEdit::drop-down { width: 34px; border: none; border-top-right-radius: 12px; border-bottom-right-radius: 12px; background-color: #1e293b; }"
        "QDateTimeEdit::down-arrow { image: none; width: 0; height: 0; border-left: 5px solid transparent; border-right: 5px solid transparent; border-top: 6px solid #f8fafc; margin: 0 auto; }"
    );
    m_deadline->setFixedHeight(44);
    m_deadline->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    deadlineLayout->addWidget(m_deadline);

    QWidget *priorityColumn = new QWidget(row);
    QVBoxLayout *priorityLayout = new QVBoxLayout(priorityColumn);
    priorityLayout->setContentsMargins(0, 0, 0, 0);
    priorityLayout->setSpacing(8);

    QLabel *priorityLabel = new QLabel("Приоритет", priorityColumn);
    priorityLabel->setStyleSheet("color: #f8fafc; font-size: 13px; font-weight: 600;");
    priorityLayout->addWidget(priorityLabel);

    m_priority = new QComboBox(priorityColumn);
    m_priority->addItems({"Низкий", "Средний", "Высокий"});
    m_priority->setStyleSheet("QComboBox { background-color: #111827; border: 1px solid #334155; border-radius: 12px; color: #f8fafc; padding: 10px; } QComboBox::drop-down { border: none; } QComboBox QAbstractItemView { background-color: #111827; color: #f8fafc; }\n");
    m_priority->setFixedHeight(44);
    m_priority->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    priorityLayout->addWidget(m_priority);

    rowLayout->addWidget(deadlineColumn, 1);
    rowLayout->addWidget(priorityColumn, 1);
    contentLayout->addWidget(row);

    QLabel *tagsLabel = new QLabel("Теги (через запятую)", content);
    tagsLabel->setStyleSheet("color: #f8fafc; font-size: 13px; font-weight: 600;");
    contentLayout->addWidget(tagsLabel);

    m_tags = new QLineEdit(content);
    m_tags->setPlaceholderText("Например: Отчётность, Встреча, Разработка");
    m_tags->setStyleSheet("QLineEdit { background-color: #111827; border: 1px solid #334155; border-radius: 12px; color: #f8fafc; padding: 12px; } QLineEdit:focus { border-color: #0ea5e9; }\n");
    contentLayout->addWidget(m_tags);

    QWidget *buttonRow = new QWidget(content);
    QHBoxLayout *buttonLayout = new QHBoxLayout(buttonRow);
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(12);

    QPushButton *cancelBtn = new QPushButton("Отмена", buttonRow);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setStyleSheet(
        "QPushButton { background-color: #334155; border: none; border-radius: 12px; color: #cbd5e1; padding: 14px; font-size: 14px; }"
        "QPushButton:hover { background-color: #475569; }");

    m_saveBtn = new QPushButton("Создать задачу", buttonRow);
    m_saveBtn->setCursor(Qt::PointingHandCursor);
    m_saveBtn->setStyleSheet(
        "QPushButton { background-color: #0EA5E9; border: none; border-radius: 12px; color: white; padding: 14px; font-size: 14px; font-weight: 600; }"
        "QPushButton:hover { background-color: #1d4ed8; }"
    );

    buttonLayout->addWidget(cancelBtn);
    buttonLayout->addWidget(m_saveBtn);
    contentLayout->addWidget(buttonRow);
    contentLayout->addStretch();

    containerLayout->addWidget(content);

    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    // Кнопка сохранения
    connect(m_saveBtn, &QPushButton::clicked, this, [=]() {
        if (m_title->text().trimmed().isEmpty()) {
            m_title->setStyleSheet(
                "QLineEdit { background-color: #111827; border: 1px solid #EF4444; border-radius: 12px; color: #f8fafc; padding: 12px; }"
                );
            m_title->setFocus();
            return;
        }
        accept();  // Просто accept, без emit taskCreated
    });

    connect(m_title, &QLineEdit::textChanged, this, [this]() {
        m_title->setStyleSheet(
            "QLineEdit { background-color: #111827; border: 1px solid #334155; border-radius: 12px; color: #f8fafc; padding: 12px; }"
            "QLineEdit:focus { border-color: #0ea5e9; }"
            );
    });

    // После создания кнопок:
    cancelBtn->setAutoDefault(false);
    cancelBtn->setDefault(false);
    m_saveBtn->setAutoDefault(true);
    m_saveBtn->setDefault(true);
}

Task TaskDialog::getTask() const
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

// keyPressEvent
void TaskDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        if (m_title->text().trimmed().isEmpty()) {
            m_title->setStyleSheet(
                "QLineEdit { background-color: #111827; border: 1px solid #EF4444; border-radius: 12px; color: #f8fafc; padding: 12px; }"
                );
            m_title->setFocus();
            return;
        }
        accept();  // Просто accept
        return;
    }
    if (event->key() == Qt::Key_Escape) {
        reject();
        return;
    }
    QDialog::keyPressEvent(event);
}

void TaskDialog::setTask(const Task &task)
{
    m_editMode = true;
    m_title->setText(task.title);
    m_description->setPlainText(task.description);
    m_tags->setText(task.tags);
    m_deadline->setDateTime(task.deadline);

    switch (task.priority) {
    case TaskPriority::Low: m_priority->setCurrentIndex(0); break;
    case TaskPriority::Medium: m_priority->setCurrentIndex(1); break;
    case TaskPriority::High: m_priority->setCurrentIndex(2); break;
    }

    // Меняем текст кнопки
    m_saveBtn->setText("Сохранить");
}
