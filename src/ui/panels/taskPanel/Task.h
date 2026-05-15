// Task.h
#pragma once
#include <QString>
#include <QDateTime>
#include <QJsonObject>

enum class TaskStatus {
    Todo,
    InProgress,
    Done
};

enum class TaskPriority {
    Low,
    Medium,
    High
};

struct Task {
    QString id;
    QString title;
    QString description;
    QString tags;
    QDateTime deadline;
    TaskStatus status;
    TaskPriority priority;

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["id"] = id;
        obj["title"] = title;
        obj["description"] = description;
        obj["tags"] = tags;
        obj["deadline"] = deadline.toString(Qt::ISODate);

        obj["priority"] = static_cast<int>(priority); // ✅ FIX
        obj["status"] = static_cast<int>(status);     // (лучше тоже так)

        return obj;
    }

    static Task fromJson(const QJsonObject& obj) {
        Task t;
        t.id = obj["id"].toString();
        t.title = obj["title"].toString();
        t.description = obj["description"].toString();
        t.tags = obj["tags"].toString();
        t.deadline = QDateTime::fromString(obj["deadline"].toString(), Qt::ISODate);

        t.priority = static_cast<TaskPriority>(obj["priority"].toInt()); // ✅ FIX
        t.status = static_cast<TaskStatus>(obj["status"].toInt());

        return t;
    }
};
