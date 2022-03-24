import QtQuick 2.9
import QtQuick.Controls 2.2

Page {
    width: 650
    height: 440

    header: Label {
        text: backend.getStackUeberschrift        // qsTr("Page 5")
        font.pixelSize: Qt.application.font.pixelSize * 2
        padding: 10

        Text {
            id: text1
            x: 560
            y: 17
            text: qsTr("S. 5 - id " + menuIndex)
            font.pixelSize: 12
        }
    }

    ScrollView {
        id: view
        anchors.fill: parent

        TextArea {
            id: textArea
            x: 0
            y: 0
            width: 624
            height: 353
            text: backend.menu5Changed ? qsTr(backend.getAntwort) : qsTr("")
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            textFormat: Text.AutoText
        }
    }

}
