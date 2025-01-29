// settings/MidiPage.qml
import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

FormCard.FormCardPage {
    id: midiPage
    title: i18n("MIDI Configuration")

    // Center the content
    QQC2.ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth

        ColumnLayout {
            anchors.horizontalCenter: parent.horizontalCenter
            width: Math.min(parent.width, Kirigami.Units.gridUnit * 50) // Maximum width
            spacing: Kirigami.Units.largeSpacing

            FormCard.FormCard {
                Layout.fillWidth: true
                Layout.topMargin: Kirigami.Units.largeSpacing

                FormCard.FormComboBoxDelegate {
                     text: i18n("MIDI Input Device")
                     model: midiClient.inputPorts
                     textRole: "name"

                     // When the model changes (ports are updated)
                     onModelChanged: {
                         // Get saved device from settings
                         var savedDevice = settings.midiDevice

                         // Look for the saved device in the model
                         for (var i = 0; i < model.rowCount; i++) {
                             var portName = model.data(model.index(i, 0), Qt.UserRole + 1)
                             if (portName === savedDevice) {
                                 currentIndex = i
                                 return
                             }
                         }
                         currentIndex = -1  // If saved device not found
                     }

                     // When user selects a different device
                     onCurrentIndexChanged: {
                         if (currentIndex >= 0) {
                             var selectedPort = model.data(model.index(currentIndex, 0), Qt.UserRole + 1)
                             settings.midiDevice = selectedPort
                             midiClient.currentMidiDevice = selectedPort

                             // If you need to make the connection:
                             var portData = model.data(model.index(currentIndex, 0), Qt.UserRole)
                             midiClient.makeConnection(portData, null)  // Adjust based on your needs
                         }
                     }

                     Component.onCompleted: {
                         // Initial port scan
                         midiClient.getIOPorts()
                     }
                 }

                 // Add a refresh button
                 FormCard.FormButtonDelegate {
                     text: i18n("Refresh MIDI Ports")
                     icon.name: "view-refresh"
                     onClicked: {
                         midiClient.getIOPorts()
                     }
                 }

                FormCard.FormSpinBoxDelegate {
                    label: i18n("MIDI Channel")
                    value: settings.midiChannel
                    onValueChanged:{
                        settings.midiChannel = value
                        midiClient.midiChannel = value
                    }
                    from: 1
                    to: 16
                }
            }

            FormCard.FormCard {
                Layout.fillWidth: true
                Layout.topMargin: Kirigami.Units.largeSpacing

                FormCard.FormSpinBoxDelegate {
                    label: i18n("Next Page Control")
                    text: i18n("MIDI Control Number for next page (default: 64 - Sustain Pedal)")
                    value: settings.nextPageControl
                    onValueChanged: {
                        settings.nextPageControl = value
                        midiClient.nextPageControl = value
                    }
                    from: 0
                    to: 127
                }

                FormCard.FormSpinBoxDelegate {
                    label: i18n("Previous Page Control")
                    text: i18n("MIDI Control Number for previous page (default: 67 - Soft Pedal)")
                    value: settings.prevPageControl
                    onValueChanged: {
                        settings.prevPageControl = value
                        midiClient.prevPageControl = value
                    }
                    from: 0
                    to: 127
                }
            }

            // Test area card
            FormCard.FormCard {
                Layout.fillWidth: true
                Layout.topMargin: Kirigami.Units.largeSpacing

                FormCard.FormHeader {
                    title: i18n("Test MIDI Controls")
                }

                FormCard.AbstractFormDelegate {
                    background: Item {}
                    contentItem: ColumnLayout {
                        spacing: Kirigami.Units.smallSpacing

                        QQC2.Label {
                            text: i18n("Press your MIDI controls to test the configuration")
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                        }

                        QQC2.Label {
                            id: lastControlLabel
                            text: i18n("Last received control: None")
                            Layout.fillWidth: true
                        }
                    }
                }
            }

            // Add spacing at the bottom
            Item {
                Layout.fillHeight: true
                Layout.minimumHeight: Kirigami.Units.gridUnit
            }
        }
    }

    // MIDI test connections
    // Connections {
    //     target: midiClient
    //     function onControlChanged(channel, control, value) {
    //         lastControlLabel.text = i18n("Last received control: Channel %1, Control %2, Value %3",
    //                                      channel, control, value)
    //     }
    // }
}
