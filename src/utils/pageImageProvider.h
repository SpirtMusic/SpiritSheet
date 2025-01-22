// pageImageProvider.h
#ifndef PAGEIMAGEPROVIDER_H
#define PAGEIMAGEPROVIDER_H

#include <QQuickImageProvider>
#include <poppler-qt6.h>
#include <memory>

class PageImageProvider : public QQuickImageProvider
{
public:
    explicit PageImageProvider(Poppler::Document* pdfDocument = nullptr);
    QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize) override;

private:
    Poppler::Document* document; // Non-owning pointer
};

#endif // PAGEIMAGEPROVIDER_H
