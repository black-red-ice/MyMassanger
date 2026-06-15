#include "../../mainwindow.h"
#include "DocumentsPanel.h"
#include "UploadDocumentDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QUuid>
#include <QFileDialog>
#include <QFileInfo>
#include <QDateTime>
#include <QFileDialog>
#include <QEvent>
#include <QMessageBox>

DocumentsPanel::DocumentsPanel(QWidget *parent)
    : SidePanel(parent,
                "Документы",
                ":/icons/general/images/general/folder-light.svg",
                "#F59E0B",
                "Загрузить документ",
                400,
                "#F59E0B",
                ":/icons/general/images/general/cloud-arrow-up-light.svg")  // ← иконка кнопки
{
    getSearchInput()->setPlaceholderText("Поиск документов...");
    connect(getSearchInput(), &QLineEdit::textChanged, this, &DocumentsPanel::filterDocuments);

    // Убираем стандартный stretch из contentLayout
    QLayoutItem *stretch = getContentLayout()->takeAt(getContentLayout()->count() - 1);
    delete stretch;

    // Добавляем свой лейаут
    m_documentsLayout = new QVBoxLayout();
    m_documentsLayout->setSpacing(8);
    getContentLayout()->addLayout(m_documentsLayout);

    // Добавляем stretch в конец
    getContentLayout()->addStretch();

    render();
}

void DocumentsPanel::onAddClicked()
{
    QWidget *dimWidget = new QWidget(this->window());
    dimWidget->setObjectName("uploadDim");
    dimWidget->setStyleSheet("#uploadDim { background-color: rgba(0, 0, 0, 180); }");
    dimWidget->setGeometry(this->window()->rect());
    dimWidget->raise();
    dimWidget->show();

    UploadDocumentDialog dialog(dimWidget);
    if (dialog.exec() == QDialog::Accepted) {
        QStringList files = dialog.selectedFiles();

        // 🔥 СОХРАНЯЕМ ВРЕМЕННО В ОТДЕЛЬНЫЙ СПИСОК
        QList<Document> newDocuments;

        for (const QString &filePath : files) {
            QFileInfo fi(filePath);
            Document doc;
            doc.id = QUuid::createUuid().toString();
            doc.name = fi.fileName();
            doc.author = m_currentUsername;
            doc.date = QDateTime::currentDateTime().toString("dd.MM.yyyy");
            qint64 size = fi.size();
            if (size < 1024) doc.size = QString::number(size) + " Б";
            else if (size < 1024 * 1024) doc.size = QString::number(size / 1024.0, 'f', 1) + " КБ";
            else doc.size = QString::number(size / (1024.0 * 1024.0), 'f', 1) + " МБ";
            doc.category = dialog.category();

            newDocuments.append(doc);

            // Загружаем файл на сервер
            MainWindow *mw = qobject_cast<MainWindow*>(this->window());
            if (mw) {
                m_pendingDocId = doc.id;
                mw->uploadFileHttp(filePath, this, m_pendingDocId);
            }
        }

        // 🔥 ДОБАВЛЯЕМ ТОЛЬКО ОДИН РАЗ ПОСЛЕ ЦИКЛА
        if (!newDocuments.isEmpty()) {
            for (const Document &doc : newDocuments) {
                m_documents.append(doc);
            }
            render();

            MainWindow *mw = qobject_cast<MainWindow*>(this->window());
            if (mw) {
                mw->saveDocumentsToServer();
            }
        }
    }
    dimWidget->deleteLater();
}

void DocumentsPanel::filterDocuments(const QString &query)
{
    // 🔥 ОЧИЩАЕМ ВСЁ, ВКЛЮЧАЯ STRETCH
    while (m_documentsLayout->count() > 0) {
        QLayoutItem *item = m_documentsLayout->takeAt(0);
        if (item->widget()) delete item->widget();
        delete item;
    }

    for (const Document &doc : m_documents) {
        if (query.isEmpty() ||
            doc.name.contains(query, Qt::CaseInsensitive) ||
            doc.author.contains(query, Qt::CaseInsensitive) ||
            doc.category.contains(query, Qt::CaseInsensitive)) {
            QWidget *card = createDocumentCard(doc);
            m_documentsLayout->addWidget(card);
        }
    }

    // Добавляем stretch в конец
    m_documentsLayout->addStretch();
}

void DocumentsPanel::render()
{
    // 🔥 ОЧИЩАЕМ ВСЁ, ВКЛЮЧАЯ STRETCH
    while (m_documentsLayout->count() > 0) {
        QLayoutItem *item = m_documentsLayout->takeAt(0);
        if (item->widget()) delete item->widget();
        delete item;
    }

    for (const Document &doc : m_documents) {
        QWidget *card = createDocumentCard(doc);
        m_documentsLayout->addWidget(card);
    }

    // Добавляем stretch в конец
    m_documentsLayout->addStretch();
}

QString DocumentsPanel::getFileIcon(const QString &fileName) const
{
    QString ext = QFileInfo(fileName).suffix().toLower();
    QString base = ":/icons/general/images/general/";
    if (ext == "mp3" || ext == "wav" || ext == "flac" || ext == "aac" || ext == "ogg" || ext == "wma") return base + "file-audio.svg";
    if (ext == "cpp" || ext == "h" || ext == "py" || ext == "js" || ext == "html" || ext == "css" ||
        ext == "java" || ext == "php" || ext == "json" || ext == "xml" || ext == "sql") return base + "file-code.svg";
    if (ext == "csv") return base + "file-csv.svg";
    if (ext == "xls" || ext == "xlsx" || ext == "xlsm") return base + "file-excel.svg";
    if (ext == "jpg" || ext == "jpeg" || ext == "png" || ext == "gif" || ext == "bmp" ||
        ext == "svg" || ext == "webp" || ext == "ico") return base + "file-image.svg";
    if (ext == "txt" || ext == "log" || ext == "md" || ext == "rtf" || ext == "cfg" || ext == "ini") return base + "file-lines.svg";
    if (ext == "pdf") return base + "file-pdf.svg";
    if (ext == "ppt" || ext == "pptx" || ext == "pptm" || ext == "ppsx") return base + "file-powerpoint.svg";
    if (ext == "mp4" || ext == "avi" || ext == "mkv" || ext == "mov" || ext == "wmv" ||
        ext == "flv" || ext == "webm") return base + "file-video.svg";
    if (ext == "doc" || ext == "docx" || ext == "docm" || ext == "odt") return base + "file-word.svg";
    if (ext == "zip" || ext == "rar" || ext == "7z" || ext == "tar" || ext == "gz" || ext == "bz2") return base + "file-zipper.svg";
    if (fileName.isEmpty() || ext.isEmpty()) return base + "folder.svg";
    return base + "file.svg";
}

QJsonArray DocumentsPanel::getDocumentsAsJson() const
{
    QJsonArray arr;
    for (const Document &doc : m_documents) {
        QJsonObject obj;
        obj["id"] = doc.id;
        obj["name"] = doc.name;
        obj["author"] = doc.author;
        obj["date"] = doc.date;
        obj["size"] = doc.size;
        obj["category"] = doc.category;
        obj["fileUrl"] = doc.fileUrl;  // ДОБАВИТЬ
        arr.append(obj);
    }
    return arr;
}

void DocumentsPanel::setDocumentsFromJson(const QJsonArray &arr)
{
    m_documents.clear();
    for (const QJsonValue &v : arr) {
        QJsonObject obj = v.toObject();
        Document doc;
        doc.id = obj["id"].toString();
        doc.name = obj["name"].toString();
        doc.author = obj["author"].toString();
        doc.date = obj["date"].toString();
        doc.size = obj["size"].toString();
        doc.category = obj["category"].toString();
        doc.fileUrl = obj["fileUrl"].toString();  // ДОБАВИТЬ
        m_documents.append(doc);
    }
    render();
}

void DocumentsPanel::addDocument(const Document &doc)
{
    // 🔥 ПРОВЕРКА НА ДУБЛИКАТ ПО ID
    for (const Document &existing : m_documents) {
        if (existing.id == doc.id) {
            qDebug() << "Document already exists:" << doc.name;
            return;
        }
    }

    m_documents.append(doc);
    render();

    MainWindow *mw = qobject_cast<MainWindow*>(this->window());
    if (mw) {
        mw->saveDocumentsToServer();
    }
}

bool DocumentsPanel::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QWidget *widget = qobject_cast<QWidget*>(obj);

        // 🔥 ПРОВЕРЯЕМ, ЕСТЬ ЛИ PROPERTY docId У САМОГО ВИДЖЕТА ИЛИ У ЕГО РОДИТЕЛЯ
        QString docId;
        if (widget && widget->property("docId").isValid()) {
            docId = widget->property("docId").toString();
        } else if (widget && widget->parentWidget() && widget->parentWidget()->property("docId").isValid()) {
            docId = widget->parentWidget()->property("docId").toString();
        } else if (widget && widget->parentWidget() && widget->parentWidget()->parentWidget() &&
                   widget->parentWidget()->parentWidget()->property("docId").isValid()) {
            docId = widget->parentWidget()->parentWidget()->property("docId").toString();
        }

        if (!docId.isEmpty()) {
            onDocumentClicked(docId);
            return true;
        }
    }
    return SidePanel::eventFilter(obj, event);
}
void DocumentsPanel::onDocumentClicked(const QString &docId)
{
    // Найти документ по id
    Document *foundDoc = nullptr;
    for (Document &doc : m_documents) {
        if (doc.id == docId) {
            foundDoc = &doc;
            break;
        }
    }

    if (!foundDoc) return;

    // Проверяем, есть ли URL файла
    if (foundDoc->fileUrl.isEmpty()) {
        qDebug() << "No fileUrl for document:" << foundDoc->name;
        return;
    }

    // Предлагаем сохранить файл
    QString savePath = QFileDialog::getSaveFileName(this, "Сохранить файл", foundDoc->name);
    if (savePath.isEmpty()) return;

    // Скачиваем файл с сервера
    MainWindow *mw = qobject_cast<MainWindow*>(this->window());
    if (mw) {
        mw->downloadFileHttp(foundDoc->fileUrl, savePath);
    }
}

QWidget* DocumentsPanel::createDocumentCard(const Document &doc)
{
    QWidget *card = new QWidget();
    card->setStyleSheet(
        "QWidget#docCard { background: #1e293b; border-radius: 12px; border: 1px solid #334155; }"
        "QWidget#docCard:hover { border: 1px solid #F59E0B; }"
        );
    card->setObjectName("docCard");
    card->setCursor(Qt::PointingHandCursor);

    QHBoxLayout *cardLayout = new QHBoxLayout(card);
    cardLayout->setContentsMargins(16, 12, 16, 12);
    cardLayout->setSpacing(12);

    QWidget *iconContainer = new QWidget();
    iconContainer->setFixedSize(40, 40);
    iconContainer->setStyleSheet("background: #343536; border-radius: 8px;");

    QLabel *docIcon = new QLabel(iconContainer);
    QPixmap pixmap(getFileIcon(doc.name));
    docIcon->setPixmap(pixmap.scaled(22, 22, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    docIcon->setAlignment(Qt::AlignCenter);
    docIcon->setGeometry(0, 0, 40, 40);
    docIcon->setStyleSheet("background: transparent;");
    cardLayout->addWidget(iconContainer);

    QVBoxLayout *infoLayout = new QVBoxLayout();
    infoLayout->setSpacing(4);
    infoLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    QLabel *nameLabel = new QLabel(doc.name);
    nameLabel->setStyleSheet("color: white; font-size: 14px; font-weight: 600; background: transparent;");
    nameLabel->setWordWrap(true);
    nameLabel->setMaximumWidth(280);
    nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    infoLayout->addWidget(nameLabel);

    QLabel *metaLabel = new QLabel(doc.author + " | " + doc.date + " | " + doc.size);
    metaLabel->setStyleSheet("color: #94A3B8; font-size: 12px; background: transparent;");
    metaLabel->setWordWrap(true);
    metaLabel->setMaximumWidth(280);
    infoLayout->addWidget(metaLabel);

    cardLayout->addLayout(infoLayout, 1);

    card->setCursor(Qt::PointingHandCursor);

    // 🔥 УСТАНАВЛИВАЕМ ФИЛЬТР СОБЫТИЙ НА ВСЕ ДОЧЕРНИЕ ВИДЖЕТЫ
    card->installEventFilter(this);
    nameLabel->installEventFilter(this);
    metaLabel->installEventFilter(this);
    iconContainer->installEventFilter(this);
    docIcon->installEventFilter(this);

    card->setProperty("docId", doc.id);
    card->setProperty("docName", doc.name);
    card->setProperty("fileUrl", doc.fileUrl);  // 🔥 ДОБАВИТЬ
    nameLabel->setProperty("docId", doc.id);
    nameLabel->setProperty("fileUrl", doc.fileUrl);  // 🔥 ДОБАВИТЬ
    metaLabel->setProperty("docId", doc.id);
    iconContainer->setProperty("docId", doc.id);
    docIcon->setProperty("docId", doc.id);

    return card;
}

void DocumentsPanel::updateDocumentUrl(const QString &docId, const QString &fileUrl)
{
    for (Document &doc : m_documents) {
        if (doc.id == docId) {
            doc.fileUrl = fileUrl;
            qDebug() << "✅ Updated fileUrl for document:" << doc.name << "->" << fileUrl;
            break;
        }
    }
    // Перерисовываем, чтобы обновить свойство fileUrl в карточках
    render();
}
