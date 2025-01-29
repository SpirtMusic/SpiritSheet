cmake_minimum_required(VERSION 3.16)

project(SpiritSheet VERSION 0.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(ECM   REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})
set(LIBREMIDI_NO_WINUWP 0)
find_package(libremidi REQUIRED)
add_definitions(-DLIBREMIDI_HEADER_ONLY=1)
add_definitions(-DLIBREMIDI_JACK=1)
find_package(Qt6 6.5 REQUIRED COMPONENTS Quick QuickControls2 Widgets)
find_package(KF6  REQUIRED COMPONENTS Kirigami
    I18n
    CoreAddons
    ColorScheme
    IconThemes
    QQC2DesktopStyle

)
find_package(Poppler COMPONENTS Qt6)
set_package_properties("Poppler" PROPERTIES
    TYPE RECOMMENDED
    PURPOSE "Support for PDF file operations.")
qt_standard_project_setup(REQUIRES 6.5)
qt_policy(SET QTP0001 OLD)
qt_add_executable(appSpiritSheet
    main.cpp
)

qt_add_qml_module(appSpiritSheet
    URI SpiritSheet
    VERSION 1.0
    QML_FILES
    contents/ui/Main.qml
    SOURCES backend/jackclient.cpp backend/jackclient.h backend/midiutils.h
    SOURCES backend/midiclient.cpp
    SOURCES backend/midiclient.h
    SOURCES backend/midiportmodel.cpp backend/midiportmodel.h
    SOURCES utils/pageImageProvider.cpp utils/pageImageProvider.h utils/pdfModel.cpp utils/pdfModel.h
    QML_FILES contents/ui/components/PDFView.qml
    QML_FILES contents/ui/settings/GeneralPage.qml
    QML_FILES contents/ui/settings/MidiPage.qml
    SOURCES utils/settings.h utils/settings.cpp
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appSpiritSheet PROPERTIES
    #    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appSpiritSheet
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appSpiritSheet
    PRIVATE Qt6::Quick  Qt6::QuickControls2 Qt6::Widgets
    KF6::Kirigami
    KF6::I18n
    KF6::CoreAddons
    KF6::ColorScheme
    KF6::IconThemes
    jack
    Poppler::Qt6
)

include(GNUInstallDirs)
install(TARGETS appSpiritSheet
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
