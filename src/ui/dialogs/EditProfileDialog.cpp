#include "mainwindow.h"
#include "EditProfileDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QPixmap>
#include <QIcon>
#include <QFileDialog>
#include <QSettings>
#include <QKeyEvent>
#include <QStandardPaths>
#include <QDir>

EditProfileDialog::EditProfileDialog(QWidget *parent)
    : OverlayDialog(parent)
{
    setFixedSize(450, 610);  // Увеличили высоту
    setStyleSheet("background: transparent;");

    QVBoxLayout *dialogLayout = new QVBoxLayout(this);
    dialogLayout->setContentsMargins(0, 0, 0, 0);
    dialogLayout->setSpacing(0);

    QWidget *container = new QWidget();
    container->setObjectName("editProfileContainer");
    container->setStyleSheet(
        "#editProfileContainer { background: #1e293b; border-radius: 18px; border: 1px solid #334155; }"
        );
    dialogLayout->addWidget(container);

    QVBoxLayout *cl = new QVBoxLayout(container);
    cl->setContentsMargins(0, 0, 0, 0);
    cl->setSpacing(0);

    // Заголовок
    QWidget *header = new QWidget();
    header->setStyleSheet(
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #1d4ed8, stop:1 #2563eb);"
        "border-top-left-radius: 18px;"
        "border-top-right-radius: 18px;"
        );
    header->setFixedHeight(60);

    QHBoxLayout *hl = new QHBoxLayout(header);
    hl->setContentsMargins(24, 0, 24, 0);

    QLabel *title = new QLabel("Редактирование профиля");
    title->setStyleSheet("color: white; font-size: 18px; font-weight: 600; background: transparent;");

    QPushButton *closeBtn = new QPushButton("✕");
    closeBtn->setFixedSize(36, 36);
    closeBtn->setStyleSheet(
        "QPushButton { background: rgba(255,255,255,0.1); border: none; border-radius: 18px; color: white; font-size: 16px; }"
        "QPushButton:hover { background: rgba(255,255,255,0.2); }"
        );
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);

    hl->addWidget(title);
    hl->addStretch();
    hl->addWidget(closeBtn);
    cl->addWidget(header);

    // Контент
    QWidget *content = new QWidget();
    content->setStyleSheet(
        "background: #1e293b;"
        "border-bottom-left-radius: 18px;"
        "border-bottom-right-radius: 18px;"
        );

    QVBoxLayout *formLayout = new QVBoxLayout(content);
    formLayout->setContentsMargins(24, 30, 24, 24);
    formLayout->setSpacing(0);

    // Аватар с возможностью смены
    QWidget *avatarContainer = new QWidget();
    QHBoxLayout *avatarLayout = new QHBoxLayout(avatarContainer);
    avatarLayout->setAlignment(Qt::AlignCenter);
    avatarLayout->setContentsMargins(0, 0, 0, 10);

    m_avatarButton = new QPushButton();
    m_avatarButton->setFixedSize(90, 90);
    m_avatarButton->setCursor(Qt::PointingHandCursor);

    // Функция для установки аватара по умолчанию
    auto setDefaultAvatar = [this]() {
        QPixmap defaultPixmap(":/icons/darkTheme/images/darkTheme/user.svg");
        if (!defaultPixmap.isNull()) {
            m_avatarButton->setIcon(QIcon(defaultPixmap));
            m_avatarButton->setIconSize(QSize(45, 45));
            m_avatarButton->setText("");
            m_avatarButton->setStyleSheet(
                "QPushButton {"
                "  background: #1d4ed8;"
                "  border-radius: 12px;"
                "  border: 3px solid #334155;"
                "}"
                "QPushButton:hover {"
                "  background: #2563eb;"
                "  border-color: #1d4ed8;"
                "}"
                );
        } else {
            m_avatarButton->setText("AI");
            m_avatarButton->setStyleSheet(
                "QPushButton {"
                "  background: #1d4ed8;"
                "  border-radius: 12px;"
                "  border: 3px solid #334155;"
                "  color: white;"
                "  font-size: 28px;"
                "  font-weight: bold;"
                "}"
                "QPushButton:hover {"
                "  background: #2563eb;"
                "  border-color: #1d4ed8;"
                "}"
                );
        }
    };

    setDefaultAvatar();

    connect(m_avatarButton, &QPushButton::clicked, this, [this, setDefaultAvatar]() {
        QString fileName = QFileDialog::getOpenFileName(
            this,
            "Выберите фото профиля",
            QString(),
            "Изображения (*.png *.jpg *.jpeg *.bmp *.svg)");

        if (!fileName.isEmpty()) {
            QPixmap pixmap(fileName);

            if (!pixmap.isNull()) {
                m_avatarPath = fileName;

                QPixmap rounded =
                    makeRoundedPixmap(pixmap, 90);

                m_avatarButton->setIcon(QIcon(rounded));
                m_avatarButton->setIconSize(QSize(90, 90));
                m_avatarButton->setText("");

                m_avatarButton->setStyleSheet(
                    "QPushButton { border-radius: 12px; border: 3px solid #334155; }"
                    "QPushButton:hover { border-color: #1d4ed8; }"
                    );
            }
        }
    });

    avatarLayout->addWidget(m_avatarButton);
    formLayout->addWidget(avatarContainer);
    formLayout->addSpacing(4);

    // Подсказка под аватаром
    QLabel *avatarHint = new QLabel("Нажмите на фото, чтобы изменить");
    avatarHint->setAlignment(Qt::AlignCenter);
    avatarHint->setStyleSheet("color: #64748B; font-size: 11px; background: transparent; margin-bottom: 20px;");
    formLayout->addWidget(avatarHint);

    // Поля ввода
    auto createField = [](const QString &label, QWidget *widget) -> QVBoxLayout* {
        QVBoxLayout *layout = new QVBoxLayout();
        layout->setSpacing(4);
        QLabel *lbl = new QLabel(label);
        lbl->setStyleSheet("color: #94A3B8; font-size: 12px; background: transparent;");
        layout->addWidget(lbl);
        layout->addWidget(widget);
        return layout;
    };

    QString lineEditStyle =
        "QLineEdit { background: #0f172a; border: 1px solid #334155; border-radius: 10px; color: #f1f5f9; padding: 12px 16px; font-size: 14px; }"
        "QLineEdit:focus { border-color: #1d4ed8; }";

    QString comboBoxStyle =
        "QComboBox { background: #0f172a; border: 1px solid #334155; border-radius: 10px; color: #f1f5f9; padding: 12px 16px; font-size: 14px; }"
        "QComboBox::drop-down { border: none; width: 30px; }"
        "QComboBox QAbstractItemView { background: #0f172a; color: #f1f5f9; border: 1px solid #334155; selection-background-color: #1d4ed8; }";

    // Имя
    m_name = new QLineEdit();
    m_name->setText("Александр Иванов");
    m_name->setStyleSheet(lineEditStyle);
    m_name->setFixedHeight(44);
    formLayout->addLayout(createField("Имя", m_name));
    formLayout->addSpacing(12);

    // Должность
    m_position = new QLineEdit();
    m_position->setText("Senior Developer");
    m_position->setStyleSheet(lineEditStyle);
    m_position->setFixedHeight(44);
    formLayout->addLayout(createField("Должность", m_position));
    formLayout->addSpacing(12);

    // Отдел и Табельный
    QHBoxLayout *row1 = new QHBoxLayout();
    row1->setSpacing(12);

    QVBoxLayout *deptCol = new QVBoxLayout();
    deptCol->setSpacing(4);
    QLabel *deptLabel = new QLabel("Отдел");
    deptLabel->setStyleSheet("color: #94A3B8; font-size: 12px; background: transparent;");
    m_department = new QComboBox();
    m_department->addItems({"IT", "Продажи", "HR", "Финансы"});
    m_department->setCurrentText("IT");
    m_department->setStyleSheet(comboBoxStyle);
    m_department->setFixedHeight(44);
    deptCol->addWidget(deptLabel);
    deptCol->addWidget(m_department);
    row1->addLayout(deptCol, 1);

    QVBoxLayout *tabCol = new QVBoxLayout();
    tabCol->setSpacing(4);
    QLabel *tabLabel = new QLabel("Табельный");
    tabLabel->setStyleSheet("color: #94A3B8; font-size: 12px; background: transparent;");
    m_tabNumber = new QLineEdit();
    m_tabNumber->setText("#IT-007");
    m_tabNumber->setStyleSheet(lineEditStyle);
    m_tabNumber->setFixedHeight(44);
    tabCol->addWidget(tabLabel);
    tabCol->addWidget(m_tabNumber);
    row1->addLayout(tabCol, 1);

    formLayout->addLayout(row1);
    formLayout->addSpacing(12);

    // Email и Телефон
    QHBoxLayout *row2 = new QHBoxLayout();
    row2->setSpacing(12);

    QVBoxLayout *emailCol = new QVBoxLayout();
    emailCol->setSpacing(4);
    QLabel *emailLabel = new QLabel("Email");
    emailLabel->setStyleSheet("color: #94A3B8; font-size: 12px; background: transparent;");
    m_email = new QLineEdit();
    m_email->setText("a.ivanov@company.com");
    m_email->setStyleSheet(lineEditStyle);
    m_email->setFixedHeight(44);
    emailCol->addWidget(emailLabel);
    emailCol->addWidget(m_email);
    row2->addLayout(emailCol, 1);

    QVBoxLayout *phoneCol = new QVBoxLayout();
    phoneCol->setSpacing(4);
    QLabel *phoneLabel = new QLabel("Телефон");
    phoneLabel->setStyleSheet("color: #94A3B8; font-size: 12px; background: transparent;");
    m_phone = new QLineEdit();
    m_phone->setText("+7 (999) 123-45-67");
    m_phone->setStyleSheet(lineEditStyle);
    m_phone->setFixedHeight(44);
    phoneCol->addWidget(phoneLabel);
    phoneCol->addWidget(m_phone);
    row2->addLayout(phoneCol, 1);

    formLayout->addLayout(row2);
    formLayout->addSpacing(24);

    // Форматирование телефона
    connect(m_phone, &QLineEdit::textChanged, this, [this](const QString &text) {
        // Блокируем сигналы, чтобы избежать рекурсии
        m_phone->blockSignals(true);

        // Оставляем только цифры и +
        QString digits;
        for (const QChar &ch : text) {
            if (ch.isDigit() || ch == '+') {
                digits += ch;
            }
        }

        // Форматируем
        QString formatted;
        if (digits.startsWith("+7") || digits.startsWith("7") || digits.startsWith("8")) {
            if (digits.startsWith("8")) digits = "+7" + digits.mid(1);
            if (digits.startsWith("7")) digits = "+7" + digits.mid(1);

            formatted = digits.left(2); // +7
            if (digits.length() > 2) formatted += " (" + digits.mid(2, 3);
            if (digits.length() > 5) formatted += ") " + digits.mid(5, 3);
            if (digits.length() > 8) formatted += "-" + digits.mid(8, 2);
            if (digits.length() > 10) formatted += "-" + digits.mid(10, 2);
        } else {
            formatted = digits;
        }

        m_phone->setText(formatted);

        // Ставим курсор в конец
        m_phone->setCursorPosition(formatted.length());

        m_phone->blockSignals(false);
    });

    // Кнопки
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(12);

    QPushButton *cancelBtn = new QPushButton("Отмена");
    cancelBtn->setFixedHeight(44);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setAutoDefault(false);
    cancelBtn->setStyleSheet(
        "QPushButton { background: #334155; border: none; border-radius: 10px; color: #cbd5e1; font-size: 14px; font-weight: 500; }"
        "QPushButton:hover { background: #475569; }"
        );
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    QPushButton *saveBtn = new QPushButton("Сохранить");
    saveBtn->setFixedHeight(44);
    saveBtn->setCursor(Qt::PointingHandCursor);
    saveBtn->setDefault(true);        // ← Добавить
    saveBtn->setAutoDefault(true);
    saveBtn->setStyleSheet(
        "QPushButton { background: #1d4ed8; border: none; border-radius: 10px; color: white; font-size: 14px; font-weight: 600; }"
        "QPushButton:hover { background: #2563eb; }"
        );
    connect(saveBtn, &QPushButton::clicked, this, [this]() {
        accept();
    });

    saveBtn->setDefault(true);     // Enter будет нажимать "Сохранить"
    saveBtn->setAutoDefault(true);
    cancelBtn->setAutoDefault(false);

    buttonLayout->addWidget(cancelBtn, 1);
    buttonLayout->addWidget(saveBtn, 1);
    formLayout->addLayout(buttonLayout);

    // Устанавливаем порядок табуляции - "Сохранить" будет последним
    setTabOrder(cancelBtn, saveBtn);

    // Устанавливаем фокус на первое поле ввода
    m_name->setFocus();

    // Явно указываем, что saveBtn - кнопка по умолчанию
    saveBtn->setDefault(true);
    saveBtn->setAutoDefault(true);
    cancelBtn->setAutoDefault(false);

    cl->addWidget(content);
}

QPixmap EditProfileDialog::makeRoundedPixmap(const QPixmap &source, int size)
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
    path.addRoundedRect(0, 0, size, size, 12, 12);  // ← Заменили addEllipse на addRoundedRect
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, square);
    painter.end();

    return rounded;
}

void EditProfileDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        accept();
        return;
    }
    QDialog::keyPressEvent(event);
}

QString EditProfileDialog::getName() const { return m_name->text(); }
QString EditProfileDialog::getPosition() const { return m_position->text(); }
QString EditProfileDialog::getDepartment() const { return m_department->currentText(); }
QString EditProfileDialog::getEmail() const { return m_email->text(); }
QString EditProfileDialog::getPhone() const { return m_phone->text(); }
QString EditProfileDialog::getTabNumber() const { return m_tabNumber->text(); }
QString EditProfileDialog::getAvatarPath() const { return m_avatarPath; }

void EditProfileDialog::setName(const QString &name) { m_name->setText(name); }
void EditProfileDialog::setPosition(const QString &position) { m_position->setText(position); }
void EditProfileDialog::setDepartment(const QString &department) { m_department->setCurrentText(department); }
void EditProfileDialog::setEmail(const QString &email) { m_email->setText(email); }
void EditProfileDialog::setPhone(const QString &phone) { m_phone->setText(phone); }
void EditProfileDialog::setTabNumber(const QString &tabNumber) { m_tabNumber->setText(tabNumber); }
void EditProfileDialog::setAvatarPath(const QString &path)
{
    qDebug() << "=== EditProfileDialog::setAvatarPath:" << path;
    m_avatarPath = path;

    if (!path.isEmpty()) {
        QPixmap pixmap;
        QString loadPath = path;

        if (path.startsWith("/files/")) {
            // Серверный URL — пробуем кеш
            qint64 userId = m_mainWindow ? m_mainWindow->getCurrentUserId() : 0;
            QString cachePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
                                + "/avatars/" + QString::number(userId) + ".jpg";
            if (QFile::exists(cachePath)) {
                loadPath = cachePath;
                qDebug() << "Loading avatar from cache:" << loadPath;
            } else {
                qDebug() << "Avatar not in cache yet for user:" << userId;
                // Показываем первую букву
                m_avatarButton->setText(m_name->text().left(1).toUpper());
                m_avatarButton->setIcon(QIcon());
                m_avatarButton->setStyleSheet(
                    "QPushButton { background: #1d4ed8; border-radius: 12px; border: 3px solid #334155; color: white; font-size: 28px; font-weight: bold; }"
                    "QPushButton:hover { background: #2563eb; border-color: #1d4ed8; }"
                    );
                return;
            }
        }

        if (QFile::exists(loadPath)) {
            pixmap.load(loadPath);
            if (!pixmap.isNull()) {
                QPixmap rounded = makeRoundedPixmap(pixmap, 90);
                m_avatarButton->setIcon(QIcon(rounded));
                m_avatarButton->setIconSize(QSize(90, 90));
                m_avatarButton->setText("");
                m_avatarButton->setStyleSheet("QPushButton { border-radius: 12px; border: 3px solid #334155; }");
                return;
            }
        }
    }

    // Если не загрузили - показываем первую букву
    QString name = m_name ? m_name->text() : "";
    if (name.isEmpty()) {
        name = "User";
    }
    m_avatarButton->setText(name.left(1).toUpper());
    m_avatarButton->setIcon(QIcon());
    m_avatarButton->setStyleSheet(
        "QPushButton { background: #1d4ed8; border-radius: 12px; border: 3px solid #334155; color: white; font-size: 28px; font-weight: bold; }"
        "QPushButton:hover { background: #2563eb; border-color: #1d4ed8; }"
        );
}
void EditProfileDialog::setAvatarKey(const QString &key) { m_avatarKey = key; }
