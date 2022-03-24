import QtQuick 2.9
import QtQuick.Controls 2.2
import "clock" as Content
import QtQml 2.11
import io.qt.examples.backend 1.0
import QtQuick.Layouts 1.11

ApplicationWindow {
    id: mainWindow
    visible: true
    width: 800
    height: 480
    title: qsTr("Tabs")

    // Funktion getHandshake() aufrufen, damit die Parameter PrinterName usw. initialisiert werden.
    // Wird nur einmal aufgerufen
    Component.onCompleted: {
        backend.getHandshake
        backend.getMenu0
        menuEbene = 0
        menuIndex = "0"
    }

    BackEnd {
        id: backend
    }

    property int pageIndex
    property int menuEbene             // Die Tiefe des Menüs
    property string menuIndex          // Der Index des Schalters, auf den zuletzt geklickt wurde

    SwipeView {
        id: swipeView
        y: 0
        height: 480
        anchors.right: parent.right
        anchors.rightMargin: 0
        anchors.left: parent.left
        anchors.leftMargin: 150
        currentIndex: pageIndex
   //     currentIndex: tabBar.currentIndex      // wird momentan nicht verwendet, weil es nur zwei Seiten gibt

        Page1Form {
        }

        Page2 {
        }

/*        Page3 {
        }

        Page4Form {
        }*/

        Page5Form {
        }
    }

    footer: TabBar {
        id: tabBar
        anchors.right: parent.right
        anchors.rightMargin: 0
        anchors.left: parent.left
        anchors.leftMargin: 150
        currentIndex: pageIndex
//        currentIndex: swipeView.currentIndex      // wird momentan nicht verwendet, weil es nur zwei Seiten gibt

        TabButton {
            id: tabButton1
            text: backend.menu1Changed ? qsTr(backend.menu1) : ("Page 1")
            font.pointSize: 10

            Rectangle {
                border.color: "#00000000"
                color: (stateGroup.state == 'Tab1') ? "orange" : "RoyalBlue"
                anchors.fill: parent
            }

            onClicked: {
                        stateGroup.state = 'Tab1'
                        print ("menuIndex: " + menuIndex + ", backend.menu1id: " + backend.menu1id)
                        if (backend.menu2id > 0 || menuIndex != backend.menu1id)        // soll sich nicht selbst aufrufen können
                        {
                            // Die backend.menu1id ist der Wert aus der Initialisierung (XML-Übertragung des Druckers). Wenn es eine Beschriftung gibt, gibt
                            // es auch eine ID. D.h. der Schalter kann betätigt werden.
                            //
                            if (backend.menu1id > 0) menuEbene ++   // Somit wird beim Klicken auf den Schalter die Ebene erhöht.
                            // In menuIndex wird die ID dieses Schalters abgelegt (gesichert), damit die nächste wieder hierhin zurück kommen kann.
                            if (menuEbene > 0)
                            {
                                menuIndex = backend.menu1id                     // ID sichern
                            }
                            // Durch das Setzen von backend.menu1 mit der vorherigen ID wird ein neues Menü geladen (menuButtonClicked) und es gibt eine neue ID.
                            backend.menu1 = backend.menu1id
                            backend.stackUeberschrift = true                             // Die aktuelle Überschrift wird gesichert

                            backend.bInhaltVorhanden == true ? pageIndex = 1 : pageIndex = 0        // Mit Inhalt die Seite 2 anzeigen

                            print ("menuEbene " + menuEbene + ", menuIndex (aktuell): " + menuIndex)
                        }

            }

        }
        TabButton {
            id: tabButton2
            text: backend.menu2Changed ? qsTr(backend.menu2) : ("Page 2")
            font.pointSize: 10


            Rectangle {
                border.color: "#00000000"
                color: (stateGroup.state == 'Tab2') ? "orange" : "RoyalBlue"
                anchors.fill: parent
            }

 /*           Image {                   // Test mit Hintergrundbild
                anchors.fill: parent
                fillMode: Image.Tile

                source: parent.activeFocus ? "image/bl_ws.png" : "image/ws_bl.png"
            }*/
            onClicked: { if (backend.menu2.length > 0)
                {
                    stateGroup.state = 'Tab1'
                    if (backend.menu2id > 0) menuEbene ++           // alt, vorher, wenn > 0 dann Ebene erhöhen
                    if (menuEbene > 0)
                    {
                        menuIndex = backend.menu2id                     // ID sichern
                    }
                    backend.menu2 = backend.menu2id                     // neues Menü laden
                    backend.stackUeberschrift = true

                    backend.bInhaltVorhanden == true ? pageIndex = 1 : pageIndex = 0

                    print ("menuEbene " + menuEbene + ", menuIndex (aktuell): " + menuIndex)
                }

            }
        }
        TabButton {
            id: tabButton3
            text: backend.menu3Changed ? qsTr(backend.menu3) : (backend.userName)
            font.pointSize: 10

            background :
                Rectangle {
                    color: "white"
                }

            Rectangle {
      //          radius: 10
                border.color: "#00000000"
                color: (stateGroup.state == 'Tab3') ? "orange" : "RoyalBlue"
                anchors.fill: parent
            }
            onClicked: { if (backend.menu3.length > 0)
                {
                    stateGroup.state = 'Tab1'
                    if (backend.menu3id > 0) menuEbene ++           // alt, vorher, wenn > 0 dann Ebene erhöhen
                    if (menuEbene > 0)
                    {
                        menuIndex = backend.menu3id                     // ID sichern
                    }
                    backend.menu3 = backend.menu3id                 // neues Menü laden
                    backend.stackUeberschrift = true

                    backend.bInhaltVorhanden == true ? pageIndex = 1 : pageIndex = 0

                    print ("menuEbene " + menuEbene + ", menuIndex (aktuell): " + menuIndex )
                }

            }
        }
        TabButton {
            id: tabButton4
            text: backend.menu4Changed ? qsTr(backend.menu4) : ("Page 4")
            font.pointSize: 10

            Rectangle {
        //        radius: 10
                border.color: "#00000000"
                color: (stateGroup.state == 'Tab4') ? "orange" : "RoyalBlue"
                anchors.fill: parent
            }
            onClicked: { if (backend.menu4.length > 0)
                {
                    stateGroup.state = 'Tab1'
                    if (backend.menu4id > 0) menuEbene ++           // alt, vorher, wenn > 0 dann Ebene erhöhen
                    if (menuEbene > 0)
                    {
                        menuIndex = backend.menu4id                     // ID sichern
                    }
                    backend.menu4 = backend.menu4id                 // neues Menü laden
                    backend.stackUeberschrift = true

                    backend.bInhaltVorhanden == true ? pageIndex = 1 : pageIndex = 0

                    print ("menuEbene " + menuEbene + ", menuIndex (aktuell): " + menuIndex)
                }

            }
        }
    }

    header: TabBar {
        id:tabBarHeader
        anchors.right: parent.right
        anchors.rightMargin: 0
        anchors.left: parent.left
        anchors.leftMargin: 150
//        currentIndex: pageIndex + 4

        TabButton {
            id: tabButton5
            text: backend.menu5Changed ? qsTr(backend.menu5) : ("Page 5")
            font.pointSize: 10

            Rectangle {
                border.color: "#00000000"
                color: (stateGroup.state == 'Tab5') ? "orange" : "RoyalBlue"
                anchors.fill: parent
            }
            Text {
                color: "white"      // geht nicht
            }
            onClicked: { if (backend.menu5.length > 0)
                {
                    stateGroup.state = 'Tab1'
                    if (backend.menu5id > 0) menuEbene ++           // alt, vorher, wenn > 0 dann Ebene erhöhen
                    if (menuEbene > 0)
                    {
                        menuIndex = backend.menu5id                     // ID sichern
                    }
                    backend.menu5 = backend.menu5id                 // neues Menü laden
                    backend.stackUeberschrift = true

                    backend.bInhaltVorhanden == true ? pageIndex = 1 : pageIndex = 0

                    print ("menuEbene " + menuEbene + ", menuIndex (aktuell): " + menuIndex + ", pageIndex: " + pageIndex)
                }

            }
        }

        TabButton {
            id: tabButton6
            text: backend.menu6Changed ? qsTr(backend.menu6) : ("Page 6")
            font.pointSize: 10

            Rectangle {
                border.color: "#00000000"
                color: (stateGroup.state == 'Tab6') ? "orange" : "RoyalBlue"
                anchors.fill: parent
            }
            onClicked: { if (backend.menu6.length > 0)
                {
                    stateGroup.state = 'Tab1'
                    if (backend.menu6id > 0) menuEbene ++           // alt, vorher, wenn > 0 dann Ebene erhöhen
                    if (menuEbene > 0)
                    {
                        menuIndex = backend.menu6id                     // ID sichern
                    }
                    backend.menu6 = backend.menu6id                 // neues Menü laden
                    backend.stackUeberschrift = true

                    backend.bInhaltVorhanden == true ? pageIndex = 1 : pageIndex = 0

                    print ("menuEbene " + menuEbene + ", menuIndex (aktuell): " + menuIndex)
                }

            }
        }

        TabButton {
            id: tabButton7
            text: backend.menu7Changed ? qsTr(backend.menu7) : ("Page 7")
            font.pointSize: 10

            Rectangle {
                border.color: "#00000000"
                color: (stateGroup.state == 'Tab7') ? "orange" : "RoyalBlue"
                anchors.fill: parent
            }
            onClicked: { if (backend.menu7.length > 0)
                {
                    stateGroup.state = 'Tab1'
                    if (backend.menu7id > 0) menuEbene ++           // alt, vorher, wenn > 0 dann Ebene erhöhen
                    if (menuEbene > 0)
                    {
                        menuIndex = backend.menu7id                     // ID sichern
                    }
                    backend.menu7 = backend.menu7id                 // neues Menü laden
                    backend.stackUeberschrift = true

                    backend.bInhaltVorhanden == true ? pageIndex = 1 : pageIndex = 0

                    print ("menuEbene " + menuEbene + ", menuIndex (aktuell): " + menuIndex)
                }
            }
        }

        TabButton {
            id: tabButton8
            text: backend.menu8Changed ? qsTr(backend.menu8) : ("Page 8")
            font.pointSize: 10

            Rectangle {
                border.color: "#00000000"
                color: (stateGroup.state == 'Tab8') ? "orange" : "RoyalBlue"
                anchors.fill: parent
            }
            onClicked: { if (backend.menu8.length > 0)
                {
                    stateGroup.state = 'Tab1'
                    if (backend.menu8id > 0) menuEbene ++           // alt, vorher, wenn > 0 dann Ebene erhöhen
                    if (menuEbene > 0)
                    {
                        menuIndex = backend.menu8id                     // ID sichern
                    }
                    backend.menu8 = backend.menu8id                 // neues Menü laden
                    backend.stackUeberschrift = true

                    backend.bInhaltVorhanden == true ? pageIndex = 1 : pageIndex = 0

                    print ("menuEbene " + menuEbene + ", menuIndex (aktuell): " + menuIndex)
                }

            }
        }
    }

    Sidebar {
        id: sidebar
        x: 0
        width: 150
        anchors.top: parent.top
        anchors.topMargin: -40
        anchors.bottom: parent.bottom
        anchors.bottomMargin: -40
    }

/*    property int hours : 24
    property int minutes : 12
    property int seconds : 03
    property string hour
    property string minute
    property string second
    property real shift
    property bool internationalTime: false //Unset for local time

    function timeChanged() {
        var date = new Date;
        hours = internationalTime ? date.getUTCHours() + Math.floor(this.shift) : date.getHours()
        minutes = internationalTime ? date.getUTCMinutes() + ((this.shift % 1) * 60) : date.getMinutes()
        seconds = date.getUTCSeconds();
        if (hours < 10)
        {
            hour = "0" + hours.toString()
        }
        else
        {
            hour = hours.toString()
        }
        if (minutes < 10)
        {
            minute = "0" + minutes.toString()
        }
        else
        {
            minute = minutes.toString()
        }
        if (seconds < 10)
        {
            second = "0" + seconds.toString()
        }
        else
        {
            second = seconds.toString()
        }
    } */

    // Property zur Verwendung in Sidebar
    property string timeZeit
    Timer {
        interval: 500; running: true; repeat: true;
/*        onTriggered: {
 //           internationalTime = true
            timeChanged()
            timeZeit = hour + ":" + minute + ":" + second
        } */

        property date currentTime
        onTriggered: {
            currentTime = new Date()
            timeZeit = currentTime.toLocaleTimeString(locale, Locale.LongFormat);
        }
    }



    property string status
    property string druckerName
    property string firmware
    property int iStatus
    Timer {
        interval: 1000; running: true; repeat: true;

        onTriggered: {
            status = backend.socketStatus
            iStatus = backend.getSocketStatusColor
            druckerName = backend.printerName
            firmware = backend.firmware
        }

    }

    // StateGroup setzt den Status des jeweiligen Tabs. So bleibt die Farbe gesetzt.
    StateGroup {
        id: stateGroup
        states: [
            State {
                name: "Tab1"
            },
            State {
                name: "Tab2"
            },
/*            State {
                name: "Tab3"
            },
            State {
                name: "Tab4"
            },*/
            State {
                name: "Tab5"
            }
        ]
    }
}
