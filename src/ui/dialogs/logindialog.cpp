#include "logindialog.h"
#include "mainwindow.h"
#include "../network/NetworkManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QCryptographicHash>
#include <QDebug>
#include <QTimer>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QMouseEvent>
#include <QScreen>
#include <QApplication>
#include <QIcon>
#include <QPainter>
#include <QStyleHints>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QKeyEvent>
#include <QSettings>
#include <QRegularExpression>
#include <QInputDialog>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

// Базовые размеры окон (будут масштабироваться с учётом DPI)
const int LOGIN_WINDOW_WIDTH = 420;
const int LOGIN_WINDOW_HEIGHT = 600;
const int REGISTER_FIRST_STEP_HEIGHT = 880;
const int REGISTER_SECOND_STEP_HEIGHT = 840;

// Функция для получения масштабированного размера
static int scaledSize(int size)
{
    QScreen *screen = QApplication::primaryScreen();
    qreal dpr = screen ? screen->devicePixelRatio() : 1.0;
    return qRound(size * dpr);
}

// ========== КЛАСС ДЛЯ ИНДИКАТОРА ШАГОВ ==========
class StepIndicator : public QWidget {
public:
    StepIndicator(QWidget *parent = nullptr) : QWidget(parent) {
        setFixedHeight(70);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        QWidget::paintEvent(event);
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        int centerY = 17;
        int firstCircleX = width() / 4 - 17;
        int secondCircleX = width() * 3 / 4 - 17;

        painter.setPen(QPen(QColor("#e0e0e0"), 2));
        painter.drawLine(0, centerY, width(), centerY);

        painter.setBrush(QBrush(QColor("#0088cc")));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(firstCircleX, 0, 34, 34);

        painter.setPen(QPen(QColor("white")));
        painter.setFont(QFont("Segoe UI", 12, QFont::Bold));
        painter.drawText(QRect(firstCircleX, 0, 34, 34), Qt::AlignCenter, "1");

        painter.setBrush(QBrush(QColor("#e0e0e0")));
        painter.drawEllipse(secondCircleX, 0, 34, 34);

        painter.setPen(QPen(QColor("#999")));
        painter.drawText(QRect(secondCircleX, 0, 34, 34), Qt::AlignCenter, "2");

        painter.setFont(QFont("Segoe UI", 9));
        painter.setPen(QPen(QColor("#0088cc")));
        painter.drawText(QRect(firstCircleX - 10, 40, 34 + 20, 20), Qt::AlignCenter, "Основное");

        painter.setPen(QPen(QColor("#999")));
        painter.drawText(QRect(secondCircleX - 10, 40, 34 + 20, 20), Qt::AlignCenter, "Пароль");
    }
};

LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent)
{
    qDebug() << "PTR before set:" << m_networkManager;

    qDebug() << "LD STEP 1";
    m_passwordStepCreated = false;
    qDebug() << "LD STEP 2";
    setupUI();
    qDebug() << "LD STEP 3";
    setWindowTitle("Aura - Вход");
    qDebug() << "LD STEP 4";
    setModal(true);
    qDebug() << "LD STEP 5";
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    qDebug() << "LD STEP 6";
    setAttribute(Qt::WA_TranslucentBackground);
    qDebug() << "LD STEP 7";
    setFocusPolicy(Qt::NoFocus);
    qDebug() << "LD STEP 8";
    resizeToLoginSize();
    qDebug() << "LD STEP 9";
    installEventFilter(this);
    qDebug() << "LD STEP 10";
    m_alternativeWidget = nullptr;
}

LoginDialog::~LoginDialog() {
    qDebug() << "=== LoginDialog DESTRUCTOR ===";
}

void LoginDialog::centerWindow()
{
    QRect screenGeometry = QApplication::primaryScreen()->availableGeometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
}

void LoginDialog::resizeToLoginSize()
{
    setMinimumSize(LOGIN_WINDOW_WIDTH, LOGIN_WINDOW_HEIGHT);
    setMaximumSize(LOGIN_WINDOW_WIDTH, LOGIN_WINDOW_HEIGHT);
    resize(LOGIN_WINDOW_WIDTH, LOGIN_WINDOW_HEIGHT);
    centerWindow();
}

void LoginDialog::resizeToRegisterFirstStep()
{
    setMinimumSize(LOGIN_WINDOW_WIDTH, REGISTER_FIRST_STEP_HEIGHT);
    setMaximumSize(LOGIN_WINDOW_WIDTH, REGISTER_FIRST_STEP_HEIGHT);
    resize(LOGIN_WINDOW_WIDTH, REGISTER_FIRST_STEP_HEIGHT);
    centerWindow();
}

void LoginDialog::resizeToRegisterSecondStep()
{
    setMinimumSize(LOGIN_WINDOW_WIDTH, REGISTER_SECOND_STEP_HEIGHT);
    setMaximumSize(LOGIN_WINDOW_WIDTH, REGISTER_SECOND_STEP_HEIGHT);
    resize(LOGIN_WINDOW_WIDTH, REGISTER_SECOND_STEP_HEIGHT);
    //centerWindow();
}

void LoginDialog::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    // Сохраняем соотношение сторон при изменении размера
    if (width() != LOGIN_WINDOW_WIDTH) {
        resize(LOGIN_WINDOW_WIDTH, height());
    }
}

void LoginDialog::setupUI()
{
    QWidget *mainContainer = new QWidget(this);
    mainContainer->setStyleSheet(
        "QWidget { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "stop:0 #4facfe, stop:1 #00f2fe); border-radius: 18px; }"
        );

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(mainContainer);

    QVBoxLayout *containerLayout = new QVBoxLayout(mainContainer);
    containerLayout->setContentsMargins(0, 0, 0, 0);

    QWidget *cardWidget = new QWidget();
    cardWidget->setStyleSheet(
        "QWidget { background-color: white; border-radius: 18px; }"
        );
    cardWidget->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *cardLayout = new QVBoxLayout(cardWidget);
    cardLayout->setContentsMargins(0, 0, 0, 0);
    cardLayout->setSpacing(0);

    // Область для перетаскивания
    m_dragArea = new QWidget(cardWidget);
    m_dragArea->setFixedHeight(40);
    m_dragArea->setStyleSheet(
        "QWidget { background-color: white; border-top-left-radius: 18px; "
        "border-top-right-radius: 18px; }"
        );
    m_dragArea->setCursor(Qt::SizeAllCursor);
    m_dragArea->installEventFilter(this);

    QHBoxLayout *dragLayout = new QHBoxLayout(m_dragArea);
    dragLayout->setContentsMargins(16, 0, 16, 0);

    QLabel *dragLogo = new QLabel("Aura");
    dragLogo->setStyleSheet(
        "color: #0088cc; font-size: 16px; font-weight: bold; background-color: transparent;"
        );

    QPushButton *closeDragBtn = new QPushButton("✕");
    closeDragBtn->setFixedSize(28, 28);
    closeDragBtn->setCursor(Qt::PointingHandCursor);
    closeDragBtn->setStyleSheet(
        "QPushButton { background-color: transparent; border: none; border-radius: 14px; "
        "color: #999; font-size: 14px; }"
        "QPushButton:hover { background-color: #ff3b30; color: white; }"
        );
    // ✅ Отключаем реакцию на Enter
    closeDragBtn->setAutoDefault(false);
    closeDragBtn->setDefault(false);
    closeDragBtn->setFocusPolicy(Qt::NoFocus);
    connect(closeDragBtn, &QPushButton::clicked, this, &QDialog::reject);

    dragLayout->addWidget(dragLogo);
    dragLayout->addStretch();
    dragLayout->addWidget(closeDragBtn);

    cardLayout->addWidget(m_dragArea);

    // Тело с формами
    QWidget *bodyWidget = new QWidget();
    bodyWidget->setContentsMargins(30, 20, 30, 25);
    bodyWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QVBoxLayout *bodyLayout = new QVBoxLayout(bodyWidget);
    bodyLayout->setSpacing(10);

    // Табы
    QWidget *tabsWidget = new QWidget();
    tabsWidget->setStyleSheet(
        "QWidget { background-color: #e3f2fd; border-radius: 12px; }"
        );
    tabsWidget->setFixedHeight(54);

    QHBoxLayout *tabsLayout = new QHBoxLayout(tabsWidget);
    tabsLayout->setContentsMargins(4, 4, 4, 4);
    tabsLayout->setSpacing(4);

    m_loginTab = new QPushButton("Вход");
    m_loginTab->setCursor(Qt::PointingHandCursor);
    m_loginTab->setStyleSheet(
        "QPushButton { background-color: #0088cc; border: none; border-radius: 8px; "
        "padding: 10px; color: white; font-size: 15px; font-weight: 600; }"
        );
    connect(m_loginTab, &QPushButton::clicked, this, &LoginDialog::onSwitchToLogin);

    m_registerTab = new QPushButton("Регистрация");
    m_registerTab->setCursor(Qt::PointingHandCursor);
    m_registerTab->setStyleSheet(
        "QPushButton { background-color: transparent; border: none; border-radius: 8px; "
        "padding: 10px; color: #666666; font-size: 15px; font-weight: 600; }"
        );
    connect(m_registerTab, &QPushButton::clicked, this, &LoginDialog::onSwitchToRegister);

    tabsLayout->addWidget(m_loginTab);
    tabsLayout->addWidget(m_registerTab);

    bodyLayout->addWidget(tabsWidget);

    // Стековый виджет для форм
    m_stackedWidget = new QStackedWidget();
    m_stackedWidget->setStyleSheet("background-color: transparent;");

    setupLoginForm();
    setupRegisterForm();

    m_stackedWidget->addWidget(m_loginWidget);
    m_stackedWidget->addWidget(m_registerWidget);

    bodyLayout->addWidget(m_stackedWidget);

    cardLayout->addWidget(bodyWidget);

    // Футер
    QWidget *footerWidget = new QWidget();
    footerWidget->setStyleSheet(
        "QWidget { background-color: #e3f2fd; }"
        );
    footerWidget->setFixedHeight(55);

    QVBoxLayout *footerLayout = new QVBoxLayout(footerWidget);
    footerLayout->setContentsMargins(20, 8, 20, 10);
    footerLayout->setSpacing(4);

    QLabel *copyrightLabel = new QLabel("© 2026 Мессенджер. Все права защищены.");
    copyrightLabel->setStyleSheet("color: #666666; font-size: 11px; background-color: transparent;");
    copyrightLabel->setAlignment(Qt::AlignCenter);

    QHBoxLayout *footerLinksLayout = new QHBoxLayout();
    footerLinksLayout->setSpacing(15);
    footerLinksLayout->setAlignment(Qt::AlignCenter);

    QLabel *helpLabel = new QLabel("<a href='#' style='color: #0088cc; text-decoration: none;'>Помощь</a>");
    helpLabel->setOpenExternalLinks(false);
    helpLabel->setStyleSheet("background-color: transparent;");

    QLabel *privacyLabel = new QLabel("<a href='#' style='color: #0088cc; text-decoration: none;'>Конфиденциальность</a>");
    privacyLabel->setOpenExternalLinks(false);
    privacyLabel->setStyleSheet("background-color: transparent;");

    footerLinksLayout->addWidget(helpLabel);
    footerLinksLayout->addWidget(privacyLabel);

    footerLayout->addWidget(copyrightLabel);
    footerLayout->addLayout(footerLinksLayout);

    cardLayout->addWidget(footerWidget);

    containerLayout->addWidget(cardWidget);
    adjustSize();

    resizeToLoginSize();
}

void LoginDialog::setupLoginForm()
{
    m_loginWidget = new QWidget();
    m_loginWidget->setStyleSheet("background-color: transparent;");

    QVBoxLayout *layout = new QVBoxLayout(m_loginWidget);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    // Заголовок
    QLabel *title = new QLabel("С возвращением!");
    title->setStyleSheet("font-size: 24px; font-weight: bold; color: #212121; background-color: transparent;");
    title->setAlignment(Qt::AlignCenter);

    QLabel *subtitle = new QLabel("Войдите, чтобы продолжить общение");
    subtitle->setStyleSheet("font-size: 13px; color: #666666; background-color: transparent;");
    subtitle->setAlignment(Qt::AlignCenter);

    layout->addWidget(title);
    layout->addSpacing(8);
    layout->addWidget(subtitle);
    layout->addSpacing(25);

    // Поле Email/телефон
    m_loginUsername = new QLineEdit();
    m_loginUsername->setPlaceholderText("Email или телефон");
    m_loginUsername->setMinimumHeight(48);
    m_loginUsername->setTextMargins(45, 0, 0, 0);
    m_loginUsername->setStyleSheet(
        "QLineEdit { background-color: white; border: 2px solid #e0e0e0; border-radius: 10px; "
        "padding: 12px 14px; color: #212121; font-size: 14px; }"
        "QLineEdit:focus { border-color: #0088cc; }"
        );

    QLabel *emailIcon = new QLabel(m_loginUsername);
    QPixmap userPixmap(":/icons/darkTheme/images/darkTheme/user.svg");
    if (!userPixmap.isNull()) {
        emailIcon->setPixmap(userPixmap.scaled(20, 20, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    emailIcon->setStyleSheet("background-color: transparent;");
    emailIcon->setGeometry(14, (48 - 20) / 2, 20, 20);

    layout->addWidget(m_loginUsername);
    layout->addSpacing(16);

    // Поле Пароль
    m_loginPassword = new QLineEdit();
    m_loginPassword->setPlaceholderText("Пароль");
    m_loginPassword->setEchoMode(QLineEdit::Password);
    m_loginPassword->setMinimumHeight(48);
    m_loginPassword->setTextMargins(45, 0, 0, 0);
    m_loginPassword->setStyleSheet(
        "QLineEdit { background-color: white; border: 2px solid #e0e0e0; border-radius: 10px; "
        "padding: 12px 14px; color: #212121; font-size: 14px; }"
        "QLineEdit:focus { border-color: #0088cc; }"
        );

    QLabel *lockIcon = new QLabel(m_loginPassword);
    QPixmap lockPixmap(":/icons/darkTheme/images/darkTheme/lock.svg");
    if (!lockPixmap.isNull()) {
        lockIcon->setPixmap(lockPixmap.scaled(20, 20, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    lockIcon->setStyleSheet("background-color: transparent;");
    lockIcon->setGeometry(14, (48 - 20) / 2, 20, 20);

    layout->addWidget(m_loginPassword);
    layout->addSpacing(16);

    // Чекбокс и забыли пароль
    QWidget *optionsWidget = new QWidget();
    optionsWidget->setStyleSheet("background-color: transparent;");
    QHBoxLayout *optionsLayout = new QHBoxLayout(optionsWidget);
    optionsLayout->setContentsMargins(0, 0, 0, 0);
    optionsLayout->setSpacing(10);

    m_rememberMe = new QCheckBox("Запомнить меня");
    m_rememberMe->setStyleSheet(
        "QCheckBox { color: #666666; font-size: 13px; spacing: 8px; }"
        "QCheckBox::indicator { width: 18px; height: 18px; }"
        );

    QPushButton *forgotPasswordBtn = new QPushButton("Забыли пароль?");
    forgotPasswordBtn->setCursor(Qt::PointingHandCursor);
    forgotPasswordBtn->setStyleSheet(
        "QPushButton { background-color: transparent; border: none; color: #0088cc; "
        "font-size: 13px; font-weight: 500; }"
        "QPushButton:hover { text-decoration: underline; }"
        );
    connect(forgotPasswordBtn, &QPushButton::clicked, this, &LoginDialog::onForgotPassword);

    optionsLayout->addWidget(m_rememberMe);
    optionsLayout->addStretch();
    optionsLayout->addWidget(forgotPasswordBtn);

    layout->addWidget(optionsWidget);
    layout->addSpacing(20);

    // Кнопка входа
    m_loginBtn = new QPushButton("Войти в аккаунт");
    m_loginBtn->setCursor(Qt::PointingHandCursor);
    m_loginBtn->setMinimumHeight(50);
    m_loginBtn->setStyleSheet(
        "QPushButton { background-color: #0088cc; border: none; border-radius: 10px; "
        "padding: 14px; color: white; font-size: 15px; font-weight: 600; }"
        "QPushButton:hover { background-color: #0077b3; }"
        );
    connect(m_loginBtn, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);

    layout->addWidget(m_loginBtn);
    layout->addSpacing(16);

    m_loginPassword->setFocus();  // убираем фокус с кнопки

    // Кнопка альтернативного входа
    QPushButton *alternativeLoginBtn = new QPushButton(" Альтернативный вход");
    alternativeLoginBtn->setCursor(Qt::PointingHandCursor);
    alternativeLoginBtn->setMinimumHeight(46);

    QIcon shieldIcon(":/icons/darkTheme/images/darkTheme/shield.svg");
    alternativeLoginBtn->setIcon(shieldIcon);
    alternativeLoginBtn->setIconSize(QSize(18, 18));

    alternativeLoginBtn->setStyleSheet(
        "QPushButton { background-color: transparent; border: 2px solid #0088cc; border-radius: 10px; "
        "padding: 12px; color: #0088cc; font-size: 14px; font-weight: 600; text-align: center; }"
        "QPushButton:hover { background-color: #e3f2fd; }"
        );
    connect(alternativeLoginBtn, &QPushButton::clicked, this, &LoginDialog::onAlternativeLogin);

    layout->addWidget(alternativeLoginBtn);
    layout->addSpacing(16);

    // Ссылка "Нет аккаунта? Зарегистрироваться"
    QWidget *registerLinkWidget = new QWidget();
    QHBoxLayout *registerLinkLayout = new QHBoxLayout(registerLinkWidget);
    registerLinkLayout->setContentsMargins(0, 0, 0, 0);
    registerLinkLayout->setSpacing(5);
    registerLinkLayout->setAlignment(Qt::AlignCenter);

    QLabel *noAccountLabel = new QLabel("Нет аккаунта?");
    noAccountLabel->setStyleSheet("color: #666666; font-size: 13px; background-color: transparent;");

    QPushButton *registerLinkBtn = new QPushButton("Зарегистрироваться");
    registerLinkBtn->setCursor(Qt::PointingHandCursor);
    registerLinkBtn->setStyleSheet(
        "QPushButton { background-color: transparent; border: none; color: #0088cc; "
        "font-size: 13px; font-weight: 600; }"
        "QPushButton:hover { text-decoration: underline; }"
        );
    connect(registerLinkBtn, &QPushButton::clicked, this, &LoginDialog::onSwitchToRegister);

    registerLinkLayout->addWidget(noAccountLabel);
    registerLinkLayout->addWidget(registerLinkBtn);

    layout->addWidget(registerLinkWidget);
    layout->addStretch();

    // Загружаем сохраненные данные
    QSettings settings("Aura", "Messenger");
    if (settings.value("rememberMe", false).toBool()) {
        m_rememberMe->setChecked(true);
        m_loginUsername->setText(settings.value("savedUsername").toString());
        m_loginPassword->setText(settings.value("savedPassword").toString());
    }
}


void LoginDialog::setupRegisterForm()
{
    m_registerWidget = new QWidget();
    m_registerWidget->setStyleSheet("background-color: transparent;");

    QVBoxLayout *layout = new QVBoxLayout(m_registerWidget);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    QLabel *title = new QLabel("Начните общение");
    title->setStyleSheet("font-size: 24px; font-weight: bold; color: #212121; background-color: transparent;");
    title->setAlignment(Qt::AlignCenter);

    QLabel *subtitle = new QLabel("Создайте аккаунт за пару минут");
    subtitle->setStyleSheet("font-size: 13px; color: #666666; background-color: transparent;");
    subtitle->setAlignment(Qt::AlignCenter);

    layout->addWidget(title);
    layout->addSpacing(8);
    layout->addWidget(subtitle);
    layout->addSpacing(25);

    // ========== ИНДИКАТОР ШАГОВ ==========
    StepIndicator *stepsWidget = new StepIndicator();
    layout->addWidget(stepsWidget);
    layout->addSpacing(20);

    // ========== Поля ввода ==========
    // Поле Имя и фамилия
    QLabel *nameLabel = new QLabel("Имя и фамилия");
    nameLabel->setStyleSheet("color: #212121; font-weight: 500; font-size: 14px;");

    m_regFullName = new QLineEdit();
    m_regFullName->setPlaceholderText("Иван Иванов");
    m_regFullName->setMinimumHeight(48);
    m_regFullName->setTextMargins(45, 0, 0, 0);
    m_regFullName->setStyleSheet(
        "QLineEdit { background-color: white; border: 2px solid #e0e0e0; border-radius: 10px; "
        "padding: 12px 14px; color: #212121; font-size: 14px; }"
        "QLineEdit:focus { border-color: #0088cc; }"
        );

    QLabel *userIcon = new QLabel(m_regFullName);
    QPixmap userPixmap(":/icons/darkTheme/images/darkTheme/user.svg");
    if (!userPixmap.isNull()) {
        userIcon->setPixmap(userPixmap.scaled(20, 20, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    userIcon->setStyleSheet("background-color: transparent;");
    userIcon->setGeometry(14, (48 - 20) / 2, 20, 20);

    layout->addWidget(nameLabel);
    layout->addSpacing(6);
    layout->addWidget(m_regFullName);
    layout->addSpacing(16);

    // Поле Email
    QLabel *emailLabel = new QLabel("Email");
    emailLabel->setStyleSheet("color: #212121; font-weight: 500; font-size: 14px;");

    m_regEmail = new QLineEdit();
    m_regEmail->setPlaceholderText("example@mail.com");
    m_regEmail->setMinimumHeight(48);
    m_regEmail->setTextMargins(45, 0, 0, 0);
    m_regEmail->setStyleSheet(
        "QLineEdit { background-color: white; border: 2px solid #e0e0e0; border-radius: 10px; "
        "padding: 12px 14px; color: #212121; font-size: 14px; }"
        "QLineEdit:focus { border-color: #0088cc; }"
        );

    QLabel *emailIcon = new QLabel(m_regEmail);
    if (!userPixmap.isNull()) {
        emailIcon->setPixmap(userPixmap.scaled(20, 20, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    emailIcon->setStyleSheet("background-color: transparent;");
    emailIcon->setGeometry(14, (48 - 20) / 2, 20, 20);

    layout->addWidget(emailLabel);
    layout->addSpacing(6);
    layout->addWidget(m_regEmail);
    layout->addSpacing(16);

    // Поле Телефон
    QLabel *phoneLabel = new QLabel("Телефон (необязательно)");
    phoneLabel->setStyleSheet("color: #212121; font-weight: 500; font-size: 14px;");

    m_regPhone = new QLineEdit();
    m_regPhone->setPlaceholderText("+7 (999) 123-45-67");
    m_regPhone->setMinimumHeight(48);
    m_regPhone->setTextMargins(45, 0, 0, 0);
    m_regPhone->setStyleSheet(
        "QLineEdit { background-color: white; border: 2px solid #e0e0e0; border-radius: 10px; "
        "padding: 12px 14px; color: #212121; font-size: 14px; }"
        "QLineEdit:focus { border-color: #0088cc; }"
        );

    QLabel *phoneIcon = new QLabel(m_regPhone);
    QPixmap phonePixmap(":/icons/darkTheme/images/darkTheme/phone.svg");
    if (!phonePixmap.isNull()) {
        phoneIcon->setPixmap(phonePixmap.scaled(20, 20, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    phoneIcon->setStyleSheet("background-color: transparent;");
    phoneIcon->setGeometry(14, (48 - 20) / 2, 20, 20);

    layout->addWidget(phoneLabel);
    layout->addSpacing(6);
    layout->addWidget(m_regPhone);
    layout->addSpacing(16);

    // Поле Дата рождения
    QLabel *birthLabel = new QLabel("Дата рождения (необязательно)");
    birthLabel->setStyleSheet("color: #212121; font-weight: 500; font-size: 14px;");

    m_regBirthDate = new QLineEdit();
    m_regBirthDate->setPlaceholderText("ДД.ММ.ГГГГ");
    m_regBirthDate->setMinimumHeight(48);
    m_regBirthDate->setTextMargins(45, 0, 0, 0);
    m_regBirthDate->setStyleSheet(
        "QLineEdit { background-color: white; border: 2px solid #e0e0e0; border-radius: 10px; "
        "padding: 12px 14px; color: #212121; font-size: 14px; }"
        "QLineEdit:focus { border-color: #0088cc; }"
        );

    QLabel *calendarIcon = new QLabel(m_regBirthDate);
    QPixmap calendarPixmap(":/icons/darkTheme/images/darkTheme/calendar.svg");
    if (!calendarPixmap.isNull()) {
        calendarIcon->setPixmap(calendarPixmap.scaled(20, 20, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    calendarIcon->setStyleSheet("background-color: transparent;");
    calendarIcon->setGeometry(14, (48 - 20) / 2, 20, 20);

    layout->addWidget(birthLabel);
    layout->addSpacing(6);
    layout->addWidget(m_regBirthDate);
    layout->addSpacing(30);

    // Кнопки
    QWidget *navButtonsWidget = new QWidget();
    QHBoxLayout *navButtonsLayout = new QHBoxLayout(navButtonsWidget);
    navButtonsLayout->setSpacing(12);

    QPushButton *backBtn = new QPushButton("Назад");
    backBtn->setCursor(Qt::PointingHandCursor);
    backBtn->setMinimumHeight(48);
    backBtn->setStyleSheet(
        "QPushButton { background-color: transparent; border: 2px solid #0088cc; border-radius: 10px; "
        "padding: 12px; color: #0088cc; font-size: 14px; font-weight: 600; }"
        );
    backBtn->setAutoDefault(false);
    backBtn->setDefault(false);
    backBtn->setFocusPolicy(Qt::NoFocus);
    //connect(backBtn, &QPushButton::clicked, this, &LoginDialog::onSwitchToLogin);

    m_registerNextBtn = new QPushButton("Далее");
    m_registerNextBtn->setCursor(Qt::PointingHandCursor);
    m_registerNextBtn->setMinimumHeight(48);
    m_registerNextBtn->setStyleSheet(
        "QPushButton { background-color: #0088cc; border: none; border-radius: 10px; "
        "padding: 12px; color: white; font-size: 15px; font-weight: 600; }"
        );
    // Используем лямбда для отладки
    connect(m_registerNextBtn, &QPushButton::clicked, this, [this]() {
        qDebug() << "Next button clicked!";
        onRegisterNextStep();
    });

    m_registerNextBtn->setAutoDefault(false);
    m_registerNextBtn->setDefault(false);

    navButtonsLayout->addWidget(backBtn);
    navButtonsLayout->addWidget(m_registerNextBtn);

    layout->addWidget(navButtonsWidget);
    layout->addStretch();
}


void LoginDialog::setupPasswordStep()
{
    qDebug() << "=== setupPasswordStep START ===";

    if (m_passwordStepCreated) {
        qDebug() << "Already created, returning";
        return;
    }

    m_passwordWidget = new QWidget();
    m_passwordWidget->setStyleSheet("background-color: transparent;");

    QVBoxLayout *layout = new QVBoxLayout(m_passwordWidget);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    // Заголовок
    QLabel *title = new QLabel("Начните общение");
    title->setStyleSheet("font-size: 24px; font-weight: bold; color: #212121; background-color: transparent;");
    title->setAlignment(Qt::AlignCenter);

    QLabel *subtitle = new QLabel("Создайте аккаунт за пару минут");
    subtitle->setStyleSheet("font-size: 13px; color: #666666; background-color: transparent;");
    subtitle->setAlignment(Qt::AlignCenter);

    layout->addWidget(title);
    layout->addSpacing(8);
    layout->addWidget(subtitle);
    layout->addSpacing(25);

    // ========== ИНДИКАТОР ШАГОВ ==========
    class StepIndicator2 : public QWidget {
    public:
        StepIndicator2(QWidget *parent = nullptr) : QWidget(parent) {
            setFixedHeight(70);
        }

    protected:
        void paintEvent(QPaintEvent *event) override {
            QWidget::paintEvent(event);
            QPainter painter(this);
            painter.setRenderHint(QPainter::Antialiasing);

            int centerY = 17;
            int firstCircleX = width() / 4 - 17;
            int secondCircleX = width() * 3 / 4 - 17;

            painter.setPen(QPen(QColor("#e0e0e0"), 2));
            painter.drawLine(0, centerY, width(), centerY);

            painter.setBrush(QBrush(QColor("#22c55e")));
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(firstCircleX, 0, 34, 34);

            painter.setPen(QPen(QColor("white")));
            painter.setFont(QFont("Segoe UI", 12, QFont::Bold));
            painter.drawText(QRect(firstCircleX, 0, 34, 34), Qt::AlignCenter, "✓");

            painter.setBrush(QBrush(QColor("#0088cc")));
            painter.drawEllipse(secondCircleX, 0, 34, 34);

            painter.setPen(QPen(QColor("white")));
            painter.drawText(QRect(secondCircleX, 0, 34, 34), Qt::AlignCenter, "2");

            painter.setFont(QFont("Segoe UI", 9));
            painter.setPen(QPen(QColor("#22c55e")));
            painter.drawText(QRect(firstCircleX - 10, 40, 34 + 20, 20), Qt::AlignCenter, "Основное");

            painter.setPen(QPen(QColor("#0088cc")));
            painter.drawText(QRect(secondCircleX - 10, 40, 34 + 20, 20), Qt::AlignCenter, "Пароль");
        }
    };

    StepIndicator2 *stepsWidget = new StepIndicator2();
    layout->addWidget(stepsWidget);
    layout->addSpacing(20);

    // Поле Пароль
    QLabel *passwordLabel = new QLabel("Пароль");
    passwordLabel->setStyleSheet("color: #212121; font-weight: 500; font-size: 14px;");

    qDebug() << "Creating m_regPassword";
    m_regPassword = new QLineEdit();  // ← ИНИЦИАЛИЗАЦИЯ
    m_regPassword->setPlaceholderText("Создайте надежный пароль");
    m_regPassword->setEchoMode(QLineEdit::Password);
    m_regPassword->setMinimumHeight(48);
    m_regPassword->setTextMargins(45, 0, 0, 0);
    m_regPassword->setStyleSheet(
        "QLineEdit { background-color: white; border: 2px solid #e0e0e0; border-radius: 10px; "
        "padding: 12px 14px; color: #212121; font-size: 14px; }"
        "QLineEdit:focus { border-color: #0088cc; }"
        );

    QPixmap lockPixmap(":/icons/darkTheme/images/darkTheme/lock.svg");
    QLabel *lockIcon = new QLabel(m_regPassword);
    if (!lockPixmap.isNull()) {
        lockIcon->setPixmap(lockPixmap.scaled(20, 20, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    lockIcon->setStyleSheet("background-color: transparent;");
    lockIcon->setGeometry(14, (48 - 20) / 2, 20, 20);

    layout->addWidget(passwordLabel);
    layout->addSpacing(6);
    layout->addWidget(m_regPassword);
    layout->addSpacing(10);

    // Маркированный список
    QLabel *passwordHints = new QLabel(
        "• Не менее 8 символов<br>"
        "• Цифры и буквы<br>"
        "• Можно использовать спецсимволы"
        );
    passwordHints->setStyleSheet("color: #666666; font-size: 12px; background-color: transparent;");
    passwordHints->setWordWrap(true);
    layout->addWidget(passwordHints);
    layout->addSpacing(20);

    // Поле Подтверждение пароля
    QLabel *confirmLabel = new QLabel("Подтверждение пароля");
    confirmLabel->setStyleSheet("color: #212121; font-weight: 500; font-size: 14px;");

    qDebug() << "Creating m_regConfirmPassword";
    m_regConfirmPassword = new QLineEdit();  // ← ИНИЦИАЛИЗАЦИЯ
    m_regConfirmPassword->setPlaceholderText("Повторите пароль");
    m_regConfirmPassword->setEchoMode(QLineEdit::Password);
    m_regConfirmPassword->setMinimumHeight(48);
    m_regConfirmPassword->setTextMargins(45, 0, 0, 0);
    m_regConfirmPassword->setStyleSheet(
        "QLineEdit { background-color: white; border: 2px solid #e0e0e0; border-radius: 10px; "
        "padding: 12px 14px; color: #212121; font-size: 14px; }"
        "QLineEdit:focus { border-color: #0088cc; }"
        );

    QLabel *confirmIcon = new QLabel(m_regConfirmPassword);
    if (!lockPixmap.isNull()) {
        confirmIcon->setPixmap(lockPixmap.scaled(20, 20, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    confirmIcon->setStyleSheet("background-color: transparent;");
    confirmIcon->setGeometry(14, (48 - 20) / 2, 20, 20);

    layout->addWidget(confirmLabel);
    layout->addSpacing(6);
    layout->addWidget(m_regConfirmPassword);
    layout->addSpacing(16);

    // Галочка
    QWidget *termsWidget = new QWidget();
    QHBoxLayout *termsLayout = new QHBoxLayout(termsWidget);
    termsLayout->setContentsMargins(0, 0, 0, 0);
    termsLayout->setSpacing(10);

    qDebug() << "Creating m_termsCheckbox";
    m_termsCheckbox = new QCheckBox();  // ← ИНИЦИАЛИЗАЦИЯ
    m_termsCheckbox->setStyleSheet(
        "QCheckBox::indicator { width: 18px; height: 18px; }"
        );
    m_termsCheckbox->setChecked(false);

    QLabel *termsLabel = new QLabel(
        "Я согласен с <a href='#' style='color: #0088cc; text-decoration: none;'>Условиями</a> и "
        "<a href='#' style='color: #0088cc; text-decoration: none;'>Политикой</a>"
        );
    termsLabel->setOpenExternalLinks(false);
    termsLabel->setStyleSheet("color: #666666; font-size: 13px; background-color: transparent;");

    termsLayout->addWidget(m_termsCheckbox);
    termsLayout->addWidget(termsLabel);
    termsLayout->addStretch();

    layout->addWidget(termsWidget);
    layout->addSpacing(25);

    // Кнопки
    QWidget *navButtonsWidget = new QWidget();
    QHBoxLayout *navButtonsLayout = new QHBoxLayout(navButtonsWidget);
    navButtonsLayout->setContentsMargins(0, 0, 0, 0);
    navButtonsLayout->setSpacing(12);

    QPushButton *backBtn = new QPushButton("Назад");
    backBtn->setCursor(Qt::PointingHandCursor);
    backBtn->setMinimumHeight(48);
    backBtn->setStyleSheet(
        "QPushButton { background-color: transparent; border: 2px solid #0088cc; border-radius: 10px; "
        "padding: 12px; color: #0088cc; font-size: 14px; font-weight: 600; }"
        "QPushButton:hover { background-color: #e3f2fd; }"
        );
    backBtn->setAutoDefault(false);
    backBtn->setDefault(false);
    backBtn->setFocusPolicy(Qt::NoFocus);
    connect(backBtn, &QPushButton::clicked, this, &LoginDialog::onBackToFirstStep);

    qDebug() << "Creating m_registerSubmitBtn";
    m_registerSubmitBtn = new QPushButton("Зарегистрироваться");  // ← ИНИЦИАЛИЗАЦИЯ
    m_registerSubmitBtn->setCursor(Qt::PointingHandCursor);
    m_registerSubmitBtn->setMinimumHeight(48);
    m_registerSubmitBtn->setStyleSheet(
        "QPushButton { background-color: #0088cc; border: none; border-radius: 10px; "
        "padding: 12px; color: white; font-size: 15px; font-weight: 600; }"
        "QPushButton:hover { background-color: #0077b3; }"
        );
    connect(m_registerSubmitBtn, &QPushButton::clicked, this, &LoginDialog::onRegisterClicked);

    navButtonsLayout->addWidget(backBtn);
    navButtonsLayout->addWidget(m_registerSubmitBtn);

    m_registerSubmitBtn->setAutoDefault(false);
    m_registerSubmitBtn->setDefault(false);

    layout->addWidget(navButtonsWidget);
    layout->addStretch();

    m_passwordStepCreated = true;  // ← ВАЖНО: установить флаг в конце
    qDebug() << "=== setupPasswordStep END, m_passwordStepCreated =" << m_passwordStepCreated;
}

void LoginDialog::onRegisterNextStep()
{
    qDebug() << "=== onRegisterNextStep START ===";

    m_tempFullName = m_regFullName->text().trimmed();
    m_tempEmail = m_regEmail->text().trimmed();
    m_tempPhone = m_regPhone->text().trimmed();
    m_tempBirthDate = m_regBirthDate->text().trimmed();

    if (m_tempFullName.isEmpty() || m_tempEmail.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Заполните имя и email");
        return;
    }

    qDebug() << "m_passwordStepCreated:" << m_passwordStepCreated;

    if (!m_passwordStepCreated) {
        setupPasswordStep();
        m_passwordStepCreated = true;
    }

    if (m_stackedWidget && m_passwordWidget) {
        if (m_stackedWidget->indexOf(m_passwordWidget) == -1) {
            m_stackedWidget->addWidget(m_passwordWidget);
        }
        m_stackedWidget->setCurrentWidget(m_passwordWidget);

        // ✅ Отключаем виджет шага 1, чтобы его кнопки не перехватывали Enter
        m_registerWidget->setEnabled(false);

        if (m_regPassword) {
            m_regPassword->setFocus();
        }
    }

    QWidget::setTabOrder(m_regPassword, m_regConfirmPassword);
    QWidget::setTabOrder(m_regConfirmPassword, m_registerSubmitBtn);

    // Меняем размер окна ПОСЛЕ установки фокуса
    resizeToRegisterSecondStep();
    centerWindow();

    qDebug() << "=== onRegisterNextStep END ===";
}

void LoginDialog::onRegisterClicked()
{
    qDebug() << "=== onRegisterClicked START ===";
    qDebug() << "m_mainWindow =" << m_mainWindow;

    QString username = m_regFullName->text().trimmed();
    QString email = m_regEmail->text().trimmed();
    QString password = m_regPassword->text();
    QString confirmPassword = m_regConfirmPassword->text();

    qDebug() << "Username:" << username << "Email:" << email;

    // Валидация
    if (username.isEmpty() || email.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Пожалуйста, заполните все поля");
        return;
    }

    if (password != confirmPassword) {
        QMessageBox::warning(this, "Ошибка", "Пароли не совпадают");
        return;
    }

    if (password.length() < 6) {
        QMessageBox::warning(this, "Ошибка", "Пароль должен содержать минимум 6 символов");
        return;
    }

    QRegularExpression emailRegex(R"((\w+)(\.\w+)*@(\w+)(\.\w+)+)");
    if (!emailRegex.match(email).hasMatch()) {
        QMessageBox::warning(this, "Ошибка", "Введите корректный email адрес");
        return;
    }

    if (!m_termsCheckbox->isChecked()) {
        QMessageBox::warning(this, "Ошибка", "Примите условия использования");
        return;
    }

    if (!m_mainWindow) {
        QMessageBox::warning(this, "Ошибка", "Внутренняя ошибка приложения");
        return;
    }

    // Проверяем, что networkManager существует
    NetworkManager *nm = m_mainWindow->getNetworkManager();
    if (!nm) {
        QMessageBox::warning(this, "Ошибка", "Нет подключения к сети");
        m_registerSubmitBtn->setEnabled(true);
        m_registerSubmitBtn->setText("Зарегистрироваться");
        return;
    }

    // Сохраняем данные
    m_tempEmail = email;
    m_tempUsername = username;
    m_tempPassword = password;

    // Отключаем кнопку
    m_registerSubmitBtn->setEnabled(false);
    m_registerSubmitBtn->setText("Отправка...");

    if (!m_mainWindow) {
        QMessageBox::warning(this, "Ошибка", "Внутренняя ошибка приложения");
        m_registerSubmitBtn->setEnabled(true);
        m_registerSubmitBtn->setText("Зарегистрироваться");
        return;
    }

    qDebug() << "About to call sendEmailConfirmation";
    m_mainWindow->sendEmailConfirmation(email, username, password);
    qDebug() << "After sendEmailConfirmation";

    // 🔥 ПОКАЗЫВАЕМ ДИАЛОГ ПОДТВЕРЖДЕНИЯ
    showEmailConfirmationDialog();

    qDebug() << "=== onRegisterClicked END ===";
}

void LoginDialog::showEmailConfirmationDialog()
{
    qDebug() << "showEmailConfirmationDialog START";

    m_confirmationDialog = new QDialog(this);
    m_confirmationDialog->setWindowTitle("Подтверждение email");
    m_confirmationDialog->setFixedSize(400, 280);
    m_confirmationDialog->setStyleSheet(
        "QDialog { background-color: #1e293b; }"
        "QLabel { color: #f1f5f9; background: transparent; }"
        "QLineEdit { background: #0f172a; border: 1px solid #334155; border-radius: 10px; color: #f1f5f9; padding: 12px; font-size: 14px; }"
        "QPushButton { background: #1d4ed8; border: none; border-radius: 10px; color: white; font-size: 14px; font-weight: 600; padding: 10px; }"
        "QPushButton:hover { background: #2563eb; }"
        );

    QVBoxLayout *layout = new QVBoxLayout(m_confirmationDialog);
    layout->setSpacing(16);
    layout->setContentsMargins(20, 20, 20, 20);

    QLabel *infoLabel = new QLabel(
        "Код подтверждения отправлен на вашу почту " + m_tempEmail + "\n\n"
                                                                     "Пожалуйста, введите полученный код для завершения регистрации."
        );
    infoLabel->setWordWrap(true);
    infoLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(infoLabel);

    m_confirmationCodeEdit = new QLineEdit();
    m_confirmationCodeEdit->setPlaceholderText("Введите код подтверждения");
    m_confirmationCodeEdit->setFixedHeight(44);
    layout->addWidget(m_confirmationCodeEdit);

    QPushButton *confirmBtn = new QPushButton("Подтвердить");
    confirmBtn->setFixedHeight(44);
    confirmBtn->setCursor(Qt::PointingHandCursor);
    layout->addWidget(confirmBtn);

    QPushButton *cancelBtn = new QPushButton("Отмена");
    cancelBtn->setFixedHeight(44);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setStyleSheet("background: #334155;");
    layout->addWidget(cancelBtn);

    connect(confirmBtn, &QPushButton::clicked, this, [this]() {
        QString code = m_confirmationCodeEdit->text().trimmed();
        if (code.isEmpty()) {
            QMessageBox::warning(m_confirmationDialog, "Ошибка", "Введите код подтверждения");
            return;
        }

        if (m_mainWindow) {
            m_mainWindow->confirmEmail(m_tempEmail, code);
        }
        m_confirmationDialog->accept();
    });

    connect(cancelBtn, &QPushButton::clicked, m_confirmationDialog, &QDialog::reject);

    qDebug() << "Dialog created, showing...";
    m_confirmationDialog->exec();
    qDebug() << "Dialog closed";
}

void LoginDialog::onConfirmationSent(bool success)
{
    if (!success && m_confirmationDialog) {
        QMessageBox::warning(m_confirmationDialog, "Ошибка",
                             "Не удалось отправить код подтверждения. Проверьте email и попробуйте снова.");
    }
}

void LoginDialog::onConfirmationFailed(const QString &error)
{
    if (m_confirmationDialog) {
        QMessageBox::warning(m_confirmationDialog, "Ошибка",
                             "Неверный код подтверждения: " + error);
    }
}

void LoginDialog::onRegistrationCompleted()
{
    qDebug() << "onRegistrationCompleted called";

    if (m_confirmationDialog) {
        m_confirmationDialog->accept();
    }

    QMessageBox::information(this, "Успех", "Аккаунт создан!");

    // 🔥 ДОБАВЛЯЕМ: закрываем LoginDialog с успехом
    accept();  // ← ЭТО ЗАКРОЕТ ДИАЛОГ ЛОГИНА И ВЕРНЁТ QDialog::Accepted
}

void LoginDialog::onLoginClicked()
{
    qDebug() << "=== onLoginClicked CALLED ===";
    QString username = m_loginUsername->text().trimmed();
    QString password = m_loginPassword->text().trimmed();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Заполните все поля");
        return;
    }

    if (!m_networkManager || !m_networkManager->isConnected()) {
        QMessageBox::warning(this, "Ошибка", "Нет подключения к серверу");
        return;
    }

    // Сохраняем или удаляем логин/пароль
    QSettings settings("Aura", "Messenger");
    if (m_rememberMe->isChecked()) {
        settings.setValue("rememberMe", true);
        settings.setValue("savedUsername", username);
        settings.setValue("savedPassword", password);
    } else {
        settings.setValue("rememberMe", false);
        settings.remove("savedUsername");
        settings.remove("savedPassword");
    }

    m_loginBtn->setEnabled(false);
    m_loginBtn->setText("Вход...");

    QJsonObject data;
    data["username"] = username;
    data["password"] = password;

    m_pendingAction = "login";
    m_waitingForResponse = true;

    m_networkManager->sendJson("login", data);

    qDebug() << "LOGIN JSON sent:" << data;
}

void LoginDialog::onSwitchToRegister()
{
    m_stackedWidget->setCurrentWidget(m_registerWidget);
    setWindowTitle("Aura - Регистрация");
    resizeToRegisterFirstStep();  // Используем размер для первого шага

    m_registerTab->setStyleSheet(
        "QPushButton { background-color: #0088cc; border: none; border-radius: 8px; "
        "padding: 10px; color: white; font-size: 15px; font-weight: 600; }"
        );
    m_loginTab->setStyleSheet(
        "QPushButton { background-color: transparent; border: none; border-radius: 8px; "
        "padding: 10px; color: #666666; font-size: 15px; font-weight: 600; }"
        );
}

void LoginDialog::onSwitchToLogin()
{
    qDebug() << "=== onSwitchToLogin CALLED ===";
    // ✅ Включаем виджет шага 1 обратно при возврате на логин
    m_registerWidget->setEnabled(true);

    m_stackedWidget->setCurrentWidget(m_loginWidget);
    setWindowTitle("Aura - Вход");
    resizeToLoginSize();

    m_loginTab->setStyleSheet(
        "QPushButton { background-color: #0088cc; border: none; border-radius: 8px; "
        "padding: 10px; color: white; font-size: 15px; font-weight: 600; }"
        );
    m_registerTab->setStyleSheet(
        "QPushButton { background-color: transparent; border: none; border-radius: 8px; "
        "padding: 10px; color: #666666; font-size: 15px; font-weight: 600; }"
        );
}

void LoginDialog::onForgotPassword()
{
    QMessageBox::information(this, "Восстановление пароля",
                             "Функция восстановления пароля будет доступна в следующей версии.");
}

void LoginDialog::onAlternativeLogin()
{
    if (!m_alternativeWidget) {
        setupAlternativeLoginForm();
        m_stackedWidget->addWidget(m_alternativeWidget);
    }
    m_stackedWidget->setCurrentWidget(m_alternativeWidget);
    setWindowTitle("Aura - Альтернативный вход");
    setMinimumSize(LOGIN_WINDOW_WIDTH, 783);
    setMaximumSize(LOGIN_WINDOW_WIDTH, 783);
    resize(LOGIN_WINDOW_WIDTH, 783);
    centerWindow();

    // Находим bodyWidget (родитель m_stackedWidget) и убираем верхний отступ
    QWidget *bodyWidget = m_stackedWidget->parentWidget();
    if (bodyWidget) {
        bodyWidget->setContentsMargins(30, 0, 30, 25);  // верхний отступ 0
    }
}

void LoginDialog::onNetworkConnected()
{
    qDebug() << "Connected to server";
}

void LoginDialog::onNetworkDisconnected()
{
    qDebug() << "Disconnected from server";
    if (m_waitingForResponse) {
        m_loginBtn->setEnabled(true);
        m_registerSubmitBtn->setEnabled(true);
        m_loginBtn->setText("Войти в аккаунт");
        m_registerSubmitBtn->setText("Зарегистрироваться");
        m_waitingForResponse = false;
        QMessageBox::warning(this, "Ошибка", "Нет подключения к серверу");
    }
}

void LoginDialog::onJsonResponse(const QJsonObject &obj)
{
    qDebug() << "=== onJsonResponse CALLED ===";
    qDebug() << "JSON response:" << QJsonDocument(obj).toJson(QJsonDocument::Compact);
    qDebug() << "TYPE:" << obj["type"].toString();
    qDebug() << "STATUS (root):" << obj["status"].toString();
    qDebug() << "STATUS (data):" << obj["data"].toObject()["status"].toString();

    QString type = obj["type"].toString();
    QJsonObject data = obj["data"].toObject();
    QString status = data["status"].toString();

    qDebug() << "TYPE:" << type;
    qDebug() << "STATUS:" << status;

    if (m_pendingAction == "login") {
        if (type == "login" && status == "ok") {
            qDebug() << "LOGIN SUCCESS";

            m_currentUserId = data["user_id"].toInt();

            if (m_loginUsername->text().isEmpty()) {
                m_currentUsername = m_tempUsername;
            } else {
                m_currentUsername = m_loginUsername->text();
            }

            m_waitingForResponse = false;

            if (m_loginBtn) {
                m_loginBtn->setEnabled(true);
                m_loginBtn->setText("Войти в аккаунт");
            }

            qDebug() << "Accepting dialog with userId:" << m_currentUserId;
            accept();  // ← ЭТО КЛЮЧЕВОЙ МОМЕНТ!
            return;
        }
    }
    if (m_pendingAction == "register") {
        if (type == "register" && status == "ok") {
            qDebug() << "REGISTER SUCCESS";

            m_waitingForResponse = false;

            if (m_registerSubmitBtn) {
                m_registerSubmitBtn->setEnabled(true);
                m_registerSubmitBtn->setText("Зарегистрироваться");
            }

            // 🔥 Устанавливаем данные пользователя
            m_currentUserId = 0; // ID будет получен при логине
            m_currentUsername = m_tempUsername;

            // 🔥 ВЫПОЛНЯЕМ АВТО-ЛОГИН
            QJsonObject loginData;
            loginData["username"] = m_tempUsername;
            loginData["password"] = m_tempPassword;

            m_pendingAction = "login";
            m_networkManager->sendJson("login", loginData);
            qDebug() << "Sending login with username:" << m_tempUsername << "password:" << m_tempPassword;
            return; // Ждём ответа на логин
        }

        if (type == "register" && status == "user_exists") {
            QMessageBox::warning(this, "Ошибка", "Пользователь уже существует");
            m_waitingForResponse = false;

            if (m_registerSubmitBtn) {
                m_registerSubmitBtn->setEnabled(true);
                m_registerSubmitBtn->setText("Зарегистрироваться");
            }
            return;
        }

        if (type == "register") {
            QMessageBox::warning(this, "Ошибка", "Ошибка регистрации");
            m_waitingForResponse = false;

            if (m_registerSubmitBtn) {
                m_registerSubmitBtn->setEnabled(true);
                m_registerSubmitBtn->setText("Зарегистрироваться");
            }
            return;
        }
    }
    else if (type == "email_confirmed") {
        QString status = data["status"].toString();
        if (status == "ok") {
            qDebug() << "Email confirmed, performing auto-login...";

            // 🔥 ВЫПОЛНЯЕМ АВТО-ЛОГИН
            if (m_mainWindow) {
                QJsonObject loginData;
                loginData["username"] = m_tempUsername;
                loginData["password"] = m_tempPassword;

                // 🔥 УСТАНАВЛИВАЕМ m_pendingAction
                m_pendingAction = "login";
                m_networkManager->sendJson("login", loginData);
                qDebug() << "Auto-login sent for:" << m_tempUsername;

                // Не закрываем диалог, ждём ответа на логин
                return; // ← ВАЖНО: не продолжать выполнение
            }
        } else {
            QString error = data["error"].toString();
            QMessageBox::warning(this, "Ошибка", "Неверный код подтверждения: " + error);
            if (m_confirmationDialog) {
                m_confirmationDialog->reject();
            }
        }
    }
    m_waitingForResponse = false;
}

void LoginDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        // Находим текущий виджет в стеке
        QWidget *currentWidget = m_stackedWidget->currentWidget();

        // Логин
        if (currentWidget == m_loginWidget) {
            onLoginClicked();
            return;
        }

        // Регистрация шаг 1
        if (currentWidget == m_registerWidget) {
            onRegisterNextStep();
            return;
        }

        // Регистрация шаг 2
        if (currentWidget == m_passwordWidget) {
            onRegisterClicked();
            return;
        }
    }

    QDialog::keyPressEvent(event);
}

void LoginDialog::onBackToFirstStep()
{
    // ✅ Включаем виджет шага 1 обратно
    m_registerWidget->setEnabled(true);
    m_registerNextBtn->setEnabled(true);

    resizeToRegisterFirstStep();
    m_stackedWidget->setCurrentWidget(m_registerWidget);
}

void LoginDialog::setNetworkManager(NetworkManager *manager)
{
    if (!manager) {
        qFatal("LoginDialog: NetworkManager is null!");
        return;
    }

    if (m_networkManager == manager) {
        qDebug() << "LoginDialog: same NetworkManager, skip";
        return;
    }

    if (m_networkManager) {
        disconnect(m_networkManager, nullptr, this, nullptr);
    }

    m_networkManager = manager;

    qDebug() << "LoginDialog using NetworkManager:" << m_networkManager;

    connect(m_networkManager, &NetworkManager::connected,
            this, &LoginDialog::onNetworkConnected);

    connect(m_networkManager, &NetworkManager::disconnected,
            this, &LoginDialog::onNetworkDisconnected);

    connect(m_networkManager, &NetworkManager::jsonReceived,
            this, &LoginDialog::onJsonResponse);
}

bool LoginDialog::eventFilter(QObject *obj, QEvent *event)
{
    // Обработка перетаскивания окна
    if (obj == m_dragArea || obj->objectName() == "dragArea") {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent*>(event);
            if (mouseEvent && mouseEvent->button() == Qt::LeftButton) {
                m_dragging = true;
                m_dragPosition = mouseEvent->globalPosition().toPoint() - frameGeometry().topLeft();
                return true;
            }
        }
        else if (event->type() == QEvent::MouseMove) {
            QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent*>(event);
            if (mouseEvent && m_dragging && (mouseEvent->buttons() & Qt::LeftButton)) {
                move(mouseEvent->globalPosition().toPoint() - m_dragPosition);
                return true;
            }
        }
        else if (event->type() == QEvent::MouseButtonRelease) {
            if (m_dragging) {
                m_dragging = false;
                return true;
            }
        }
    }

    if (obj == m_qrWidget && event->type() == QEvent::MouseButtonPress) {
        onAlternativeQR();
        return true;
    }

    // Перехватываем Enter
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = dynamic_cast<QKeyEvent*>(event);
        if (keyEvent && (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)) {
            keyPressEvent(keyEvent);
            return true;
        }
    }

    return QDialog::eventFilter(obj, event);
}

void LoginDialog::setupAlternativeLoginForm()
{
    m_alternativeWidget = new QWidget();
    m_alternativeWidget->setStyleSheet("background-color: transparent;");
    m_alternativeWidget->installEventFilter(this);

    QVBoxLayout *layout = new QVBoxLayout(m_alternativeWidget);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    // Контент
    QWidget *contentWidget = new QWidget();
    contentWidget->setStyleSheet("background-color: transparent;");
    QVBoxLayout *content = new QVBoxLayout(contentWidget);
    content->setContentsMargins(0, 12, 0, 25);
    content->setSpacing(0);

    // 1. Заголовок
    QLabel *title = new QLabel("Альтернативный вход");
    title->setStyleSheet("font-size: 22px; font-weight: bold; color: #212121; background: transparent;");
    title->setAlignment(Qt::AlignCenter);
    content->addWidget(title);
    content->addSpacing(6);

    // 2. Подзаголовок
    QLabel *subtitle = new QLabel("Выберите удобный способ входа");
    subtitle->setStyleSheet("font-size: 13px; color: #666666; background: transparent;");
    subtitle->setAlignment(Qt::AlignCenter);
    content->addWidget(subtitle);
    content->addSpacing(20);

    // 3. Кнопки OAuth — равномерно, как табы
    QGridLayout *oauthGrid = new QGridLayout();
    oauthGrid->setSpacing(4);                     // как у табов
    oauthGrid->setContentsMargins(0, 0, 0, 0);

    // 3. Кнопки OAuth — точные 110×68, отступ 11px, поля 4px (как у табов)
    auto createOAuthBtn = [&](const QString &text, const QString &iconPath) -> QPushButton* {
        QPushButton *btn = new QPushButton();
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFixedSize(110, 68);                          // ширина 110, чтобы влезло 3 шт.
        btn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        btn->setStyleSheet(
            "QPushButton { background: white; border: 2px solid #e0e0e0; border-radius: 12px; }"
            "QPushButton:hover { background: #f5f5f5; border-color: #cccccc; }"
            );
        QVBoxLayout *btnLayout = new QVBoxLayout(btn);
        btnLayout->setContentsMargins(0, 12, 0, 8);
        btnLayout->setSpacing(6);
        QLabel *iconLabel = new QLabel();
        QPixmap pixmap(iconPath);
        iconLabel->setPixmap(pixmap.scaled(28, 28, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        iconLabel->setAlignment(Qt::AlignCenter);
        iconLabel->setStyleSheet("background: transparent;");
        btnLayout->addWidget(iconLabel);
        QLabel *textLabel = new QLabel(text);
        textLabel->setAlignment(Qt::AlignCenter);
        textLabel->setStyleSheet("color: #333; font-size: 12px; background: transparent;");
        btnLayout->addWidget(textLabel);
        return btn;
    };

    QPushButton *googleBtn = createOAuthBtn("Google", ":/icons/authorization/images/authorization/google-brands.svg");
    QPushButton *githubBtn = createOAuthBtn("GitHub", ":/icons/authorization/images/authorization/github-brands.svg");
    QPushButton *appleBtn = createOAuthBtn("Apple", ":/icons/authorization/images/authorization/apple-brands.svg");

    connect(googleBtn, &QPushButton::clicked, this, &LoginDialog::onAlternativeGoogle);
    connect(githubBtn, &QPushButton::clicked, this, &LoginDialog::onAlternativeGitHub);
    connect(appleBtn, &QPushButton::clicked, this, &LoginDialog::onAlternativeApple);

    QHBoxLayout *oauthRow = new QHBoxLayout();
    oauthRow->setSpacing(8);                     // расстояние между кнопками
    oauthRow->setContentsMargins(4, 0, 4, 0);     // ← ТАКИЕ ЖЕ отступы, как у табов
    oauthRow->addWidget(googleBtn);
    oauthRow->addWidget(githubBtn);
    oauthRow->addWidget(appleBtn);
    content->addLayout(oauthRow);
    content->addSpacing(20);

    // 4. Разделитель "Или"
    QWidget *orWidget = new QWidget();
    QHBoxLayout *orLayout = new QHBoxLayout(orWidget);
    orLayout->setContentsMargins(0, 0, 0, 0);
    QFrame *lineLeft = new QFrame(); lineLeft->setFrameShape(QFrame::HLine); lineLeft->setStyleSheet("color: #e0e0e0;");
    QLabel *orLabel = new QLabel("Или"); orLabel->setStyleSheet("color: #999; font-size: 13px; background: transparent; padding: 0 10px;");
    QFrame *lineRight = new QFrame(); lineRight->setFrameShape(QFrame::HLine); lineRight->setStyleSheet("color: #e0e0e0;");
    orLayout->addWidget(lineLeft, 1); orLayout->addWidget(orLabel); orLayout->addWidget(lineRight, 1);
    content->addWidget(orWidget);
    content->addSpacing(16);

    // 5. QR-код
    m_qrWidget = new QWidget();
    m_qrWidget->setCursor(Qt::PointingHandCursor);
    QVBoxLayout *qrLayout = new QVBoxLayout(m_qrWidget);
    qrLayout->setContentsMargins(20, 8, 20, 8);
    qrLayout->setSpacing(10);
    qrLayout->setAlignment(Qt::AlignCenter);

    QWidget *qrBox = new QWidget();
    qrBox->setFixedSize(160, 160);
    qrBox->setStyleSheet("background: #f0f0f0; border: 2px solid #e0e0e0; border-radius: 12px;");
    QWidget *qrInner = new QWidget(qrBox);
    qrInner->setFixedSize(40, 40);
    qrInner->move(60, 60);
    qrInner->setStyleSheet("background: #0088cc; border-radius: 4px;");
    qrLayout->addWidget(qrBox, 0, Qt::AlignCenter);

    QLabel *qrTitle = new QLabel("Войти по QR-коду");
    qrTitle->setStyleSheet("color: #212121; font-size: 14px; font-weight: 600; background: transparent;");
    qrTitle->setAlignment(Qt::AlignCenter);
    qrLayout->addWidget(qrTitle);

    QLabel *qrDesc = new QLabel("1. Откройте приложение на телефоне\n2. Нажмите \"Войти по QR-коду\"\n3. Наведите камеру на этот код");
    qrDesc->setStyleSheet("color: #999; font-size: 11px; background: transparent;");
    qrDesc->setAlignment(Qt::AlignCenter);
    qrLayout->addWidget(qrDesc);

    content->addWidget(m_qrWidget);
    content->addSpacing(16);

    // 6. Кнопка "Назад"
    QPushButton *backBtn = new QPushButton("← Назад к обычному входу");
    backBtn->setCursor(Qt::PointingHandCursor);
    backBtn->setMinimumHeight(44);
    backBtn->setStyleSheet("QPushButton { background: transparent; border: none; color: #0088cc; font-size: 14px; } QPushButton:hover { text-decoration: underline; }");
    connect(backBtn, &QPushButton::clicked, this, &LoginDialog::onBackFromAlternative);
    content->addWidget(backBtn);

    layout->addWidget(contentWidget);
}

void LoginDialog::onAlternativeGoogle()
{
    QMessageBox::information(this, "Google", "Вход через Google будет доступен в следующей версии.");
}

void LoginDialog::onAlternativeGitHub()
{
    QMessageBox::information(this, "GitHub", "Вход через GitHub будет доступен в следующей версии.");
}

void LoginDialog::onAlternativeApple()
{
    QMessageBox::information(this, "Apple", "Вход через Apple будет доступен в следующей версии.");
}

void LoginDialog::onAlternativeQR()
{
    QMessageBox::information(this, "QR-код", "Вход по QR-коду будет доступен в следующей версии.");
}

void LoginDialog::onBackFromAlternative()
{
    m_alternativeWidget->setContentsMargins(0, 0, 0, 0);  // ✅ сброс
    m_stackedWidget->setCurrentWidget(m_loginWidget);
    setWindowTitle("Aura - Вход");
    resizeToLoginSize();
}

void LoginDialog::setMainWindow(MainWindow *mw)
{
    qDebug() << "LoginDialog::setMainWindow called with:" << mw;
    m_mainWindow = mw;
}
