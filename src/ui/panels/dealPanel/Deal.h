#pragma once
#include <QString>
#include <QJsonObject>

enum class DealStage {
    Contact,
    Meeting,
    Contract,
    Won,
    Lost
};

struct Deal {
    QString id;
    QString name;
    QString phone;
    QString email;
    QString inn;
    QString extra;
    int delayDays = 0;  // ✅ новое поле
    DealStage stage = DealStage::Contact;

    Deal() = default;

    Deal(const QString &id, const QString &name, const QString &phone,
         const QString &email, const QString &inn, const QString &extra,
         DealStage stage)
        : id(id), name(name), phone(phone), email(email),
        inn(inn), extra(extra), stage(stage) {}

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["id"] = id;
        obj["name"] = name;
        obj["phone"] = phone;
        obj["email"] = email;
        obj["inn"] = inn;
        obj["extra"] = extra;
        obj["delayDays"] = delayDays;
        obj["stage"] = static_cast<int>(stage);
        return obj;
    }

    static Deal fromJson(const QJsonObject& obj) {
        Deal d;
        d.id = obj["id"].toString();
        d.name = obj["name"].toString();
        d.phone = obj["phone"].toString();
        d.email = obj["email"].toString();
        d.inn = obj["inn"].toString();
        d.extra = obj["extra"].toString();
        d.delayDays = obj["delayDays"].toInt();
        d.stage = static_cast<DealStage>(obj["stage"].toInt());
        return d;
    }
};
