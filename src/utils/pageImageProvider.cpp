// pageImageProvider.cpp
#include "pageImageProvider.h"
#include "pdfModel.h"
#include <QElapsedTimer>
#include <QDebug>

PageImageProvider::PageImageProvider(Poppler::Document* pdfDocument)
    : QQuickImageProvider(QQuickImageProvider::Image,
                         QQmlImageProviderBase::ForceAsynchronousImageLoading)
    , document(pdfDocument)
{}

QImage PageImageProvider::requestImage(const QString& id, QSize* size,
                                     const QSize& requestedSize)
{
    QElapsedTimer timer;
    timer.start();

    QString type = id.section("/", 0, 0);
    QImage result;

    if (document && type == "page")
    {
        bool ok;
        int numPage = id.section("/", 1, 1).toInt(&ok);

        if (!ok)
        {
            qWarning() << "Invalid page number in request:" << id;
            return result;
        }

        DEBUG << "Page" << numPage << "requested";

        std::unique_ptr<Poppler::Page> page(document->page(numPage - 1));
        if (!page)
        {
            qWarning() << "Failed to load page" << numPage;
            return result;
        }

        QSizeF pageSize = page->pageSizeF();
        DEBUG << "Requested size:" << requestedSize << "Page size:" << pageSize;

        // Calculate resolution
        double res;
        if (requestedSize.isValid() && requestedSize.width() > 0)
        {
            res = requestedSize.width() / (pageSize.width() / 72.0);
        }
        else
        {
            res = 72.0; // Default to 72 DPI if no size requested
        }

        DEBUG << "Rendering resolution:" << res << "dpi";

        result = page->renderToImage(res, res);
        if (result.isNull())
        {
            qWarning() << "Failed to render page" << numPage;
            return QImage();
        }

        if (size)
        {
            *size = result.size();
        }

        DEBUG << "Page rendered in" << timer.elapsed() << "ms." << result.size();
    }
    else
    {
        qWarning() << "Invalid request or no document:" << id;
    }

    return result;
}
