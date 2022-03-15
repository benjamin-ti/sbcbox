import QtQuick 2.9
import QtQuick.Controls 2.2

Page {
    width: 650
    height: 440

    header: Label {
        text: backend.szUeberschrift        // qsTr("Page 7")
        font.pixelSize: Qt.application.font.pixelSize * 2
        padding: 10
    }

    ScrollView {
        id: view
        anchors.fill: parent

        TextArea {
            id: textArea
            x: 10
            y: 29
            width: 624
            height: 353
            text: backend.menu7Changed ? qsTr(backend.getAntwort) : qsTr("")
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            textFormat: Text.AutoText
        }
    }

    ListView {
        id: column1
        x: 45
        y: 26
        width: 200
        height: 251

        model: backend.szBezeichnung
        delegate: Rectangle {
                height: 20
                width: 100
                Text { text: modelData }
        }

    }
    ListView {
        id: column2
        x: 270
        y: 26
        width: 200
        height: 251
        model: backend.szWert
        delegate: Rectangle {
                height: 20
                width: 100
                Text { text: modelData }
        }
    }
}
