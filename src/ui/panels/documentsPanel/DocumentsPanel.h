#pragma once
#include "../SidePanel.h"
#include <QList>

struct Document {
    QString id;
    QString name;
    QString author;
    QString date;
    QString size;
    QString category;
    QString fileUrl;
};

class DocumentsPanel : public SidePanel
{
    Q_OBJECT

signals:
    void downloadFileRequested(const QString &fileUrl, const QString &fileName);

public:
    explicit DocumentsPanel(QWidget *parent = nullptr);
    QJsonArray getDocumentsAsJson() const;
    void setDocumentsFromJson(const QJsonArray &arr);
    void addDocument(const Document &doc);
    void setUsername(const QString &username) { m_currentUsername = username; }
    bool eventFilter(QObject *obj, QEvent *event);
    void updateDocumentUrl(const QString &docId, const QString &fileUrl);

protected:
    void onAddClicked() override;

private slots:
    void filterDocuments(const QString &query);
    void onDocumentClicked(const QString &fileId);

private:
    QList<Document> m_documents;
    QVBoxLayout *m_documentsLayout;

    void render();
    QWidget* createDocumentCard(const Document &doc);
    QString getFileIcon(const QString &fileName) const;
    QString m_currentUsername;
    QString m_pendingDocId;
};
