#include "UserProfileDialog.h"
#include "PhotoViewer.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPixmap>
#include <QFile>
#include <QPainter>
#include <QPainterPath>
#include <QJsonObject>
#include <QStandardPaths>

UserProfileDialog::UserProfileDialog(int userId, const QString &username,
                                     const QString &avatarPath, bool online,
                                     QJsonObject profileData,
                                     QWidget *parent)
    : OverlayDialog(parent)
    , m_userId(userId)
    , m_avatarPath(avatarPath)
{
    setFixedSize(420, 460);
    setStyleSheet("background: transparent;");

    QWidget *container = new QWidget(this);
    container->setObjectName("userProfileContainer");
    container->setStyleSheet(
        "#userProfileContainer { background: #1e293b; border-radius: 18px; border: 1px solid #334155; }"
        );

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(container);

    QVBoxLayout *layout = new QVBoxLayout(container);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(0);

    // Аватар
    m_avatarButton = new QPushButton();
    m_avatarButton->setFixedSize(80, 80);
    m_avatarButton->setCursor(Qt::PointingHandCursor);

    if (!avatarPath.isEmpty() && QFile::exists(avatarPath)) {
        QPixmap pixmap(avatarPath);
        if (!pixmap.isNull()) {
            QPixmap rounded = makeRoundedPixmap(pixmap, 80);
            m_avatarButton->setIcon(QIcon(rounded));
            m_avatarButton->setIconSize(QSize(80, 80));
            m_avatarButton->setStyleSheet("background: transparent; border: none;");
        } else {
            m_avatarButton->setText(username.left(1).toUpper());
            m_avatarButton->setStyleSheet(
                "QPushButton { background: #1d4ed8; border-radius: 12px; border: none; color: white; font-size: 28px; font-weight: bold; }"
                );
        }
    } else {
        m_avatarButton->setText(username.left(1).toUpper());
        m_avatarButton->setStyleSheet(
            "QPushButton { background: #1d4ed8; border-radius: 12px; border: none; color: white; font-size: 28px; font-weight: bold; }"
            );
    }

    connect(m_avatarButton, &QPushButton::clicked, this, [this, avatarPath]() {
        if (!avatarPath.isEmpty() && QFile::exists(avatarPath)) {
            showFullPhoto(avatarPath);
        }
    });

    QHBoxLayout *avatarRow = new QHBoxLayout();
    avatarRow->setAlignment(Qt::AlignCenter);
    avatarRow->addWidget(m_avatarButton);
    layout->addLayout(avatarRow);
    layout->addSpacing(16);

    // Имя
    m_nameLabel = new QLabel(username);
    m_nameLabel->setAlignment(Qt::AlignCenter);
    m_nameLabel->setStyleSheet("color: #f1f5f9; font-size: 20px; font-weight: bold; background: transparent;");
    layout->addWidget(m_nameLabel);
    layout->addSpacing(8);

    // Должность
    QString position = profileData["position"].toString();
    QLabel *positionLabel = new QLabel(position.isEmpty() ? "—" : position);
    positionLabel->setAlignment(Qt::AlignCenter);
    positionLabel->setStyleSheet("color: #94A3B8; font-size: 14px; background: transparent;");
    layout->addWidget(positionLabel);
    layout->addSpacing(8);

    // Статус
    m_statusLabel = new QLabel(online ? "🟢 В сети" : "⚫ Не в сети");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet(
        QString("color: %1; font-size: 14px; background: transparent;")
            .arg(online ? "#10B981" : "#94A3B8")
        );
    layout->addWidget(m_statusLabel);
    layout->addSpacing(20);

    // Сетка 2x2 с данными
    QWidget *infoGrid = new QWidget();
    QHBoxLayout *gridLayout = new QHBoxLayout(infoGrid);
    gridLayout->setSpacing(12);

    auto createField = [](const QString &label, const QString &value) -> QWidget* {
        QWidget *w = new QWidget();
        QVBoxLayout *l = new QVBoxLayout(w);
        l->setAlignment(Qt::AlignCenter);
        l->setSpacing(4);
        QLabel *lbl = new QLabel(label);
        lbl->setAlignment(Qt::AlignCenter);
        lbl->setStyleSheet("color: #94A3B8; font-size: 12px; background: transparent;");
        QLabel *val = new QLabel(value.isEmpty() ? "—" : value);
        val->setAlignment(Qt::AlignCenter);
        val->setStyleSheet("color: #f1f5f9; font-size: 14px; font-weight: bold; background: transparent;");
        l->addWidget(lbl);
        l->addWidget(val);
        return w;
    };

    QVBoxLayout *leftCol = new QVBoxLayout();
    leftCol->setSpacing(12);
    leftCol->addWidget(createField("Отдел", profileData["department"].toString()));
    leftCol->addWidget(createField("Email", profileData["email"].toString()));
    gridLayout->addLayout(leftCol);

    QVBoxLayout *rightCol = new QVBoxLayout();
    rightCol->setSpacing(12);
    rightCol->addWidget(createField("Табельный", profileData["tabNumber"].toString()));
    rightCol->addWidget(createField("Телефон", profileData["phone"].toString()));
    gridLayout->addLayout(rightCol);

    layout->addWidget(infoGrid);
    layout->addSpacing(24);

    // Кнопка закрыть
    QPushButton *closeBtn = new QPushButton("Закрыть");
    closeBtn->setFixedHeight(40);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(
        "QPushButton { background: #334155; border: none; border-radius: 10px; color: #cbd5e1; font-size: 14px; font-weight: 500; }"
        "QPushButton:hover { background: #475569; }"
        );
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);
    layout->addWidget(closeBtn);

    loadAvatar();
}

void UserProfileDialog::loadAvatar()
{
    if (!m_avatarButton) return;

    QPixmap pixmap;
    QString finalPath = m_avatarPath;

    // Пробуем загрузить из кеша если это URL
    if (finalPath.startsWith("/files/")) {
        QString cachePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
        + "/avatars/" + QString::number(m_userId) + ".jpg";
        if (QFile::exists(cachePath)) {
            finalPath = cachePath;
            qDebug() << "Loading avatar from cache:" << finalPath;
        } else {
            qDebug() << "Avatar not in cache yet for user:" << m_userId;
            // Можно оставить первую букву
            m_avatarButton->setText(QString::number(m_userId).left(1).toUpper());
            return;
        }
    }

    if (!finalPath.isEmpty() && QFile::exists(finalPath)) {
        pixmap.load(finalPath);
        if (!pixmap.isNull()) {
            QPixmap rounded = makeRoundedPixmap(pixmap, 80);
            m_avatarButton->setIcon(QIcon(rounded));
            m_avatarButton->setIconSize(QSize(80, 80));
            m_avatarButton->setText("");
            m_avatarButton->setStyleSheet("background: transparent; border: none;");
            qDebug() << "Avatar loaded successfully in dialog";
            return;
        }
    }

    // Если не загрузили - показываем первую букву
    QString name = m_nameLabel->text();
    if (name.isEmpty()) {
        name = QString::number(m_userId);
    }
    m_avatarButton->setText(name.left(1).toUpper());
    m_avatarButton->setIcon(QIcon());
    m_avatarButton->setStyleSheet(
        "QPushButton { background: #1d4ed8; border-radius: 12px; border: none; color: white; font-size: 28px; font-weight: bold; }"
        );
}

QPixmap UserProfileDialog::makeRoundedPixmap(const QPixmap &source, int size)
{
    QPixmap scaled = source.scaled(size, size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    int x = (scaled.width() - size) / 2;
    int y = (scaled.height() - size) / 2;
    QPixmap square = scaled.copy(x, y, size, size);

    QPixmap rounded(size, size);
    rounded.fill(Qt::transparent);
    QPainter painter(&rounded);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QPainterPath path;
    path.addRoundedRect(0, 0, size, size, 12, 12);
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, square);
    painter.end();
    return rounded;
}

void UserProfileDialog::showFullPhoto(const QString &imagePath)
{
    PhotoViewer *viewer = new PhotoViewer(imagePath, this);
    viewer->setAttribute(Qt::WA_DeleteOnClose);
    viewer->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    viewer->show();
}
