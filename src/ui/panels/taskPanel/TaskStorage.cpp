// TaskStorage.cpp
#include "TaskStorage.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>

QList<Task> TaskStorage::load()
{
    QList<Task> tasks;

    QFile file("tasks.json");
    if (!file.open(QIODevice::ReadOnly))
        return tasks;

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);

    if (error.error != QJsonParseError::NoError) {
        qDebug() << "tasks.json parse error:" << error.errorString();
        return tasks;
    }

    QJsonArray arr = doc.array();
    for (auto val : arr) {
        Task t = Task::fromJson(val.toObject());
        if (!t.title.isEmpty()) {  // Пропускаем задачи с пустым названием
            tasks.append(t);
        }
    }

    return tasks;
}

void TaskStorage::save(const QList<Task>& tasks)
{
    QJsonArray arr;
    for (const Task& t : tasks) {
        arr.append(t.toJson());
    }

    QFile file("tasks.json");
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(arr).toJson(QJsonDocument::Indented));
        file.close();
    }
}
