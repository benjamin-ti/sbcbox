import QtQuick 2.4

Item {
    id: sidebar
    width: 149
    height: 480
    property alias dateTime: dateTime
    property alias statusBackground: statusBackground
    property alias statusText: statusText
    property alias printerName: printerName

    Rectangle {
        id: rectangle
        x: 0
        y: 0
        width: 149
        height: 361
        color: "#ffffff"

    }

    Image {
        id: image
        x: 25
        y: 8
        width: 110
        height: 110
        anchors.horizontalCenter: parent.horizontalCenter
        fillMode: Image.PreserveAspectFit
        source: "image/CV_Logo_vertikal_110x106.png"
    }

    Rectangle {
        id: statusBackground
        x: 0
        y: 328
        width: 149
        height: 80
        /*        gradient: Gradient {
            GradientStop {
                position: 0
                color: "#ffffff"
            }

            GradientStop {
                position: 1
                color: "#508000"
            }
        } */

        color: (iStatus == 1) ? "ForestGreen" : (iStatus == 2) ? "red" : (iStatus == 3) ? "DarkGreen" : (iStatus == 4) ? "yellow" : "white"
        anchors.horizontalCenterOffset: 0
        border.color: "#00000000"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: backButton.top
        anchors.bottomMargin: 0

        Text {
            id: statusText
            x: 45
            y: 18
            text: status        // kommt von: property string status in main.qml (Timer)

       //     text: backend.socketStatus        // geht auch, aktualisiert aber nicht

            color: (iStatus == 1) ? "white" : (iStatus == 2) ? "white" : (iStatus == 3) ? "white" : "black"
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            font.pixelSize: 20
        }
    }

    Text {
        id: dateTime
        x: 20
        text: qsTr(timeZeit)
        anchors.horizontalCenterOffset: 0
        anchors.top: image.bottom
        anchors.topMargin: 20
        anchors.horizontalCenter: image.horizontalCenter
        font.pixelSize: 16
    }

/*    Connections {
        target: sidebar
        onClicked: print("clicked")
    }
*/
    Text {
        id: printerName
        x: 20
        y: 180
        text: qsTr(druckerName)
        anchors.horizontalCenter: parent.horizontalCenter
 /*
              backend.setSendString = "FCRX--raction=\"handshake\"&sessionid=\"0\""
              textArea.text = backend.getAntwort
              print("clicked " + backend.getAntwort + ", " + backend.setSendString)
*/
        font.pixelSize: 12
    }

    Text {
        id: textFirmware
        x: 31
        y: 221
        width: 97
        height: 79
        text: "#qsTr(firmware)#"
        horizontalAlignment: Text.AlignHCenter
        anchors.horizontalCenter: parent.horizontalCenter
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        font.pixelSize: 12
    }

    Rectangle {
        id: backButton
        x: 0
        y: 426
        width: 149
        height: 40
        color: menuEbene > 0 ? "DodgerBlue" : (iStatus == 1) ? "ForestGreen" : (iStatus == 2) ? "red" : (iStatus == 3) ? "DarkGreen" : (iStatus == 4) ? "yellow" : "white"
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 0
        anchors.horizontalCenter: parent.horizontalCenter
        visible: backend.nextButton == true ? false : true

        Text {
            id: text1
            x: 64
            y: 8
//            text: qsTr("<<")
            text: menuEbene == 0 ? qsTr("") : qsTr("<<")
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            font.pixelSize: 16
            color: "#ffffff"
        }

        MouseArea {
            id: mouseArea1
            width: 149
            anchors.fill: parent
        }
    }

    Rectangle {
        id: backButton1
        width: 73
        height: 40
        color: "DodgerBlue"
        anchors.top: statusBackground.bottom
        anchors.topMargin: 0
        anchors.left: parent.left
        anchors.leftMargin: 0
        visible: backend.nextButton == true ? true : false

        Text {
            id: text2
            x: 64
            y: 8
            text: qsTr("<<")
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            font.pixelSize: 16
            color: "white"
        }

        MouseArea {
            id: mouseArea2
            width: 74
            anchors.fill: parent
        }
    }

    Rectangle {
        id: nextButton
        y: 426
        height: 40
        color: "DodgerBlue"
        anchors.top: statusBackground.bottom
        anchors.topMargin: 0
        anchors.left: backButton1.right
        anchors.leftMargin: 0
        anchors.right: parent.right
        anchors.rightMargin: 1
        visible: backend.nextButton == true ? true : false

        Rectangle {
            id: borderLeft
            width: 1
            height: parent.height
            anchors.left: parent.left
            color: "white"
        }

        Text {
            id: text3
            x: 64
            y: 8
            text: qsTr(">>")
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            font.pixelSize: 16
            color: "white"
        }

        MouseArea {
            id: mouseArea3
            width: 73
            anchors.fill: parent
        }
    }

    Connections {
        target: mouseArea1
        onClicked: {
            if (menuEbene > 0)
            {
                backend.stackUeberschrift = false

                menuEbene --


                if (menuEbene == 0)
                {
                    backend.menu1 = 0                   // ?
                    menuIndex = "0"
                }
                else
                {
                    backend.menu1 = backend.getStackMenuId
                }
                print("Sidebar: menuIndex " + menuIndex + ", menuEbene " + menuEbene)
            }

            stateGroup.state = 'Tab1'
            backend.bInhaltVorhanden === true ? pageIndex = 1 : pageIndex = 0
        }
    }

    Connections {
        target: mouseArea2              // backButton1
        onClicked:  {
            if (backend.nextEbene > 0)
            {
                backend.nextButton = false
                backend.getMenu0
            }
            else
            {
                backend.stackUeberschrift = false

                if (menuEbene > 0)
                {
                    menuEbene --

                    if (menuEbene == 0)
                    {
                        backend.menu1 = 0                   // ?
                        menuIndex = "0"
                    }
                    else
                    {
                        backend.menu1 = backend.getStackMenuId
                    }
                    print("Sidebar: menuIndex " + menuIndex + ", menuEbene " + menuEbene)
                }
            }
            stateGroup.state = 'Tab1'
            backend.bInhaltVorhanden === true ? pageIndex = 1 : pageIndex = 0
        }
    }

    Connections {
        target: mouseArea3              // nextButton
        onClicked: {
            backend.nextButton = true
            backend.getMenu0

            stateGroup.state = 'Tab1'
            backend.bInhaltVorhanden == true ? pageIndex = 1 : pageIndex = 0
        }
    }


}
