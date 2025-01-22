#include <QApplication>
#include <QQmlApplicationEngine>
#include <QtQml>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <KLocalizedContext>
#include <KLocalizedString>
#include <Kirigami/Platform/PlatformTheme>
#include <KColorSchemeManager>
#include <KIconThemes/kicontheme.h>

#include <backend/midiclient.h>
#include <utils/pdfModel.h>


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    KIconTheme::current();
    QApplication::setStyle("breeze");
    KLocalizedString::setApplicationDomain("Music");
    QCoreApplication::setOrganizationName(QStringLiteral("SpiritMusic"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("Spiritmusic.com"));
    QCoreApplication::setApplicationName(QStringLiteral("SpiritSheet"));

    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
        QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));
    }





    QQmlApplicationEngine engine;

    MidiClient *midiClient = new MidiClient();
    engine.rootContext()->setContextProperty("midiClient", midiClient);

    qmlRegisterType<PdfModel>("com.SpiritMusic.Poppler", 1, 0, "Poppler");

    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    const QUrl url(QStringLiteral("qrc:/SpiritSheet/contents/ui/Main.qml"));
    QObject::connect(
                &engine,
                &QQmlApplicationEngine::objectCreationFailed,
                &app,
                []() { QCoreApplication::exit(-1); },
    Qt::QueuedConnection);
    engine.load(url);
//    engine.loadFromModule("SpiritSheet", "Main");

    return app.exec();
}
