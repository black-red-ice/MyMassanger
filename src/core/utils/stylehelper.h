#ifndef STYLEHELPER_H
#define STYLEHELPER_H

#include <QComboBox>
#include <QStringList>

class StyleHelper
{
public:
    static QComboBox* createComboBox(const QStringList &items);
};

#endif // STYLEHELPER_H
