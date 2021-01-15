#include "backend.h"
#include "tcpsocket.h"

BackEnd::BackEnd(QObject *parent) :
    QObject(parent)
{
    m_iNextEbene = 0;
    m_stackUeberschrift.clear();
    m_AnzahlSwitche = 0;
    m_AnzahlComboboxen = 0;
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
    s.doConnect(true);
    m_status = s.SocketStatus();
    m_status = cleanupString(m_status);

   // s_handshake h = getHandshake();
    return m_status;
}

QString BackEnd::getHandshake()
{
    TCPSocket s;
    s.doConnect(true);

    s.setSendString("FCRX--raction=\"handshake\"&sessionid=\"0\"");
    s.doConnect(true);
    QString temp = s.GetAntwort();
    temp = cleanupString(temp);
    if (temp.contains("TTZ"))
    {
       temp.remove("TTZ");
    }

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
    s.doConnect(true);

    if (m_menu_suchstring.length() == 0)
    {
        m_menu_suchstring = "FCRX--raction=\"getmenu\"&menu=\"0\"&sessionid=\"" + m_sessionid + "\"";
    }
    s.setSendString(m_menu_suchstring);
    qDebug() << "getMenu0() " << m_menu_suchstring;

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

    s.doConnect(true);
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

//    qDebug() << "temp\r\n" << temp << "\r\n";

    bool bParameterVorhanden = false;
    bool bUeberschrift = false;
    QXmlStreamReader xml(temp);                         // https://gist.github.com/lamprosg/2133804
    xml.readNext();
    bUeberschrift = false;
    m_szUeberschrift = "";
    emit ueberschriftChanged();

//    if (i < 8)          // Erst einmal alle Einträge löschen
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

    int iAbfragewert = 0;
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

                                    iAbfragewert = 0;
                                    if (m_iAnzahlMenus > 8)                     // Funktioniert bei z.B. 17 Menüs. Nur wenn auf der Ebene 1 anschließend 3 Menüs
                                    {                                           // kommen, würde i auf -5 reduziert werden und somit wird nichts angezeigt.
                                        iAbfragewert = m_iNextEbene * 8;        // Deshalb entsprechend korrigieren.
                                    }
                                    if ((i - iAbfragewert) >= 0)
                                    {
                                        switch (i - iAbfragewert)
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
                                }
                                if (attr.name().toString() == QLatin1String("name"))
                                {
                                    QString attribute_value = attr.value().toString();
                                    qDebug() << "name" << attribute_value;

                                    iAbfragewert = 0;
                                    if (m_iAnzahlMenus > 8)
                                    {
                                        iAbfragewert = m_iNextEbene * 8;
                                    }

                                    if ((i - iAbfragewert) >= 0)
                                    {
                                        qDebug() << "name i " << i << ", attribute_value: " << attribute_value;
                                        if ((i - iAbfragewert) == 0)
                                        {
                                            m_menu_0_1_text = attribute_value; emit menu1Changed();
                                            if (bUeberschrift == false)
                                            {
                                                 if (m_menu_0_1_text != "Root")
                                                 {
                                                    m_szUeberschrift = m_menu_0_1_text;
                                                    emit ueberschriftChanged();
                                                 }
                                                 bUeberschrift = true;
                                            }
                                        }
                                        // else: attribute_value = attr.value().toString();
                                    }
                                    else
                                    {
                                        attribute_value = "";
                                    }
                                    switch (abs(i - iAbfragewert))
                                    {
                                        case 1:  m_menu_0_2_text = attribute_value; emit menu2Changed(); break;
                                        case 2:  m_menu_0_3_text = attribute_value; emit menu3Changed(); break;
                                        case 3:  m_menu_0_4_text = attribute_value; emit menu4Changed(); break;
                                        case 4:  m_menu_0_5_text = attribute_value; emit menu5Changed(); break;
                                        case 5:  m_menu_0_6_text = attribute_value; emit menu6Changed(); break;
                                        case 6:  m_menu_0_7_text = attribute_value; emit menu7Changed(); break;
                                        case 7:  m_menu_0_8_text = attribute_value; emit menu8Changed(); break;
                                    }

                /*                    if ((i - iAbfragewert) >= 0)
                                    {
                                        qDebug() << "name i " << i << ", attribute_value: " << attribute_value;
                                        switch (i - iAbfragewert)
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
                                    else
                                    {
                                        switch (iAbfragewert - i)
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

                                    }  */
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

    qDebug() << temp << "\r\n";

    m_ListeBezeichnung.clear();
    m_ListeWert.clear();


    m_ListeComboBezeichnung.clear();
//    for (int i = 0; i < MAXCOMBO; i++) {    m_ListeComboInhalt[i].clear();             }
    for (int i = 0; i < MAXCOMBO; i++) {    m_ListeComboInhalt[i].clear();      m_ListeComboIdInhalt[i].clear();       }
    m_ListeComboAktuell.clear();
    m_ListeButtonBezeichnung.clear();
    m_ListeButtonInhalt.clear();
    m_ListeSwitchBezeichnung.clear();
    for (int i = 0; i < MAXCOMBO; i++) {    m_ListeSwitchInhalt[i].clear();     m_ListeSwitchIdInhalt[i].clear();    }
    m_ListeSwitchAktuell.clear();
    m_ListeEditBezeichnung.clear();
    m_ListeEditInhalt.clear();
    m_ListeEditId.clear();
    m_ListeComboId.clear();
    m_ListeSwitchId.clear();
    m_ListeButtonId.clear();

    m_AnzahlComboboxen = 0;
    m_AnsichtComboNr = 0;
    m_AnzahlSwitche = 0;
    m_AnsichtSwitchNr = 0;

    QString tempInhalt;
    QString tempIdInhalt;

    if (bParameterVorhanden == true)
    {
//        qDebug() << "Parameter vorhanden";

        QString attribute_value;
        QString textid;
        QString id;
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
                                    m_ListeComboAktuell.removeLast();
                                    m_ListeComboId.removeLast();                    // keine Combobox

                                    m_ListeSwitchBezeichnung.removeLast();          // kein Schalter
                                    m_ListeSwitchAktuell.removeLast();
                                    m_ListeSwitchId.removeLast();

                            //        qDebug() << "ein Edit-Feld";
                                }
                                else
                                if (bListIdElement == true && bListElement == false &&          // Liste
                                    bButtonGefunden == false && bReadonlyGefunden == false)     // kein Schalter und auch kein Label
                                {
                                    m_ListeEditBezeichnung.removeLast();            // kein Edit
                                    m_ListeEditInhalt.removeLast();
                                    m_ListeEditId.removeLast();

                                    // Eine Combobox könnte auch ein Switch sein
                                    if (m_ListeComboInhalt[m_AnzahlComboboxen].length() == 2)   // verdächtig, On Off sind nur zwei
                                    {
                                        if ((m_ListeComboIdInhalt[m_AnzahlComboboxen][0] == "On" && m_ListeComboIdInhalt[m_AnzahlComboboxen][1] == "Off") ||
                                            (m_ListeComboIdInhalt[m_AnzahlComboboxen][1] == "On" && m_ListeComboIdInhalt[m_AnzahlComboboxen][0] == "Off"))
                                        {
                                                         // Zwischenspeichern, damit die Reihenfolge stimmt
                                            tempInhalt = m_ListeComboInhalt[m_AnzahlComboboxen].last();         //
                                            tempIdInhalt = m_ListeComboIdInhalt[m_AnzahlComboboxen].last();         //
                                            m_ListeComboInhalt[m_AnzahlComboboxen].removeLast();
                                            m_ListeComboIdInhalt[m_AnzahlComboboxen].removeLast();

                                            m_ListeSwitchInhalt[m_AnzahlSwitche].append(m_ListeComboInhalt[m_AnzahlComboboxen].last());
                                            m_ListeSwitchIdInhalt[m_AnzahlSwitche].append(m_ListeComboIdInhalt[m_AnzahlComboboxen].last());
                                            m_ListeSwitchInhalt[m_AnzahlSwitche].append(tempInhalt);
                                            m_ListeSwitchIdInhalt[m_AnzahlSwitche].append(tempIdInhalt);

                                            m_ListeComboBezeichnung.removeLast();           // keine Combobox
                                            m_ListeComboAktuell.removeLast();
                                            m_ListeComboInhalt[m_AnzahlComboboxen].clear();
                                            m_ListeComboIdInhalt[m_AnzahlComboboxen].clear();
                                            m_ListeComboId.removeLast();

                                            m_AnzahlSwitche ++;
                                        }
                                        else            // Combo
                                        {
                                            m_ListeSwitchBezeichnung.removeLast();          // kein Schalter
                                            m_ListeSwitchAktuell.removeLast();
                                            m_ListeSwitchId.removeLast();

                                            m_AnzahlComboboxen ++;
                                        }
                                    }
                                    else                // Combo
                                    {
                                        m_ListeSwitchBezeichnung.removeLast();              // kein Schalter
                                        m_ListeSwitchAktuell.removeLast();
                                        m_ListeSwitchId.removeLast();

                                        m_AnzahlComboboxen ++;
                                    }
                                }
                                bButtonGefunden = false;        // zurücksetzen
                                bListElement = false;
                                bListIdElement = false;
                            }
                            xml1.readNext();                // nächstes Element
                        }
                        else if(xml1.isStartElement())
                        {
                           // qDebug() << "Name: " << xml1.name();
                            if(xml1.name() == "parameter")
                            {
                                bReadonlyGefunden = false;
                                bButtonGefunden = false;

                                i = 0;
                                foreach(const QXmlStreamAttribute &attr, xml1.attributes())
                                {
                                    if (i == 0)         // Voreinstellung für jeden Parameter
                                    {
                                        m_ListeMinWert.append("0");
                                        m_ListeMaxWert.append("0");
                                    }
                                    attribute_value = attr.value().toString();

                                    if (attr.name().toString() == QLatin1String("textid"))
                                    {
                                        textid = attribute_value;
                                    }
                                    else
                                    if (attr.name().toString() == QLatin1String("name"))
                                    {
                            //            qDebug() << "attribute_value: " << attribute_value;
                                        m_ListeBezeichnung.append(attribute_value);
                                    }
                                    else
                                    if (attr.name().toString() == QLatin1String("idvalue"))
                                    {
                                        m_ListeWert.append(attribute_value);
                                    }
                                    else
                                    if (attr.name().toString() == QLatin1String("id"))
                                    {
                                        id = attribute_value;
                                    }
                                    else
                                    if (attr.name().toString() == QLatin1String("size"))
                                    {
                                    }
                                    else
                                    if (attr.name().toString() == QLatin1String("readonly"))
                                    {
                                        // ReadOnly ist ein einfaches Label, kein Schalter oder Eingabe
                                        //
                                        bReadonlyGefunden = true;
                                    }

                                    if (attr.name().toString() == QLatin1String("minvalue"))
                                    {
                                        m_ListeMinWert.removeLast();                // 0 durch den richtigen Wert ersetzen
                                        m_ListeMinWert.append(attribute_value);
                                    }
                                    if (attr.name().toString() == QLatin1String("maxvalue"))
                                    {
                                        m_ListeMaxWert.removeLast();                // 0 durch den richtigen Wert ersetzen
                                        m_ListeMaxWert.append(attribute_value);
                                    }
                                }
                                if (bReadonlyGefunden == false)
                                {
                                    // Kein readonly, also ist es kein einfaches Label
                                    //
                                    if (m_ListeWert.last() == "Button")                            // ein Schalter
                                    {
                                        m_ListeButtonBezeichnung.append(m_ListeBezeichnung.last());
                                        m_ListeButtonInhalt.append(m_ListeWert.last());
                                        m_ListeButtonId.append(id);

                                        bButtonGefunden = true;
                                    }
                                    else        // Edit, Combo oder Switch: erst einmal beides sichern
                                    {
                                        m_ListeEditBezeichnung.append(m_ListeBezeichnung.last());       // "name"
                                        m_ListeEditInhalt.append(m_ListeWert.last());                   // "idvalue"
                                        m_ListeEditId.append(id);                                       // "id"

                                        m_ListeSwitchBezeichnung.append(m_ListeBezeichnung.last());
                                        m_ListeSwitchAktuell.append(m_ListeWert.last());                // "idvalue"
                                        m_ListeSwitchId.append(id);                                     // "id"

                                        m_ListeComboBezeichnung.append(m_ListeBezeichnung.last());
                                        m_ListeComboAktuell.append(m_ListeWert.last());                 // "idvalue"
                                        m_ListeComboId.append(id);                                      // "id"
                                    }

                                    m_ListeBezeichnung.removeLast();
                                    m_ListeWert.removeLast();
                                }
                            }
                            else if(xml1.name() == "list")
                            {
                                bListElement = true;
                                bListIdElement = false;
                       //         qDebug() << "list: " ;
                            }
                            else if(xml1.name() == "idlist")
                            {
                                bListElement = false;
                                bListIdElement = true;            // wenn es kein Listelement gibt, ist es ein Edit-Feld
                       //         qDebug() << "idlist: " ;
                            }
                            else if(xml1.name() == "element" && bButtonGefunden == false && bReadonlyGefunden == false)
                            {
                                idvalue = xml1.readElementText();
                       //         qDebug() << "element " << idvalue;

                                if (bListElement == true)
                                {
                                    m_ListeComboInhalt[m_AnzahlComboboxen].append(idvalue);
                                }
                                else if (bListIdElement == true)
                                {
                                    m_ListeComboIdInhalt[m_AnzahlComboboxen].append(idvalue);
                                }
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
    }

    // Wenn z.B. emit bezeichnungChanged(); in Abhängigkeit des Inhaltes kommen würde, würde der Text in der Anzeige nicht gelöscht werden,
    // wenn der Inhalt leer ist. Nur mit emit ... erfolgt ein Update der Anzeige. Das bedeutet auch, wenn kein Parameter anzuzeigen ist,
    // würden die vorherigen Parameter erhalten bleiben. Also generell bei jedem Durchlauf emit ... ausführen.

    emit bezeichnungChanged();
    emit wertChanged();
    emit buttonChanged();
    emit switchChanged();
    emit comboChanged();
    emit editChanged();

    qDebug() << "m_iAnzahlMenus" << m_iAnzahlMenus << "m_iNextEbene" << m_iNextEbene << ", i: " << i;

    if (m_iAnzahlMenus > 8)
    {
        m_bNextSchalter = true;
    }
/*    else
    {
        m_iNextEbene = 0;
    }*/
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

 //   qDebug() << "menuButtonClicked() vor m_stackMenuId.append " << m_stackMenuId.length() << ", param: << param";
    if (param == "0")
    {
        m_stackMenuId.clear();
    }
    else
    {
        if (m_stackMenuId.length() > 0)                 // last() kann nur aufgerufen werden, wenn mind. 1 Element drin ist
        {
            if (param != m_stackMenuId.last())          // müssen ungleich sein, weil ansonsten beim Zurückgehen von 31 - 57 auf 31 die 31 nochmal addiert würde
            {
                m_stackMenuId.append(param);
            }
        }
        else    m_stackMenuId.append(param);
    }
    emit stackMenuIdChanged();

    getMenu0();             // neues Menü laden
}


void BackEnd::setSendString(const QString &param)
{
//    qDebug() << "setSendString() param: " << param;
    TCPSocket s;
    s.setSendString(param);
    s.doConnect(true);
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
    qDebug() << "menuNextClicked() m_iNextEbene: " << m_iNextEbene;
}


QStringList BackEnd::szBezeichnung()
{
    return m_ListeBezeichnung;
}

QStringList BackEnd::szWert()
{
//    qDebug() << m_ListeWert;
    return m_ListeWert;
}


QStringList BackEnd::szButton()
{
/*    qDebug() << "m_ListeBezeichnung      " << m_ListeBezeichnung;
    qDebug() << "m_ListeWert             " << m_ListeWert;

    qDebug() << "m_ListeComboBezeichnung " << m_ListeComboBezeichnung;
    qDebug() << "m_ListeComboInhalt      " << m_ListeComboInhalt[0];
    qDebug() << "m_ListeButtonBezeichnung" << m_ListeButtonBezeichnung;
    qDebug() << "m_ListeButtonInhalt     " << m_ListeButtonInhalt;
    qDebug() << "m_ListeSwitchBezeichnung" << m_ListeSwitchBezeichnung;
    qDebug() << "m_ListeSwitchInhalt     " << m_ListeSwitchInhalt;
    qDebug() << "m_ListeEditBezeichnung  " << m_ListeEditBezeichnung;
    qDebug() << "m_ListeEditInhalt       " << m_ListeEditInhalt;
*/

/*
    {
        qDebug() << "m_ListeSwitchBezeichnung" << m_ListeSwitchBezeichnung;
        qDebug() << "m_ListeSwitchInhalt[0]     " << m_ListeSwitchInhalt[0];
        qDebug() << "m_ListeSwitchInhalt[1]     " << m_ListeSwitchInhalt[1];
        qDebug() << "m_ListeSwitchIdInhalt[0]   " << m_ListeSwitchIdInhalt[0];
        qDebug() << "m_ListeSwitchIdInhalt[1]   " << m_ListeSwitchIdInhalt[1];
        qDebug() << "m_ListeSwitchAktuell       " << m_ListeSwitchAktuell;
    }
*/
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
 //   qDebug() << "szSwitchValue()";

    QStringList qslTemp;
    if (m_AnzahlSwitche > 0)
    {
        if (m_AnsichtSwitchNr < m_AnzahlSwitche)
            qslTemp = m_ListeSwitchInhalt[m_AnsichtSwitchNr];
        else
            qslTemp = m_ListeSwitchInhalt[m_AnzahlSwitche - 1];
    }
    m_AnsichtSwitchNr ++;

 //   qDebug() << "szSwitchValue qslTemp" << qslTemp << ", m_AnzahlSwitche " << m_AnzahlSwitche << ", m_AnsichtSwitchNr " << m_AnsichtSwitchNr;
    return qslTemp;

}

QStringList BackEnd::szSwitchAktuell()
{
    return m_ListeSwitchAktuell;
}

QStringList BackEnd::szCombo()
{
    return m_ListeComboBezeichnung;
}

QStringList BackEnd::szComboValue()
{
//    qDebug() << "szComboValue m_AnsichtComboNr: " << m_AnsichtComboNr << ", m_AnzahlComboboxen: " << m_AnzahlComboboxen;
    QStringList qslTemp;
    // Wenn es mehrere Comboboxen gibt, werden diese per  model: backend.szCombo.length und delegate: ComboBox aufgerufen.
    // Mit jedem Aufruf wird m_AnsichtComboNr um 1 erhöht. Damit kann dem eigentlichen Aufruf von model: backend.szComboValue (Füllen der Combobox)
    // ein anderer Inhalt vorgegaukelt werden.

    if (m_AnzahlComboboxen > 0)
    {
        if (m_AnsichtComboNr < m_AnzahlComboboxen)
            qslTemp = m_ListeComboInhalt[m_AnsichtComboNr];
        else
            qslTemp = m_ListeComboInhalt[m_AnzahlComboboxen - 1];
    }
    m_AnsichtComboNr ++;
//    qDebug() << "szComboValue qslTemp" << qslTemp;
    return qslTemp;
}

QStringList BackEnd::szComboAktuell()
{
  //  qDebug() << "szComboAktuell " << m_ListeComboAktuell;
    return m_ListeComboAktuell;
}

int BackEnd::iGetPreselectedComboId()
{
    int len = m_ListeComboIdInhalt[m_ListeIndex].length();
    for (int i = 0; i < len; i++)
    {
        if (m_ListeComboIdInhalt[m_ListeIndex][i] == m_ListeComboAktuell[m_ListeIndex])
        {
            return i;
        }
    }
    return -1;
}

QStringList BackEnd::szEdit()
{
    return m_ListeEditBezeichnung;
}

QStringList BackEnd::szEditValue()
{
    return m_ListeEditInhalt;
}

bool BackEnd::bInhaltVorhanden()
{
 //   qDebug() << "Label " <<m_ListeBezeichnung.length() << ", Combo " << m_ListeComboBezeichnung.length() << ", Button " << m_ListeButtonBezeichnung.length() <<
 //               ", Switch " << m_ListeSwitchBezeichnung.length() << ", Edit " << m_ListeEditBezeichnung.length();

    if (m_ListeBezeichnung.length() > 0 || m_ListeComboBezeichnung.length() > 0 || m_ListeButtonBezeichnung.length() > 0 ||
        m_ListeSwitchBezeichnung.length() > 0 || m_ListeEditBezeichnung.length() > 0)
        return true;
    else
        return false;
}

bool BackEnd::stackUeberschriftAdd(bool bAdd)
{
    if (m_szUeberschrift.length() == 0)
    {
        return false;
    }

    if (bAdd == true)
    {
        m_stackUeberschrift.append(m_szUeberschrift);
    }
    else        // bei false einen Menü-Step zurück
    {
        if (m_stackUeberschrift.length() > 0)
        {
            m_stackUeberschrift.removeLast();
        }

        if (m_stackMenuId.length() > 0)
        {
       //     qDebug() << "stackUeberschriftAdd() vor m_stackMenuId.removeLast " << m_stackMenuId.length();
            m_stackMenuId.removeLast();
        }
    }
    emit stackUeberschriftChanged();
    emit stackMenuIdChanged();

 //   qDebug() << "stackUeberschrift: " << m_stackUeberschrift << ", m_szUeberschrift: " << m_szUeberschrift << ", m_stackMenuId: " << m_stackMenuId;
    return true;
}

QString BackEnd::getStackUeberschrift()             // Eine sog. Brotkrumen-Überschrift erzeugen
{
    QString strTemp = "";
    if (m_stackUeberschrift.length() > 0)
    {
        strTemp = m_stackUeberschrift.first();
        int i = 1;

        while (i < m_stackUeberschrift.length())
        {
            strTemp += " - " + m_stackUeberschrift[i];
            i++;
        }
    }
 //   qDebug() << "getStackUeberschrift() " << strTemp;
    return strTemp;
}

bool BackEnd::stackUeberschrift()       // dummy
{
    return true;
}

QString BackEnd::getStackMenuId()
{
    if (m_stackMenuId.length() > 0)
        return m_stackMenuId.last();
    else
        return "0";
}

//--------- Rückmeldungen der Kontrollelemente des Dialoges ---------------
//
bool BackEnd::SendParamToPrinter(QString id, QString param)
{
    TCPSocket s;
    s.setSendString("FCRX--raction=\"setparam\"&param=\"" + id + "\"&value=\"" + param + "\"");
    s.doConnect(true);
    m_Antwort = s.GetAntwort();
    qDebug() << m_Antwort;
    if (m_Antwort.contains("Error"))
    {
        return false;
    }
    return true;
}

void BackEnd::setIndex(int id)
{
    m_ListeIndex = id;
}

void BackEnd::setSwitchValue(const QString &param)
{
  //  if (m_ListeSwitchAktuell[m_ListeIndex] != param)
    {
        qDebug() << "setSwitchValue() " << m_ListeSwitchBezeichnung[m_ListeIndex] << " " << param << ", Id " << m_ListeSwitchId[m_ListeIndex];

        if (SendParamToPrinter(m_ListeSwitchId[m_ListeIndex], param))
        {
   //         m_ListeSwitchAktuell[m_ListeIndex] != param;
        }
    }
}

void BackEnd::setComboValue(int id)
{
    if (m_ListeComboAktuell[m_ListeIndex] != m_ListeComboIdInhalt[m_ListeIndex][id])
    {
        qDebug() << "setComboValue() " << m_ListeComboBezeichnung[m_ListeIndex] << " " << m_ListeComboInhalt[m_ListeIndex][id] << ", Id " << m_ListeComboId[m_ListeIndex] << ", Aktuell " << m_ListeComboAktuell[m_ListeIndex];

        if (SendParamToPrinter(m_ListeComboId[m_ListeIndex], m_ListeComboInhalt[m_ListeIndex][id]))
        {
            m_ListeComboAktuell[m_ListeIndex] = m_ListeComboIdInhalt[m_ListeIndex][id];
        }
    }
}

void BackEnd::setEditValue(const QString &param)
{
    if (m_ListeEditInhalt[m_ListeIndex] != param)
    {
        qDebug() << "setEditValue() " << m_ListeEditBezeichnung[m_ListeIndex] << " " << m_ListeEditInhalt[m_ListeIndex] << " " << param << ", Id " << m_ListeEditId[m_ListeIndex];
        m_ListeEditInhalt[m_ListeIndex] = param;

        if (SendParamToPrinter(m_ListeEditId[m_ListeIndex], param))
        {
            m_ListeEditInhalt[m_ListeIndex] = param;
        }
    }
}

void BackEnd::setButtonValue(const QString &param)
{
    qDebug() << "setButtonValue() " << m_ListeButtonInhalt[m_ListeIndex] << " " << param << ", Id " << m_ListeButtonId[m_ListeIndex];
}
