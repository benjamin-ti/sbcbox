import QtQuick 2.9
import QtQuick.Controls 2.2

Page {
    width: 650
    height: 440

    header: Label {
        text: backend.szUeberschrift        // qsTr("Page 8")
        font.pixelSize: Qt.application.font.pixelSize * 2
        padding: 10
    }

    ScrollView {
        id: view
        anchors.fill: parent

        TextArea {
            id: textArea
            x: 10
            width: 617
            y: 100
            text: backend.menu8Changed ? qsTr(backend.getAntwort) : qsTr("")
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 39
            anchors.top: parent.top
            anchors.topMargin: 262
            Text { color: "Blue" }
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            textFormat: Text.AutoText
        }
    }


    ListView {
        id: column1
        x: 60
        width: 200
        height: 216

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
        height: 216
        model: backend.szWert
        delegate: Rectangle {
                height: 20
                width: 100
                Text { text: modelData }
        }
    }

}
