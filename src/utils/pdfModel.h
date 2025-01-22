// pdfModel.h
#ifndef PDFMODEL_H
#define PDFMODEL_H

#include <QObject>
#include <memory>
#include <poppler-qt6.h>

#define DEBUG if (qgetenv("POPPLERPLUGIN_DEBUG") == "1") qDebug() << "Poppler plugin:"

class PdfModel : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(PdfModel)

public:
    explicit PdfModel(QObject* parent = nullptr);
    virtual ~PdfModel();

    Q_PROPERTY(QString path READ getPath WRITE setPath NOTIFY pathChanged)
    Q_PROPERTY(bool loaded READ getLoaded NOTIFY loadedChanged)
    Q_PROPERTY(QVariantList pages READ getPages NOTIFY pagesChanged)

    void setPath(QString& pathName);
    QString getPath() const { return path; }
    QVariantList getPages() const;
    bool getLoaded() const;

    Q_INVOKABLE QVariantList search(int page, const QString& text,
                                   Qt::CaseSensitivity caseSensitivity = Qt::CaseSensitive);

signals:
    void pathChanged(const QString& newPath);
    void loadedChanged();
    void error(const QString& errorMessage);
    void pagesChanged();

private:
    void loadProvider();
    void clear();

    std::unique_ptr<Poppler::Document> document;
    QString providerName;
    QString path;
    QVariantList pages;
};

Q_DECLARE_METATYPE(PdfModel*)

#endif // PDFMODEL_H
