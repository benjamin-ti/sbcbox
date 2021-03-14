/***************************************************************************/
/*                             Druckersoftware                             */
/*                    Copyright (C) Carl Valentin GmbH                     */
/***************************************************************************/

/**
 *\file
 * <!-- ################################################################# -->
 *\brief Framework zur Erstellung und Verwaltung von Kurven mittels
 *       Basispunkten
 * <!-- ################################################################# -->
 *
 * Das Framework erstellt eine Tabelle bei der alle Zwischenpunkte
 * berechnet werden. Die Zwischenpunkte werden jedoch nicht interpoliert.
 * Die Basispunkte werden mit Geraden verbunden auf denen dann die
 * Zwischenpunkte liegen.
 *
 *\section Curve_Example Beschreibung am Beispiel
 *
 * Es soll eine Kurve Berechnet werden mit folgenden St&uuml;tzpunkten:
 *
 *\verbatim
                    Y-Achse
                     ^
                     |
           uiEndY 90 +                        ....*
                     |                   .....
                     |              .....
 puiBasePointX[0] 50 +      ..*.....
                     |   ...
        uiStartY  40 *...
                     |
                     +---------+------------------+--> X-Achse
                     |         |                  |
                     0         7                  16
                     uiStartX  puiBasePointsX[0]  uiEndX
  \endverbatim
 *
 * Die Schrittweite soll +2 betragen. Der Code um die Tabelle zu erstellen
 * sieht dann so aus :
 *
 *\code
    unsigned int GetNextX(unsigned int uiX, unsigned int uiY)
    {
        return uiX+2;
    }

    static CURVEOBJ* psCurveObj = NULL;
    CURVE_BASE_POINTS sCurveIncreaseBasePoints;
    CURVE sCurveIncrease;
    unsigned int ui;

    {
        static unsigned int puiBasePointsX[] = {7};
        static unsigned int puiBasePointsY[] = {50};

        sCurveIncreaseBasePoints.uiStartX = 0;
        sCurveIncreaseBasePoints.uiStartY = 40;
        sCurveIncreaseBasePoints.usNoOfBasePoints = 1;
        sCurveIncreaseBasePoints.puiBasePointsX = puiBasePointsX;
        sCurveIncreaseBasePoints.puiBasePointsY = puiBasePointsY;
        sCurveIncreaseBasePoints.uiEndX = 16;
        sCurveIncreaseBasePoints.uiEndY = 90;

        CurveDecreaseBasePoints.uiStartX = 0;
        CurveDecreaseBasePoints.uiStartY = 40;
        CurveDecreaseBasePoints.usNoOfBasePoints = 1;
        CurveDecreaseBasePoints.puiBasePointsX = puiBasePointsX;
        CurveDecreaseBasePoints.puiBasePointsY = puiBasePointsY;
        CurveDecreaseBasePoints.uiEndX = 16;
        CurveDecreaseBasePoints.uiEndY = 90;

        Curve_Create(&psCurveObj, "Demo", VC_TRUE, &sCurveIncreaseBasePoints,
                     &GetNextX, NULL, 0, NULL, VC_TRUE, NULL);

        sCurveIncrease = Curve_Get(psCurveObj);

        printf("i X  Y\n");
        for (ui=0; ui<sCurveIncrease.uiCurveTableSize; ui++)
        {
            printf("%1u %2u %u\n", ui, ui*2,
                                   sCurveIncrease.puiCurveTable[ui]);
        }

        printf("Switch2 Y=75: %u\n", Curve_GetIndex(psCurveObj, 75));
    }
  \endcode
 *
 * Die Ausgabe w&uuml;rde dann wie folgt aussehen:
 *\verbatim
    i X  Y
    0  0 40
    1  2 42
    2  4 45
    3  6 48
    4  8 54
    5 10 63
    6 12 72
    7 14 81
    8 16 90
    Switch2 Y=75: 7
  \endverbatim
 *
 * Ein weiteres hier demonstriertes Feature sind die GetIndex-Funktionen
 * Ihnen kann man einen Y-Wert &uuml;bergeben
 * und erh&auml;lt als R&uuml;ckgabewert den zugeh&ouml;rigen Index in der
 * Kurventabelle. Es ist jedoch zu beachten, dass der Wert gerundet wird,
 * je nach Tabelle in eine andere Richtung:
 *
 * Handelt es sich um eine aufsteigende Increase-Tabelle wie in obigem
 * Beispiel, dann wird auf einen Index mit einem gr&ouml;sseren Y-Wert
 * gerundet: Das Beispiel 75 exisitert in der Tabelle nicht.
 * Da es sich um eine aufsteigenden Tabelle handelt (StartY ist
 * kleiner als EndY), wird der Index zu einem m&ouml;glichst nahen
 * n&auml;chstgr&ouml;sseren Y-Wert gesucht:\n
 * In diesem Fall w&auml;re das 81 und damit der Index-Wert 7. Aufgrund von
 * internen Rundungsfehlern kann es allerdings auch vorkommen, dass der
 * Index 8 zur&uuml;ckgegeben wird. Dies h&auml;ngt mit dem besonderen
 * GetIndex-Verfahren zusammen (siehe \ref Curve_GetIndexKnowHow). Auf jeden Fall
 * wird immer ein Index zu einem gr&ouml;sseren Wert zur&uuml;ckgegeben.
 *
 * Handelt es sich um eine aufsteigende Tabelle, dann wird auf
 * einen Index mit einem kleineren Y-Wert gerundet: In obigem Beispiel von
 * Y=70 auf Index 5: Y=63.
 *
 * Eine weitere Besonderheit ist die Benutzung von absteigenden Tabellen:
 * Hier ist der StartY-Wert gr&ouml;sser als der EndY-Wert und in in der
 * Tabelle dementsprechend der Y-Wert am Index 0 der gr&ouml;sste.\n
 * Bei diesen Tabellen ist das Rundungsverhalten genau umgekehrt: Hier wird
 * bei der Increase-Tabelle auf einen kleineren Wert gerundet und bei
 * der Decrease-Tabelle auf einen gr&ouml;sseren.\n
 * Das ist z.B. dann geschickt, wenn man mit Geschwindigkeiten und
 * Timerwerten arbeitet: Die Geschwindigkeitstabelle ist dann aufsteigend
 * (z.B. von 50 mm/s bis 300 mm/s), w&auml;hrend die zugeh&ouml;rige
 * Timer-Wert-Tabelle absteigend ist (z.B. von 5000 bis 450).
 *
 *\section Curve_GetIndex KnowHow Funktionsweise der GetIndex-Funktionen
 *
 * Um die GetIndex-Funktionen mit m&ouml;glichst kleiner Laufzeit zu
 * implementieren, werden intern sog. GetIndex-Tabellen angelegt.
 * Die Idee ist, den Y-Wert als Offset f&uuml;r eine solche Tabelle zu
 * verwenden und aus der Tabelle direkt den Offset f&uuml;r die richtige
 * Tabelle zu erhalten. Da die Y-Werte manchmal jedoch sehr gross sein
 * k&ouml;nnen (bei Timer z.B. bis zu 65000), muss der Y-Werte-Bereich
 * auf einen kleineren Bereich zusammengestaucht werden, da sonst sehr
 * grosse Tabellen entstehen w&uuml;rden (bei Y-Max 65000 w&uuml;rde eine
 * Tabelle mit 260 KB entstehen!).\n
 * Deshalb wird die GetIndex-Tabelle nur so gross gemacht wie die
 * zugeh&ouml;rige Kurventabelle. Die Werte werden dann so berechnet,
 * dass die Y-Werte nur geshiftet werden m&uuml;ssen und dann als Index
 * f&uuml;r die GetIndex-Tabelle genommen werden k&ouml;nnen. Das kann
 * allerdings dazu f&uuml;hren, dass bei den Rundungen
 * Ungenauigkeiten auftreten und die Ergebnisse der GetIndex-Funktionen
 * vom Idealwert leicht abweichen k&ouml;nnen.\n
 * Genauer geht es wenn man alle Werte mit einer for-Schleife vergleicht,
 * was allerdings dann wieder Laufzeitm&auml;&szlig;ig ungeschickt ist.
 */

/*. IMPORT ================================================================*/

#include <vcplatform.h>
#include <pjconfig.h>
#include <vcstdlib.h>
#include <vccomponents.h>

#define CV_LOG_GROUP "Components_Curves"
#include <cvlog.h>

#ifdef VALENTIN
#include <comitf.h>
#endif
#include "curve.h"

/*. ENDIMPORT =============================================================*/


/*. EXPORT ================================================================*/

/*. ENDEXPORT =============================================================*/


/*. LOCAL =================================================================*/

/*-------------------------------- Makros ---------------------------------*/

#define CURVEOBJ_BUF_SIZE 64

/*--------------------------- Typdeklarationen ----------------------------*/

/*------------------------------ Prototypen -------------------------------*/

static void Curve_CalcIndexTable(CURVEOBJ* psCurveObj,
                                 unsigned short* pusCurveGetIndexVal,   // Rueckgabewert
                                 unsigned int** ppuiCurveGetIndexTable, // Rueckgabewert
                                 unsigned int uiEndYTabelVal,
                                 unsigned int uiStartYTabelVal, VC_BOOL bAutoSwitch2UpdatedCurve,
                                 unsigned int* puiCurveTable, unsigned int uiCurveTableSize);
static void Curve_Calc(CURVEOBJ* psCurveObj, VC_BOOL bAutoSwitch2UpdatedCurve);

/*------------------------ Konstantendeklarationen ------------------------*/

/*------------------------- modulglobale Variable -------------------------*/

static SLLIST* lk_pSLList = NULL;
static CURVE   lk_sCurveNull = (CURVE){NULL, 0, 0};
static CURVE_BASE_POINTS lk_sCurveBasePointsNull
           = (CURVE_BASE_POINTS){0, 0, 0, NULL, NULL, 0, 0};

/*. ENDLOCAL ==============================================================*/


/*=========================================================================*/
CURVE Curve_Get(CURVEOBJ* psCurveObj)
{
    CURVE sCurve;

    if ((psCurveObj == NULL) || (psCurveObj->ui32ID != CURVE_ID))
    {
        return lk_sCurveNull;
    }
    if (psCurveObj->bError)
    {
        return lk_sCurveNull;
    }

    sCurve.puiCurveTable = psCurveObj->puiCurveTable;
    sCurve.uiCurveTableSize = psCurveObj->uiCurveTableSize;
    sCurve.uiStartX = psCurveObj->uiStartX;
    sCurve.uiStartY = psCurveObj->uiStartY;

    return sCurve;
}


/*=========================================================================*/
CURVE Curve_GetAndLock(CURVEOBJ* psCurveObj)
{
    psCurveObj->uiLockCounter++;
    return Curve_Get(psCurveObj);
}


/*=========================================================================*/
void Curve_Unlock(CURVEOBJ* psCurveObj)
{
    if (psCurveObj->uiLockCounter == 0)
        return;

    psCurveObj->uiLockCounter--;
    if (psCurveObj->uiLockCounter == 0)
    {
        if (psCurveObj->InformOnUpdate != NULL)
        {
            psCurveObj->InformOnUpdate(psCurveObj, VC_FALSE,
                    psCurveObj->argc, psCurveObj->argv);
        }
    }
}


/*=========================================================================*/
CURVE_BASE_POINTS Curve_GetBasePoints(CURVEOBJ* psCurveObj)
{
    if ((psCurveObj == NULL) || (psCurveObj->ui32ID != CURVE_ID))
    {
        return lk_sCurveBasePointsNull;
    }
    if (psCurveObj->bError || psCurveObj->bSimpleNoBasepoints)
    {
        return lk_sCurveBasePointsNull;
    }

    return psCurveObj->sCurveBasePointsOrg;
}


/*=========================================================================*/
/**
 * (siehe \ref Curve_Example)
 *\return Gerundeter zu uiY-gehoeriger Index der Increase-Tabelle
 */
unsigned int Curve_GetIndex(CURVEOBJ* psCurveObj, unsigned int uiY)
{
    unsigned int uiTemp;

    if ((psCurveObj == NULL) || (psCurveObj->ui32ID != CURVE_ID))
    {
        return 0;
    }
    if (psCurveObj->bError)
    {
        return 0;
    }

    uiTemp = uiY >> psCurveObj->usCurveGetIndexVal;
    if (uiTemp > psCurveObj->uiCurveTableSize-1)
    {
        uiTemp = psCurveObj->uiCurveTableSize-1;
    }
    return psCurveObj->puiCurveGetIndexTable[uiTemp];
}


/*=========================================================================*/
unsigned int Curve_GetIndexHiPrec(CURVEOBJ* psCurveObj, unsigned int uiY)
{
    unsigned int ui;

    if ((psCurveObj == NULL) || (psCurveObj->ui32ID != CURVE_ID))
    {
        return 0;
    }
    if (psCurveObj->bError)
    {
        return 0;
    }

    for (ui=0; ui<psCurveObj->uiCurveTableSize; ui++)
    {
        if (psCurveObj->bEndYAsMaxVal)
        {
            if (psCurveObj->bIncrease)
            {
                if (uiY <= psCurveObj->puiCurveTable[ui])
                {
                    break;
                }
            }
            else
            {
                if (ui+1 < psCurveObj->uiCurveTableSize)
                {
                    if (uiY < psCurveObj->puiCurveTable[ui+1])
                    {
                        break;
                    }
                }
            }
        }
        else
        {
            if (psCurveObj->bIncrease)
            {
                if (uiY >= psCurveObj->puiCurveTable[ui])
                {
                    break;
                }
            }
            else
            {
                if (ui+1 < psCurveObj->uiCurveTableSize)
                {
                    if (uiY > psCurveObj->puiCurveTable[ui+1])
                    {
                        break;
                    }
                }
            }
        }
    }

    if (ui >= psCurveObj->uiCurveTableSize)
    {
        ui = psCurveObj->uiCurveTableSize-1;
    }

    return ui;
}


/*=========================================================================*/
static unsigned int Curve_GetNextXDflt(unsigned int uiX, unsigned int uiY,
                                       int argc, const char* argv[])
{
    return uiX+1;
}


/*=========================================================================*/
static unsigned int Curve_ChangeIdealY2TabelValDflt(unsigned int uiIdealY,
                        int argc, const char* argv[])
{
    return uiIdealY;
}


/*=========================================================================*/
static void Curve_CorrectBasePoints(CURVE_BASE_POINTS* psCurveBasePoints)
{
    if (    (psCurveBasePoints->uiStartX == 0)
         && (psCurveBasePoints->uiStartY == 0)
         && (psCurveBasePoints->uiEndX == -1)
         && (psCurveBasePoints->uiEndY == -1) )
    {
        psCurveBasePoints->uiStartX
            = psCurveBasePoints->puiBasePointsX[0];
        psCurveBasePoints->uiStartY
            = psCurveBasePoints->puiBasePointsY[0];
        psCurveBasePoints->uiEndX
            = psCurveBasePoints->puiBasePointsX
                  [psCurveBasePoints->usNoOfBasePoints-1];
        psCurveBasePoints->uiEndY
            = psCurveBasePoints->puiBasePointsY
                  [psCurveBasePoints->usNoOfBasePoints-1];

        psCurveBasePoints->puiBasePointsX++;
        psCurveBasePoints->puiBasePointsY++;
        psCurveBasePoints->usNoOfBasePoints-=2;
    }
}


/*=========================================================================*/
/*
 * (siehe \ref Curve_Example)
 */
VC_STATUS Curve_Create(CURVEOBJ* psCurveObj, char* pcName, VC_BOOL bIncrease,
                       CURVE_BASE_POINTS* psCurveBasePoints,
                       unsigned int (*GetNextX)(unsigned int uiX,
                                                unsigned int uiY,
                                                int argc, const char* argv[]),
                       unsigned int (*ChangeIdealY2TabelVal)(unsigned int
                                                             uiIdealY,
                                                             int argc,
                                                             const char* argv[]),
                       int argc, const char* argv[],
                       VC_BOOL bAutoSwitch2UpdatedCurve,
                       void (*InformOnUpdate)(CURVEOBJ* psCurveObj,
                               VC_BOOL bSwitchLocked, int argc, const char* argv[]))
{
    //. Pruefe ob Uebergabeparameter korrekt
    if ((psCurveObj == NULL) || (psCurveObj->ui32ID == CURVE_ID))
    {
        CV_HANDLE_STATUS(VC_ERROR);
        return VC_ERROR;
    }
    if (psCurveBasePoints == NULL)
    {
        CV_HANDLE_STATUS(VC_ERROR);
        return VC_ERROR;
    }

    //. Init Objekt
    memset(psCurveObj, 0, sizeof(CURVEOBJ));

    psCurveObj->pcName = pcName;
    psCurveObj->bIncrease = bIncrease;

    psCurveObj->sCurveBasePoints = *psCurveBasePoints;
    psCurveObj->sCurveBasePointsOrg = *psCurveBasePoints;

    psCurveObj->bAutoSwitch2UpdatedCurve = bAutoSwitch2UpdatedCurve;
    psCurveObj->InformOnUpdate = InformOnUpdate;

    Curve_CorrectBasePoints(&psCurveObj->sCurveBasePoints);

    if (GetNextX != NULL)
    {
        psCurveObj->GetNextX = GetNextX;
    }
    else
    {
        psCurveObj->GetNextX = &Curve_GetNextXDflt;
    }
    if (ChangeIdealY2TabelVal != NULL)
    {
        psCurveObj->ChangeIdealY2TabelVal = ChangeIdealY2TabelVal;
    }
    else
    {
        psCurveObj->ChangeIdealY2TabelVal = &Curve_ChangeIdealY2TabelValDflt;
    }
    psCurveObj->argc = argc;
    psCurveObj->argv = argv;

    Curve_Calc(psCurveObj, VC_TRUE);

    _ACONST_U32_(psCurveObj->ui32ID) = CURVE_ID;

    if (psCurveObj->InformOnUpdate != NULL)
    {
        psCurveObj->InformOnUpdate(psCurveObj, VC_FALSE,
                psCurveObj->argc, psCurveObj->argv);
    }

    if (psCurveObj->bError)
        return VC_ERROR;

    return VC_SUCCESS;
}

/*=========================================================================*/
VC_STATUS Curve_CreateSimple(CURVEOBJ* psCurveObj, char* pcName, VC_BOOL bIncrease,
                             CURVE sCurve, VC_BOOL bAutoSwitch2UpdatedCurve,
                             void (*InformOnUpdate)(CURVEOBJ* psCurveObj,
                                     VC_BOOL bSwitchLocked, int argc, const char* argv[]),
                             int argc, const char* argv[])
{
    unsigned short usCurveGetIndexVal;
    unsigned int* puiCurveGetIndexTable;

    //. Pruefe ob Uebergabeparameter korrekt
    if ((psCurveObj == NULL) || (psCurveObj->ui32ID == CURVE_ID))
    {
        CV_HANDLE_STATUS(VC_ERROR);
        return VC_ERROR;
    }

    if (sCurve.puiCurveTable == NULL)
        return VC_ERROR;

    //. Init Objekt
    memset(psCurveObj, 0, sizeof(CURVEOBJ));

    psCurveObj->pcName = pcName;
    psCurveObj->bIncrease = bIncrease;

    psCurveObj->bSimpleNoBasepoints = VC_TRUE;
    psCurveObj->bAutoSwitch2UpdatedCurve = bAutoSwitch2UpdatedCurve;

    psCurveObj->puiCurveTable = (unsigned int*)sCurve.puiCurveTable;
    psCurveObj->uiCurveTableSize = sCurve.uiCurveTableSize;
    psCurveObj->uiStartX = sCurve.uiStartX;
    psCurveObj->uiStartY = sCurve.uiStartY;

    Curve_CalcIndexTable(psCurveObj, &usCurveGetIndexVal,
                         &puiCurveGetIndexTable,
                         psCurveObj->puiCurveTable[psCurveObj->uiCurveTableSize-1],
                         psCurveObj->puiCurveTable[0],
                         VC_TRUE,
                         psCurveObj->puiCurveTable, psCurveObj->uiCurveTableSize);

    if (psCurveObj->InformOnUpdate != NULL)
    {
        psCurveObj->InformOnUpdate(psCurveObj, VC_FALSE,
                psCurveObj->argc, psCurveObj->argv);
    }
    psCurveObj->argc = argc;
    psCurveObj->argv = argv;

    _ACONST_U32_(psCurveObj->ui32ID) = CURVE_ID;

    return VC_SUCCESS;
}

/*=========================================================================*/
/*
 * (siehe \ref Curve_Example)
 */
VC_STATUS Curve_Delete(CURVEOBJ* psCurveObj)
{
    unsigned int ui;

    if ((psCurveObj == NULL) || (psCurveObj->ui32ID != CURVE_ID))
    {
        return VC_ERROR;
    }

    Curve_FreeUnusedMem(psCurveObj);

    if (!psCurveObj->bSimpleNoBasepoints)
    {
        if (psCurveObj->puiCurveTableNew != NULL)
        {
            CV_HANDLE_STATUS_A_F( MemFree(psCurveObj->puiCurveTable) );
            psCurveObj->puiCurveTable = psCurveObj->puiCurveTableNew;
            psCurveObj->uiCurveTableSize = psCurveObj->uiCurveTableSizeNew;
            psCurveObj->puiCurveTableNew = NULL;
        }

        if (psCurveObj->puiCurveGetIndexTableNew != NULL)
        {
            CV_HANDLE_STATUS_A_F( MemFree(psCurveObj->puiCurveGetIndexTable) );
            psCurveObj->usCurveGetIndexVal = psCurveObj->usCurveGetIndexValNew;
            psCurveObj->puiCurveGetIndexTable = psCurveObj->puiCurveGetIndexTableNew;
            psCurveObj->puiCurveGetIndexTableNew = NULL;
        }

        CV_HANDLE_STATUS_A_F( MemFree(psCurveObj->puiCurveGetIndexTable) );
        CV_HANDLE_STATUS_A_F( MemFree(psCurveObj->puiCurveTable) );
    }
    else
    {
        CV_HANDLE_STATUS_A_F( MemFree(psCurveObj->puiCurveGetIndexTable) );
    }

    for (ui=0; ui<CURVE_MAX_EXT_CONTENT; ui++)
    {
        if (psCurveObj->psExtContent[ui].bValid
            && psCurveObj->psExtContent[ui].bFreeMemOnCurveDelete)
        {
            FreeMemory(psCurveObj->psExtContent[ui].pvPointer);
        }
    }

    _ACONST_U32_(psCurveObj->ui32ID) = 0;

    return VC_SUCCESS;
}


/*=========================================================================*/
/**
 * Evtl. anstehende Switches sollten jetzt gemacht werden. Geht natuerlich
 * nur wenn keine Locks anstehen.
 */
VC_STATUS Curve_SetAutoSwitch2UpdatedCurve(CURVEOBJ* psCurveObj, VC_BOOL bSet)
{
    VC_STATUS sStatus;

    if (psCurveObj->bSimpleNoBasepoints)
    {
        if (bSet)
            return VC_SUCCESS;
        else
            return VC_ERROR;
    }

    sStatus = Curve_Switch2UpdatedCurve(psCurveObj, VC_FALSE);
    if (sStatus != VC_ERROR)
    {
        psCurveObj->bAutoSwitch2UpdatedCurve = bSet;
        sStatus = VC_SUCCESS;
    }
    return sStatus;
}


/*=========================================================================*/
void Curve_SetInformOnUpdate(CURVEOBJ* psCurveObj,
                             void (*InformOnUpdate)(CURVEOBJ* psCurveObj,
                                     VC_BOOL bSwitchLocked,
                                     int argc, const char* argv[]))
{
    psCurveObj->InformOnUpdate = InformOnUpdate;
}


/*=========================================================================*/
/**
 * \param psCurveBasePoints:
 *      Wenn sich nur der Inhalt der Punkte aendert, dann uebergibt man
 *      NULL. Aendert sich aber z.B. die Anzahl der Punkte dann muss man
 *       sCurveBasePoints uebergeben.
 */
VC_STATUS Curve_Update(CURVEOBJ* psCurveObj,
                       CURVE_BASE_POINTS* psCurveBasePoints)
{
    if ((psCurveObj == NULL) || (psCurveObj->ui32ID != CURVE_ID))
    {
        return VC_ERROR;
    }

    if (psCurveObj->bSimpleNoBasepoints)
        return VC_ERROR;

    if (!psCurveObj->bAutoSwitch2UpdatedCurve)
        psCurveObj->uiLockCounter++;

    Curve_FreeUnusedMem(psCurveObj);

    if (psCurveBasePoints != NULL)
    {
        psCurveObj->sCurveBasePoints = *psCurveBasePoints;
        psCurveObj->sCurveBasePointsOrg = *psCurveBasePoints;
    }
    else
    {
        psCurveObj->sCurveBasePoints = psCurveObj->sCurveBasePointsOrg;
    }
    Curve_CorrectBasePoints(&psCurveObj->sCurveBasePoints);

    if (psCurveObj->uiLockCounter == 0)
    {
        Curve_Calc(psCurveObj, psCurveObj->bAutoSwitch2UpdatedCurve);
    }
    else
    {
        Curve_Calc(psCurveObj, VC_FALSE);
    }

    if (!psCurveObj->bAutoSwitch2UpdatedCurve)
        psCurveObj->uiLockCounter--;

    if (psCurveObj->InformOnUpdate != NULL)
    {
        if (psCurveObj->uiLockCounter > 0)
        {
            psCurveObj->InformOnUpdate(psCurveObj, VC_TRUE, psCurveObj->argc, psCurveObj->argv);
        }
        else
        {
            psCurveObj->InformOnUpdate(psCurveObj, VC_FALSE, psCurveObj->argc, psCurveObj->argv);
        }
    }

    if (psCurveObj->bError)
        return VC_ERROR;

    return VC_SUCCESS;
}

/*=========================================================================*/
VC_STATUS Curve_UpdateSimple(CURVEOBJ* psCurveObj, CURVE sCurve)
{
    if ((psCurveObj == NULL) || (psCurveObj->ui32ID != CURVE_ID))
    {
        return VC_ERROR;
    }

    if (!psCurveObj->bSimpleNoBasepoints)
        return VC_ERROR;

    Curve_FreeUnusedMem(psCurveObj);

    psCurveObj->puiCurveTable = (unsigned int*)sCurve.puiCurveTable;
    psCurveObj->uiCurveTableSize = sCurve.uiCurveTableSize;
    psCurveObj->uiStartX = sCurve.uiStartX;
    psCurveObj->uiStartY = sCurve.uiStartY;

    if (psCurveObj->uiLockCounter == 0)
    {
        Curve_Calc(psCurveObj, psCurveObj->bAutoSwitch2UpdatedCurve);
    }
    else
    {
        Curve_Calc(psCurveObj, VC_FALSE);
    }

    if (psCurveObj->InformOnUpdate != NULL)
    {
        if (psCurveObj->uiLockCounter > 0)
        {
            psCurveObj->InformOnUpdate(psCurveObj, VC_TRUE,
                    psCurveObj->argc, psCurveObj->argv);
        }
        else
        {
            psCurveObj->InformOnUpdate(psCurveObj, VC_FALSE,
                    psCurveObj->argc, psCurveObj->argv);
        }
    }

    return VC_SUCCESS;
}

/*=========================================================================*/
void Curve_InformUpdate(CURVEOBJ* psCurveObj)
{
    if (psCurveObj->InformOnUpdate != NULL)
    {
        if (psCurveObj->uiLockCounter > 0)
        {
            psCurveObj->InformOnUpdate(psCurveObj, VC_TRUE,
                    psCurveObj->argc, psCurveObj->argv);
        }
        else
        {
            psCurveObj->InformOnUpdate(psCurveObj, VC_FALSE,
                    psCurveObj->argc, psCurveObj->argv);
        }
    }
}

/*=========================================================================*/
void Curve_FreeUnusedMem(CURVEOBJ* psCurveObj)
{
    if (psCurveObj->puiCurveTableFreeMem != NULL)
    {
        CV_HANDLE_STATUS_A_F( MemFree(psCurveObj->puiCurveTableFreeMem) );
        psCurveObj->puiCurveTableFreeMem = NULL;
    }

    if (psCurveObj->puiCurveGetIndexTableFreeMem != NULL)
    {
        CV_HANDLE_STATUS_A_F( MemFree(psCurveObj->puiCurveGetIndexTableFreeMem) );
        psCurveObj->puiCurveGetIndexTableFreeMem = NULL;
    }
}

/*=========================================================================*/
VC_STATUS Curve_Switch2UpdatedCurve(CURVEOBJ* psCurveObj,
                                    VC_BOOL bFreeMemOnNextUpd)
{
    VC_STATUS sStatus = VC_NOT_FOUND;

    if (psCurveObj->bSimpleNoBasepoints)
        return VC_ERROR;

    if (psCurveObj->uiLockCounter > 0)
    {
        return VC_ERROR;
    }

    if (psCurveObj->puiCurveTableNew != NULL)
    {
        if (bFreeMemOnNextUpd)
        {
            if (psCurveObj->puiCurveTableFreeMem != NULL)
            {
                CV_LOG(("Err\n"));
            }
            psCurveObj->puiCurveTableFreeMem = psCurveObj->puiCurveTable;
        }
        else
        {
            CV_HANDLE_STATUS_A_F( MemFree(psCurveObj->puiCurveTable) );
        }

        psCurveObj->puiCurveTable = psCurveObj->puiCurveTableNew;
        psCurveObj->uiCurveTableSize = psCurveObj->uiCurveTableSizeNew;
        psCurveObj->puiCurveTableNew = NULL;
        sStatus = VC_SUCCESS;
    }

    if (psCurveObj->puiCurveGetIndexTableNew != NULL)
    {
        if (bFreeMemOnNextUpd)
        {
            if (psCurveObj->puiCurveGetIndexTableFreeMem != NULL)
            {
                CV_LOG(("Err\n"));
            }
            psCurveObj->puiCurveGetIndexTableFreeMem
                = psCurveObj->puiCurveGetIndexTable;
        }
        else
        {
            CV_HANDLE_STATUS_A_F( MemFree(psCurveObj->puiCurveGetIndexTable) );
        }

        psCurveObj->usCurveGetIndexVal = psCurveObj->usCurveGetIndexValNew;
        psCurveObj->puiCurveGetIndexTable = psCurveObj->puiCurveGetIndexTableNew;
        psCurveObj->puiCurveGetIndexTableNew = NULL;
        sStatus = VC_SUCCESS;
    }

    psCurveObj->uiStartX = psCurveObj->uiStartXNew;
    psCurveObj->uiStartY = psCurveObj->uiStartYNew;

    return sStatus;
}


/*=========================================================================*/
static unsigned int Curve_CalcIdeal(CURVE_BASE_POINTS* psCurveBasePoints,
                                    unsigned int uix)
{
    unsigned short us;
    unsigned int uiStartPartX = psCurveBasePoints->uiStartX;
    unsigned int uiStartPartY = psCurveBasePoints->uiStartY;
    unsigned int uiEndPartX = psCurveBasePoints->uiEndX;
    unsigned int uiEndPartY = psCurveBasePoints->uiEndY;
    float fA;
    float fB;

    // Bestimme Abschnitt
    for (us=0; us<psCurveBasePoints->usNoOfBasePoints; us++)
    {
        if (uix < psCurveBasePoints->puiBasePointsX[us])
        {
            uiEndPartY = psCurveBasePoints->puiBasePointsY[us];
            uiEndPartX =  psCurveBasePoints->puiBasePointsX[us] ;
            break;
        }
        else
        {
            uiStartPartY = psCurveBasePoints->puiBasePointsY[us];
            uiStartPartX =  psCurveBasePoints->puiBasePointsX[us] ;
        }
    }

    // y = ax+b; a->fA, b->fB
    fA = ((float)uiEndPartY - (float)uiStartPartY) /
         ((float)uiEndPartX  - (float)uiStartPartX);
    fB = (float)uiStartPartY - (fA * (float)uiStartPartX);

    return ( fA * (float)uix + fB );
}


/*=========================================================================*/
static void Curve_CalcTable(CURVEOBJ* psCurveObj, CURVE_BASE_POINTS* psCurveBasePoints,
                       unsigned int* puiCurveTableSize,
                       unsigned int** ppuiCurveTable,
                       VC_BOOL bFreeMemory)
{
    unsigned int uiX;
    unsigned int uiXOld;
    unsigned int uiY;
    unsigned int uiNoOfCurvePoints;

    // Zunaechst einmal ein Durchlauf zur Groessenbestimmung
    uiNoOfCurvePoints = 0;
    for (uiX=psCurveBasePoints->uiStartX;
         uiX<=psCurveBasePoints->uiEndX; uiNoOfCurvePoints++)
    {
        uiY = psCurveObj->ChangeIdealY2TabelVal(
                Curve_CalcIdeal(psCurveBasePoints, uiX), psCurveObj->argc,
                                psCurveObj->argv);
        uiXOld = uiX;
        uiX = psCurveObj->GetNextX(uiXOld, uiY, psCurveObj->argc,
                                   psCurveObj->argv);
        if (uiX == uiXOld)
        {
            psCurveObj->bError = VC_TRUE;
            CV_LOG(("Err\n"));
            return;
        }
    }

    *puiCurveTableSize = uiNoOfCurvePoints;

    // Dann Speicher allokieren
    if (bFreeMemory)
    {
        FreeMemory(*ppuiCurveTable);
    }
    *ppuiCurveTable = AllocMemory((*puiCurveTableSize+1) * sizeof(unsigned int));
    if (*ppuiCurveTable == NULL)
    {
        psCurveObj->bError = VC_TRUE;
        CV_HANDLE_STATUS_A(VC_ALLOCMEM);
        return;
    }

    // Dann ein Durchlauf um Werte in Tabelle einzutragen
    uiNoOfCurvePoints = 0;
    for (uiX=psCurveBasePoints->uiStartX;
         uiX<=psCurveBasePoints->uiEndX; uiNoOfCurvePoints++)
    {
        uiY = psCurveObj->ChangeIdealY2TabelVal(
                  Curve_CalcIdeal(psCurveBasePoints, uiX), psCurveObj->argc,
                                  psCurveObj->argv);
        (*ppuiCurveTable)[uiNoOfCurvePoints] = uiY;
        uiX = psCurveObj->GetNextX(uiX, uiY, psCurveObj->argc, psCurveObj->argv);
    }
}


/*=========================================================================*/
static void Curve_CalcGetIndexIncreaseTableWithStartYAsMaxVal(
                unsigned int uiCurveTableSize, unsigned short usGetIndexVal,
                unsigned int* puiCurveTable,  unsigned int* puiGetIndexTable)
{
    unsigned int  ui;
    unsigned int  uiIndex;
    unsigned long ulValue;

    uiIndex = 0;
    for (ui=uiCurveTableSize-1; ui>0; ui--)
    {
        ulValue = ui << usGetIndexVal;
        while (puiCurveTable[uiIndex] > ulValue)
        {
            if (uiIndex == uiCurveTableSize-1) break;
            uiIndex++;
        }
        puiGetIndexTable[ui] = uiIndex;
    }
    puiGetIndexTable[0] = uiCurveTableSize-1;
}


/*=========================================================================*/
static void Curve_CalcGetIndexIncreaseTableWithEndYAsMaxVal(
                unsigned int uiCurveTableSize, unsigned short usGetIndexVal,
                unsigned int* puiCurveTable,  unsigned int* puiGetIndexTable)
{
    unsigned int  ui;
    unsigned int  uiIndex;
    unsigned long ulValue;

    uiIndex = 0;
    for (ui=0; ui<uiCurveTableSize; ui++)
    {
        // +1 weil beim Shiften wird in den Switch2-Funktionen ein Fehler
        // in Richtung kleiner gemacht.
        ulValue = (ui+1) << usGetIndexVal;
        while (puiCurveTable[uiIndex] < ulValue)
        {
            if (uiIndex == uiCurveTableSize-1) break;
            uiIndex++;
        }
        puiGetIndexTable[ui] = uiIndex;
    }
}


/*=========================================================================*/
static void Curve_CalcGetIndexDecreaseTableWithStartYAsMaxVal(
                unsigned int uiCurveTableSize, unsigned short usGetIndexVal,
                unsigned int* puiCurveTable, unsigned int* puiGetIndexTable)
{
    unsigned int  ui;
    unsigned int  uiIndex;
    unsigned long ulValue;

    uiIndex = uiCurveTableSize-1;
    for (ui=0; ui<uiCurveTableSize; ui++)
    {
        // +1 weil beim Shiften wird in den Switch2-Funktionen ein Fehler
        // in Richtung kleiner gemacht.
        ulValue = (ui+1) << usGetIndexVal;
        while (puiCurveTable[uiIndex] < ulValue)
        {
            if (uiIndex == 0) break;
            uiIndex--;
        }
        puiGetIndexTable[ui] = uiIndex;
    }
}


/*=========================================================================*/
static void Curve_CalcGetIndexDecreaseTableWithEndYAsMaxVal(
                unsigned int uiCurveTableSize, unsigned short usGetIndexVal,
                unsigned int* puiCurveTable, unsigned int* puiGetIndexTable)
{
    unsigned int  ui;
    unsigned int  uiIndex;
    unsigned long ulValue;

    uiIndex = uiCurveTableSize-1;
    for (ui=uiCurveTableSize-1; ui>0; ui--)
    {
        ulValue = ui << usGetIndexVal;
        while (puiCurveTable[uiIndex] > ulValue)
        {
            if (uiIndex == 0) break;
            uiIndex--;
        }
        puiGetIndexTable[ui] = uiIndex;
    }
    puiGetIndexTable[0] = 0;
}


/*=========================================================================*/
static VC_BOOL Curve_CalcGetIndexTableWithStartYAsMaxVal(
                void (*Calc2Table)(unsigned int   uiCurveTableSize,
                                   unsigned short usGetIndexVal,
                                   unsigned int*  puiCurveTable,
                                   unsigned int*  puiGetIndexTable),
                unsigned int uiCurveTableSize, unsigned short* pusGetIndexVal,
                unsigned int* puiCurveTable, unsigned int** ppuiGetIndexTable,
                VC_BOOL bFreeMemory)
{
    if (bFreeMemory)
    {
        FreeMemory(*ppuiGetIndexTable);
    }
    *ppuiGetIndexTable = AllocMemory(uiCurveTableSize * sizeof(unsigned int));
    if (*ppuiGetIndexTable == NULL)
    {
        CV_HANDLE_STATUS_A(VC_ALLOCMEM);
        return VC_FALSE;
    }

    *pusGetIndexVal =
        log(puiCurveTable[0]/uiCurveTableSize) /
        log(2) + 0.5;
    do
    {
        Calc2Table(uiCurveTableSize, *pusGetIndexVal, puiCurveTable,
                   *ppuiGetIndexTable);

        if ((*ppuiGetIndexTable)[uiCurveTableSize-1] > 0)
        {
            (*pusGetIndexVal)++;
            if (*pusGetIndexVal == (sizeof(unsigned int)*8)/2)
            {
                (*pusGetIndexVal)--;
                break;
            }
        }
    }
    while ( ((*ppuiGetIndexTable)[uiCurveTableSize-1] > 0) );

    return VC_TRUE;
}


/*=========================================================================*/
static VC_BOOL Curve_CalcGetIndexWithEndYAsMaxVal(
                void (*Calc2Table)(unsigned int   uiCurveTableSize,
                                   unsigned short usGetIndexVal,
                                   unsigned int*  puiCurveTable,
                                   unsigned int*  puiGetIndexTable),
                unsigned int uiCurveTableSize, unsigned short* pusGetIndexVal,
                unsigned int* puiCurveTable, unsigned int** ppuiGetIndexTable,
                VC_BOOL bFreeMemory)
{
    if (bFreeMemory)
    {
        FreeMemory(*ppuiGetIndexTable);
    }
    *ppuiGetIndexTable = AllocMemory(uiCurveTableSize * sizeof(unsigned int));
    if (*ppuiGetIndexTable == NULL)
    {
        CV_HANDLE_STATUS_A(VC_ALLOCMEM);
        return VC_FALSE;
    }
    memset(*ppuiGetIndexTable, 0, uiCurveTableSize * sizeof(unsigned int));

    *pusGetIndexVal =
        log(puiCurveTable[uiCurveTableSize-1]/uiCurveTableSize) /
        log(2) + 0.5;
    do
    {
        Calc2Table(uiCurveTableSize, *pusGetIndexVal, puiCurveTable,
                   *ppuiGetIndexTable);

        if ((*ppuiGetIndexTable)[uiCurveTableSize-1] < uiCurveTableSize-1)
        {
            (*pusGetIndexVal)++;
            if (*pusGetIndexVal == (sizeof(unsigned int)*8)/2)
            {
                (*pusGetIndexVal)--;
                break;
            }
        }
    }
    while ( ((*ppuiGetIndexTable)[uiCurveTableSize-1] < uiCurveTableSize-1) );

    return VC_TRUE;
}


/*=========================================================================*/
static void Curve_CalcIndexTable(CURVEOBJ* psCurveObj,
                                 unsigned short* pusCurveGetIndexVal,   // Rueckgabewert
                                 unsigned int** ppuiCurveGetIndexTable, // Rueckgabewert
                                 unsigned int uiEndYTabelVal,
                                 unsigned int uiStartYTabelVal, VC_BOOL bAutoSwitch2UpdatedCurve,
                                 unsigned int* puiCurveTable, unsigned int uiCurveTableSize)
{
    void (*Curve_CalcGetIndexTableHelper)(
            unsigned int uiCurveTableSize, unsigned short usGetIndexVal,
            unsigned int* puiCurveTable,  unsigned int* puiGetIndexTable);

    VC_BOOL bSuccess;

    if (uiEndYTabelVal > uiStartYTabelVal)
    {
        psCurveObj->bEndYAsMaxVal = VC_TRUE;
        if (psCurveObj->bIncrease)
        {
            Curve_CalcGetIndexTableHelper
                = Curve_CalcGetIndexIncreaseTableWithEndYAsMaxVal;
        }
        else
        {
            Curve_CalcGetIndexTableHelper
                = Curve_CalcGetIndexDecreaseTableWithEndYAsMaxVal;
        }
        if (bAutoSwitch2UpdatedCurve)
        {
            bSuccess = Curve_CalcGetIndexWithEndYAsMaxVal(
                              Curve_CalcGetIndexTableHelper,
                              uiCurveTableSize,
                              &(psCurveObj->usCurveGetIndexVal),
                              puiCurveTable,
                              &(psCurveObj->puiCurveGetIndexTable),
                              VC_TRUE);
            *pusCurveGetIndexVal = psCurveObj->usCurveGetIndexVal;
            *ppuiCurveGetIndexTable = psCurveObj->puiCurveGetIndexTable;
        }
        else
        {
            bSuccess = Curve_CalcGetIndexWithEndYAsMaxVal(
                              Curve_CalcGetIndexTableHelper,
                              uiCurveTableSize,
                              pusCurveGetIndexVal,
                              puiCurveTable,
                              ppuiCurveGetIndexTable,
                              VC_FALSE);
        }
        if (!bSuccess)
        {
            psCurveObj->bError = VC_TRUE;
            CV_LOG(("Err\n"));
            return;
        }
    }
    else
    {
        psCurveObj->bEndYAsMaxVal = VC_FALSE;
        if (psCurveObj->bIncrease)
        {
            Curve_CalcGetIndexTableHelper
                = Curve_CalcGetIndexIncreaseTableWithStartYAsMaxVal;
        }
        else
        {
            Curve_CalcGetIndexTableHelper
                = Curve_CalcGetIndexDecreaseTableWithStartYAsMaxVal;
        }
        if (bAutoSwitch2UpdatedCurve)
        {
            bSuccess = Curve_CalcGetIndexTableWithStartYAsMaxVal(
                              Curve_CalcGetIndexTableHelper,
                              uiCurveTableSize,
                              &(psCurveObj->usCurveGetIndexVal),
                              puiCurveTable,
                              &(psCurveObj->puiCurveGetIndexTable),
                              VC_TRUE);
        }
        else
        {
            bSuccess = Curve_CalcGetIndexTableWithStartYAsMaxVal(
                              Curve_CalcGetIndexTableHelper,
                              uiCurveTableSize,
                              pusCurveGetIndexVal,
                              puiCurveTable,
                              ppuiCurveGetIndexTable,
                              VC_FALSE);
        }
        if (!bSuccess)
        {
            psCurveObj->bError = VC_TRUE;
            CV_LOG(("Err\n"));
            return;
        }
    }
}


/*=========================================================================*/
static void Curve_Calc(CURVEOBJ* psCurveObj, VC_BOOL bAutoSwitch2UpdatedCurve)
{
    unsigned int uiEndYTabelVal;
    unsigned int uiStartYTabelVal;

    unsigned int uiCurveTableSize;
    unsigned int* puiCurveTable = NULL;

    unsigned short usCurveGetIndexVal;
    unsigned int* puiCurveGetIndexTable = NULL;

    if (psCurveObj == NULL)
        return;

    if (!psCurveObj->bSimpleNoBasepoints)
    {
        if (bAutoSwitch2UpdatedCurve)
        {
            Curve_CalcTable(psCurveObj, &psCurveObj->sCurveBasePoints,
                            &(psCurveObj->uiCurveTableSize),
                            &(psCurveObj->puiCurveTable), VC_TRUE);
            puiCurveTable = psCurveObj->puiCurveTable;
            uiCurveTableSize = psCurveObj->uiCurveTableSize;
        }
        else
        {
            Curve_CalcTable(psCurveObj, &psCurveObj->sCurveBasePoints,
                            &uiCurveTableSize, &puiCurveTable, VC_FALSE);
        }

        uiEndYTabelVal = psCurveObj->ChangeIdealY2TabelVal(
                            psCurveObj->sCurveBasePoints.uiEndY,
                            psCurveObj->argc, psCurveObj->argv);
        uiStartYTabelVal = psCurveObj->ChangeIdealY2TabelVal(
                               psCurveObj->sCurveBasePoints.uiStartY,
                               psCurveObj->argc, psCurveObj->argv);

        Curve_CalcIndexTable(psCurveObj, &usCurveGetIndexVal,
                             &puiCurveGetIndexTable, uiEndYTabelVal, uiStartYTabelVal,
                             bAutoSwitch2UpdatedCurve,
                             puiCurveTable, uiCurveTableSize);
    }
    else
    {
        Curve_CalcIndexTable(psCurveObj, &usCurveGetIndexVal,
                             &puiCurveGetIndexTable,
                             psCurveObj->puiCurveTable[psCurveObj->uiCurveTableSize-1],
                             psCurveObj->puiCurveTable[0],
                             bAutoSwitch2UpdatedCurve,
                             psCurveObj->puiCurveTable, psCurveObj->uiCurveTableSize);
    }

    psCurveObj->uiStartXNew = psCurveObj->sCurveBasePoints.uiStartX;
    psCurveObj->uiStartYNew = psCurveObj->sCurveBasePoints.uiStartY;

    if (!bAutoSwitch2UpdatedCurve)
    {
        if (puiCurveTable != NULL)
        {
            psCurveObj->uiCurveTableSizeNew = uiCurveTableSize;
            if (psCurveObj->puiCurveTableNew != NULL)
            {
                CV_HANDLE_STATUS_A_F( MemFree(psCurveObj->puiCurveTableNew) );
            }
            psCurveObj->puiCurveTableNew = puiCurveTable;
        }

        if (puiCurveGetIndexTable != NULL)
        {
            psCurveObj->usCurveGetIndexValNew = usCurveGetIndexVal;
            if (psCurveObj->puiCurveGetIndexTableNew != NULL)
            {
                CV_HANDLE_STATUS_A_F( MemFree(psCurveObj->puiCurveGetIndexTableNew) );
            }
            psCurveObj->puiCurveGetIndexTableNew = puiCurveGetIndexTable;
        }
    }
    else
    {
        psCurveObj->uiStartX = psCurveObj->sCurveBasePoints.uiStartX;
        psCurveObj->uiStartY = psCurveObj->sCurveBasePoints.uiStartY;
    }
}

/*=========================================================================*/
VC_STATUS    Curve_ExtContentAdd(CURVEOBJ* psCurveObj,
                                 CURVE_EXTERNCONTENT sExtContent,
                                 unsigned int uiID)
{
    if (psCurveObj == NULL)
        return VC_ERROR;
    if (uiID >= CURVE_MAX_EXT_CONTENT)
        return VC_ERROR;
    if (psCurveObj->psExtContent[uiID].bValid)
        return VC_ERROR;

    memcpy(&(psCurveObj->psExtContent[uiID]), &sExtContent,
           sizeof(CURVE_EXTERNCONTENT));
    psCurveObj->psExtContent[uiID].bValid = VC_TRUE;

    return VC_SUCCESS;
}

/*=========================================================================*/
VC_STATUS    Curve_ExtContentGet(CURVEOBJ* psCurveObj,
                                 CURVE_EXTERNCONTENT* psExtContent,
                                 unsigned int uiID)
{
    if (psCurveObj == NULL)
        return VC_ERROR;
    if (uiID >= CURVE_MAX_EXT_CONTENT)
        return VC_ERROR;
    if (!psCurveObj->psExtContent[uiID].bValid)
        return VC_ERROR;

    memcpy(psExtContent, &(psCurveObj->psExtContent[uiID]),
           sizeof(CURVE_EXTERNCONTENT));

    return VC_SUCCESS;
}

/*=========================================================================*/
VC_STATUS    Curve_ExtContentReplace(CURVEOBJ* psCurveObj,
                                     CURVE_EXTERNCONTENT sExtContent,
                                     unsigned int uiID)
{
    if (psCurveObj == NULL)
        return VC_ERROR;
    if (uiID >= CURVE_MAX_EXT_CONTENT)
        return VC_ERROR;
    if (!psCurveObj->psExtContent[uiID].bValid)
        return VC_ERROR;

    memcpy(&(psCurveObj->psExtContent[uiID]), &sExtContent,
           sizeof(CURVE_EXTERNCONTENT));
    psCurveObj->psExtContent[uiID].bValid = VC_TRUE;

    return VC_SUCCESS;
}

/*=========================================================================*/
VC_STATUS    Curve_ExtContentDel(CURVEOBJ* psCurveObj, unsigned int uiID)
{
    if (psCurveObj == NULL)
        return VC_ERROR;
    if (uiID >= CURVE_MAX_EXT_CONTENT)
        return VC_ERROR;
    if (!psCurveObj->psExtContent[uiID].bValid)
        return VC_ERROR;

    psCurveObj->psExtContent[uiID].bValid = VC_FALSE;

    return VC_SUCCESS;
}

#ifdef VALENTIN
/*=========================================================================*/
void Curve_Reg4CVPL(CURVEOBJ* psCurveObj,
                    unsigned int (*ChangeY2Ideal)(unsigned int uiY, int argc,
                                                  const char* argv[]))
{
    if (lk_pSLList == NULL)
    {
        IF_CHECK_STATUS_FAILED(
            SLL_Create(&lk_pSLList, VC_TRUE,  VC_SUSPEND,  NULL) )
        {
            return;
        }
    }

    psCurveObj->ChangeY2Ideal = ChangeY2Ideal;
    CV_HANDLE_STATUS_F( SLL_Append(lk_pSLList, psCurveObj) );
}

/*=========================================================================*/
void Curve_CVPL(char *pcData, unsigned long uiLength, unsigned char ucPort,
                SDCB* psDCB)
{
    SLL_Last sllLast;
    CURVEOBJ* psCurveObj;
    unsigned long ul;
    unsigned long ulNoOfItems;
    CURVE sCurve;
    unsigned int ui;
    unsigned int uiX;
    char*        pcBuf;
    int          piData[2];
    char*        pcSend;
    int          iSendLen;

    GetStartStopChar(piData);
    for (ui=8; ui<uiLength; ui++)
    {
        if (pcData[ui] == piData[1]) pcData[ui] = '\0';
    }

    if (pcData[6] == 'L')
    {
        if (pcData[7] != 'w')
        {
            return;
        }

        pcSend = AllocMemory(CURVEOBJ_BUF_SIZE);
        if (pcSend != NULL)
        {
            ulNoOfItems = SLL_GetNoOfItems(lk_pSLList);
            memset(&sllLast, 0, sizeof(SLL_Last));
            for (ul=0; ul<ulNoOfItems; ul++)
            {
                psCurveObj = SLL_GetItem(lk_pSLList, ul, &sllLast);

                iSendLen = snprintf(pcSend, CURVEOBJ_BUF_SIZE, "A%s;%s", pcData+8,
                                    psCurveObj->pcName);
                SendCom(ucPort, pcSend, iSendLen, psDCB, SCOMSOHETB_ADD, VC_FALSE);
            }
            iSendLen = snprintf(pcSend, CURVEOBJ_BUF_SIZE, "A%s;--------", pcData+8);
            SendCom(ucPort, pcSend, iSendLen, psDCB, SCOMSOHETB_ADD, VC_FALSE);
        }
        FreeMemory(pcSend);
        return;
    }

    if ((pcData[7] != 'w') || (uiLength < 10) )
    {
        return;
    }

    pcBuf=AllocMemory(CURVEOBJ_BUF_SIZE);

    ulNoOfItems = SLL_GetNoOfItems(lk_pSLList);
    memset(&sllLast, 0, sizeof(SLL_Last));
    for (ul=0; ul<ulNoOfItems; ul++)
    {
        psCurveObj = SLL_GetItem(lk_pSLList, ul, &sllLast);

        if (strcmp(pcData+8, psCurveObj->pcName) == 0)
        {
            sCurve = Curve_Get(psCurveObj);
            snprintf(pcBuf, CURVEOBJ_BUF_SIZE, "%s: Inc", psCurveObj->pcName);

            SendCom(ucPort, pcBuf, strlen(pcBuf), psDCB, SCOMSOHETB_ADD, VC_FALSE);
            snprintf(pcBuf, CURVEOBJ_BUF_SIZE, "i X  Y");
            SendCom(ucPort, pcBuf, strlen(pcBuf), psDCB, SCOMSOHETB_ADD, VC_FALSE);
            uiX = 1;
            for (ui=0; ui<sCurve.uiCurveTableSize; ui++)
            {
                if ( (psCurveObj->ChangeY2Ideal == NULL) || (pcData[6] == 'B') )
                {
                    snprintf(pcBuf, CURVEOBJ_BUF_SIZE, "%1u %2u %u", ui,
                             uiX, sCurve.puiCurveTable[ui]);
                }
                else
                {
                    snprintf(pcBuf, CURVEOBJ_BUF_SIZE, "%1u %2u %u", ui,
                             uiX/1000,
                             psCurveObj->ChangeY2Ideal(
                                     sCurve.puiCurveTable[ui],
                                     psCurveObj->argc, psCurveObj->argv));
                }
                SendCom(ucPort,  pcBuf,  strlen(pcBuf), psDCB,  SCOMSOHETB_ADD, VC_FALSE);
                uiX = psCurveObj->GetNextX(uiX, sCurve.puiCurveTable[ui],
                                        psCurveObj->argc, psCurveObj->argv);
            }
            snprintf(pcBuf, CURVEOBJ_BUF_SIZE, "------");
            SendCom(ucPort,  pcBuf,  strlen(pcBuf), psDCB,  SCOMSOHETB_ADD, VC_FALSE);
        }
    } // for

    FreeMemory(pcBuf);
}
#endif // VALENTIN
