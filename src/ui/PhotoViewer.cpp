#include "PhotoViewer.h"
#include <QVBoxLayout>
#include <QGuiApplication>
#include <QScreen>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QShowEvent>
#include <QPainter>
#include <QTimer>
#include <QStandardPaths>
#include <QFile>

PhotoViewer::PhotoViewer(const QString &imagePath, QWidget *parent)
    : QDialog(parent, Qt::FramelessWindowHint | Qt::Dialog)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose, true);
    setModal(false);

    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenRect = screen->geometry();
    setGeometry(screenRect);

    QPixmap originalPixmap;

    QString finalPath = imagePath;

    if (imagePath.startsWith("/files/") ||
        imagePath.startsWith("http"))
    {
        QString cachePath =
            QStandardPaths::writableLocation(
                QStandardPaths::CacheLocation)
            + "/avatars/view.jpg";

        if (QFile::exists(cachePath))
        {
            finalPath = cachePath;
        }
    }

    originalPixmap.load(finalPath);

    if (originalPixmap.isNull()) {
        QTimer::singleShot(0, this, &PhotoViewer::close);
        return;
    }

    int maxWidth = screenRect.width() * 0.8;
    int maxHeight = screenRect.height() * 0.85;

    QPixmap scaled = originalPixmap.scaled(maxWidth, maxHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    int containerWidth = scaled.width() + 40;
    int containerHeight = scaled.height() + 50;
    containerWidth = qMax(containerWidth, 300);
    containerHeight = qMax(containerHeight, 200);

    m_photoContainer = new QWidget(this);
    m_photoContainer->setObjectName("photoContainer");
    m_photoContainer->setFixedSize(containerWidth, containerHeight);
    m_photoContainer->setStyleSheet(
        "#photoContainer {"
        "  background: #1e293b;"
        "  border-radius: 16px;"
        "  border: 1px solid #334155;"
        "}"
        );

    QLabel *photoLabel = new QLabel(m_photoContainer);
    photoLabel->setPixmap(scaled);
    photoLabel->setAlignment(Qt::AlignCenter);
    photoLabel->setGeometry(
        (containerWidth - scaled.width()) / 2,
        20,
        scaled.width(),
        scaled.height()
        );
    photoLabel->setStyleSheet("background: transparent;");

    QPushButton *closeBtn = new QPushButton("✕", m_photoContainer);
    closeBtn->setFixedSize(36, 36);
    closeBtn->move(containerWidth - 46, 5);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(
        "QPushButton {"
        "  background: rgba(0, 0, 0, 0.6);"
        "  border: none;"
        "  border-radius: 18px;"
        "  color: white;"
        "  font-size: 16px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "  background: rgba(239, 68, 68, 0.8);"
        "}"
        );

    int x = (screenRect.width() - containerWidth) / 2;
    int y = (screenRect.height() - containerHeight) / 2;
    m_photoContainer->move(x, y);

    connect(closeBtn, &QPushButton::clicked, this, &PhotoViewer::close);
    installEventFilter(this);
}

PhotoViewer::~PhotoViewer() {}

void PhotoViewer::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.fillRect(rect(), QColor(0, 0, 0, 220));
    QDialog::paintEvent(event);
}

void PhotoViewer::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenRect = screen->geometry();
    setGeometry(screenRect);
    if (m_photoContainer) {
        int x = (screenRect.width() - m_photoContainer->width()) / 2;
        int y = (screenRect.height() - m_photoContainer->height()) / 2;
        m_photoContainer->move(x, y);
    }
}

bool PhotoViewer::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (m_photoContainer) {
            QPoint localPos = m_photoContainer->mapFromGlobal(mouseEvent->globalPos());
            if (!m_photoContainer->rect().contains(localPos)) {
                close();
                return true;
            }
        }
    }
    return QDialog::eventFilter(obj, event);
}

void PhotoViewer::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        close();
    }
    QDialog::keyPressEvent(event);
}
