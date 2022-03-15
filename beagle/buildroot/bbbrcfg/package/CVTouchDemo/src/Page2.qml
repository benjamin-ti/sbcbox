import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.11

Page {
    width: 650
    height: 440

    header: Label {

        text: qsTr(backend.getStackUeberschrift)

        font.pixelSize: Qt.application.font.pixelSize * 2
        padding: 10

        Text {
            id: text1
            x: 500
            y: 17
            text: qsTr("S. 2 - id " + menuIndex + " - Ebene " + menuEbene)
            font.pixelSize: 11
        }
    }


    ScrollView {
        id: view
        anchors.fill: parent

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
            y: 10 + column2.height

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

                       /*     onReleased: {
                                print("onReleased " + index + ", " + text2.displayText)
                            }

                            onTextChanged: {
                                print("onTextChanged " + index + ", " + text2.displayText)
                            } */


                            onFocusChanged: {
                                print("onFocusChanged " + index + ", " + text2.displayText)

                                backend.setIndex = index
                                backend.setEditValue = text2.displayText
                            }
                        }
                    }

                visible: backend.szEdit.length > 0 ? true : false
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

                model: backend.szCombo.length               // die Anzahl der Comboboxen
                delegate:
                    ComboBox {
                    id: comboBox

                        model: backend.szComboValue
                        Rectangle {
                            anchors.fill: parent
                            color: "#ffffff"                // weißer Rahmen
                            Rectangle {
                                anchors.margins: 1
                                anchors.fill: parent
                           //     color: "#21be2b"            // Fläche
                                border.color: "#21be2b"
                            }
                        }

                        onVisibleChanged: {
                            if (comboBox.currentIndex == -1)              // kommt nur beim Erstellen mit -1
                            {
                            //    print("Combo onVisibleChanged " + comboBox.currentIndex + ", " + index )
                                backend.setIndex = index
                                comboBox.currentIndex = backend.iGetPreselectedComboId
                            }
                        }

                        onCurrentIndexChanged: {
                          //  print("Combo onCurrentIndexChanged " + comboBox.currentIndex + ", " + index + ", " + comboBox.displayText )

                            backend.setIndex = index
                            backend.setComboValue = comboBox.currentIndex
                        }
                    }



                visible: backend.szCombo.length > 0 ? true : false
            }
        }

        //------ Ein/Aus-Schalter ----- Rechts -----
        //
        Row {
            spacing: 10
            x: 320
            y: 10 + column4.height

            ListView {
                id: column5
                x: 20
                width: 150
                height: backend.szSwitch.length * 36

                model: backend.szSwitch
                delegate: Rectangle {
                    id: controlText
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
                height: backend.szSwitch.length * 36

                model: backend.szSwitch.length               // die Anzahl der Switche
                delegate: Switch {
                    id: control
              //      text: backend.szSwitchAktuell[index]     // modelData
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
                        text: control.text //  + " " + index
                        font: control.font
                        opacity: enabled ? 1.0 : 0.3
                        color: control.checked ? "#17a81a" : "#999999"
                        verticalAlignment: Text.AlignVCenter
                        leftPadding: control.indicator.width + control.spacing
                    }

                    checked: backend.szSwitchAktuell[index] === "On" ? true : false

                    onVisibleChanged: {
                      //  print("Switch onVisibleChanged " + control.text + ", " + backend.szSwitchValue)
                        control.text = backend.szSwitchValue[control.position]
                    }

                    onClicked: {
                        control.text = backend.szSwitchValue[control.position]

                        backend.setIndex = index
                        backend.setSwitchValue = control.text
                    }
                }
                visible: backend.szSwitchValue.length > 0 ? true : false


            }
        }

        //------ Schalter ----- Rechts -----
        //
        Row {
            id: rowButton
            spacing: 10
            x: 380
            y: 10 + column4.height + column6.height

            ListView {
                id: column10
                width: 200
                height: (backend.szButton.length * 34)

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

                        onClicked: {
                  //          print (rowButton.y + " switch " + backend.szSwitch.length + " " + (backend.szSwitch.length * 30))
                            backend.setIndex = index
                            backend.setButtonValue = button.text
                        }
                }

                visible: backend.szButtonValue.length > 0 ? true : false
            }
        }
    }

/*        Button {
            id: button1
            x: 151
            y: -20
            text: qsTr("Button")

        }

        Connections {
            target: button1
            onClicked: {
    //            status = backend.socketStatus
    //            print("clicked " + status)
          //      backend.setSendString = "FCRX--raction=\"getquickview\"&vers=2&sessionid=2"
          //      backend.setSendString = "FCRX--raction=\"handshake\"&sessionid=\"0\""
          //      backend.setSendString = "FCRX--raction=\"getmenu\"&menu=\"0\""

              //    backend.setSendString = "FCRX--raction=\"getparam\"&param=\"217068236\""                  // Beispiel: Parameter Schlitzlänge
              //    backend.setSendString = "FCRX--raction=\"setparam\"&param=\"217068236\"&value=\"100\""
                backend.setSendString = qsTr(textEdit.getText(0, textEdit.length - 1))
        //        textArea.text = backend.getAntwort
                print("clicked " + backend.getAntwort + ", " + backend.setSendString)
            }
        }

            TextField {
                id: textEdit
                x: 320
                y: -10
                width: 311
                height: 30
                placeholderText: qsTr("FCRX--raction=")

                font.pixelSize: 12
            }
*/

}
