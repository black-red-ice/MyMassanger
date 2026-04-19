// roundbuttons.h
#ifndef ROUNDBUTTONS_H
#define ROUNDBUTTONS_H

#include <QPushButton>

inline void addHoverEffect(QPushButton* btn, const QString& hoverColor, int radius = 10, const QString& normalColor = "") {
    if (!btn) return;

    QString currentStyle = btn->styleSheet();

    // Если normalColor не указан, сохраняем текущий фон
    if (normalColor.isEmpty()) {
        // Добавляем hover эффект и закругление к существующему стилю
        if (!currentStyle.contains("border-radius")) {
            btn->setStyleSheet(currentStyle + QString(
                                                  "QPushButton { border-radius: %1px; }"
                                                  "QPushButton:hover { background-color: %2; }"
                                                  ).arg(radius).arg(hoverColor));
        } else {
            btn->setStyleSheet(currentStyle + QString(
                                                  "QPushButton:hover { background-color: %1; }"
                                                  ).arg(hoverColor));
        }
    } else {
        // Полностью переопределяем стиль с normal, hover и закруглением
        btn->setStyleSheet(QString(
                               "QPushButton {"
                               "   background-color: %1;"
                               "   border-radius: %2px;"
                               "}"
                               "QPushButton:hover {"
                               "   background-color: %3;"
                               "}"
                               ).arg(normalColor).arg(radius).arg(hoverColor));
    }
}

#endif // ROUNDBUTTONS_H
