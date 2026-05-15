// TaskStorage.h
#pragma once
#include "Task.h"
#include <QList>

class TaskStorage {
public:
    static QList<Task> load();
    static void save(const QList<Task>& tasks);
};
