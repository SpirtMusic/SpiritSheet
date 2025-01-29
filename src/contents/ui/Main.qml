import QtQuick
import QtCore
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Dialogs
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as KirigamiAddons
import org.kde.kirigamiaddons.settings as KirigamiSettings
import "components"

Kirigami.ApplicationWindow {
    id: root
    width: 800
    height: 600
    title: i18nc("@title:window", "PDF Viewer")

    property bool pdfLoaded: false
    property real zoomValue: 100  // Add this property
    property int viewMode: 0  // 0: Scroll, 1: Single Page, 2: Book View
    pageStack.initialPage: Kirigami.Page {
        id: mainPage
        padding: 0
        actions: [
            Kirigami.Action {
                icon.name: "configure"
                text: i18n("Settings")
                onTriggered: settingsView.open()
            }
        ]
        // PDF View
        PDFView {
            id: pdfView
            anchors.fill: parent
            visible: root.pdfLoaded
            property real rotation: 0
            clip:true
            transform: Rotation {
                angle: pdfView.rotation
                origin.x: pdfView.width/2
                origin.y: pdfView.height/2
            }
            // add: Transition {
            //     NumberAnimation { properties: "x"; from: isHorizontal ? width : 0; duration: 200 }
            // }
            // remove: Transition {
            //     NumberAnimation { properties: "x"; to: isHorizontal ? -width : 0; duration: 200 }
            // }
            onErrorOccurred: {
                console.error("PDF Error:", message)
                root.pdfLoaded = false
                showPassiveNotification("Error loading PDF: " + message, "long")
            }
        }

        // Floating Toolbar
        KirigamiAddons.FloatingToolBar {
            id: toolBar
            visible: root.pdfLoaded
            z: 100
            anchors {
                horizontalCenter: parent.horizontalCenter
                bottom: parent.bottom
                bottomMargin: Kirigami.Units.largeSpacing * 2
            }

            contentItem: Kirigami.ActionToolBar {
                actions: [
                    // File Operations
                    Kirigami.Action {
                        icon.name: "document-open"
                        text: i18n("Open")
                        displayHint: Kirigami.DisplayHint.IconOnly
                        tooltip: i18n("Open PDF File")
                        onTriggered: fileDialog.open()
                    },

                    Kirigami.Action { separator: true },

                    // Navigation
                    Kirigami.Action {
                        icon.name: "go-first"
                        text: i18n("First Page")
                        displayHint: Kirigami.DisplayHint.IconOnly
                        tooltip: i18n("Go to First Page")
                        enabled: pdfView.currentPage > 0
                        onTriggered: pdfView.positionViewAtIndex(0, ListView.Beginning)
                    },
                    Kirigami.Action {
                        icon.name: "go-previous"
                        text: i18n("Previous")
                        displayHint: Kirigami.DisplayHint.IconOnly
                        tooltip: i18n("Previous Page")
                        enabled: pdfView.currentIndex > 0
                        onTriggered: pdfView.goToPreviousPage()
                    },

                    Kirigami.Action {
                        displayComponent: QQC2.SpinBox {
                            id: pageSpinBox
                            from: 1
                            to: pdfView.count
                            value: pdfView.isBookMode ?
                                       (pdfView.currentIndex * 2) + 1 :
                                       pdfView.currentIndex + 1
                            onValueModified: {
                                var newIndex = pdfView.isBookMode ?
                                    Math.floor((value - 1) / 2) :
                                    value - 1
                                pdfView.currentIndex = newIndex
                            }
                        }
                    },
                    Kirigami.Action {
                        displayComponent: QQC2.Label {
                            text: i18n("of %1", pdfView.count)
                        }
                    },
                    Kirigami.Action {
                        icon.name: "go-next"
                        text: i18n("Next")
                        displayHint: Kirigami.DisplayHint.IconOnly
                        tooltip: i18n("Next Page")
                        enabled: pdfView.currentIndex < pdfView.count - 1
                        onTriggered: pdfView.goToNextPage()
                    },


                    Kirigami.Action {
                        icon.name: "go-last"
                        text: i18n("Last Page")
                        displayHint: Kirigami.DisplayHint.IconOnly
                        tooltip: i18n("Go to Last Page")
                        enabled: pdfView.currentPage < pdfView.count - 1
                        onTriggered: pdfView.positionViewAtIndex(pdfView.count - 1, ListView.Beginning)
                    },

                    Kirigami.Action { separator: true },

                    // Zoom Controls
                    Kirigami.Action {
                        icon.name: "zoom-out"
                        text: i18n("Zoom Out")
                        displayHint: Kirigami.DisplayHint.IconOnly
                        tooltip: i18n("Decrease Zoom")
                        enabled: root.zoomValue > 10  // minimum zoom
                        onTriggered: root.zoomValue = Math.max(root.zoomValue - 10, 10)
                    },
                    Kirigami.Action {
                        displayComponent: QQC2.SpinBox {
                            from: 10
                            to: 500
                            stepSize: 10
                            value: root.zoomValue
                            onValueChanged: {
                                root.zoomValue = value
                                pdfView.zoom = value / 100
                            }
                        }
                    },
                    Kirigami.Action {
                        icon.name: "zoom-in"
                        text: i18n("Zoom In")
                        displayHint: Kirigami.DisplayHint.IconOnly
                        tooltip: i18n("Increase Zoom")
                        enabled: root.zoomValue < 500  // maximum zoom
                        onTriggered: root.zoomValue = Math.min(root.zoomValue + 10, 500)
                    },
                    Kirigami.Action {
                        icon.name: "zoom-fit-width"
                        text: i18n("Fit Width")
                        displayHint: Kirigami.DisplayHint.IconOnly
                        tooltip: i18n("Fit to Width")
                        onTriggered: {
                            if (pdfView.currentPage >= 0) {
                                var pageWidth = pdfView.poppler.pages[pdfView.currentPage].size.width
                                var scale = (pdfView.width - 40) / pageWidth
                                root.zoomValue = scale * 100
                            }
                        }
                    },
                    Kirigami.Action {
                        icon.name: "zoom-fit-best"
                        text: i18n("Fit Page")
                        displayHint: Kirigami.DisplayHint.IconOnly
                        tooltip: i18n("Fit to Page")
                        onTriggered: root.zoomValue = 100
                    },

                    Kirigami.Action { separator: true },

                    // Rotation Controls
                    Kirigami.Action {
                        icon.name: "object-rotate-left"
                        text: i18n("Rotate Left")
                        displayHint: Kirigami.DisplayHint.IconOnly
                        tooltip: i18n("Rotate Left 90°")
                        onTriggered: pdfView.rotation = (pdfView.rotation - 90) % 360
                    },
                    Kirigami.Action {
                        icon.name: "object-rotate-right"
                        text: i18n("Rotate Right")
                        displayHint: Kirigami.DisplayHint.IconOnly
                        tooltip: i18n("Rotate Right 90°")
                        onTriggered: pdfView.rotation = (pdfView.rotation + 90) % 360
                    },

                    Kirigami.Action { separator: true },

                    // View Mode Controls
                    Kirigami.Action {
                        icon.name: "view-list-details"
                        text: i18n("View Mode")
                        displayHint: Kirigami.DisplayHint.IconOnly
                        tooltip: i18n("Change View Mode")

                        // Sub-actions for different view modes
                        children: [
                            Kirigami.Action {
                                icon.name: "view-scroll"  // Use appropriate icon
                                text: i18n("Scroll View")
                                checkable: true
                                checked: root.viewMode === 0
                                onTriggered: root.viewMode = 0
                            },
                            Kirigami.Action {
                                icon.name: "view-pages-single"  // Use appropriate icon
                                text: i18n("Single Page")
                                checkable: true
                                checked: root.viewMode === 1
                                onTriggered: root.viewMode = 1
                            },
                            Kirigami.Action {
                                icon.name: "view-pages-book"  // Use appropriate icon
                                text: i18n("Book View")
                                checkable: true
                                checked: root.viewMode === 2
                                onTriggered: root.viewMode = 2
                            }
                        ]
                    },
                    // Search
                    Kirigami.Action {
                        displayComponent: QQC2.TextField {
                            id: searchField
                            placeholderText: i18n("Search...")
                            Layout.preferredWidth: 150
                            onAccepted: pdfView.search(text)
                        }
                    },
                    Kirigami.Action {
                        icon.name: "edit-find"
                        text: i18n("Search")
                        displayHint: Kirigami.DisplayHint.IconOnly
                        tooltip: i18n("Find in Document")
                        onTriggered: pdfView.search(searchField.text)
                    }
                ]
            }
        }
        // Placeholder Message
        Kirigami.PlaceholderMessage {
            id: emptyStateMessage
            anchors.centerIn: parent
            z: 99
            width: parent.width - (Kirigami.Units.largeSpacing * 4)
            visible: !root.pdfLoaded
            text: i18n("There are no PDF file. Please add a new file.")
            icon.name: "dvipdf"

            helpfulAction: Kirigami.Action {
                icon.name: "list-add"
                text: "Add PDF"
                onTriggered: fileDialog.open()
            }
        }
    }

    FileDialog {
        id: fileDialog
        title: "Please choose a PDF file"
        nameFilters: ["PDF files (*.pdf)"]
        currentFolder: StandardPaths.standardLocations(StandardPaths.DocumentsLocation)[0]

        onAccepted: {
            var path = selectedFile.toString()
            path = path.replace(/^(file:\/{2})/,"")
            path = decodeURIComponent(path)
            pdfView.path = path
            root.pdfLoaded = true
            console.log("Loading PDF:", path)
        }
    }

    // Keyboard Shortcuts
    Shortcut {
        sequence: StandardKey.Open
        onActivated: fileDialog.open()
    }

    Shortcut {
        sequence: StandardKey.ZoomIn
        onActivated: zoomSpinBox.value = Math.min(zoomSpinBox.value + zoomSpinBox.stepSize, zoomSpinBox.to)
    }

    Shortcut {
        sequence: StandardKey.ZoomOut
        onActivated: zoomSpinBox.value = Math.max(zoomSpinBox.value - zoomSpinBox.stepSize, zoomSpinBox.from)
    }

    Shortcut {
        sequence: "Ctrl+0"
        onActivated: zoomSpinBox.value = 100
    }

    Shortcut {
        sequence: StandardKey.Find
        onActivated: searchField.forceActiveFocus()
    }

    Shortcut {
        sequence: Qt.Key_Left
        onActivated: if (pdfView.currentPage > 0) pdfView.positionViewAtIndex(pdfView.currentPage - 1, ListView.Beginning)
    }

    Shortcut {
        sequence: Qt.Key_Right
        onActivated: if (pdfView.currentPage < pdfView.count - 1) pdfView.positionViewAtIndex(pdfView.currentPage + 1, ListView.Beginning)
    }
    Connections{
        target : midiClient
        function onGoToNextPage(){
        }
    }
    KirigamiSettings.ConfigurationView {
        id: settingsView

        window: root

        title: i18n("Settings")

        modules: [
            KirigamiSettings.ConfigurationModule {
                moduleId: "general"
                text: i18n("General")
                icon.name: "configure"
                page: () => Qt.createComponent("settings/GeneralPage.qml")
            },
            KirigamiSettings.ConfigurationModule {
                moduleId: "midi"
                text: i18n("MIDI Configuration")
                icon.name: "audio-midi"
                page: () => Qt.createComponent("settings/MidiPage.qml")
            },
            KirigamiSettings.ConfigurationModule {
                moduleId: "about"
                text: i18n("About PDF Viewer")
                icon.name: "help-about"
                page: () => Qt.createComponent("org.kde.kirigamiaddons.formcard", "AboutPage")
                category: i18nc("@title:group", "About")
            }
            // KirigamiSettings.ConfigurationModule {
            //     moduleId: "aboutkde"
            //     text: i18n("About KDE")
            //     icon.name: "kde"
            //     page: () => Qt.createComponent("org.kde.kirigamiaddons.formcard", "AboutKDE")
            //     category: i18nc("@title:group", "About")
            // }
        ]
    }
}
