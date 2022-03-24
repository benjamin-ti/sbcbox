#ifndef BACKEND_H
#define BACKEND_H

#include <QObject>
#include <QString>
#include <QXmlStreamReader>
#include <QtCore>
//#include <QtXML>


/*struct s_handshake {
    QString m_version;
    QString m_printerName;
    QString m_firmware;
    QString m_sessionid;
    QString m_locale;
    QString m_rtpreview;
}; Q_DECLARE_METATYPE(s_handshake);
*/
class BackEnd : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString userName READ userName WRITE setUserName NOTIFY userNameChanged)
    // Über Q_PROPERTY werden Funktionen usw. nach außen in QML sichtbar gemacht.
    Q_PROPERTY(QString socketStatus READ socketStatus)
    Q_PROPERTY(int getSocketStatusColor READ getSocketStatusColor)
    Q_PROPERTY(QString setSendString WRITE setSendString )
    Q_PROPERTY(QString getAntwort READ getAntwort NOTIFY antwortChanged)
    Q_PROPERTY(QString printerName READ printerName )
    Q_PROPERTY(QString firmware READ firmware )

    Q_PROPERTY(QString getHandshake READ getHandshake )
    Q_PROPERTY(QString getMenu0 READ getMenu0 )

    Q_PROPERTY(QString menu1 READ menu1 WRITE menuButtonClicked NOTIFY menu1Changed )
    Q_PROPERTY(QString menu2 READ menu2 WRITE menuButtonClicked NOTIFY menu2Changed )
    Q_PROPERTY(QString menu3 READ menu3 WRITE menuButtonClicked NOTIFY menu3Changed )
    Q_PROPERTY(QString menu4 READ menu4 WRITE menuButtonClicked NOTIFY menu4Changed )
    Q_PROPERTY(QString menu5 READ menu5 WRITE menuButtonClicked NOTIFY menu5Changed )
    Q_PROPERTY(QString menu6 READ menu6 WRITE menuButtonClicked NOTIFY menu6Changed )
    Q_PROPERTY(QString menu7 READ menu7 WRITE menuButtonClicked NOTIFY menu7Changed )
    Q_PROPERTY(QString menu8 READ menu8 WRITE menuButtonClicked NOTIFY menu8Changed )

    Q_PROPERTY(QString menu1id READ menu1id )
    Q_PROPERTY(QString menu2id READ menu2id )
    Q_PROPERTY(QString menu3id READ menu3id )
    Q_PROPERTY(QString menu4id READ menu4id )
    Q_PROPERTY(QString menu5id READ menu5id )
    Q_PROPERTY(QString menu6id READ menu6id )
    Q_PROPERTY(QString menu7id READ menu7id )
    Q_PROPERTY(QString menu8id READ menu8id )

    Q_PROPERTY(bool nextButton READ nextButton WRITE menuNextClicked NOTIFY nextButtonChanged)      // wird nicht verwendet ?
    Q_PROPERTY(bool nextEbene READ nextEbene)

    Q_PROPERTY(QStringList szWert READ szWert NOTIFY wertChanged )
    Q_PROPERTY(QStringList szBezeichnung READ szBezeichnung NOTIFY bezeichnungChanged )

    Q_PROPERTY(QStringList szButton READ szButton NOTIFY buttonChanged )
    Q_PROPERTY(QStringList szButtonValue READ szButtonValue NOTIFY buttonChanged )
    Q_PROPERTY(QStringList szSwitch READ szSwitch NOTIFY switchChanged )
    Q_PROPERTY(QStringList szSwitchValue READ szSwitchValue NOTIFY switchChanged )
    Q_PROPERTY(QStringList szSwitchAktuell READ szSwitchAktuell NOTIFY switchChanged )
    Q_PROPERTY(QStringList szCombo READ szCombo NOTIFY comboChanged )
    Q_PROPERTY(QStringList szComboAktuell READ szComboAktuell NOTIFY comboChanged )
    Q_PROPERTY(QStringList szComboValue READ szComboValue NOTIFY comboChanged )
    Q_PROPERTY(QStringList szEdit READ szEdit NOTIFY editChanged )
    Q_PROPERTY(QStringList szEditValue READ szEditValue NOTIFY editChanged )

    Q_PROPERTY(int iGetPreselectedComboId READ iGetPreselectedComboId  NOTIFY comboChanged )


    Q_PROPERTY(bool bInhaltVorhanden READ bInhaltVorhanden)
    Q_PROPERTY(QString getStackUeberschrift READ getStackUeberschrift NOTIFY stackUeberschriftChanged() )
    Q_PROPERTY(bool stackUeberschrift READ stackUeberschrift WRITE stackUeberschriftAdd NOTIFY stackUeberschriftChanged() )
    Q_PROPERTY(QString getStackMenuId READ getStackMenuId NOTIFY stackMenuIdChanged() )

    Q_PROPERTY(int setIndex WRITE setIndex )
    Q_PROPERTY(QString setSwitchValue WRITE setSwitchValue )
    Q_PROPERTY(int setComboValue WRITE setComboValue )
    Q_PROPERTY(QString setEditValue WRITE setEditValue )
    Q_PROPERTY(QString setButtonValue WRITE setButtonValue )

public:
    explicit BackEnd(QObject *parent = nullptr);

    static const int MAXCOMBO = 10;

    QString userName();
    void setUserName(const QString &userName);
    QString socketStatus();
    int getSocketStatusColor();
    void setSendString(const QString &param);
    QString getAntwort();
    QString printerName();
    QString firmware();
    QString getHandshake();
    QString getMenu0();

    QString menu1();
    QString menu2();
    QString menu3();
    QString menu4();
    QString menu5();
    QString menu6();
    QString menu7();
    QString menu8();

    void menuButtonClicked(const QString &param);
    void menuNextClicked(bool bVal);

    QString menu1id();
    QString menu2id();
    QString menu3id();
    QString menu4id();
    QString menu5id();
    QString menu6id();
    QString menu7id();
    QString menu8id();

    bool nextButton();
    int nextEbene();

    QStringList szWert();
    QStringList szBezeichnung();

    QStringList szButton();
    QStringList szButtonValue();
    QStringList szSwitch();
    QStringList szSwitchValue();
    QStringList szSwitchAktuell();
    QStringList szCombo();
    QStringList szComboAktuell();
    QStringList szComboValue();
    QStringList szEdit();
    QStringList szEditValue();

    bool bInhaltVorhanden();
    bool stackUeberschriftAdd(bool bAdd);
    bool stackUeberschrift();
    QString getStackUeberschrift();
    QString getStackMenuId();

    int iGetPreselectedComboId();

    void setIndex(int id);
    void setSwitchValue(const QString &param);
    void setComboValue(int id);
    void setEditValue(const QString &param);
    void setButtonValue(const QString &param);

signals:
    void userNameChanged();
    void statusChanged();
    void menu1Changed();
    void menu2Changed();
    void menu3Changed();
    void menu4Changed();
    void menu5Changed();
    void menu6Changed();
    void menu7Changed();
    void menu8Changed();
    void antwortChanged();
    void nextButtonChanged();
    void ueberschriftChanged();
    void wertChanged();
    void bezeichnungChanged();

    void buttonChanged();
    void buttonValueChanged();
    void switchChanged();
    void switchValueChanged();
    void comboChanged();
    void comboValueChanged();
    void editChanged();
    void editValueChanged();

    void stackUeberschriftChanged();
    void stackMenuIdChanged();


private:
    QString m_userName;
    QString m_status;
    QString m_Antwort;

    QString m_version;
    QString m_printerName;
    QString m_firmware;
    QString m_sessionid;
    QString m_locale;
    QString m_rtpreview;

    QString m_parameter;
    QString m_menu_suchstring;

    QString m_menu_0_1;
    QString m_menu_0_2;
    QString m_menu_0_3;
    QString m_menu_0_4;
    QString m_menu_0_5;
    QString m_menu_0_6;
    QString m_menu_0_7;
    QString m_menu_0_8;

    QString m_menu_0_1_text;
    QString m_menu_0_2_text;
    QString m_menu_0_3_text;
    QString m_menu_0_4_text;
    QString m_menu_0_5_text;
    QString m_menu_0_6_text;
    QString m_menu_0_7_text;
    QString m_menu_0_8_text;

    int m_iAnzahlMenus;
    bool m_bNextSchalter;
    int m_iNextEbene;
    QString m_szUeberschrift;
    QStringList m_stackUeberschrift;        // Eine Liste mit den einzelnen Überschriften im Menübaum
    QStringList m_stackMenuId;              // Ein Stack mit den IDs der aufgerufenen Menüs

    QStringList m_ListeBezeichnung;
    QStringList m_ListeWert;


    QStringList m_ListeButtonBezeichnung;
    QStringList m_ListeButtonId;
    QStringList m_ListeButtonInhalt;
    QStringList m_ListeSwitchBezeichnung;           // Bezeichnung, Name
    QStringList m_ListeSwitchId;                    // Id des Parameters
    QStringList m_ListeSwitchAktuell;               // Aktueller Wert, id (Englisch)
    QStringList m_ListeSwitchInhalt[MAXCOMBO];      // Deutsch
    QStringList m_ListeSwitchIdInhalt[MAXCOMBO];    // Englisch - Id
    QStringList m_ListeComboBezeichnung;            // Bezeichnung, Name
    QStringList m_ListeComboId;                     // Id des Parameters
    QStringList m_ListeComboAktuell;                // Aktueller Wert, id (Englisch)
    QStringList m_ListeComboInhalt[MAXCOMBO];       // Deutsch
    QStringList m_ListeComboIdInhalt[MAXCOMBO];     // Englisch - Id
    QStringList m_ListeComboInhalt1;            // Absturz, wird wohl irgendwo verwendet
    QStringList m_ListeEditBezeichnung;
    QStringList m_ListeEditId;
    QStringList m_ListeEditInhalt;


    QStringList m_ListeMinWert;
    QStringList m_ListeMaxWert;

    int m_ListeIndex;

    int m_AnzahlComboboxen;
    int m_AnzahlSwitche;
    int m_AnsichtComboNr;                   // welche Box anzuzeigen ist
    int m_AnsichtSwitchNr;                   // welche Box anzuzeigen ist

    QString cleanupString(QString str);
    bool SendParamToPrinter(QString id, QString param);
};

#endif // BACKEND_H
