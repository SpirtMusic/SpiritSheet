// settings/GeneralPage.qml
import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

FormCard.FormCardPage {
    id: generalPage
    title: i18n("General")

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

                FormCard.FormCheckDelegate {
                    text: i18n("Auto-open last file")
                    description: i18n("Automatically open the last viewed PDF file on startup")
                    checked: settings.autoOpenLast
                    onCheckedChanged: settings.autoOpenLast = checked
                }

                FormCard.FormSpinBoxDelegate {
                    label: i18n("Default zoom level")
                    value: settings.defaultZoom
                    onValueChanged: settings.defaultZoom = value
                    from: 10
                    to: 500
                    stepSize: 10
                }

                FormCard.FormComboBoxDelegate {
                    text: i18n("Default view mode")
                    model: [i18n("Scroll View"), i18n("Single Page"), i18n("Book View")]
                    currentIndex: settings.defaultViewMode
                    onCurrentIndexChanged: settings.defaultViewMode = currentIndex
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
