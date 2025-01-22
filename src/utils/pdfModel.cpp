// pdfModel.cpp
#include "pdfModel.h"
#include "pageImageProvider.h"
#include <QDebug>
#include <QQmlEngine>
#include <QQmlContext>

static QVariantMap convertDestination(const Poppler::LinkDestination& destination)
{
    QVariantMap result;
    result["page"] = destination.pageNumber() - 1;
    result["top"] = destination.top();
    result["left"] = destination.left();
    return result;
}

PdfModel::PdfModel(QObject* parent)
    : QObject(parent)
{}

void PdfModel::setPath(QString& pathName)
{
    if (pathName == path)
        return;

    if (pathName.isEmpty())
    {
        DEBUG << "Can't load the document, path is empty.";
        emit error("Can't load the document, path is empty.");
        return;
    }

    this->path = pathName;
    emit pathChanged(pathName);

    // Load document
    clear();
    DEBUG << "Loading document...";
    document = std::unique_ptr<Poppler::Document>(Poppler::Document::load(path));

    if (!document || document->isLocked())
    {
        DEBUG << "ERROR : Can't open the document located at " + pathName;
        emit error("Can't open the document located at " + pathName);
        document.reset();
        return;
    }

    // Create image provider
    document->setRenderHint(Poppler::Document::Antialiasing, true);
    document->setRenderHint(Poppler::Document::TextAntialiasing, true);
    loadProvider();

    // Fill in pages data
    const int numPages = document->numPages();
    for (int i = 0; i < numPages; ++i)
    {
        std::unique_ptr<Poppler::Page> page(document->page(i));

        QVariantMap pageData;
        pageData["image"] = "image://" + providerName + "/page/" + QString::number(i + 1);
        pageData["size"] = page->pageSizeF();

        QVariantList pageLinks;
        auto links = page->links();
        for (const auto& link : links)
        {
            if (link->linkType() == Poppler::Link::Goto)
            {
                auto* gotoLink = dynamic_cast<Poppler::LinkGoto*>(link.get());
                if (gotoLink && !gotoLink->isExternal())
                {
                    QVariantMap linkMap;
                    linkMap["rect"] = link->linkArea().normalized();
                    linkMap["destination"] = convertDestination(gotoLink->destination());
                    pageLinks.append(linkMap);
                }
            }
        }
        pageData["links"] = pageLinks;

        pages.append(pageData);
    }
    emit pagesChanged();

    DEBUG << "Document loaded successfully";
    emit loadedChanged();
}

void PdfModel::loadProvider()
{
    DEBUG << "Loading image provider...";
    QQmlEngine* engine = QQmlEngine::contextForObject(this)->engine();

    const QString& prefix = QString::number(quintptr(this));
    providerName = "poppler" + prefix;
    engine->addImageProvider(providerName, new PageImageProvider(document.get()));

    DEBUG << "Image provider loaded successfully !" << qPrintable("(" + providerName + ")");
}

void PdfModel::clear()
{
    if (!providerName.isEmpty())
    {
        QQmlEngine* engine = QQmlEngine::contextForObject(this)->engine();
        if (engine)
            engine->removeImageProvider(providerName);
        providerName.clear();
    }

    document.reset();
    emit loadedChanged();
    pages.clear();
    emit pagesChanged();
}

QVariantList PdfModel::getPages() const
{
    return pages;
}

bool PdfModel::getLoaded() const
{
    return document != nullptr;
}

QVariantList PdfModel::search(int page, const QString& text, Qt::CaseSensitivity caseSensitivity)
{
    QVariantList result;
    if (!document)
    {
        qWarning() << "Poppler plugin: no document to search";
        return result;
    }

    if (page >= document->numPages() || page < 0)
    {
        qWarning() << "Poppler plugin: search page" << page << "isn't in a document";
        return result;
    }

    std::unique_ptr<Poppler::Page> p(document->page(page));
    auto searchResult = p->search(text, caseSensitivity == Qt::CaseInsensitive ?
                                Poppler::Page::IgnoreCase :
                                static_cast<Poppler::Page::SearchFlag>(0));

    auto pageSize = p->pageSizeF();
    for (const auto& r : searchResult)
    {
        result.append(QRectF(r.left() / pageSize.width(),
                            r.top() / pageSize.height(),
                            r.width() / pageSize.width(),
                            r.height() / pageSize.height()));
    }
    return result;
}

PdfModel::~PdfModel()
{
    clear();
}
