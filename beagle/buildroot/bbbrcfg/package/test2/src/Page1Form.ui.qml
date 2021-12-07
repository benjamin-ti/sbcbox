import QtQuick 2.9
import QtQuick.Controls 2.2
import "clock" as Content

Page {
    id: page
    width: 650
    height: 440

    header: Label {
        width: 400
        text: backend.getStackUeberschrift
        anchors.left: parent.left
        anchors.leftMargin: 0
        font.pixelSize: Qt.application.font.pixelSize * 2
        padding: 10

        Text {
            id: text1
            x: 500
            y: 17
            text: qsTr("S. 1 - id " + menuIndex + " - Ebene " + menuEbene)
            font.pixelSize: 12
        }
    }

    ListView {
        id: clockview
        y: 71
        width: 241
        height: 205
        orientation: ListView.Horizontal
        cacheBuffer: 2000
        snapMode: ListView.SnapOneItem
        highlightRangeMode: ListView.ApplyRange
        anchors.left: parent.left
        anchors.leftMargin: 205
        anchors.topMargin: 100

        delegate: Content.Clock { shift: timeShift }
        model: ListModel {
            ListElement { timeShift: 2 }
        }

    //    visible: menuEbene == 0 ? true : false
    }

/*    ScrollView {
        id: view
        x: 17
        y: 6
        width: 614
        height: 364

        TextArea {
            id: textArea
            text: backend.menu1Changed ? qsTr(backend.getAntwort) : qsTr("")
            //            anchors.fill: parent
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            textFormat: Text.AutoText

        }

        enabled: false
//        enabled: menuEbene > 0 ? true : false
    } */

}
