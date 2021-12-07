/***************************************************************************/
/*                             Printerfirmware                             */
/*                    Copyright (C) Carl Valentin GmbH                     */
/*                       http://www.carl-valentin.de                       */
/*                  This code is licenced under the LGPL                   */
/***************************************************************************/

/*. IMPORT ================================================================*/

#include <vcplatform.h> // INSERTED BY checkIncludes.sh

#include "../vccomponents_intern.h"


#define CV_LOG_GROUP "Components_Motor"
#include <cvlog.h>

#include "../Utilities/curve.h"

#include "MotNT.h"

#ifdef AVOID_INLINE
#define MOTNTCALC_CFILE_THAT_DOES_THE_IMPLEMENT
#endif
#include "MotNTCalc.h"

/*. ENDIMPORT =============================================================*/

/*. EXPORT ================================================================*/

/*. ENDEXPORT =============================================================*/

/*. LOCAL =================================================================*/

/*-------------------------------- Makros ---------------------------------*/

#define MOTNT_CALC_ID 0x45A6

/*--------------------------- Typdeklarationen ----------------------------*/

typedef enum
{
    MOTNTCALCCURVE_TABLE_TIME, MOTNTCALCCURVE_TABLE_MAXSPEED
} MOTNTCALCCURVE_TABLE;

/*------------------------------ Prototypen -------------------------------*/

static VC_UINT16
MotNTCalcCurve_GetTime(MOTNT_CALC* psCalc, MOTNT_DIR motntDir,
                       VC_UINT16 ui16StartSpeed, VC_UINT16 ui16EndSpeed,
                       VC_UINT16 ui16Dist);
static VC_UINT16
MotNTCalcCurve_GetDist(MOTNT_CALC* psCalc, MOTNT_DIR motntDir,
                       VC_UINT16 ui16StartSpeed, VC_UINT16 ui16EndSpeed,
                       MOTNT_DISTUNIT sDistUnitReturnValue);
static VC_UINT16
MotNTCalcCurve_GetMaxSpeed(MOTNT_CALC* psCalc, MOTNT_DIR motntDir,
                           VC_UINT16 ui16StartSpeed, VC_UINT16 ui16EndSpeed,
                           VC_UINT16 ui16Dist);
static VC_UINT16
MotNTCalcCurve_GetTime4CompleteCycle(MOTNT_CALC* psCalc, MOTNT_DIR motntDir,
                                     VC_UINT16 ui16StartSpeed,
                                     VC_UINT16 ui16MaxSpeed,
                                     VC_UINT16 ui16EndSpeed,
                                     VC_UINT16 ui16Dist);

/*------------------------ Konstantendeklarationen ------------------------*/

/*------------------------- modulglobale Variable -------------------------*/

/*. ENDLOCAL ==============================================================*/

/*=========================================================================*/
void
MotNTCalcCurve_InitStruct(MOTNT_CALC* psCalc)
{
    if (psCalc == NULL) return;

    memset(psCalc, 0, sizeof(MOTNT_CALC));

    _ACONST_U16_(psCalc->ui16ID) = MOTNT_CALC_ID;
}

/*=========================================================================*/
static VC_BOOL
MotNTCalcCurve_CalcTimeTable(MOTNT* psCalc, CURVE curve,
                             unsigned int** ppuiTimeTable)
{
    unsigned int ui;
    VC_UINT32 ui32TimeSum = 0;

    if (psCalc == NULL) return VC_FALSE;

    *ppuiTimeTable = AllocMemory(
            curve.uiCurveTableSize * sizeof(unsigned int));
    if (*ppuiTimeTable == NULL) return VC_FALSE;
    for (ui=0; ui<curve.uiCurveTableSize; ui++)
    {
        ui32TimeSum += MotNT_TimerVal2Time(psCalc, curve.puiCurveTable[ui],
                1000);
        (*ppuiTimeTable)[ui] = ui32TimeSum / 1000;
    }

    return VC_TRUE;
}

/*=========================================================================*/
static VC_BOOL
MotNTCalcCurve_CalcMaxSpeed4DistTable(MOTNT_CALC* psCalc, CURVE curveInc,
                                      CURVE curveDec,
                                      VC_UINT16** ppui16MaxSpeed4DistTable,
                                      unsigned int* puiMaxSpeed4DistSize)
{
    unsigned int ui;

    VC_UINT16 ui16IncEndTimerVal;
    VC_UINT16 ui16DecEndTimerVal;
    VC_UINT16 ui16EndTimerVal = 0xFFFF;
    VC_UINT16 ui16MaxEndTimerVal = 0;

    unsigned int uiInc = 0;
    unsigned int uiDec = 0;

    if (curveInc.uiCurveTableSize > curveDec.uiCurveTableSize)
    {
        *ppui16MaxSpeed4DistTable = AllocMemory(
                curveInc.uiCurveTableSize * sizeof(unsigned int));
    }
    else
    {
        *ppui16MaxSpeed4DistTable = AllocMemory(
                curveDec.uiCurveTableSize * sizeof(unsigned int));
    }

    if ((*ppui16MaxSpeed4DistTable == NULL)
            || (curveInc.puiCurveTable == NULL)
            || (curveDec.puiCurveTable == NULL))
    {
        return VC_FALSE;
    }

    if (curveInc.puiCurveTable[curveInc.uiCurveTableSize - 1]
            > curveDec.puiCurveTable[curveDec.uiCurveTableSize - 1])
    {
        ui16MaxEndTimerVal = curveInc.puiCurveTable[curveInc.uiCurveTableSize
                - 1];
    }
    else
    {
        ui16MaxEndTimerVal = curveDec.puiCurveTable[curveDec.uiCurveTableSize
                - 1];
    }

    for (ui = 0; ui16EndTimerVal > ui16MaxEndTimerVal; ui++)
    {
        ui16IncEndTimerVal = curveInc.puiCurveTable[uiInc];
        ui16DecEndTimerVal = curveDec.puiCurveTable[uiDec];

        if (ui16IncEndTimerVal < ui16DecEndTimerVal) // IncSpeed > DecSpeed
        {
            if (uiDec < curveDec.uiCurveTableSize - 1) uiDec++;
            ui16EndTimerVal = ui16DecEndTimerVal;
        }
        else
        {
            if (uiInc < curveInc.uiCurveTableSize - 1) uiInc++;
            ui16EndTimerVal = ui16IncEndTimerVal;
        }

        if ((uiInc == curveInc.uiCurveTableSize - 1)
                && (uiDec == curveDec.uiCurveTableSize - 1))
        {
            ui--;
            break;
        }

        (*ppui16MaxSpeed4DistTable)[ui] = ui16EndTimerVal;
    }

    *puiMaxSpeed4DistSize = ui + 1;

    return VC_TRUE;
}

/*=========================================================================*/
VC_BOOL
MotNTCalcCurve_Constructor(MOTNT_CALC* psCalc, VC_BOOL bHighPrec)
{
    VC_BOOL bRet;

    //. Pruefe Pointer auf Gueltigkeit
    if (psCalc == NULL)
    {
        CV_LOG_ERROR(("NULL-Pointer!\n"));
        return VC_FALSE;
    }

    //. Pruefen ob die Struktur-Initialisierungsfunktion aufgerufen wurde
    if (psCalc->ui16ID != MOTNT_CALC_ID)
    {
        CV_LOG_ERROR(("Do InitStruct 1st!\n"));
        return VC_FALSE;
    }

    //. Pruefen ob Init-Struktur ausgefuellt wurde
    if (psCalc->pMot == NULL) return VC_FALSE;

    // Variablen Initialisieren
    bRet = MotNTCalcCurve_Update(psCalc, NULL);

    psCalc->GetTime = &MotNTCalcCurve_GetTime;
    psCalc->GetDist = &MotNTCalcCurve_GetDist;
    psCalc->GetMaxSpeed = &MotNTCalcCurve_GetMaxSpeed;
    psCalc->GetTime4CompleteCycle = &MotNTCalcCurve_GetTime4CompleteCycle;

    psCalc->bHighPrec = bHighPrec;

    return bRet;
}

/*=========================================================================*/
static VC_BOOL
MotNTCalcCurve_CalcTimeTableAndExtCont(MOTNT_CALC* psCalc,
                                       CURVEOBJ* psCurveObj)
{
    VC_STATUS sStatus;
    CURVE_EXTERNCONTENT sExtContent;

    sExtContent.uiPointerInfo = 0;
    sExtContent.bFreeMemOnCurveDelete = VC_TRUE;

    if (MotNTCalcCurve_CalcTimeTable(psCalc->pMot, Curve_Get(psCurveObj),
            (unsigned int**)&(sExtContent.pvPointer)))
    {
        sStatus = Curve_ExtContentAdd(psCurveObj, sExtContent,
                MOTNTCALCCURVE_TABLE_TIME);
        if (sStatus != VC_SUCCESS)
        {
            FreeMemory(sExtContent.pvPointer);
            return VC_FALSE;
        }
    }
    else
    {
        return VC_FALSE;
    }

    return VC_TRUE;
}

/*=========================================================================*/
static VC_BOOL
MotNTCalcCurve_CompareTable(VC_UINT16* pui16Table1, unsigned int uiTable1Size,
                            VC_UINT16* pui16Table2, unsigned int uiTable2Size)
{
    unsigned int ui;

    if (uiTable1Size != uiTable2Size) return VC_FALSE;

    for (ui = 0; ui < uiTable1Size; ui++)
    {
        if (pui16Table1[ui] != pui16Table2[ui]) return VC_FALSE;
    }

    return VC_TRUE;
}

/*=========================================================================*/
static VC_BOOL
MotNTCalcCurve_UpdateMaxSpeed4Dist(MOTNT_CALC* psCalc, CURVEOBJ* psCurveObj1,
                                   CURVEOBJ* psCurveObj2)
{
    VC_UINT16* pui16MaxSpeed4Dist;
    unsigned int uiMaxSpeed4DistForwSize;

    VC_STATUS sStatus1;
    VC_STATUS sStatus2;
    CURVE_EXTERNCONTENT sExtContent1;
    CURVE_EXTERNCONTENT sExtContent2;
    unsigned int uiContentID;

    if (!MotNTCalcCurve_CalcMaxSpeed4DistTable(psCalc, Curve_Get(psCurveObj1),
            Curve_Get(psCurveObj2), &(pui16MaxSpeed4Dist),
            &(uiMaxSpeed4DistForwSize)))
    {
        return VC_FALSE;
    }

    uiContentID = MOTNTCALCCURVE_TABLE_MAXSPEED;
    while (uiContentID < CURVE_MAX_EXT_CONTENT)
    {
        sStatus1 = Curve_ExtContentGet(psCurveObj1, &sExtContent1,
                uiContentID);
        sStatus2 = Curve_ExtContentGet(psCurveObj2, &sExtContent2,
                uiContentID);
        if ((sStatus1 == VC_SUCCESS) && (sStatus2 == VC_SUCCESS))
        {
            if (sExtContent1.pvPointer == sExtContent2.pvPointer)
            {
                if (MotNTCalcCurve_CompareTable(sExtContent1.pvPointer,
                        sExtContent1.uiPointerInfo, pui16MaxSpeed4Dist,
                        uiMaxSpeed4DistForwSize))
                {
                    // Alles in Ordnung
                    FreeMemory(pui16MaxSpeed4Dist);
                    return VC_TRUE;
                }
            }
        }
        else if ((sStatus1 != VC_SUCCESS) && (sStatus2 != VC_SUCCESS))
        {
            // neue Tabelle erstellen. Beiden zuweisen
            sExtContent1.pvPointer = pui16MaxSpeed4Dist;
            sExtContent1.uiPointerInfo = uiMaxSpeed4DistForwSize;
            sExtContent1.bFreeMemOnCurveDelete = VC_TRUE;
            sStatus1 = Curve_ExtContentAdd(psCurveObj1, sExtContent1,
                    uiContentID);
            if (sStatus1 != VC_SUCCESS)
            {
                FreeMemory(pui16MaxSpeed4Dist);
                return VC_FALSE;
            }
            sStatus1 = Curve_ExtContentAdd(psCurveObj2, sExtContent1,
                    uiContentID);
            if (sStatus1 != VC_SUCCESS)
            {
                return VC_FALSE;
            }
            return VC_TRUE;
        }
        else
        {
            // keine neue Tabelle erstelln nur die von der einen der anderen zuweisen
            if (sStatus1 == VC_SUCCESS)
            {
                sStatus2 = Curve_ExtContentAdd(psCurveObj2, sExtContent1,
                        uiContentID);
                if (sStatus2 != VC_SUCCESS)
                {
                    CV_LOG_A_ERROR(("%u\n", uiContentID));
                    FreeMemory(pui16MaxSpeed4Dist);
                    return VC_FALSE;
                }

                if (MotNTCalcCurve_CompareTable(sExtContent1.pvPointer,
                        sExtContent1.uiPointerInfo, pui16MaxSpeed4Dist,
                        uiMaxSpeed4DistForwSize))
                {
                    // Alles in Ordnung
                    FreeMemory(pui16MaxSpeed4Dist);
                    return VC_TRUE;
                }
            }
            else
            {
                sStatus1 = Curve_ExtContentAdd(psCurveObj1, sExtContent2,
                        uiContentID);
                if (sStatus1 != VC_SUCCESS)
                {
                    CV_LOG_A_ERROR(("%u\n", uiContentID));
                    FreeMemory(pui16MaxSpeed4Dist);
                    return VC_FALSE;
                }

                if (MotNTCalcCurve_CompareTable(sExtContent2.pvPointer,
                        sExtContent2.uiPointerInfo, pui16MaxSpeed4Dist,
                        uiMaxSpeed4DistForwSize))
                {
                    // Alles in Ordnung
                    FreeMemory(pui16MaxSpeed4Dist);
                    return VC_TRUE;
                }
            }
        }
        uiContentID++;
    }

    FreeMemory(pui16MaxSpeed4Dist);
    return VC_FALSE;
}

/*=========================================================================*/
VC_BOOL
MotNTCalcCurve_Update(MOTNT_CALC* psCalc,
                      MOTNT_CURVES_CONTAINER* psCurvesContainer)
{
    VC_BOOL bRet = VC_TRUE;
    VC_STATUS sStatus1;
    CURVEOBJ* psCurveObj1;
    CURVEOBJ* psCurveObj2;
    CURVE_EXTERNCONTENT sExtContent1;

    // Timetable IncForw berechnen
    if (psCurvesContainer == NULL)
    {
        psCurveObj1 = MotNT_CurveObjGet(psCalc->pMot, MOTNT_MOVETYPE_FWINC);
    }
    else
    {
        psCurveObj1 = &psCurvesContainer->sCurveObjFwInc;
    }
    sStatus1 = Curve_ExtContentGet(psCurveObj1, &sExtContent1,
            MOTNTCALCCURVE_TABLE_TIME);
    if (sStatus1 != VC_SUCCESS)
    {
        if (!MotNTCalcCurve_CalcTimeTableAndExtCont(psCalc, psCurveObj1))
        {
            CV_HANDLE_STATUS_A(VC_ERROR);
            bRet = VC_FALSE;
        }
    }

    // Timetable IncBackw berechnen
    if (psCurvesContainer == NULL)
    {
        psCurveObj1 = MotNT_CurveObjGet(psCalc->pMot, MOTNT_MOVETYPE_BWINC);
    }
    else
    {
        psCurveObj1 = &psCurvesContainer->sCurveObjBwInc;
    }
    sStatus1 = Curve_ExtContentGet(psCurveObj1, &sExtContent1,
            MOTNTCALCCURVE_TABLE_TIME);
    if (sStatus1 != VC_SUCCESS)
    {
        if (!MotNTCalcCurve_CalcTimeTableAndExtCont(psCalc, psCurveObj1))
        {
            CV_HANDLE_STATUS_A(VC_ERROR);
            bRet = VC_FALSE;
        }
    }

    // Timetable DecForw berechnen
    if (psCurvesContainer == NULL)
    {
        psCurveObj1 = MotNT_CurveObjGet(psCalc->pMot, MOTNT_MOVETYPE_FWDEC);
    }
    else
    {
        psCurveObj1 = &psCurvesContainer->sCurveObjFwDec;
    }
    sStatus1 = Curve_ExtContentGet(psCurveObj1, &sExtContent1,
            MOTNTCALCCURVE_TABLE_TIME);
    if (sStatus1 != VC_SUCCESS)
    {
        if (!MotNTCalcCurve_CalcTimeTableAndExtCont(psCalc, psCurveObj1))
        {
            CV_HANDLE_STATUS_A(VC_ERROR);
            bRet = VC_FALSE;
        }
    }

    // Timetable DecBackw berechnen
    if (psCurvesContainer == NULL)
    {
        psCurveObj1 = MotNT_CurveObjGet(psCalc->pMot, MOTNT_MOVETYPE_BWDEC);
    }
    else
    {
        psCurveObj1 = &psCurvesContainer->sCurveObjBwDec;
    }
    sStatus1 = Curve_ExtContentGet(psCurveObj1, &sExtContent1,
            MOTNTCALCCURVE_TABLE_TIME);
    if (sStatus1 != VC_SUCCESS)
    {
        if (!MotNTCalcCurve_CalcTimeTableAndExtCont(psCalc, psCurveObj1))
        {
            CV_HANDLE_STATUS_A(VC_ERROR);
            bRet = VC_FALSE;
        }
    }

    // MaxSpeed4DistForw berechnen
    if (psCurvesContainer == NULL)
    {
        psCurveObj1 = MotNT_CurveObjGet(psCalc->pMot, MOTNT_MOVETYPE_FWINC);
        psCurveObj2 = MotNT_CurveObjGet(psCalc->pMot, MOTNT_MOVETYPE_FWDEC);
    }
    else
    {
        psCurveObj1 = &psCurvesContainer->sCurveObjFwInc;
        psCurveObj2 = &psCurvesContainer->sCurveObjFwDec;
    }
    if (!MotNTCalcCurve_UpdateMaxSpeed4Dist(psCalc, psCurveObj1, psCurveObj2))
    {
        CV_HANDLE_STATUS_A(VC_ERROR);
        bRet = VC_FALSE;
    }

    // MaxSpeed4DistBackw berechnen
    if (psCurvesContainer == NULL)
    {
        psCurveObj1 = MotNT_CurveObjGet(psCalc->pMot, MOTNT_MOVETYPE_BWINC);
        psCurveObj2 = MotNT_CurveObjGet(psCalc->pMot, MOTNT_MOVETYPE_BWDEC);
    }
    else
    {
        psCurveObj1 = &psCurvesContainer->sCurveObjBwInc;
        psCurveObj2 = &psCurvesContainer->sCurveObjBwDec;
    }
    if (!MotNTCalcCurve_UpdateMaxSpeed4Dist(psCalc, psCurveObj1, psCurveObj2))
    {
        CV_HANDLE_STATUS_A(VC_ERROR);
        bRet = VC_FALSE;
    }

    return bRet;
}

/*=========================================================================*/
static VC_BOOL
MotNTCalcCurve_GetMaxSpeedTable(CURVE_EXTERNCONTENT* psExtContent, // Rueckgabewert
        CURVEOBJ* psCurveObjInc, CURVEOBJ* psCurveObjDec)
{
    VC_STATUS sStatusInc;
    VC_STATUS sStatusDec;
    CURVE_EXTERNCONTENT sExtContentInc;
    CURVE_EXTERNCONTENT sExtContentDec;
    unsigned int uiContentID;

    uiContentID = MOTNTCALCCURVE_TABLE_MAXSPEED;

    while (uiContentID < CURVE_MAX_EXT_CONTENT)
    {
        sStatusInc = Curve_ExtContentGet(psCurveObjInc, &sExtContentInc,
                uiContentID);
        sStatusDec = Curve_ExtContentGet(psCurveObjDec, &sExtContentDec,
                uiContentID);
        if ((sStatusInc != VC_SUCCESS) && (sStatusDec != VC_SUCCESS))
        {
            return VC_FALSE;
        }

        if ((sStatusInc == VC_SUCCESS) && (sStatusDec == VC_SUCCESS)
                && (sExtContentInc.pvPointer == sExtContentDec.pvPointer))
        {
            memcpy(psExtContent, &sExtContentInc,
                    sizeof(CURVE_EXTERNCONTENT));
            return VC_TRUE;
        }
        else
        {
            uiContentID++;
        }
    }

    return VC_FALSE;
}

/*=========================================================================*/
unsigned int MotNTCalcCurve_GetIndex(MOTNT_CALC* psCalc,
                                     CURVEOBJ* psCurveObj, unsigned int uiY)
{
    if (psCalc->bHighPrec)
    {
        return Curve_GetIndexHiPrec(psCurveObj, uiY);
    }
    else
    {
        return Curve_GetIndex(psCurveObj, uiY);
    }

}

/*=========================================================================*/
static VC_UINT16
MotNTCalcCurve_GetTime(MOTNT_CALC* psCalc, MOTNT_DIR motntDir,
                       VC_UINT16 ui16StartSpeed, VC_UINT16 ui16EndSpeed,
                       VC_UINT16 ui16Dist)
{
    VC_STATUS sStatus;

    unsigned int* puiIncTimeTable;
    unsigned int* puiDecTimeTable;
    unsigned int ui;
    unsigned int uiTimeStartSpeed;
    unsigned int uiTimeEndSpeed = 0;
    unsigned int uiTimeConstantSpeed;
    VC_UINT32 ui32Steps;

    VC_INT16* pui16MaxSpeed4Dist;
    unsigned int uiMaxSpeed4DistSize;
    VC_UINT16 ui16MaxSpeed;
    unsigned int uiStartSpeedTimerVal;
    unsigned int uiEndSpeedTimerVal;
    CURVE curveInc;
    CURVE curveDec;
    unsigned int uiStart;
    unsigned int uiEnd;

    unsigned int uiIncI;
    unsigned int uiDecI;

    CURVEOBJ* psCurveObjInc;
    CURVEOBJ* psCurveObjDec;
    CURVE_EXTERNCONTENT sExtContent;

    unsigned int uiTimeInc;
    unsigned int uiTimeDec;
    unsigned int uiTime0ToEndSpeed;
    unsigned int uiTime0ToStartSpeed;
    unsigned int uiTimeEndSpeedTo0;
    unsigned int uiTimeStartSpeedTo0;

    if (psCalc == NULL) return VC_FALSE;

    if (motntDir == MOTNT_DIR_FW)
    {
        psCurveObjInc = MotNT_CurveObjGet(psCalc->pMot, MOTNT_MOVETYPE_FWINC);
        psCurveObjDec = MotNT_CurveObjGet(psCalc->pMot, MOTNT_MOVETYPE_FWDEC);

        sStatus = Curve_ExtContentGet(psCurveObjInc, &sExtContent,
                MOTNTCALCCURVE_TABLE_TIME);
        if (sStatus != VC_SUCCESS) return VC_FALSE;
        puiIncTimeTable = sExtContent.pvPointer;

        sStatus = Curve_ExtContentGet(psCurveObjDec, &sExtContent,
                MOTNTCALCCURVE_TABLE_TIME);
        if (sStatus != VC_SUCCESS) return VC_FALSE;
        puiDecTimeTable = sExtContent.pvPointer;

        if (!MotNTCalcCurve_GetMaxSpeedTable(&sExtContent, psCurveObjInc,
                psCurveObjDec))
        {
            return VC_FALSE;
        }
        pui16MaxSpeed4Dist = sExtContent.pvPointer;
        uiMaxSpeed4DistSize = sExtContent.uiPointerInfo;
        ui16MaxSpeed = psCalc->pMot->ui16MaxSpeedForward;
    }
    else
    {
        psCurveObjInc = MotNT_CurveObjGet(psCalc->pMot, MOTNT_MOVETYPE_BWINC);
        psCurveObjDec = MotNT_CurveObjGet(psCalc->pMot, MOTNT_MOVETYPE_BWDEC);

        sStatus = Curve_ExtContentGet(psCurveObjInc, &sExtContent,
                MOTNTCALCCURVE_TABLE_TIME);
        if (sStatus != VC_SUCCESS) return VC_FALSE;
        puiIncTimeTable = sExtContent.pvPointer;

        sStatus = Curve_ExtContentGet(psCurveObjDec, &sExtContent,
                MOTNTCALCCURVE_TABLE_TIME);
        if (sStatus != VC_SUCCESS) return VC_FALSE;
        puiDecTimeTable = sExtContent.pvPointer;

        if (!MotNTCalcCurve_GetMaxSpeedTable(&sExtContent, psCurveObjInc,
                psCurveObjDec))
        {
            return VC_FALSE;
        }
        pui16MaxSpeed4Dist = sExtContent.pvPointer;
        uiMaxSpeed4DistSize = sExtContent.uiPointerInfo;
        ui16MaxSpeed = psCalc->pMot->ui16MaxSpeedBackward;
    }

    uiStartSpeedTimerVal = MotNT_Speed2TimerVal(psCalc->pMot, ui16StartSpeed);
    uiEndSpeedTimerVal = MotNT_Speed2TimerVal(psCalc->pMot, ui16EndSpeed);

    if (ui16Dist == 0)
    {
        // Wenn Beschleunigung
        if (ui16StartSpeed < ui16EndSpeed)
        {
            if (ui16StartSpeed == 0)
            {
                uiTimeStartSpeed = 0;
            }
            else
            {
                ui = MotNTCalcCurve_GetIndex(psCalc, psCurveObjInc, uiStartSpeedTimerVal);
                if (ui == 0) uiTimeStartSpeed = 0;
                else uiTimeStartSpeed = puiIncTimeTable[ui-1];
            }

            if (ui16EndSpeed == 0)
            {
                ui16EndSpeed = 0;
            }
            else
            {
                ui = MotNTCalcCurve_GetIndex(psCalc, psCurveObjInc, uiEndSpeedTimerVal);
                if (ui == 0) uiTimeEndSpeed = 0;
                else uiTimeEndSpeed = puiIncTimeTable[ui-1];
            }

            return (VC_UINT16)(uiTimeEndSpeed - uiTimeStartSpeed);
        }
        else // (ui16StartSpeed > ui16EndSpeed)
        {    // Abbremsen
            if (ui16StartSpeed == 0)
            {
                uiTimeStartSpeed = 0;
            }
            else
            {
                ui = MotNTCalcCurve_GetIndex(psCalc, psCurveObjDec, uiStartSpeedTimerVal);
                if (ui == 0) uiTimeStartSpeed = 0;
                else uiTimeStartSpeed = puiDecTimeTable[ui-1];
            }

            if (ui16EndSpeed == 0)
            {
                ui16EndSpeed = 0;
            }
            else
            {
                ui = MotNTCalcCurve_GetIndex(psCalc, psCurveObjDec, uiEndSpeedTimerVal);
                if (ui == 0) uiTimeEndSpeed = 0;
                else uiTimeEndSpeed = puiDecTimeTable[ui-1];
            }

            return (VC_UINT16)(uiTimeStartSpeed - uiTimeEndSpeed);
        }
        // Endif (ui16StartSpeed < ui16EndSpeed)
    }
    else // (ui16Dist != 0)
    {
        ui32Steps = MotNT_Dist2Steps(psCalc->pMot, ui16Dist);
        curveInc = Curve_Get(psCurveObjInc);
        curveDec = Curve_Get(psCurveObjDec);

        // Wenn von 0 bis 0
        if ((ui16StartSpeed == 0) && (ui16EndSpeed == 0))
        {
            // Wenn Gesamtanzahl Steps groesser wie Step-MaxSpeed-Table-Size
            // Also wenn die Strecke reicht um die Maximalgeschw. zu erreichen
            if (ui32Steps > (VC_UINT32)uiMaxSpeed4DistSize)
            {
                // ... dann wird Maximalgeschw. erreicht
                // ... und da die TimeTables ueber TimerVal funzen:
                uiEndSpeedTimerVal = MotNT_Speed2TimerVal(psCalc->pMot,
                        ui16MaxSpeed);
                // bestimme noch die Zeit fuer konstante Geschw.
                // Hinweis: KurvenIndizes entsprechen Steps
                // ui = Constante Strecke in Steps
                ui = ui32Steps - curveInc.uiCurveTableSize
                        - curveDec.uiCurveTableSize;
                uiTimeConstantSpeed = MotNT_TimerVal2Time(psCalc->pMot,
                        uiEndSpeedTimerVal * ui, 1);
            }
            else // Strecke reicht NICHT um die Maximalgeschw. zu erreichen
            {
                // Bestimme Wert direkt aus eigens hierzu generierter Tabelle
                uiEndSpeedTimerVal = pui16MaxSpeed4Dist[ui32Steps];
                uiTimeConstantSpeed = 0;
            }

            uiIncI = MotNTCalcCurve_GetIndex(psCalc, psCurveObjInc, uiEndSpeedTimerVal);
            if (uiIncI == 0) uiTimeInc = 0;
            else uiTimeInc = puiIncTimeTable[uiIncI-1];
            uiDecI = MotNTCalcCurve_GetIndex(psCalc, psCurveObjDec, uiEndSpeedTimerVal);
            if (uiDecI == 0) uiTimeDec = 0;
            else uiTimeDec = puiDecTimeTable[uiDecI-1];

            return (uiTimeInc + uiTimeConstantSpeed + uiTimeDec);
        }
        else // (ui16StartSpeed != 0) || (ui16EndSpeed != 0)
        {
            // Wenn Beschleunigung
            if (ui16StartSpeed < ui16EndSpeed)
            {
                // Pruefe ob wir bei gegebener Strecke ueberhaupt EndSpeed
                // erreichen (Index in Beschl.-Tabellen entspricht Steps)
                uiStart = MotNTCalcCurve_GetIndex(psCalc, psCurveObjInc, uiStartSpeedTimerVal);
                uiEnd = MotNTCalcCurve_GetIndex(psCalc, psCurveObjInc, uiEndSpeedTimerVal);
                // Wenn angegeben EndSpeed nicht erreicht werden kann
                if (uiEnd - uiStart > ui32Steps)
                {
                    // Bestimme Timerval fuer neue Endspeed
                    uiEndSpeedTimerVal = curveInc.puiCurveTable[uiStart
                            + ui32Steps];
                    uiTimeConstantSpeed = 0;
                }
                else // Angegebene Endspeed wird erreicht
                {
                    // Berechne Zeit fuer konstante Geschwindigkeit
                    ui = ui32Steps - (uiEnd - uiStart); // Anzahl der Steps im
                    // konstanten
                    // Geschwindigkeitsbereich
                    uiTimeConstantSpeed = MotNT_TimerVal2Time(psCalc->pMot,
                            uiEndSpeedTimerVal * ui, 1);
                }
                // Rueckgabewert = Zeit fuer 0-Beschleunigung bis Endspeed
                //                 + Zeit fuer konstante Geschwindigkeit
                //                 - Zeit fuer 0-Beschleunigung bis Startspeed
                uiEnd = MotNTCalcCurve_GetIndex(psCalc, psCurveObjInc, uiEndSpeedTimerVal);
                if (uiEnd == 0) uiTime0ToEndSpeed = 0;
                else uiTime0ToEndSpeed = puiIncTimeTable[uiEnd-1];
                uiStart = MotNTCalcCurve_GetIndex(psCalc, psCurveObjInc, uiStartSpeedTimerVal);
                if (uiStart == 0) uiTime0ToStartSpeed = 0;
                else uiTime0ToStartSpeed = puiIncTimeTable[uiStart-1];
                return (uiTime0ToEndSpeed + uiTimeConstantSpeed
                        - uiTime0ToStartSpeed);
            }
            else // (ui16StartSpeed > ui16EndSpeed)
            {
                // Pruefe ob wir bei gegebener Strecke ueberhaupt EndSpeed
                // erreichen (Index in Beschl.-Tabellen entspricht Steps)
                uiStart = MotNTCalcCurve_GetIndex(psCalc, psCurveObjDec, uiStartSpeedTimerVal);
                uiEnd = MotNTCalcCurve_GetIndex(psCalc, psCurveObjDec, uiEndSpeedTimerVal);
                // Wenn angegeben EndSpeed nicht erreicht werden kann
                if (uiStart - uiEnd > ui32Steps)
                {
                    // Bestimme Timerval fuer neue Endspeed
                    uiEndSpeedTimerVal = curveDec.puiCurveTable[uiStart
                            - ui32Steps];
                    uiTimeConstantSpeed = 0;
                }
                else // Angegebene Endspeed wird erreicht
                {
                    // Berechne Zeit fuer konstante Geschwindigkeit
                    ui = ui32Steps - (uiStart - uiEnd); // Anzahl der Steps im
                    // konstanten
                    // Geschwindigkeitsbereich
                    uiTimeConstantSpeed = MotNT_TimerVal2Time(psCalc->pMot,
                            uiStartSpeedTimerVal * ui, 1);
                }
                // Rueckgabewert = Zeit fuer konstante Geschwindigkeit
                //                 + Zeit von StartSpeed bis 0
                //                 - Zeit von EndSpeed bis 0
                uiStart = MotNTCalcCurve_GetIndex(psCalc, psCurveObjDec, uiStartSpeedTimerVal);
                if (uiStart == 0) uiTimeStartSpeedTo0 = 0;
                else uiTimeStartSpeedTo0 = puiDecTimeTable[uiStart-1];
                uiEnd = MotNTCalcCurve_GetIndex(psCalc, psCurveObjDec, uiEndSpeedTimerVal);
                if (uiEnd == 0) uiTimeEndSpeedTo0 = 0;
                else uiTimeEndSpeedTo0 = puiDecTimeTable[uiEnd-1];
                return (uiTimeConstantSpeed + uiTimeStartSpeedTo0
                        - uiTimeEndSpeedTo0);
            }
        }
    } // Endif (ui16Dist == 0)
}

/*=========================================================================*/
static VC_UINT16
MotNTCalcCurve_GetDist(MOTNT_CALC* psCalc, MOTNT_DIR motntDir,
                       VC_UINT16 ui16StartSpeed, VC_UINT16 ui16EndSpeed,
                       MOTNT_DISTUNIT sDistUnitReturnValue)
{
    unsigned int uiStartSpeedTimerVal;
    unsigned int uiEndSpeedTimerVal;
    unsigned int uiStart;
    unsigned int uiEnd;
    unsigned int uiResult;

    CURVEOBJ* psCurveObjInc;
    CURVEOBJ* psCurveObjDec;

    if (psCalc == NULL) return VC_FALSE;

    uiStartSpeedTimerVal = MotNT_Speed2TimerVal(psCalc->pMot, ui16StartSpeed);
    uiEndSpeedTimerVal = MotNT_Speed2TimerVal(psCalc->pMot, ui16EndSpeed);

    if (motntDir == MOTNT_DIR_FW)
    {
        psCurveObjInc = MotNT_CurveObjGet(psCalc->pMot, MOTNT_MOVETYPE_FWINC);
        psCurveObjDec = MotNT_CurveObjGet(psCalc->pMot, MOTNT_MOVETYPE_FWDEC);
    }
    else
    {
        psCurveObjInc = MotNT_CurveObjGet(psCalc->pMot, MOTNT_MOVETYPE_BWINC);
        psCurveObjDec = MotNT_CurveObjGet(psCalc->pMot, MOTNT_MOVETYPE_BWDEC);
    }

    if ((psCurveObjInc == NULL) || (psCurveObjDec == NULL))
    {
        return 0;
    }

    // Wenn Beschleunigung
    if (ui16StartSpeed < ui16EndSpeed)
    {
        uiStart = MotNTCalcCurve_GetIndex(psCalc, psCurveObjInc, uiStartSpeedTimerVal);
        uiEnd = MotNTCalcCurve_GetIndex(psCalc, psCurveObjInc, uiEndSpeedTimerVal);
        if (ui16StartSpeed == 0) uiStart = 0;
        uiResult = uiEnd - uiStart;
        switch (sDistUnitReturnValue)
        {
            case MOTNT_DISTUNIT_STEPS:
                return uiResult;
                break;

            case MOTNT_DISTUNIT_MM:
                return MotNT_Steps2Dist(psCalc->pMot, uiResult);
                break;
        }
    }
    else
    {
        uiStart = MotNTCalcCurve_GetIndex(psCalc, psCurveObjDec, uiStartSpeedTimerVal);
        uiEnd = MotNTCalcCurve_GetIndex(psCalc, psCurveObjDec, uiEndSpeedTimerVal);
        uiResult = uiStart - uiEnd;
        switch (sDistUnitReturnValue)
        {
            case MOTNT_DISTUNIT_STEPS:
                return uiResult;
                break;

            case MOTNT_DISTUNIT_MM:
                return MotNT_Steps2Dist(psCalc->pMot, uiResult);
                break;
        }
    }

    return 0;
}

/*=========================================================================*/
static VC_UINT16
MotNTCalcCurve_GetMaxSpeed(MOTNT_CALC* psCalc, MOTNT_DIR motntDir,
                           VC_UINT16 ui16StartSpeed, VC_UINT16 ui16EndSpeed,
                           VC_UINT16 ui16Dist)
{
    VC_UINT32 ui32StepsToDo;
    VC_UINT16 ui16MaxSpeed;
    VC_UINT16 ui16MinSpeed;
    unsigned int uiStartSpeedTimerVal;
    unsigned int uiMaxSpeedTimerVal;
    unsigned int uiEndSpeedTimerVal;
    VC_UINT32 ui32Steps4Accel;
    VC_UINT32 ui32Steps4MaxInc;
    VC_UINT32 ui32Steps4MaxDec;
    VC_UINT32 ui32Steps4Brake;
    VC_UINT32 ui32Steps;

    CURVEOBJ* psCurveObjInc;
    CURVEOBJ* psCurveObjDec;

    if (psCalc == NULL) return VC_FALSE;

    ui32StepsToDo = MotNT_Dist2Steps(psCalc->pMot, ui16Dist);
    ui16MaxSpeed = MotNT_GetInf(psCalc->pMot, MOTNT_INF_MAX_SPEED, motntDir);
    ui16MinSpeed = MotNT_GetInf(psCalc->pMot, MOTNT_INF_MIN_SPEED, motntDir);

    if (ui16StartSpeed < ui16MinSpeed) ui16StartSpeed = ui16MinSpeed;
    if (ui16EndSpeed < ui16MinSpeed) ui16EndSpeed = ui16MinSpeed;

    if ((ui16StartSpeed > ui16MaxSpeed) || (ui16EndSpeed > ui16MaxSpeed))
    {
        return ui16MaxSpeed;
    }

    if (motntDir == MOTNT_DIR_FW)
    {
        psCurveObjInc = MotNT_CurveObjGet(psCalc->pMot, MOTNT_MOVETYPE_FWINC);
        psCurveObjDec = MotNT_CurveObjGet(psCalc->pMot, MOTNT_MOVETYPE_FWDEC);
    }
    else
    {
        psCurveObjInc = MotNT_CurveObjGet(psCalc->pMot, MOTNT_MOVETYPE_BWINC);
        psCurveObjDec = MotNT_CurveObjGet(psCalc->pMot, MOTNT_MOVETYPE_BWDEC);
    }

    if ((psCurveObjInc == NULL) || (psCurveObjDec == NULL))
    {
        return 0;
    }

    uiStartSpeedTimerVal = MotNT_Speed2TimerVal(psCalc->pMot, ui16StartSpeed);
    uiEndSpeedTimerVal = MotNT_Speed2TimerVal(psCalc->pMot, ui16EndSpeed);

    ui32Steps4Accel = MotNTCalcCurve_GetIndex(psCalc, psCurveObjInc, uiStartSpeedTimerVal);
    ui32Steps4Brake = MotNTCalcCurve_GetIndex(psCalc, psCurveObjDec, uiEndSpeedTimerVal);

    ui16MaxSpeed++;
    do
    {
        ui16MaxSpeed--;
        uiMaxSpeedTimerVal = MotNT_Speed2TimerVal(psCalc->pMot, ui16MaxSpeed);

        ui32Steps4MaxInc = MotNTCalcCurve_GetIndex(psCalc, psCurveObjInc, uiMaxSpeedTimerVal);
        ui32Steps4MaxDec = MotNTCalcCurve_GetIndex(psCalc, psCurveObjDec, uiMaxSpeedTimerVal);

        ui32Steps = (ui32Steps4MaxInc - ui32Steps4Accel)
                + (ui32Steps4MaxDec - ui32Steps4Brake);
    }
    while (ui32StepsToDo < ui32Steps);

    return ui16MaxSpeed;
}

/*=========================================================================*/
static VC_UINT16
MotNTCalcCurve_GetTime4CompleteCycle(MOTNT_CALC* psCalc, MOTNT_DIR motntDir,
                                     VC_UINT16 ui16StartSpeed,
                                     VC_UINT16 ui16MaxSpeed,
                                     VC_UINT16 ui16EndSpeed,
                                     VC_UINT16 ui16Dist)
{
    VC_STATUS sStatus;

    VC_UINT16 ui16MaxSpeedLimit;
    unsigned int* puiIncTimeTable;
    unsigned int* puiDecTimeTable;
    VC_UINT32 ui32StepsToDo;
    VC_UINT16 ui16MinSpeed;
    unsigned int uiStartSpeedTimerVal;
    unsigned int uiMaxSpeedTimerVal;
    unsigned int uiEndSpeedTimerVal;
    VC_UINT32 ui32Steps4Accel;
    VC_UINT32 ui32Steps4MaxInc;
    VC_UINT32 ui32Steps4MaxDec;
    VC_UINT32 ui32Steps4Brake;
    VC_UINT32 ui32Steps;
    unsigned int uiTimeStartSpeed;
    unsigned int uiTimeMaxSpeedInc;
    unsigned int uiTimeMaxSpeedDec;
    unsigned int uiTimeEndSpeed;
    unsigned int uiTimeMaxSpeedIncDiff;
    unsigned int uiTimeMaxSpeedDecDiff;
    unsigned int uiTimeConstantSpeed = 0;

    CURVEOBJ* psCurveObjInc;
    CURVEOBJ* psCurveObjDec;
    CURVE_EXTERNCONTENT sExtContent;

    if (psCalc == NULL) return 0;

    if (ui16MaxSpeed == 0)
    {
        ui16MaxSpeed = MotNT_GetInf(psCalc->pMot, MOTNT_INF_MAX_SPEED,
                motntDir);
    }

    if ((ui16StartSpeed > ui16MaxSpeed) || (ui16EndSpeed > ui16MaxSpeed))
    {
        return 0;
    }

    ui16MinSpeed = MotNT_GetInf(psCalc->pMot, MOTNT_INF_MIN_SPEED, motntDir);
    if (ui16MaxSpeed < ui16MinSpeed) ui16MaxSpeed = ui16MinSpeed;
    if (ui16StartSpeed < ui16MinSpeed) ui16StartSpeed = ui16MinSpeed;
    if (ui16EndSpeed < ui16MinSpeed) ui16EndSpeed = ui16MinSpeed;

    if (ui16Dist > 0)
    {
        ui16MaxSpeedLimit = MotNTCalcCurve_GetMaxSpeed(psCalc, motntDir,
                ui16StartSpeed, ui16EndSpeed, ui16Dist);
        if (ui16MaxSpeed > ui16MaxSpeedLimit) ui16MaxSpeed =
                ui16MaxSpeedLimit;
    }

    if (motntDir == MOTNT_DIR_FW)
    {
        psCurveObjInc = MotNT_CurveObjGet(psCalc->pMot, MOTNT_MOVETYPE_FWINC);
        psCurveObjDec = MotNT_CurveObjGet(psCalc->pMot, MOTNT_MOVETYPE_FWDEC);

        sStatus = Curve_ExtContentGet(psCurveObjInc, &sExtContent,
                MOTNTCALCCURVE_TABLE_TIME);
        if (sStatus != VC_SUCCESS) return VC_FALSE;
        puiIncTimeTable = sExtContent.pvPointer;

        sStatus = Curve_ExtContentGet(psCurveObjDec, &sExtContent,
                MOTNTCALCCURVE_TABLE_TIME);
        if (sStatus != VC_SUCCESS) return VC_FALSE;
        puiDecTimeTable = sExtContent.pvPointer;

    }
    else
    {
        psCurveObjInc = MotNT_CurveObjGet(psCalc->pMot, MOTNT_MOVETYPE_BWINC);
        psCurveObjDec = MotNT_CurveObjGet(psCalc->pMot, MOTNT_MOVETYPE_BWDEC);

        sStatus = Curve_ExtContentGet(psCurveObjInc, &sExtContent,
                MOTNTCALCCURVE_TABLE_TIME);
        if (sStatus != VC_SUCCESS) return VC_FALSE;
        puiIncTimeTable = sExtContent.pvPointer;

        sStatus = Curve_ExtContentGet(psCurveObjDec, &sExtContent,
                MOTNTCALCCURVE_TABLE_TIME);
        if (sStatus != VC_SUCCESS) return VC_FALSE;
        puiDecTimeTable = sExtContent.pvPointer;
    }

    if ((psCurveObjInc == NULL) || (psCurveObjDec == NULL))
    {
        return 0;
    }

    uiStartSpeedTimerVal = MotNT_Speed2TimerVal(psCalc->pMot, ui16StartSpeed);
    uiMaxSpeedTimerVal = MotNT_Speed2TimerVal(psCalc->pMot, ui16MaxSpeed);
    uiEndSpeedTimerVal = MotNT_Speed2TimerVal(psCalc->pMot, ui16EndSpeed);

    ui32Steps4Accel = MotNTCalcCurve_GetIndex(psCalc, psCurveObjInc, uiStartSpeedTimerVal);
    ui32Steps4MaxInc = MotNTCalcCurve_GetIndex(psCalc, psCurveObjInc, uiMaxSpeedTimerVal);
    ui32Steps4MaxDec = MotNTCalcCurve_GetIndex(psCalc, psCurveObjDec, uiMaxSpeedTimerVal);
    ui32Steps4Brake = MotNTCalcCurve_GetIndex(psCalc, psCurveObjDec, uiEndSpeedTimerVal);

    if (ui16Dist > 0)
    {
        ui32StepsToDo = MotNT_Dist2Steps(psCalc->pMot, ui16Dist);
        ui32Steps = (ui32Steps4MaxInc - ui32Steps4Accel)
                + (ui32Steps4MaxDec - ui32Steps4Brake);
        if (ui32StepsToDo > ui32Steps)
        {
            uiTimeConstantSpeed = MotNT_TimerVal2Time(psCalc->pMot,
                    uiMaxSpeedTimerVal * (ui32StepsToDo - ui32Steps), 1);
        }
    }

    if (ui32Steps4Accel == 0) uiTimeStartSpeed = 0;
    else uiTimeStartSpeed = puiIncTimeTable[ui32Steps4Accel-1];
    if (ui32Steps4MaxInc == 0) uiTimeMaxSpeedInc = 0;
    else uiTimeMaxSpeedInc = puiIncTimeTable[ui32Steps4MaxInc-1];
    if (ui32Steps4MaxDec == 0) uiTimeMaxSpeedDec = 0;
    else uiTimeMaxSpeedDec = puiDecTimeTable[ui32Steps4MaxDec-1];
    if (ui32Steps4Brake == 0) uiTimeEndSpeed = 0;
    else uiTimeEndSpeed = puiDecTimeTable[ui32Steps4Brake-1];

    uiTimeMaxSpeedIncDiff = uiTimeMaxSpeedInc - uiTimeStartSpeed;
    uiTimeMaxSpeedDecDiff = uiTimeMaxSpeedDec - uiTimeEndSpeed;

    return (uiTimeMaxSpeedIncDiff + uiTimeMaxSpeedDecDiff
            + uiTimeConstantSpeed);
}
