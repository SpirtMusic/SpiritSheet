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

                FormCard.FormHeader {
                    title: i18n("MIDI Connection")
                }

                FormCard.FormComboBoxDelegate {
                    id: comboBox
                    text: i18n("MIDI Input Device")
                    model: midiClient.inputPorts
                    textRole: "name"

                    onCurrentIndexChanged: {
                        if (currentIndex >= 0) {
                            var currentName = model.data(model.index(currentIndex, 0), Qt.UserRole + 1)
                            settings.midiDevice = currentName
                        }
                    }

                    onModelChanged: {
                        var savedDevice = settings.midiDevice
                        for (var i = 0; i < model.rowCount; i++) {
                            var portName = model.data(model.index(i, 0), Qt.UserRole + 1)
                            if (portName === savedDevice) {
                                currentIndex = i
                                return
                            }
                        }
                        currentIndex = -1
                    }

                    Component.onCompleted: {
                        midiClient.getIOPorts()
                    }
                }

                // Connection status display
                FormCard.AbstractFormDelegate {
                    background: Item {}
                    contentItem: RowLayout {
                        spacing: Kirigami.Units.smallSpacing

                        QQC2.Label {
                            text: i18n("Connection Status:")
                            Layout.fillWidth: true
                        }

                        QQC2.Label {
                            id: connectionStatusLabel
                            text: midiClient.isInputPortConnected ? i18n("Connected") : i18n("Disconnected")
                            color: midiClient.isInputPortConnected ? "#4CAF50" : "#F44336"
                            font.bold: true

                            // Add this to force UI updates when connection status changes
                            Connections {
                                target: midiClient
                                function onConnectionStatusChanged() {
                                    // Force the label to update by temporarily changing text
                                    connectionStatusLabel.text = midiClient.isInputPortConnected ?
                                        i18n("Connected!") : i18n("Disconnected!");

                                    // Reset after a tiny delay
                                    refreshTimer.start();
                                }
                            }

                            Timer {
                                id: refreshTimer
                                interval: 50
                                repeat: false
                                onTriggered: {
                                    connectionStatusLabel.text = midiClient.isInputPortConnected ?
                                        i18n("Connected") : i18n("Disconnected");
                                }
                            }
                        }
                    }
                }

                // Connect/Disconnect button
                FormCard.FormButtonDelegate {
                    text: midiClient.isInputPortConnected ? i18n("Disconnect") : i18n("Connect")
                    icon.name: midiClient.isInputPortConnected ? "network-disconnect" : "network-connect"
                    enabled: comboBox.currentIndex >= 0 || midiClient.isInputPortConnected

                    onClicked: {
                        if (midiClient.isInputPortConnected) {
                            // Disconnect if currently connected
                            midiClient.makeDisconnect();
                        } else if (comboBox.currentIndex >= 0) {
                            // First disconnect (just to be safe)
                            midiClient.makeDisconnect();

                            // Small delay before connecting
                            connectionTimer.start();
                        }
                    }

                    Timer {
                        id: connectionTimer
                        interval: 100  // Short delay to allow disconnection to complete
                        repeat: false
                        onTriggered: {
                            var inputPort = comboBox.model.data(
                                comboBox.model.index(comboBox.currentIndex, 0),
                                Qt.UserRole + 2);
                            midiClient.makeConnection(inputPort, null);
                        }
                    }
                }

                // Refresh MIDI ports button
                FormCard.FormButtonDelegate {
                    text: i18n("Refresh MIDI Ports")
                    icon.name: "view-refresh"
                    onClicked: midiClient.getIOPorts()
                }

                FormCard.FormSpinBoxDelegate {
                    label: i18n("MIDI Channel")
                    value: settings.midiChannel
                    onValueChanged: {
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

                FormCard.FormHeader {
                    title: i18n("Page Navigation Controls")
                }

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

                        // Action label to show what would happen
                        QQC2.Label {
                            id: actionLabel
                            text: ""
                            visible: text !== ""
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap
                        }

                        // MIDI test connections
                        Connections {
                            target: midiClient
                            function onMidiMessageReceived(channel, control, value) {
                                lastControlLabel.text = i18n("Last received: Channel %1, Control %2, Value %3",
                                                          channel, control, value)

                                // Show what action would be triggered
                                if (channel === midiClient.midiChannel) {
                                    if (control === midiClient.nextPageControl) {
                                        actionLabel.text = i18n("Action: Next Page")
                                        actionLabel.color = "#4CAF50" // Green
                                    } else if (control === midiClient.prevPageControl) {
                                        actionLabel.text = i18n("Action: Previous Page")
                                        actionLabel.color = "#2196F3" // Blue
                                    } else {
                                        actionLabel.text = i18n("No action assigned to this control")
                                        actionLabel.color = "#757575" // Gray
                                    }
                                } else {
                                    actionLabel.text = i18n("Message not on configured MIDI channel (%1)", midiClient.midiChannel)
                                    actionLabel.color = "#757575" // Gray
                                }
                            }
                        }
                    }
                }
            }

            // Common MIDI Controls info
            FormCard.FormCard {
                Layout.fillWidth: true
                Layout.topMargin: Kirigami.Units.largeSpacing

                FormCard.FormHeader {
                    title: i18n("Common MIDI Control Numbers")
                }

                FormCard.AbstractFormDelegate {
                    background: Item {}
                    contentItem: ColumnLayout {
                        spacing: Kirigami.Units.smallSpacing

                        QQC2.Label {
                            text: i18n("Here are some common MIDI control numbers you can use:")
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                        }

                        ListView {
                            Layout.fillWidth: true
                            Layout.preferredHeight: Kirigami.Units.gridUnit * 6
                            clip: true
                            model: ListModel {
                                ListElement { cc: "64"; name: "Sustain Pedal (Hold)" }
                                ListElement { cc: "65"; name: "Portamento On/Off" }
                                ListElement { cc: "66"; name: "Sostenuto On/Off" }
                                ListElement { cc: "67"; name: "Soft Pedal On/Off" }
                                ListElement { cc: "68"; name: "Legato Footswitch" }
                                ListElement { cc: "69"; name: "Hold 2" }
                            }
                            delegate: QQC2.ItemDelegate {
                                width: ListView.view.width
                                contentItem: QQC2.Label {
                                    text: "CC " + cc + ": " + name
                                    elide: Text.ElideRight
                                }
                            }
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
}
