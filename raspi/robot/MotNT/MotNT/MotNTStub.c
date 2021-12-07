/***************************************************************************/
/*                             Printerfirmware                             */
/*                    Copyright (C) Carl Valentin GmbH                     */
/*                       http://www.carl-valentin.de                       */
/*                  This code is licenced under the LGPL                   */
/***************************************************************************/

/*. IMPORT ================================================================*/

#include <vcplatform.h> // INSERTED BY checkIncludes.sh

#define CV_LOG_GROUP "Components_Motor"
#include <cvlog.h>

#include "../Utilities/curve.h"

#ifdef AVOID_INLINE
#define MOTNT_CFILE_THAT_DOES_THE_IMPLEMENT
#endif
#include "MotNT.h"

#include "MotNTStub.h"

/*. ENDIMPORT =============================================================*/

/*. EXPORT ================================================================*/

/*. ENDEXPORT =============================================================*/

/*. LOCAL =================================================================*/

/*-------------------------------- Makros ---------------------------------*/

/*--------------------------- Typdeklarationen ----------------------------*/

/*------------------------------ Prototypen -------------------------------*/

static VC_BOOL
MotNTStub_Start(MOTNT* psMot, VC_UINT16 ui16Speed, UINT16 ui16Dist,
                MOTNT_DIR sDir, const char* pcFile, int iLine);
static VC_BOOL
MotNTStub_StartFlex(MOTNT* psMotNT, VC_UINT16 ui16Speed, VC_UINT16 ui16Dist,
                    MOTNT_DIR motntDirection, VC_BOOL bExecAddToFunctions,
                    MOTNT_STARTFLEX_PARM* psParm, const char* pcFile, int iLine);
static void
MotNTStub_Stop(MOTNT* psMot, const char* pcFile, int iLine);

static unsigned int
MotNTStub_Speed2TimerVal(struct SMOTNT* psMot, VC_UINT16 ui16Speed);
static VC_UINT16
MotNTStub_TimerVal2Speed(struct SMOTNT* psMot, unsigned int uiTimerVal);
static VC_UINT32
MotNTStub_Dist2Steps(struct SMOTNT* psMot, VC_UINT16 ui16Dist);
static VC_UINT32
MotNTStub_Steps2Dist(struct SMOTNT* psMot, VC_UINT32 ui32Steps);
static unsigned int
MotNTStub_TimerVal2Time(struct SMOTNT* psMot, unsigned int uiTimerVal,
                        unsigned int uiFac);
static unsigned int
MotNTStub_Time2TimerVal(struct SMOTNT* psMot, unsigned int uiTimer,
                        unsigned int uiFac);

static VC_STATUS
MotNTStub_CurveObjCreate(MOTNT* psMotNT, CURVEOBJ* psCurveObj, char* pcName,
                         VC_BOOL bIncrease,
                         CURVE_BASE_POINTS* psCurveBasePoints);
static void
MotNTStub_CurveObjSet(MOTNT* psMotNT, CURVEOBJ* psCurveObj,
                      MOTNT_MOVETYPE sMoveType);
static CURVEOBJ*
MotNTStub_CurveObjGet(MOTNT* psMotNT, MOTNT_MOVETYPE sMoveType);

/*------------------------ Konstantendeklarationen ------------------------*/

/*------------------------- modulglobale Variable -------------------------*/

/*. ENDLOCAL ==============================================================*/

/*=========================================================================*/
static unsigned int
MotNTStub_TimerVal2Time(MOTNT* psMotNT, unsigned int uiTimerVal,
                        unsigned int uiFac)
{
    // MOTNTSTUB* psMot = (MOTNTSTUB*)psMotNT;

    return (uiTimerVal * uiFac / 1000);
}

/*=========================================================================*/
static unsigned int
MotNTStub_Time2TimerVal(MOTNT* psMotNT, unsigned int uiTime,
                        unsigned int uiFac)
{
    // MOTNTSTUB* psMot = (MOTNTSTUB*)psMotNT;

    if (uiFac == 0) return 0;

    return (uiTime / uiFac * 1000);
}

/*=========================================================================*/
void
MotNTStub_InitStruct(MOTNTSTUB* psMot)
{
    if (psMot == NULL) return;

    memset(psMot, 0, sizeof(MOTNTSTUB));

    _ACONST_U16_(psMot->sMotNTItf.ui16ID) = MOTNT_ID;
}

/*=========================================================================*/
static unsigned int
MotNTStub_GetNextX(unsigned int uiXTime, unsigned int uiYSpeedTimerVal,
                   int argc, const char* argv[])
{
    MOTNTSTUB* psMot = (MOTNTSTUB*)argv;
    return uiXTime
            + MotNTStub_TimerVal2Time((MOTNT*)psMot, uiYSpeedTimerVal, 1000);
}

/*=========================================================================*/
static unsigned int
MotNTStub_ChangeIdealY2TabelVal(unsigned int uiIdealYSpeed, int argc,
                                const char* argv[])
{
    MOTNT* psMot = (MOTNT*)argv;
    return MotNT_Speed2TimerVal(psMot, uiIdealYSpeed);
}

/*=========================================================================*/
static VC_UINT16
MotNTStub_GetCurSpeed(MOTNT* psMotNT, MOTNT_SPEEDUNIT sSpeedUnit)
{
    return 0;
}

/*=========================================================================*/
VC_BOOL
MotNTStub_Init(MOTNTSTUB* psMot)
{
    MOTNT_CURVES sCurves;
    CURVE_BASE_POINTS sCurveIncreaseBasePointsForward;
    CURVE_BASE_POINTS sCurveIncreaseBasePointsBackward;

    //. Pruefe Pointer auf Gueltigkeit
    if (psMot == NULL)
    {
        CV_LOG_ERROR(("NULL-Pointer!\n"));
        return VC_FALSE;
    }

    //. Pruefen ob die Struktur-Initialisierungsfunktion aufgerufen wurde
    if (psMot->sMotNTItf.ui16ID != MOTNT_ID)
    {
        CV_LOG_ERROR(("Do InitStruct 1st!\n"));
        return VC_FALSE;
    }

    //. Pruefen ob Init-Struktur ausgefuellt wurde
    if (psMot->sMotNTItf.pcName == NULL) return VC_FALSE;
    if (psMot->ui16Resolution == 0) return VC_FALSE;
    if (psMot->ui16FeedPer360Degs == 0) return VC_FALSE;

    if ((psMot->CurvesInit == NULL) || (psMot->CurvesGetFirst == NULL))
    {
        return VC_FALSE;
    }

    // Variablen Initialisieren
    psMot->sMotNTItf.ui16Res = psMot->ui16Resolution;
    psMot->sMotNTItf.ui16Mm360Deg = psMot->ui16FeedPer360Degs;

    psMot->sMotNTItf.Start = &MotNTStub_Start;
    psMot->sMotNTItf.StartFlex = &MotNTStub_StartFlex;
    psMot->sMotNTItf.Stop = &MotNTStub_Stop;
    psMot->sMotNTItf.GetCurSpeed = &MotNTStub_GetCurSpeed;
    psMot->sMotNTItf.Speed2TimerVal = &MotNTStub_Speed2TimerVal;
    psMot->sMotNTItf.TimerVal2Speed = &MotNTStub_TimerVal2Speed;
    psMot->sMotNTItf.Dist2Steps = &MotNTStub_Dist2Steps;
    psMot->sMotNTItf.Steps2Dist = &MotNTStub_Steps2Dist;
    psMot->sMotNTItf.TimerVal2Time = &MotNTStub_TimerVal2Time;
    psMot->sMotNTItf.Time2TimerVal = &MotNTStub_Time2TimerVal;

    psMot->sMotNTItf.CurveObjCreate = &MotNTStub_CurveObjCreate;
    psMot->sMotNTItf.CurveObjSet = &MotNTStub_CurveObjSet;
    psMot->sMotNTItf.CurveObjGet = &MotNTStub_CurveObjGet;

    // Private Variablen Initialisieren
    if ((psMot->CurvesInit != NULL)
            && (psMot->CurvesInit(&(psMot->sMotNTItf)) != VC_SUCCESS))
    {
        return VC_FALSE;
    }

    sCurves = psMot->CurvesGetFirst();

    MotNT_CurveObjSet(&psMot->sMotNTItf, sCurves.psCurveObjFwInc,
            MOTNT_MOVETYPE_FWINC);
    MotNT_CurveObjSet(&psMot->sMotNTItf, sCurves.psCurveObjFwDec,
            MOTNT_MOVETYPE_FWDEC);
    MotNT_CurveObjSet(&psMot->sMotNTItf, sCurves.psCurveObjBwInc,
            MOTNT_MOVETYPE_BWINC);
    MotNT_CurveObjSet(&psMot->sMotNTItf, sCurves.psCurveObjBwDec,
            MOTNT_MOVETYPE_BWDEC);

    sCurveIncreaseBasePointsForward = Curve_GetBasePoints(
            sCurves.psCurveObjFwInc);
    sCurveIncreaseBasePointsBackward = Curve_GetBasePoints(
            sCurves.psCurveObjBwInc);

    if (psMot->sMotNTItf.ui16MaxSpeedForward == 0)
    {
        if (sCurveIncreaseBasePointsForward.uiEndY == -1)
        {
            psMot->sMotNTItf.ui16MaxSpeedForward =
                    sCurveIncreaseBasePointsForward.puiBasePointsY
                    [sCurveIncreaseBasePointsForward.usNoOfBasePoints - 1];
        }
        else
        {
            psMot->sMotNTItf.ui16MaxSpeedForward =
                    sCurveIncreaseBasePointsForward.uiEndY;
        }
    }

    if (psMot->sMotNTItf.ui16MaxSpeedBackward == 0)
    {
        if (sCurveIncreaseBasePointsBackward.uiEndY == -1)
        {
            psMot->sMotNTItf.ui16MaxSpeedBackward =
                    sCurveIncreaseBasePointsBackward.puiBasePointsY
                    [sCurveIncreaseBasePointsBackward.usNoOfBasePoints - 1];
        }
        else
        {
            psMot->sMotNTItf.ui16MaxSpeedBackward =
                    sCurveIncreaseBasePointsBackward.uiEndY;
        }
    }

    if (psMot->sMotNTItf.ui16MinSpeedForward == 0)
    {
        psMot->sMotNTItf.ui16MinSpeedForward = MotNT_TimerVal2Speed(
                (MOTNT*)psMot, 0xFFFF);
    }

    if (psMot->sMotNTItf.ui16MinSpeedBackward == 0)
    {
        psMot->sMotNTItf.ui16MinSpeedBackward = MotNT_TimerVal2Speed(
                (MOTNT*)psMot, 0xFFFF);
    }

    return VC_TRUE;
}

/*=========================================================================*/
static VC_BOOL
MotNTStub_Start(MOTNT* psMotNT, VC_UINT16 ui16Speed, VC_UINT16 ui16Dist,
                MOTNT_DIR sDir, const char* pcFile, int iLine)
{
    MOTNTSTUB* psMot = (MOTNTSTUB*)psMotNT;

    psMot->sMotNTItf.bIsRunning = VC_TRUE;
    psMot->sMotNTItf.motDirection = sDir;

    CV_LOG(("MotNTStub_Start | Speed:" VC_UINT16$ "; Dist:" VC_UINT16$
            "; Dir: %u\n", ui16Speed, ui16Dist, sDir));

    return VC_TRUE;
}

/*=========================================================================*/
static VC_BOOL
MotNTStub_StartFlex(MOTNT* psMotNT, VC_UINT16 ui16Speed, VC_UINT16 ui16Dist,
                    MOTNT_DIR sDir, VC_BOOL bExecAddToFunctions,
                    MOTNT_STARTFLEX_PARM* psParm, const char* pcFile, int iLine)
{
    MOTNTSTUB* psMot = (MOTNTSTUB*)psMotNT;

    psMot->sMotNTItf.bIsRunning = VC_TRUE;
    psMot->sMotNTItf.motDirection = sDir;

    CV_LOG(("MotNTStub_StartFlex! Speed:" VC_UINT16$ "; Dist:" VC_UINT16$
            "; Dir: %u\n", ui16Speed, ui16Dist, sDir));

    return VC_TRUE;
}

/*=========================================================================*/
static void
MotNTStub_Stop(MOTNT* psMotNT, const char* pcFile, int iLine)
{
    MOTNTSTUB* psMot = (MOTNTSTUB*)psMotNT;

    psMot->sMotNTItf.bIsRunning = VC_FALSE;

    CV_LOG(("MotNTStub_Stop\n"));
}

/*=========================================================================*/
static unsigned int
MotNTStub_Speed2TimerVal(MOTNT* psMotNT, VC_UINT16 ui16Speed)
{
    MOTNTSTUB* psMot = (MOTNTSTUB*)psMotNT;
    unsigned int uiTimerVal;

    uiTimerVal = (psMot->sMotNTItf.ui16Mm360Deg * 1000.0)
                 / (ui16Speed * psMot->sMotNTItf.ui16Res) * 1000;

    return (uiTimerVal);
}

/*=========================================================================*/
static VC_UINT16
MotNTStub_TimerVal2Speed(MOTNT* psMotNT, unsigned int uiTimerVal)
{
    return (MotNT_Speed2TimerVal(psMotNT, uiTimerVal));
}

/*=========================================================================*/
static VC_UINT32
MotNTStub_Dist2Steps(MOTNT* psMotNT, VC_UINT16 ui16Dist)
{
    MOTNTSTUB* psMot = (MOTNTSTUB*)psMotNT;
    VC_UINT32 ui32Steps;

    ui32Steps = (ui16Dist * psMot->sMotNTItf.ui16Res)
                    / psMot->sMotNTItf.ui16Mm360Deg;

    return ui32Steps;
}

/*=========================================================================*/
static VC_UINT32
MotNTStub_Steps2Dist(MOTNT* psMotNT, VC_UINT32 ui32Steps)
{
    MOTNTSTUB* psMot = (MOTNTSTUB*)psMotNT;
    VC_UINT32 ui32Dist;

    ui32Dist = (ui32Steps * psMot->sMotNTItf.ui16Mm360Deg)
                    / psMot->sMotNTItf.ui16Res;

    return ui32Dist;
}

/*=========================================================================*/
static VC_STATUS
MotNTStub_CurveObjCreate(MOTNT* psMotNT, CURVEOBJ* psCurveObj, char* pcName,
                         VC_BOOL bIncrease,
                         CURVE_BASE_POINTS* psCurveBasePoints)
{
    return Curve_Create(psCurveObj, pcName, bIncrease, psCurveBasePoints,
            &MotNTStub_GetNextX, &MotNTStub_ChangeIdealY2TabelVal, 0,
            (const char**)psMotNT, VC_TRUE, NULL);
}

/*=========================================================================*/
static void
MotNTStub_CurveObjSet(MOTNT* psMotNT, CURVEOBJ* psCurveObj,
                      MOTNT_MOVETYPE sMoveType)
{
    MOTNTSTUB* psMot = (MOTNTSTUB*)psMotNT;

    switch (sMoveType)
    {
        case MOTNT_MOVETYPE_FWINC:
            psMot->psCurveObjFwInc = psCurveObj;
            break;
        case MOTNT_MOVETYPE_FWDEC:
            psMot->psCurveObjFwDec = psCurveObj;
            break;
        case MOTNT_MOVETYPE_BWINC:
            psMot->psCurveObjBwInc = psCurveObj;
            break;
        case MOTNT_MOVETYPE_BWDEC:
            psMot->psCurveObjBwDec = psCurveObj;
            break;
    }
}

/*=========================================================================*/
static CURVEOBJ*
MotNTStub_CurveObjGet(MOTNT* psMotNT, MOTNT_MOVETYPE sMoveType)
{
    MOTNTSTUB* psMot = (MOTNTSTUB*)psMotNT;

    switch (sMoveType)
    {
        case MOTNT_MOVETYPE_FWINC:
            return psMot->psCurveObjFwInc;
            break;
        case MOTNT_MOVETYPE_FWDEC:
            return psMot->psCurveObjFwDec;
            break;
        case MOTNT_MOVETYPE_BWINC:
            return psMot->psCurveObjBwInc;
            break;
        case MOTNT_MOVETYPE_BWDEC:
            return psMot->psCurveObjBwDec;
            break;
    }

    return NULL;
}
