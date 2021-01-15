#include "backend.h"
#include "tcpsocket.h"



BackEnd::BackEnd(QObject *parent) :
    QObject(parent)
{
    m_iNextEbene = 0;
}

QString BackEnd::userName()
{
    return m_userName;
}

void BackEnd::setUserName(const QString &userName)
{
    if (userName == m_userName)
        return;

    m_userName = userName;
    emit userNameChanged();
}

QString BackEnd::cleanupString(QString str)
{
    if (str.contains("\u0001"))
    {
       str.remove("\u0001");
    }
    if (str.contains("\u0017"))
    {
       str.remove("\u0017");
    }
    if (str.contains("\x01"))
    {
       str.remove("\x01");
    }
    if (str.contains("\x17"))
    {
       str.remove("\x17");
    }
    if (str.contains("#"))
    {
       str.remove("#");
    }
    return str;
}

QString BackEnd::socketStatus()
{
    TCPSocket s;
    s.doConnect();
    m_status = s.SocketStatus();
    m_status = cleanupString(m_status);

//    qDebug() << m_status;

/*
    s.setSendString("FCRX--raction=\"handshake\"&sessionid=\"0\"");
    s.doConnect();
    QString temp = s.GetAntwort();
    if (temp.contains("\x01"))
    {
       temp.remove("\x01");
    }
    if (temp.contains("\x17"))
    {
       temp.remove("\x17");
    }
    if (temp.contains("TTZ"))
    {
       temp.remove("TTZ");
    }
//        qDebug() << temp;

    // http://3gfp.com/wp/2014/07/three-ways-to-parse-xml-in-qt/
    QDomDocument document;
    QByteArray ba;
    ba.append(temp);
    document.setContent(ba);
    qDebug() << "ba: " << ba;

    QDomNodeList root = document.elementsByTagName("handshake");
    for (int i = 0; i < root.size(); i++) {
        QDomNode n = root.item(i);
 //       QDomElement from = n.firstChildElement("printer");
        m_printerName = n.firstChildElement("printer").text();
        m_firmware = n.firstChildElement("firmware").text();
        m_sessionid = n.firstChildElement("sessionid").text();
        if (m_printerName.isNull() || m_firmware.isNull() || m_sessionid.isNull())
            continue;
    }
    qDebug() << m_printerName << m_firmware << m_sessionid;

    // https://gist.github.com/lamprosg/2133804
    QXmlStreamReader xml(temp);
    xml.readNext();
    while(!xml.atEnd())
    {
        if(xml.isStartElement())
        {
            qDebug() << "1" << xml.name();
            if(xml.name() == "handshake")
            {
                while(!xml.atEnd())
                {
                    if(xml.isEndElement())
                    {
                        xml.readNext();
                    }
                    else if(xml.isStartElement())
                    {
                        qDebug() << "2" << xml.name();
                        if(xml.name() == "printer")
                        {
                            m_printerName = xml.readElementText();
                        }
                        if(xml.name() == "firmware")
                        {
                            m_firmware = xml.readElementText();
                        }
                        if(xml.name() == "sessionid")
                        {
                            m_sessionid = xml.readElementText();
                        }
                        xml.readNext();
                    }
                    else
                    {
                        xml.readNext();
                    }
                }
            }
        }
        else
        {
            xml.readNext();
        }
    }
    qDebug() << "Zeile 107" << m_printerName << m_firmware << m_sessionid;
*/

   // s_handshake h = getHandshake();
    return m_status;
}

QString BackEnd::getHandshake()
{
    TCPSocket s;
    s.doConnect();

    s.setSendString("FCRX--raction=\"handshake\"&sessionid=\"0\"");
    s.doConnect();
    QString temp = s.GetAntwort();
    temp = cleanupString(temp);
    if (temp.contains("TTZ"))
    {
       temp.remove("TTZ");
    }
    //    qDebug() << temp;

    // Zwei Versionen, um xml zu pasen.
    //
/*    // http://3gfp.com/wp/2014/07/three-ways-to-parse-xml-in-qt/
    QDomDocument document;
    QByteArray ba;
    ba.append(temp);
    document.setContent(ba);
    qDebug() << "ba: " << ba;

    QDomNodeList root = document.elementsByTagName("handshake");
    for (int i = 0; i < root.size(); i++) {
        QDomNode n = root.item(i);
 //       QDomElement from = n.firstChildElement("printer");
        m_printerName = n.firstChildElement("printer").text();
        m_firmware = n.firstChildElement("firmware").text();
        m_sessionid = n.firstChildElement("sessionid").text();
        if (m_printerName.isNull() || m_firmware.isNull() || m_sessionid.isNull())
            continue;
    }
    qDebug() << m_printerName << m_firmware << m_sessionid;
*/


    // https://gist.github.com/lamprosg/2133804
    QXmlStreamReader xml(temp);
    xml.readNext();
    while(!xml.atEnd())
    {
        if(xml.isStartElement())
        {
    //        qDebug() << "1" << xml.name();
            if(xml.name() == "handshake")
            {
                while(!xml.atEnd())
                {
                    if(xml.isEndElement())
                    {
                        xml.readNext();
                    }
                    else if(xml.isStartElement())
                    {
               //         qDebug() << "2" << xml.name();
                        if(xml.name() == "version")
                        {
                            m_version = xml.readElementText();
                        }
                        if(xml.name() == "printer")
                        {
                            m_printerName = xml.readElementText();
                        }
                        if(xml.name() == "firmware")
                        {
                            m_firmware = xml.readElementText();
                        }
                        if(xml.name() == "sessionid")
                        {
                            m_sessionid = xml.readElementText();
                        }
                        if(xml.name() == "locale")
                        {
                            m_locale = xml.readElementText();
                        }
                        if(xml.name() == "rtpreview")
                        {
                            m_rtpreview = xml.readElementText();
                        }
                        xml.readNext();
                    }
                    else
                    {
                        xml.readNext();
                    }
                }
            }
        }
        else
        {
            xml.readNext();
        }
    }

    qDebug() << m_printerName << m_firmware << m_sessionid;

    return m_version;
}

QString BackEnd::getMenu0()
{
    TCPSocket s;
    s.doConnect();

    if (m_menu_suchstring.length() == 0)
    {
        m_menu_suchstring = "FCRX--raction=\"getmenu\"&menu=\"0\"&sessionid=\"" + m_sessionid + "\"";
    }
    s.setSendString(m_menu_suchstring);
    qDebug() << m_menu_suchstring;

        // FCRX--raction="getmenu"&menu="0"&sessionid="0"   ergibt als Antwort:
        // TTZ<menu id="0" textid="Root" name="Root">
        //    <menu id="53" textid="Printer Info" name="Druckertyp"/>
        //    <menu id="42" textid="Job Status" name="Job Status"/>
        //    <menu id="31" textid="Job Ctrl" name="Job Ctrl"/>
        //    <menu id="1" textid="Config" name="Funktionen"/>
        //    <menu id="33" textid="Favorite Menu" name="Favorite Menu"/>
        //    <menu id="41" textid="Debug" name="Debug"/>
        //    <menu id="46" textid="ChangeWhilePrint" name="ChangeWhilePrint"/>
        //    <menu id="81" textid="Contact" name="Kontakt"/>
        // </menu>

        // FCRX--raction="getmenu"&menu="127"&sessionid="0"   ergibt als Antwort:
        // TTZ<menu id="127" textid="Network" name="Netzwerk">
        // <parameter id="217402444" textid="IP-Address" name="IP-Adresse" idvalue=" 10.102.  3. 62" readonly="yes" size="15"> 10.102.  3. 62
        // </parameter>
        // </menu>

    s.doConnect();
    QString temp = s.GetAntwort();
    temp = cleanupString(temp);
    if (temp.contains("TTZ"))
    {
       temp.remove("TTZ");
    }

    int i = 0;
    m_iAnzahlMenus = -1;
    m_bNextSchalter = false;
    if (m_iNextEbene > 3)
    {
        m_iNextEbene = 0;
    }

    bool bParameterVorhanden = false;
    bool bUeberschrift = false;
    QXmlStreamReader xml(temp);                         // https://gist.github.com/lamprosg/2133804
    xml.readNext();
    bUeberschrift = false;
    m_szUeberschrift = "";
    emit ueberschriftChanged();

    if (i < 8)          // Erst einmal alle Einträge löschen
    {
        for (int j = 0; j < 8; j++)
        {
            switch (j)
            {
                case 0:  m_menu_0_1 = "0";  break;
                case 1:  m_menu_0_2 = "0";  break;
                case 2:  m_menu_0_3 = "0";  break;
                case 3:  m_menu_0_4 = "0";  break;
                case 4:  m_menu_0_5 = "0";  break;
                case 5:  m_menu_0_6 = "0";  break;
                case 6:  m_menu_0_7 = "0";  break;
                case 7:  m_menu_0_8 = "0";  break;
            }
            switch (j)
            {
                case 0:  m_menu_0_1_text = ""; emit menu1Changed(); break;
                case 1:  m_menu_0_2_text = ""; emit menu2Changed(); break;
                case 2:  m_menu_0_3_text = ""; emit menu3Changed(); break;
                case 3:  m_menu_0_4_text = ""; emit menu4Changed(); break;
                case 4:  m_menu_0_5_text = ""; emit menu5Changed(); break;
                case 5:  m_menu_0_6_text = ""; emit menu6Changed(); break;
                case 6:  m_menu_0_7_text = ""; emit menu7Changed(); break;
                case 7:  m_menu_0_8_text = ""; emit menu8Changed(); break;
            }
        }
    }

    while(!xml.atEnd())
    {
        if(xml.isStartElement())
        {
            if(xml.name() == "menu")
            {
                while(!xml.atEnd())
                {
                    if(xml.isEndElement())
                    {
                        xml.readNext();
                        i++;
                    }
                    else if(xml.isStartElement())
                    {
                        if(xml.name() != "parameter" && xml.name() != "list" && xml.name() != "idlist")
                        {
                            m_iAnzahlMenus ++;
                            foreach(const QXmlStreamAttribute &attr, xml.attributes())
                            {
                                if (attr.name().toString() == QLatin1String("id"))
                                {                                  
                                    QString attribute_value = attr.value().toString();
                     //               qDebug() << "id" << attribute_value << "i" << i;

                                    switch (i - m_iNextEbene * 8)
                                    {
                                        case 0:  m_menu_0_1 = attribute_value;  break;
                                        case 1:  m_menu_0_2 = attribute_value;  break;
                                        case 2:  m_menu_0_3 = attribute_value;  break;
                                        case 3:  m_menu_0_4 = attribute_value;  break;
                                        case 4:  m_menu_0_5 = attribute_value;  break;
                                        case 5:  m_menu_0_6 = attribute_value;  break;
                                        case 6:  m_menu_0_7 = attribute_value;  break;
                                        case 7:  m_menu_0_8 = attribute_value;  break;
                                    }

                                }
                                if (attr.name().toString() == QLatin1String("name"))
                                {
                                    QString attribute_value = attr.value().toString();
                   //                 qDebug() << "name" << attribute_value;

                                    switch (i - m_iNextEbene * 8)
                                    {
                                        case 0:  m_menu_0_1_text = attribute_value; emit menu1Changed();
                                                 if (bUeberschrift == false)
                                                 {
                                                     if (m_menu_0_1_text != "Root")
                                                     {
                                                        m_szUeberschrift = m_menu_0_1_text;
                                                        emit ueberschriftChanged();
                                                     }
                                                     bUeberschrift = true;
                                                 }
                                                 break;
                                        case 1:  m_menu_0_2_text = attribute_value; emit menu2Changed(); break;
                                        case 2:  m_menu_0_3_text = attribute_value; emit menu3Changed(); break;
                                        case 3:  m_menu_0_4_text = attribute_value; emit menu4Changed(); break;
                                        case 4:  m_menu_0_5_text = attribute_value; emit menu5Changed(); break;
                                        case 5:  m_menu_0_6_text = attribute_value; emit menu6Changed(); break;
                                        case 6:  m_menu_0_7_text = attribute_value; emit menu7Changed(); break;
                                        case 7:  m_menu_0_8_text = attribute_value; emit menu8Changed(); break;
                                    }
                                }
                            }
                        }
                        else        //
                        {
                            if(xml.name() == QLatin1String("parameter"))
                            {
                                bParameterVorhanden = true;
                            }
                            m_parameter = xml.readElementText();        // nächstes Element
                        }
                        xml.readNext();
                    }
                    else
                    {
                        xml.readNext();
                    }
                }
            }
        }
        else
        {
            xml.readNext();
        }
    }


//    QStringList dataList;

    m_ListeBezeichnung.clear();
    m_ListeWert.clear();


    m_ListeComboBezeichnung.clear();
    m_ListeComboInhalt.clear();
    m_ListeButtonBezeichnung.clear();
    m_ListeButtonInhalt.clear();
    m_ListeSwitchBezeichnung.clear();
    m_ListeSwitchInhalt.clear();
    m_ListeEditBezeichnung.clear();
    m_ListeEditInhalt.clear();


    if (bParameterVorhanden == true)
    {
//        qDebug() << "Parameter vorhanden";

        QString attribute_value;
        QString textid;
        QString idvalue;
        bool bReadonlyGefunden;
        bool bButtonGefunden;
        bool bListElement;
        bool bListIdElement;

        QXmlStreamReader xml1(temp);                         // https://gist.github.com/lamprosg/2133804
        xml1.readNext();
        while(!xml1.atEnd())
        {
            if(xml1.isStartElement())
            {
                if(xml1.name() == "menu")
                {
                    bListElement = false;
                    bListIdElement = false;
                    while(!xml1.atEnd())
                    {
                        if(xml1.isEndElement())
                        {
                          //  qDebug() << "isEndElement: " << xml1.name();
                            if(xml1.name() == "parameter")
                            {
                                if (bListElement == false && bListIdElement == false &&         // keine Liste
                                    bButtonGefunden == false && bReadonlyGefunden == false)     // kein Schalter und auch kein Label
                                {
                                    m_ListeComboBezeichnung.removeLast();
                                    m_ListeComboInhalt.removeLast();
                                }
                                else
                                if (bListElement == false && bListIdElement == true &&          // Liste
                                    bButtonGefunden == false && bReadonlyGefunden == false)     // kein Schalter und auch kein Label
                                {
                                    m_ListeEditBezeichnung.removeLast();
                                    m_ListeEditInhalt.removeLast();
                                }
                                bButtonGefunden = false;        // zurücksetzen
                                bListElement = false;
                                bListIdElement = false;
                            }
                            xml1.readNext();                // nächstes Element
                        }
                        else if(xml1.isStartElement())
                        {
                     //       qDebug() << "Name: " << xml1.name();
                            if(xml1.name() == "parameter")
                            {
                                bReadonlyGefunden = false;
                                bButtonGefunden = false;

                                foreach(const QXmlStreamAttribute &attr, xml1.attributes())
                                {
                                    attribute_value = attr.value().toString();

                                    if (attr.name().toString() == QLatin1String("textid"))
                                    {
                                        textid = attribute_value;
                                    }
                                    else
                                    if (attr.name().toString() == QLatin1String("name"))
                                    {
                                        m_ListeBezeichnung.append(attribute_value);
                                    }
                                    else
                                    if (attr.name().toString() == QLatin1String("idvalue"))
                                    {
                                        m_ListeWert.append(attribute_value);
                                    }
                                    else
                                    if (attr.name().toString() == QLatin1String("id") || attr.name().toString() == QLatin1String("size"))
                                    {
                                    }
                                    else
                                    if (attr.name().toString() == QLatin1String("readonly"))
                                    {
                                        // ReadOnly ist ein einfaches Label, kein Schalter oder Eingabe
                                        //
                                        bReadonlyGefunden = true;
                                    }
                                }
                                if (bReadonlyGefunden == false)
                                {
                                    // Kein readonly, also ist es kein einfaches Label
                                    //
                                    if (m_ListeWert.last() == "Off" || m_ListeWert.last() == "On")      // ein Umschalter
                                    {
                                        m_ListeSwitchBezeichnung.append(m_ListeBezeichnung.last());
                                        m_ListeSwitchInhalt.append(m_ListeWert.last());

                                        bButtonGefunden = true;
                                    }
                                    else if (m_ListeWert.last() == "Button")                            // ein Schalter
                                    {
                                        m_ListeButtonBezeichnung.append(m_ListeBezeichnung.last());
                                        m_ListeButtonInhalt.append(m_ListeWert.last());

                                        bButtonGefunden = true;
                                    }
                                    else        // Edit oder Combo, erst einmal beides sichern
                                    {
                                        m_ListeEditBezeichnung.append(m_ListeBezeichnung.last());
                                        m_ListeEditInhalt.append(m_ListeWert.last());

                                        m_ListeComboBezeichnung.append(m_ListeBezeichnung.last());
                                        m_ListeComboInhalt.append(m_ListeWert.last());
                                    }

                                    m_ListeBezeichnung.removeLast();
                                    m_ListeWert.removeLast();
                                }
                            }
                            else if(xml1.name() == "list")
                            {
                                bListElement = true;
                                bListIdElement = false;
                        //        qDebug() << "list: " ;
                            }
                            else if(xml1.name() == "idlist")
                            {
                                bListElement = false;
                                bListIdElement = true;            // wenn es kein Listelement gibt, ist es ein Edit-Feld
                        //        qDebug() << "idlist: " ;
                            }
                            else if(xml1.name() == "element" && bListIdElement == true && bButtonGefunden == false)
                            {
                                idvalue = xml1.readElementText();
                        //        qDebug() << "idvalue" << idvalue;
                                m_ListeComboInhalt.append(idvalue);
                            }

                            xml1.readNext();
                        }
                        else
                        {
                            xml1.readNext();
                        }
                    }
                }
            }
            else
            {
                xml1.readNext();
            }
        }
//        qDebug() << szAusgabe;

        if (m_ListeBezeichnung.length() > 0) emit bezeichnungChanged();
        if (m_ListeWert.length() > 0) emit wertChanged();                   // In Abhängigkeit von der Länge

        emit buttonChanged();                                               // oder einfach nur so?
        emit switchChanged();
        emit comboChanged();
        emit editChanged();
    }

    qDebug() << "m_iAnzahlMenus" << m_iAnzahlMenus << "m_iNextEbene" << m_iNextEbene << ", i: " << i;

    if (m_iAnzahlMenus > 8)
    {
        m_bNextSchalter = true;
    }
    else
    {
        m_iNextEbene = 0;
    }
    emit nextButtonChanged();

    m_Antwort = temp;
    emit antwortChanged();

    return m_parameter;
}

QString BackEnd::printerName()
{
    return m_printerName;
}

QString BackEnd::firmware()
{
    return m_firmware;
}

QString BackEnd::menu1()
{
    return m_menu_0_1_text;
}

QString BackEnd::menu2()
{
    return m_menu_0_2_text;
}

QString BackEnd::menu3()
{
    return m_menu_0_3_text;
}

QString BackEnd::menu4()
{
    return m_menu_0_4_text;
}

QString BackEnd::menu5()
{
    return m_menu_0_5_text;
}

QString BackEnd::menu6()
{
    return m_menu_0_6_text;
}

QString BackEnd::menu7()
{
    return m_menu_0_7_text;
}

QString BackEnd::menu8()
{
    return m_menu_0_8_text;
}

QString BackEnd::menu1id()
{
    return m_menu_0_1;
}

QString BackEnd::menu2id()
{
    return m_menu_0_2;
}

QString BackEnd::menu3id()
{
    return m_menu_0_3;
}

QString BackEnd::menu4id()
{
    return m_menu_0_4;
}

QString BackEnd::menu5id()
{
    return m_menu_0_5;
}

QString BackEnd::menu6id()
{
    return m_menu_0_6;
}

QString BackEnd::menu7id()
{
    return m_menu_0_7;
}

QString BackEnd::menu8id()
{
    return m_menu_0_8;
}

void BackEnd::menuButtonClicked(const QString &param)
{
    m_menu_suchstring = "FCRX--raction=\"getmenu\"&menu=\"" + param + "\"&sessionid=\"" + m_sessionid + "\"";
    getMenu0();
}


void BackEnd::setSendString(const QString &param)
{
    TCPSocket s;
    s.setSendString(param);
    s.doConnect();
    m_Antwort = s.GetAntwort();
}

QString BackEnd::getAntwort()
{
    m_Antwort = cleanupString(m_Antwort);

//    qDebug() << "m_Antwort" << m_Antwort;

    return m_Antwort;
}

int BackEnd::getSocketStatusColor()
{
    int iRetval = 0;
    if (m_status.contains("Idle"))
    {
       iRetval = 1;
    }
    else if (m_status.contains("Error"))
    {
       iRetval = 2;
    }
    else if (m_status.contains("Printing"))
    {
       iRetval = 3;
    }
    else if (m_status.contains("Stopped"))
    {
       iRetval = 4;
    }
    return iRetval;
}

bool BackEnd::nextButton()
{
    return m_bNextSchalter;
}

int BackEnd::nextEbene()
{
    return m_iNextEbene;
}

void BackEnd::menuNextClicked(bool bVal)
{
    if (bVal == true)
    {
        m_iNextEbene ++;
    }
    else
    {
        m_iNextEbene --;
    }
}

QString BackEnd::szUeberschrift()
{
    return m_szUeberschrift;
}

QStringList BackEnd::szBezeichnung()
{
    qDebug() << "m_ListeBezeichnung      " << m_ListeBezeichnung;
    qDebug() << "m_ListeWert             " << m_ListeWert;

    qDebug() << "m_ListeComboBezeichnung " << m_ListeComboBezeichnung;
    qDebug() << "m_ListeComboInhalt      " << m_ListeComboInhalt;
    qDebug() << "m_ListeButtonBezeichnung" << m_ListeButtonBezeichnung;
    qDebug() << "m_ListeButtonInhalt     " << m_ListeButtonInhalt;
    qDebug() << "m_ListeSwitchBezeichnung" << m_ListeSwitchBezeichnung;
    qDebug() << "m_ListeSwitchInhalt     " << m_ListeSwitchInhalt;
    qDebug() << "m_ListeEditBezeichnung  " << m_ListeEditBezeichnung;
    qDebug() << "m_ListeEditInhalt       " << m_ListeEditInhalt;

    return m_ListeBezeichnung;
}

QStringList BackEnd::szWert()
{
//    qDebug() << m_ListeWert;
    return m_ListeWert;
}


QStringList BackEnd::szButton()
{
    return m_ListeButtonBezeichnung;
}

QStringList BackEnd::szButtonValue()
{
    return m_ListeButtonInhalt;
}

QStringList BackEnd::szSwitch()
{
    return m_ListeSwitchBezeichnung;
}

QStringList BackEnd::szSwitchValue()
{
    return m_ListeSwitchInhalt;
}

QStringList BackEnd::szCombo()
{
    return m_ListeComboBezeichnung;
}

QStringList BackEnd::szComboValue()
{
    return m_ListeComboInhalt;
}

QStringList BackEnd::szEdit()
{
    return m_ListeEditBezeichnung;
}

QStringList BackEnd::szEditValue()
{
    return m_ListeEditInhalt;
}

