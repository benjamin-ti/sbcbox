import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.11

Page {
    width: 650
    height: 440

    header: Label {
        text: backend.szUeberschrift
        font.pixelSize: Qt.application.font.pixelSize * 2
        padding: 10

        GridView {
            id: column1x

            width: 350
            height: 50
            y: 20
            x: 200

            model: backend.getStackUeberschrift
            delegate: Column {
                spacing: 10
                Text { text: modelData
                    font.pixelSize: 12
                    font.family: "Helvetica"
                    color: "#21be2b"
                }
            }
        }

        Text {
            id: text1
            x: 500
            y: 17
            text: qsTr("S. 3 - id " + menuIndex + " - Ebene " + menuEbene)
            font.pixelSize: 12
        }
    }

    //------ Einfache Anzeigen ----- links -----
    //
    Row {
        spacing: 10
        x: 20
        y: 10

        ListView {
            id: column1

            width: 150
            height: backend.szBezeichnung.length * 20

            model: backend.szBezeichnung
            delegate: Rectangle {
                    height: 20
                    width: 150
                    Text { text: modelData
                        font.pixelSize: 12
                        font.family: "Helvetica"
                    }
               //     border.color: "#21be2b"
            }
        }

       ListView {
            id: column2

            width: 150
            height: backend.szBezeichnung.length * 20
            model: backend.szWert
            delegate: Rectangle {
                    height: 20
                    width: 150
                    Text { text: modelData
                        font.pixelSize: 12
                        font.family: "Helvetica"
                    }
            }
        }
    }


    //------ Eingabefelder (Editbox) ----- links -----
    //
    Row {
        spacing: 10
        x: 20
        y: 10 + backend.szBezeichnung.length * column2.height

        ListView {
            id: column7
            x: 20
        //    y: 20
            width: 150
            height: backend.szEdit.length * 30

            model: backend.szEdit
            delegate: Rectangle {
                    implicitHeight: 30
                    width: 150

                    Text { text: modelData
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignRight
                        height: 30
                        font.pixelSize: 12
                        font.family: "Helvetica"
                    }
               //     border.color: "#21be2b"
            }
        }

        ListView {
            id: column8
            x: 200
         //   y: 20
            width: 150
            height: backend.szEdit.length * 30

            model: backend.szEditValue
            delegate:
                Rectangle {
                    id: rectangleC8
                    width: 100
                    height: 30
                    TextField {
                        id: text2
                        anchors.bottomMargin: 1
                        anchors.topMargin: 1
                        anchors.fill: rectangleC8

                        text: modelData
                        font.pixelSize: 12
                        font.family: "Helvetica"
                    }
                }
            visible: backend.szEditValue.length > 0 ? true : false
        }
    }



    //------ Comboboxen ----- Rechts -----
    //
    Row {
        spacing: 10
        x: 320
        y: 10

        ListView {
            id: column3
            x: 20
        //    y: 25
            width: 150
            height: backend.szCombo.length * 40

            model: backend.szCombo
            delegate: Rectangle {
                    height: 40
                    width: 150

                    Text { text: modelData
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignRight
                        height: 40
                        font.pixelSize: 12
                        font.family: "Helvetica"
                    }
               //     border.color: "#21be2b"
            }
        }
        ListView {
            id: column4
            x: 20
     //       y: 20
            width: 150
            height: backend.szCombo.length * 40
            model: backend.szCombo.length
            delegate:
                ComboBox {
                    id: comboBox
                        model: backend.szComboValue
                }
            visible: backend.szCombo.length > 0 ? true : false
        }
    }

    //------ Ein/Aus-Schalter ----- Rechts -----
    //
    Row {
        spacing: 10
        x: 320
        y: 10 + backend.szCombo.length * 40 //comboBox.height

        ListView {
            id: column5
            x: 20
            width: 150
            height: backend.szSwitch.length * 36

            model: backend.szSwitch
            delegate: Rectangle {
                    implicitHeight: 36
                    width: 150

                    Text { text: modelData
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignRight
                        height: 36
                        font.pixelSize: 12
                        font.family: "Helvetica"
                    }
               //     border.color: "#21be2b"
            }
        }
        ListView {
            id: column6
            x: 20
            width: 150
            height: backend.szSwitch.length * 30
            model: backend.szSwitchValue
            delegate: Switch {
                id: control
                text: modelData
                height: 36

                indicator: Rectangle {
                    implicitWidth: 60
                    implicitHeight: 24
                    x: control.leftPadding
                    y: parent.height / 2 - height / 2
                    radius: 12
                    color: control.checked ? "#17a81a" : "#ffffff"
                    border.color: control.checked ? "#17a81a" : "#cccccc"

                    Rectangle {
                        x: control.checked ? parent.width - width : 0
                        width: 24
                        height: 24
                        radius: 12
                        color: control.down ? "#cccccc" : "#ffffff"
                        border.color: control.checked ? (control.down ? "#17a81a" : "#21be2b") : "#999999"
                    }
                }

                contentItem: Text {
                    text: control.text
                    font: control.font
                    opacity: enabled ? 1.0 : 0.3
                    color: control.checked ? "#17a81a" : "#999999"
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: control.indicator.width + control.spacing
                }

                checked: modelData == "On" ? true : false

            }
            visible: backend.szSwitchValue.length > 0 ? true : false
        }
    }

    //------ Schalter ----- Rechts -----
    //
    Row {
        spacing: 10
        x: 380
        y: 10 + backend.szSwitch.length * 40 + backend.szCombo.length * 40  // comboBox.height

        ListView {
            id: column10
            width: 200
            height: backend.szButton.length * 34

            model: backend.szButton
            delegate: Button {
                id: button
                    text: modelData
                    width: 200
                    height: 34

                    contentItem: Text {
                        text: button.text
                        font: button.font
                        opacity: enabled ? 1.0 : 0.3
                        color: button.down ? "#ffffff" : "#216e2b"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                    }

                    background: Rectangle {
                        anchors.margins: 2
                        anchors.fill: parent
                        opacity: enabled ? 1 : 0.3
                        border.color: button.down ? "#ffffff" : "#21be2b"
                        border.width: 1
                        color: button.down ? "#21be2b" : "#ffffff"
                        radius: 2
                    }
            }

            visible: backend.szButtonValue.length > 0 ? true : false
        }
    }

}
