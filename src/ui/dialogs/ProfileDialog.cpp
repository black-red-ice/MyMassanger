#include "ProfileDialog.h"
#include "EditProfileDialog.h"
#include "mainwindow.h"
#include "PhotoViewer.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QIcon>
#include <QPixmap>
#include <QSettings>
#include <QFile>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QKeyEvent>
#include <functional>
#include <QScreen>
#include <QGuiApplication>
#include <QSharedPointer>
#include <QKeyEvent>
#include <QStandardPaths>
#include <QFileInfo>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QDir>
#include <QImage>
#include <QDialog>

ProfileDialog::ProfileDialog(QWidget *parent)
    : OverlayDialog(parent)
{
    setFixedSize(430, 520);
    setStyleSheet("background: transparent;");

    QSettings settings("Aura", "Messenger");
    QString defaultName = settings.value("savedUsername", "-").toString();

    MainWindow *mw = qobject_cast<MainWindow*>(this->parentWidget()->window());
    qint64 userId = mw ? mw->getCurrentUserId() : 0;

    // 🔥 avatarKey определяется один раз в начале
    QString avatarKey = "userAvatar_" + QString::number(userId);
    QString savedAvatar = settings.value(avatarKey).toString();

    qDebug() << "=== ProfileDialog constructor userId:" << userId;
    qDebug() << "savedAvatar:" << savedAvatar;

    QVBoxLayout *dialogLayout = new QVBoxLayout(this);
    dialogLayout->setContentsMargins(0, 0, 0, 0);
    dialogLayout->setSpacing(0);

    QWidget *container = new QWidget();
    container->setObjectName("profileContainer");
    container->setStyleSheet(
        "#profileContainer { background: #1e293b; border-radius: 18px; border: 1px solid #334155; }"
        );
    dialogLayout->addWidget(container);

    QVBoxLayout *cl = new QVBoxLayout(container);
    cl->setContentsMargins(0, 0, 0, 0);
    cl->setSpacing(0);

    QWidget *header = new QWidget();
    header->setStyleSheet(
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #1d4ed8, stop:1 #2563eb);"
        "border-top-left-radius: 18px; border-top-right-radius: 18px;"
        );
    header->setFixedHeight(60);

    QHBoxLayout *hl = new QHBoxLayout(header);
    hl->setContentsMargins(24, 0, 24, 0);

    QHBoxLayout *titleLayout = new QHBoxLayout();
    titleLayout->setSpacing(10);
    QLabel *headerIcon = new QLabel();
    headerIcon->setFixedSize(20, 20);
    headerIcon->setStyleSheet("background: transparent;");
    QPixmap iconPixmap(":/icons/general/images/general/user-light.svg");
    if (!iconPixmap.isNull()) {
        headerIcon->setPixmap(iconPixmap.scaled(20, 20, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    QLabel *title = new QLabel("Мой профиль");
    title->setStyleSheet("color: white; font-size: 18px; font-weight: 600; background: transparent;");
    titleLayout->addWidget(headerIcon);
    titleLayout->addWidget(title);
    hl->addLayout(titleLayout);
    hl->addStretch();

    QPushButton *closeBtn = new QPushButton("✕");
    closeBtn->setFixedSize(36, 36);
    closeBtn->setStyleSheet(
        "QPushButton { background: rgba(255,255,255,0.1); border: none; border-radius: 18px; color: white; font-size: 16px; }"
        "QPushButton:hover { background: rgba(255,255,255,0.2); }"
        );
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);
    hl->addWidget(closeBtn);
    cl->addWidget(header);

    QWidget *content = new QWidget();
    content->setStyleSheet("background: #1e293b; border-bottom-left-radius: 18px; border-bottom-right-radius: 18px;");

    QVBoxLayout *ccl = new QVBoxLayout(content);
    ccl->setContentsMargins(20, 20, 20, 20);
    ccl->setSpacing(0);
    ccl->addStretch();

    QWidget *avatarContainer = new QWidget();
    QHBoxLayout *avatarLayout = new QHBoxLayout(avatarContainer);
    avatarLayout->setAlignment(Qt::AlignCenter);

    m_avatarButton = new QPushButton();
    m_avatarButton->setFixedSize(80, 80);
    m_avatarButton->setCursor(Qt::PointingHandCursor);

    auto setDefaultAvatar = [this]() {
        QPixmap defaultPixmap(":/icons/darkTheme/images/darkTheme/user.svg");
        if (!defaultPixmap.isNull()) {
            m_avatarButton->setIcon(QIcon(defaultPixmap));
            m_avatarButton->setIconSize(QSize(40, 40));
            m_avatarButton->setText("");
            m_avatarButton->setStyleSheet("QPushButton { background: #1d4ed8; border-radius: 12px; border: none; } QPushButton:hover { background: #2563eb; }");
        } else {
            m_avatarButton->setText("AI");
            m_avatarButton->setStyleSheet("QPushButton { background: #1d4ed8; border-radius: 12px; border: none; color: white; font-size: 24px; font-weight: bold; } QPushButton:hover { background: #2563eb; }");
        }
    };

    // Загружаем аватар
    bool avatarSet = false;
    if (!savedAvatar.isEmpty()) {
        QString finalPath = savedAvatar;
        if (savedAvatar.startsWith("/files/")) {
            QString cachePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
            + "/avatars/" + QString::number(userId) + ".jpg";
            if (QFile::exists(cachePath)) {
                finalPath = cachePath;
            } else if (mw) {
                mw->requestUserAvatar(userId);
                setDefaultAvatar();
                avatarSet = true;
            }
        }

        if (!avatarSet && QFile::exists(finalPath)) {
            QPixmap pixmap(finalPath);
            if (!pixmap.isNull()) {
                QPixmap rounded = makeRoundedPixmap(pixmap, 80);
                m_avatarButton->setIcon(QIcon(rounded));
                m_avatarButton->setIconSize(QSize(80, 80));
                m_avatarButton->setText("");
                m_avatarButton->setStyleSheet("QPushButton { background: transparent; border-radius: 12px; border: none; }");
                avatarSet = true;
            }
        }
    }

    if (!avatarSet) {
        setDefaultAvatar();
    }

    connect(m_avatarButton, &QPushButton::clicked, this, [this, avatarKey, userId]() {
        QSettings s("Aura", "Messenger");
        QString path = s.value(avatarKey).toString();

        if (path.startsWith("/files/") || path.startsWith("http")) {
            QString cachePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
            + "/avatars/" + QString::number(userId) + ".jpg";
            if (QFile::exists(cachePath)) {
                path = cachePath;
            }
        }

        if (!path.isEmpty() && QFile::exists(path)) {
            PhotoViewer *v = new PhotoViewer(path, this);
            v->setAttribute(Qt::WA_DeleteOnClose, true);
            v->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
            v->setModal(true);
            v->show();
            connect(v, &QDialog::finished, this, [this]() {
                show();
                raise();
                activateWindow();
            });
        }
    });

    avatarLayout->addWidget(m_avatarButton);
    ccl->addWidget(avatarContainer);
    ccl->addSpacing(16);

    m_nameLabel = new QLabel(settings.value("profile/name", defaultName).toString());
    m_nameLabel->setStyleSheet("color: #f1f5f9; font-size: 18px; font-weight: bold; background: transparent;");
    m_nameLabel->setAlignment(Qt::AlignCenter);
    ccl->addWidget(m_nameLabel);
    ccl->addSpacing(4);

    m_positionLabel = new QLabel(settings.value("profile/position", "-").toString());
    m_positionLabel->setStyleSheet("color: #94A3B8; font-size: 14px; background: transparent;");
    m_positionLabel->setAlignment(Qt::AlignCenter);
    ccl->addWidget(m_positionLabel);
    ccl->addSpacing(20);

    QWidget *infoGrid = new QWidget();
    QHBoxLayout *gridLayout = new QHBoxLayout(infoGrid);
    gridLayout->setSpacing(12);

    auto createField = [&](const QString &label, QLabel *value) -> QWidget* {
        QWidget *w = new QWidget();
        QVBoxLayout *l = new QVBoxLayout(w);
        l->setAlignment(Qt::AlignCenter);
        l->setSpacing(4);
        QLabel *lbl = new QLabel(label);
        lbl->setAlignment(Qt::AlignCenter);
        lbl->setStyleSheet("color: #94A3B8; font-size: 12px; background: transparent;");
        value->setAlignment(Qt::AlignCenter);
        value->setStyleSheet("color: #f1f5f9; font-size: 14px; font-weight: bold; background: transparent;");
        l->addWidget(lbl);
        l->addWidget(value);
        return w;
    };

    QVBoxLayout *leftCol = new QVBoxLayout();
    leftCol->setSpacing(12);
    m_deptValue = new QLabel(settings.value("profile/department", "-").toString());
    m_emailValue = new QLabel(settings.value("profile/email", "-").toString());
    m_emailValue->setWordWrap(true);
    leftCol->addWidget(createField("Отдел", m_deptValue));
    leftCol->addWidget(createField("Email", m_emailValue));
    gridLayout->addLayout(leftCol);

    QVBoxLayout *rightCol = new QVBoxLayout();
    rightCol->setSpacing(12);
    m_tabValue = new QLabel(settings.value("profile/tabNumber", "-").toString());
    m_phoneValue = new QLabel(settings.value("profile/phone", "-").toString());
    rightCol->addWidget(createField("Табельный", m_tabValue));
    rightCol->addWidget(createField("Телефон", m_phoneValue));
    gridLayout->addLayout(rightCol);

    ccl->addWidget(infoGrid);
    ccl->addSpacing(12);

    // Получаем значения ДО создания лямбды
    QString currentName = settings.value("profile/name", defaultName).toString();
    QString currentPosition = settings.value("profile/position", "-").toString();
    QString currentDepartment = settings.value("profile/department", "-").toString();
    QString currentTabNumber = settings.value("profile/tabNumber", "-").toString();
    QString currentEmail = settings.value("profile/email", "-").toString();
    QString currentPhone = settings.value("profile/phone", "-").toString();
    QString currentAvatarPath = settings.value(avatarKey).toString();

    QPushButton *editBtn = new QPushButton("Редактировать профиль");
    editBtn->setFixedHeight(44);
    editBtn->setCursor(Qt::PointingHandCursor);
    QPixmap editPixmap(":/icons/general/images/general/pen-to-square.svg");
    if (!editPixmap.isNull()) {
        editBtn->setIcon(QIcon(editPixmap));
        editBtn->setIconSize(QSize(18, 18));
    }
    editBtn->setStyleSheet("QPushButton { background: #1d4ed8; border: none; border-radius: 10px; color: white; font-size: 14px; font-weight: 600; } QPushButton:hover { background: #2563eb; }");

    connect(editBtn, &QPushButton::clicked, this, [this, avatarKey, userId,
                                                   currentName, currentPosition, currentDepartment, currentTabNumber,
                                                   currentEmail, currentPhone, currentAvatarPath, defaultName]() {

        QWidget *dimWidget = new QWidget(this->parentWidget());
        dimWidget->setObjectName("editDim");
        dimWidget->setStyleSheet("#editDim { background-color: rgba(0, 0, 0, 180); }");
        dimWidget->setGeometry(this->parentWidget()->rect());
        dimWidget->raise();
        dimWidget->show();

        EditProfileDialog dlg(dimWidget);
        dlg.setAvatarKey(avatarKey);
        MainWindow *mw = qobject_cast<MainWindow*>(this->parentWidget()->window());
        dlg.setMainWindow(mw);

        dlg.setName(currentName);
        dlg.setPosition(currentPosition);
        dlg.setDepartment(currentDepartment);
        dlg.setTabNumber(currentTabNumber);
        dlg.setEmail(currentEmail);
        dlg.setPhone(currentPhone);
        dlg.setAvatarPath(currentAvatarPath);

        if (dlg.exec() == QDialog::Accepted) {
            QSettings settings("Aura", "Messenger");
            settings.setValue("profile/name", dlg.getName());
            settings.setValue("profile/position", dlg.getPosition());
            settings.setValue("profile/department", dlg.getDepartment());
            settings.setValue("profile/tabNumber", dlg.getTabNumber());
            settings.setValue("profile/email", dlg.getEmail());
            settings.setValue("profile/phone", dlg.getPhone());

            QString newAvatarPath = dlg.getAvatarPath();
            if (!newAvatarPath.isEmpty()) {
                QString avatarKeyForUser = "userAvatar_" + QString::number(userId);
                settings.setValue(avatarKeyForUser, newAvatarPath);
            }

            MainWindow *mainWin = qobject_cast<MainWindow*>(this->parentWidget()->window());
            if (mainWin) {
                mainWin->saveProfileToServer();
            }

            m_nameLabel->setText(dlg.getName());
            m_positionLabel->setText(dlg.getPosition());
            m_deptValue->setText(dlg.getDepartment());
            m_tabValue->setText(dlg.getTabNumber());
            m_emailValue->setText(dlg.getEmail());
            m_phoneValue->setText(dlg.getPhone());

            if (!newAvatarPath.isEmpty() && QFile::exists(newAvatarPath)) {
                QPixmap pixmap(newAvatarPath);

                if (!pixmap.isNull()) {
                    QPixmap rounded = makeRoundedPixmap(pixmap, 80);

                    m_avatarButton->setIcon(QIcon(rounded));
                    m_avatarButton->setIconSize(QSize(80, 80));
                    m_avatarButton->setText("");

                    m_avatarButton->setStyleSheet(
                        "QPushButton { background: transparent; border-radius: 12px; border: none; }"
                        );
                }

                // ТОЛЬКО upload
                if (mw) {
                    mw->uploadAvatarToServer(newAvatarPath);
                }
            } else {
                // Аватар не менялся — просто сохраняем профиль
                if (mw) {
                    mw->saveProfileToServer();
                }
            }
        }

        dimWidget->deleteLater();
    });

    QPushButton *logoutBtn = new QPushButton("Выйти из профиля");
    logoutBtn->setFixedHeight(44);
    logoutBtn->setCursor(Qt::PointingHandCursor);
    QPixmap logoutPixmap(":/icons/general/images/general/right-from-bracket.svg");
    if (!logoutPixmap.isNull()) {
        logoutBtn->setIcon(QIcon(logoutPixmap));
        logoutBtn->setIconSize(QSize(18, 18));
    }
    logoutBtn->setStyleSheet("QPushButton { background: #EF4444; border: none; border-radius: 10px; color: white; font-size: 14px; font-weight: 600; } QPushButton:hover { background: #DC2626; }");
    connect(logoutBtn, &QPushButton::clicked, this, [this, avatarKey]() {
        QSettings s("Aura", "Messenger");
        s.remove("profile/name"); s.remove("profile/position"); s.remove("profile/department");
        s.remove("profile/tabNumber"); s.remove("profile/email"); s.remove("profile/phone");
        s.remove(avatarKey); s.remove("rememberMe"); s.remove("savedUsername"); s.remove("savedPassword");
        reject();
        emit logoutRequested();
    });

    ccl->addWidget(editBtn);
    ccl->addSpacing(8);
    ccl->addWidget(logoutBtn);
    cl->addWidget(content);

    setFixedSize(430, 520);

    editBtn->setFocusPolicy(Qt::TabFocus);
    logoutBtn->setFocusPolicy(Qt::TabFocus);
    m_avatarButton->setFocusPolicy(Qt::TabFocus);

    qDebug() << "=== ProfileDialog constructor userId:" << userId;
}

QPixmap ProfileDialog::makeRoundedPixmap(const QPixmap &source, int size)
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

void ProfileDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        event->ignore();
        return;
    }
    if (event->key() == Qt::Key_Escape) {
        reject();
        return;
    }
    QDialog::keyPressEvent(event);
}

bool ProfileDialog::event(QEvent *event)
{
    if (event->type() == QEvent::ShortcutOverride ||
        event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Return ||
            keyEvent->key() == Qt::Key_Enter) {
            event->ignore();
            return true;
        }
    }
    return QDialog::event(event);
}
