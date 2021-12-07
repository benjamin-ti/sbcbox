/***************************************************************************/
/*                             Printerfirmware                             */
/*                    Copyright (C) Carl Valentin GmbH                     */
/*                       http://www.carl-valentin.de                       */
/*                  This code is licenced under the LGPL                   */
/***************************************************************************/


/*. IMPORT ================================================================*/

#include <vcplatform.h> // INSERTED BY checkIncludes.sh

#ifdef LINUX_SUPPORT


#define CV_LOG_GROUP "Components_Motor"
#include <cvlog.h>

#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <errno.h>

#include "../Utilities/curve.h"

#include "MotNT.h"

#include "MotNTLxS.h"

#ifdef VCHALGEN5_SUPPORT
extern void
VCHALGen5DeviceMotors_callbackPosition(MOTNT* psMotNT, int argc, const char* argv[]);
extern void
VCHALGen5DeviceMotors_callbackMotorStopped(MOTNT* psMotNT, int argc, const char* argv[]);
#endif

/*. ENDIMPORT =============================================================*/


/*. EXPORT ================================================================*/

/*. ENDEXPORT =============================================================*/


/*. LOCAL =================================================================*/

/*-------------------------------- Makros ---------------------------------*/

/*--------------------------- Typdeklarationen ----------------------------*/

/*------------------------------ Prototypen -------------------------------*/

static VC_BOOL MotNTLxS_Start(MOTNT* psMot, VC_UINT16 ui16Speed,
                    UINT16 ui16Dist, MOTNT_DIR sDir, const char* pcFile, int iLine);
static VC_BOOL MotNTLxS_StartFlex(MOTNT* psMotNT, VC_UINT16 ui16Speed, VC_UINT16 ui16Dist,
                                  MOTNT_DIR sDir, VC_BOOL bExecAddToFunctions,
                                  MOTNT_STARTFLEX_PARM* psParm, const char* pcFile, int iLine);
static void MotNTLxS_Stop(MOTNT* psMot, const char* pcFile, int iLine);

static unsigned int MotNTLxS_Speed2TimerVal(struct SMOTNT* psMot,
                                            VC_UINT16 ui16Speed);
static VC_UINT16 MotNTLxS_TimerVal2Speed(struct SMOTNT* psMot,
                                         unsigned int uiTimerVal);
static VC_UINT32 MotNTLxS_Dist2Steps(struct SMOTNT* psMot,
                                     VC_UINT16 ui16Dist);
static VC_UINT32 MotNTLxS_Steps2Dist(struct SMOTNT* psMot,
                                     VC_UINT32 ui32Steps);
static VC_UINT32 MotNTLxS_GetSteps2Stop(MOTNT* psMotNT);
static unsigned int MotNTLxS_TimerVal2Time(struct SMOTNT* psMot,
                                           unsigned int uiTimerVal,
                                           unsigned int uiFac);
static unsigned int MotNTLxS_Time2TimerVal(struct SMOTNT* psMot,
                                           unsigned int uiTimer,
                                           unsigned int uiFac);
static VC_UINT16 MotNTLxS_GetFollowTimerVal4Mot(MOTNTLXS* psMot);

static VC_STATUS MotNTLxS_CurveObjCreate(MOTNT* psMotNT, CURVEOBJ* psCurveObj,
                                       char* pcName, VC_BOOL bIncrease,
                                       CURVE_BASE_POINTS* psCurveBasePoints);
static void MotNTLxS_CurveObjSet(MOTNT* psMotNT, CURVEOBJ* psCurveObj,
                               MOTNT_MOVETYPE sMoveType);
static CURVEOBJ* MotNTLxS_CurveObjGet(MOTNT* psMotNT, MOTNT_MOVETYPE sMoveType);

static VC_BOOL
MotNTLxS_ExecAtDist(MOTNT* psMotNT, VC_UINT32 ui32Dist, MOTNT_DISTUNIT sDistUnit,
                     VC_BOOL bRelative, MOTNT_DIR sDir4Relative, void
                     (*ExecAtDistFunc)(MOTNT* psMot, int argc, const char* argv[]),
                     int argc, const char* argv[]);
static VC_BOOL MotNTLxS_AddToStep(struct SMOTNT* psMotNT,
                     void (*AddToStepFunc)(struct SMOTNT* psMotNT,
                                           int argc, const char* argv[]),
                     int argc, const char* argv[]);
static VC_BOOL MotNTLxS_AddToStateSwitch(struct SMOTNT* psMotNT,
                            void (*AddToStateSwitchFunc)(
                                    struct SMOTNT* psMotNT,
                                   MOTNT_STATE sCurState,
                                   MOTNT_STATE sNextState,
                                   VC_BOOL bStopAfterDec,
                                   int argc, const char* argv[]),
                            int argc, const char* argv[]);
static VC_BOOL MotNTLxS_AddToStop(MOTNT* psMot,
                            void (*AddToStopFunc)(
                                   MOTNT* psMot,
                                   int argc, const char* argv[]),
                            int argc, const char* argv[]);

static VC_UINT32 MotNTLxS_GetPositionFromMotor(struct SMOTNT* psMotNT);
static void MotNTLxS_StopAtPosition(struct SMOTNT* psMotNT,
                                unsigned int position);
static VC_BOOL MotNTLxS_SetPositionCallback(struct SMOTNT* psMotNT, unsigned int uiPosition,
                                            void* pvCbParms);
static VC_BOOL MotNTLxS_ClearPositionCallbacks(struct SMOTNT* psMotNT);

static void DoStart(MOTNTLXS* psMot, VC_UINT16 ui16Speed,
                    VC_UINT16 ui16Dist, MOTNT_DIR sDir,
                    VC_UINT16 ui16StartSpeedInTimerVal);
static void SetCondIncrease(MOTNTLXS* psMot, U16 ui16EndSpeedTimerVal);
static void DoCondIncrease(MOTNTLXS* psMot, U16 ui16EndSpeedTimerVal);
static void SetCondDecrease(MOTNTLXS* psMot, U16 ui16EndSpeedTimerVal,
                            VC_BOOL bStopAfterDec);
static void DoCondDecrease(MOTNTLXS* psMot, U16 ui16EndSpeedTimerVal,
                           VC_BOOL bStopAfterDec);
static void MotNTLxS_PauseStateMachineChange(MOTNTLXS* psMot, VC_BOOL bPause);

static void* MotNTLxS_StepTimerThread(void* pvObject);
static void MotNTLxS_StateMachineExe(MOTNTLXS* psMot);

/*------------------------ Konstantendeklarationen ------------------------*/

/*------------------------- modulglobale Variable -------------------------*/

static MOTNT_STATE lk_sStateChart[MOTNT_STATE_NO][MOTNTLXS_COND_NO] =
{
    // MOTNT_STATE_STOPPED
    {
        /* MOTNTLXS_COND_INC               */ MOTNT_STATE_INCREASE,
        /* MOTNTLXS_COND_ENDSPEED          */ MOTNT_STATE_NO,
        /* MOTNTLXS_COND_DEC               */ MOTNT_STATE_NO,
        /* MOTNTLXS_COND_STOPSPEED         */ MOTNT_STATE_NO,
    },

    // MOTNT_STATE_INCREASE
    {
        /* MOTNTLXS_COND_INC               */ MOTNT_STATE_INCREASE,
        /* MOTNTLXS_COND_ENDSPEED          */ MOTNT_STATE_RUNSPEED,
        /* MOTNTLXS_COND_DEC               */ MOTNT_STATE_DECREASE,
        /* MOTNTLXS_COND_STOPSPEED         */ MOTNT_STATE_NO,
    },

    // MOTNT_STATE_RUNSPEED
    {
        /* MOTNTLXS_COND_INC               */ MOTNT_STATE_INCREASE,
        /* MOTNTLXS_COND_ENDSPEED          */ MOTNT_STATE_NO,
        /* MOTNTLXS_COND_DEC               */ MOTNT_STATE_DECREASE,
        /* MOTNTLXS_COND_STOPSPEED         */ MOTNT_STATE_NO,
    },

    // MOTNT_STATE_DECREASE
    {
        /* MOTNTLXS_COND_INC               */ MOTNT_STATE_INCREASE,
        /* MOTNTLXS_COND_ENDSPEED          */ MOTNT_STATE_RUNSPEED,
        /* MOTNTLXS_COND_DEC               */ MOTNT_STATE_DECREASE,
        /* MOTNTLXS_COND_STOPSPEED         */ MOTNT_STATE_STOPPED,
    }
};

/*. ENDLOCAL ==============================================================*/

/*=========================================================================*/
static inline void SetCurveTabelIndexInc(MOTNTLXS* psMot)
{
    VC_UINT16 ui16StepIntCompValue;
    VC_UINT16 ui16MinSpeedInTimerVal;

    if (psMot->sCurState == MOTNT_STATE_STOPPED)
    {
        if (psMot->sDoStart_Dir == MOTNT_DIR_FW)
        {
            ui16MinSpeedInTimerVal = MotNT_Speed2TimerVal(&psMot->sMotNTItf,
                                         psMot->sMotNTItf.ui16MinSpeedForward);
        }
        else
        {
            ui16MinSpeedInTimerVal = MotNT_Speed2TimerVal(&psMot->sMotNTItf,
                                         psMot->sMotNTItf.ui16MinSpeedBackward);
        }
        if ( (psMot->ui16DoStart_StartSpeedInTimerVal != 0xFFFF)
             && (psMot->ui16DoStart_StartSpeedInTimerVal != 0)
             && (psMot->ui16DoStart_StartSpeedInTimerVal != ui16MinSpeedInTimerVal) )
        {
            ui16StepIntCompValue = psMot->ui16DoStart_StartSpeedInTimerVal;
            if (psMot->psCurveObjCurInc != NULL)
            {
                psMot->uiCurCurveTableIndex =
                    Curve_GetIndex(psMot->psCurveObjCurInc, ui16StepIntCompValue);
            }
        }
        else
        {
            psMot->uiCurCurveTableIndex = 0;
        }
    }
    else
    {
        if (psMot->psCurveObjCurInc != NULL)
        {
            psMot->uiCurCurveTableIndex =
                Curve_GetIndex(psMot->psCurveObjCurInc,
                               psMot->uiStepTimerValue_us);
        }
    }
}


/*=========================================================================*/
static inline void SetCurveTabelIndexDec(MOTNTLXS* psMot)
{
    if (psMot->psCurveObjCurDec != NULL)
    {
        psMot->uiCurCurveTableIndex =
            Curve_GetIndex(psMot->psCurveObjCurDec,
                           psMot->uiStepTimerValue_us);
    }
}

/*=========================================================================*/
static unsigned int MotNTLxS_TimerVal2Time(MOTNT* psMotNT,
                                           unsigned int uiTimerVal,
                                           unsigned int uiFac)
{
    // MOTNTLXS* psMot = (MOTNTLXS*)psMotNT;

    return ( uiTimerVal * uiFac / 1000 );
}


/*=========================================================================*/
static unsigned int MotNTLxS_Time2TimerVal(MOTNT* psMotNT,
                                           unsigned int uiTime,
                                           unsigned int uiFac)
{
    // MOTNTLXS* psMot = (MOTNTLXS*)psMotNT;

    if (uiFac == 0) return 0;

    return ( uiTime * 1000 / uiFac );
}


/*=========================================================================*/
void MotNTLxS_InitStruct(MOTNTLXS* psMot)
{
    if (psMot == NULL) return;

    memset(psMot, 0, sizeof(MOTNTLXS));

    _ACONST_U16_(psMot->sMotNTItf.ui16ID) = MOTNT_ID;
}

/*=========================================================================*/
static unsigned int MotNTLxS_GetNextX(unsigned int uiXTime,
                             unsigned int uiYSpeedTimerVal,
                             int argc, const char* argv[])
{
    MOTNTLXS* psMot = (MOTNTLXS*)argv;
    return uiXTime+MotNTLxS_TimerVal2Time((MOTNT*)psMot,
                                          uiYSpeedTimerVal, 1000);
}


/*=========================================================================*/
static unsigned int MotNTLxS_ChangeIdealY2TabelVal(unsigned int uiIdealYSpeed,
                                          int argc, const char* argv[])
{
    MOTNT* psMot = (MOTNT*)argv;
    return  MotNT_Speed2TimerVal(psMot, uiIdealYSpeed);
}


/*=========================================================================*/
static VC_UINT16  MotNTLxS_GetCurSpeed(MOTNT* psMotNT,
                                       MOTNT_SPEEDUNIT sSpeedUnit)
{
    MOTNTLXS* psMot = (MOTNTLXS*)psMotNT;

    if (sSpeedUnit == MOTNT_SPEEDUNIT_MM_S)
    {
        return MotNT_TimerVal2Speed(psMotNT, psMot->uiStepTimerValue_us);
    }
    else
    {
        return psMot->uiStepTimerValue_us;
    }
}


/*=========================================================================*/
static MOTNT_STATE MotNTLxS_GetCurState(MOTNT* psMotNT)
{
    MOTNTLXS* psMot = (MOTNTLXS*)psMotNT;

    return psMot->sCurState;
}


/*=========================================================================*/
static void MotNTLxS_RecalcFollowTimer(struct SMOTNT* psMotNT)
{
    MOTNTLXS* psMot = (MOTNTLXS*)psMotNT;
    VC_UINT32 ui32FollowTimerVal;
    VC_UINT32 ui32OurTimerVal;

    if (psMot->CalcFollowTimerVal != NULL)
    {
        ui32FollowTimerVal = psMot->CalcFollowTimerVal(psMot->pvFollowTimerValObj,
                psMot->sMotNTItf.ui16MaxSpeedForward);
        ui32OurTimerVal = MotNTLxS_Speed2TimerVal((MOTNT*)psMot,
                psMot->sMotNTItf.ui16MaxSpeedForward);
        psMot->ui32FollowTimerFacNewCalc = (ui32OurTimerVal << 16)
                                           / ui32FollowTimerVal;
        CV_LOG_A(("FollowTimerFac: %lu\n", psMot->ui32FollowTimerFacNewCalc));

        psMot->ui16FollowTimerMinSpeedValNewCalc =
            psMot->CalcFollowTimerVal(psMot->pvFollowTimerValObj,
                                      psMot->sMotNTItf.ui16MinSpeedForward);
    }
    else
    {
        psMot->ui16FollowTimerMinSpeedValNewCalc = 0xFFFF;
    }
    psMot->bFollowTimerNewCalc = VC_TRUE;
}


/*=========================================================================*/
VC_BOOL
MotNTLxS_Init(MOTNTLXS* psMot)
{
    MOTNT_CURVES sCurves;
    CURVE_BASE_POINTS sCurveIncreaseBasePointsForward;
    CURVE_BASE_POINTS sCurveIncreaseBasePointsBackward;

    int iRet;

    //. Pruefe Pointer auf Gueltigkeit
    if (psMot == NULL)
    {
        CV_LOG_A_ERROR(("NULL-Pointer!\n"));
        return VC_FALSE;
    }

    //. Pruefen ob die Struktur-Initialisierungsfunktion aufgerufen wurde
    if (psMot->sMotNTItf.ui16ID != MOTNT_ID)
    {
        CV_LOG_A_ERROR(("Do InitStruct 1st!\n"));
        return VC_FALSE;
    }

    //. Pruefen ob Init-Struktur ausgefuellt wurde
    if (psMot->sMotNTItf.pcName == NULL) return VC_FALSE;
    if (psMot->ui16Resolution == 0) return VC_FALSE;
    if (psMot->ui16FeedPer360Degs == 0) return VC_FALSE;
    if (psMot->timer_id == 0) return VC_FALSE;

    if ( (psMot->CurvesInit == NULL) || (psMot->CurvesGetFirst == NULL) )
    {
        return VC_FALSE;
    }

    psMot->pui32StepsDone = (VC_UINT32*)&psMot->sMotNTItf.ui32StepsDoneForward;
    if (psMot->DoStepNative == NULL) return VC_FALSE;

    // Variablen Initialisieren
    psMot->sMotNTItf.ui16Res = psMot->ui16Resolution;
    psMot->sMotNTItf.ui16Mm360Deg = psMot->ui16FeedPer360Degs;

    psMot->sMotNTItf.Start = &MotNTLxS_Start;
    psMot->sMotNTItf.StartFlex = &MotNTLxS_StartFlex;
    psMot->sMotNTItf.Stop  = &MotNTLxS_Stop;
    // psMot->sMotNTItf.StopAdv = &MotNTLxS_StopAdv;
    psMot->sMotNTItf.GetCurSpeed = &MotNTLxS_GetCurSpeed;
    psMot->sMotNTItf.GetCurState = &MotNTLxS_GetCurState;
    psMot->sMotNTItf.Speed2TimerVal = &MotNTLxS_Speed2TimerVal;
    psMot->sMotNTItf.TimerVal2Speed = &MotNTLxS_TimerVal2Speed;
    psMot->sMotNTItf.Dist2Steps = &MotNTLxS_Dist2Steps;
    psMot->sMotNTItf.Steps2Dist = &MotNTLxS_Steps2Dist;
    psMot->sMotNTItf.GetSteps2Stop = &MotNTLxS_GetSteps2Stop;
    psMot->sMotNTItf.TimerVal2Time = &MotNTLxS_TimerVal2Time;
    psMot->sMotNTItf.Time2TimerVal = &MotNTLxS_Time2TimerVal;
    psMot->sMotNTItf.ExecAtDist = &MotNTLxS_ExecAtDist;
    psMot->sMotNTItf.AddToStep = &MotNTLxS_AddToStep;
    // psMot->sMotNTItf.AddToStepDelete = &MotNTLxS_AddToStepDelete;
    psMot->sMotNTItf.AddToStateSwitch = &MotNTLxS_AddToStateSwitch;
    psMot->sMotNTItf.AddToStop = &MotNTLxS_AddToStop;
    psMot->sMotNTItf.RecalcFollowTimer = &MotNTLxS_RecalcFollowTimer;
    // psMot->sMotNTItf.SetRes = &MotNTLxS_SetRes;
    // psMot->sMotNTItf.SetMm360Deg = &MotNTLxS_SetMm360Deg;
    // psMot->sMotNTItf.DebugOutp = &MotNTLxS_DebugOutp;

    psMot->sMotNTItf.CurveObjCreate = &MotNTLxS_CurveObjCreate;
    psMot->sMotNTItf.CurveObjSet =    &MotNTLxS_CurveObjSet;
    psMot->sMotNTItf.CurveObjGet =    &MotNTLxS_CurveObjGet;

    psMot->sMotNTItf.GetPositionFromMotor = &MotNTLxS_GetPositionFromMotor;
    psMot->sMotNTItf.StopAtPosition = &MotNTLxS_StopAtPosition;
    psMot->sMotNTItf.SetPositionCallback = &MotNTLxS_SetPositionCallback;
    psMot->sMotNTItf.ClearPositionCallbacks = &MotNTLxS_ClearPositionCallbacks;

    // Private Variablen Initialisieren
    if ( (psMot->CurvesInit != NULL)
         && (psMot->CurvesInit(&(psMot->sMotNTItf)) != VC_SUCCESS) )
    {
        return VC_FALSE;
    }

    memset(psMot->pExecAtDistFuncs, 0,
           MOTNTLXS_NO_OF_ADD_TO_FUNCS*sizeof(MotNTLxS_ExecAtDistFuncs));

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
                    [sCurveIncreaseBasePointsForward.usNoOfBasePoints-1];
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
                    [sCurveIncreaseBasePointsBackward.usNoOfBasePoints-1];
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

    psMot->bAllowStateMachineChange = VC_TRUE;
    psMot->bExecAddToFunctions = VC_TRUE;

    MotNTLxS_RecalcFollowTimer((MOTNT*)psMot);

    MotNTOSSync_Create((MOTNT*)psMot);

    // Timer Init
    psMot->wakeups_missed = 0;

    iRet = pthread_create(&psMot->tid, NULL, &MotNTLxS_StepTimerThread, psMot);
    if (iRet != 0)
    {
        CV_HANDLE_STATUS_A(-1);
        return VC_FALSE;
    }
    else
    {
        struct sched_param param;
        param.sched_priority = 90;
        iRet = pthread_setschedparam(psMot->tid, SCHED_FIFO, &param);
        if (iRet != 0)
        {
            CV_HANDLE_STATUS_A(-1);
        }

        iRet = pthread_detach(psMot->tid);
        if (iRet)
        {
            CV_HANDLE_STATUS_A(-1); // wird eh nicht passieren, wenn vorangegangenes pthread_create erfolgreich war, was wahrscheinlich ist, wenn es 0 zurueckgab ;-)
        }
    }

    return VC_TRUE;
}


/*=========================================================================*/
static void MotNTLxS_TimerValueSet(MOTNTLXS* psMot, unsigned int uiTimerValue_us,
                            unsigned int uiLineNo)
{
    int iErr;
    unsigned int iNs;
    unsigned int iSec;
    struct itimerspec sItval;

    if (uiTimerValue_us == 0)
    {
        if (timer_gettime(psMot->timer_id, &sItval) == -1)
        {
            iErr = errno;
            CV_LOG_A_ERROR(("%u: %s\n", uiLineNo, strerror(iErr)));
            uiTimerValue_us = 1;
        }

        if ( (sItval.it_interval.tv_sec == 0) && (sItval.it_interval.tv_nsec == 0)
             && (sItval.it_value.tv_sec == 0) && (sItval.it_value.tv_nsec == 0) )
        {
            uiTimerValue_us = 1;
        }
    }

    if (uiTimerValue_us != 0)
    {
        iSec = uiTimerValue_us/1000000;
        iNs = (uiTimerValue_us - (iSec * 1000000)) * 1000;
        sItval.it_interval.tv_sec = iSec;
        sItval.it_interval.tv_nsec = iNs;
        sItval.it_value.tv_sec = iSec;
        sItval.it_value.tv_nsec = iNs;
    }
    if (timer_settime(psMot->timer_id, 0, &sItval, NULL) == -1)
    {
        iErr = errno;
        CV_LOG_A_ERROR(("%u: %s\n", uiLineNo, strerror(iErr)));
    }
}


/*=========================================================================*/
static void MotNTLxS_TimerValueStop(MOTNTLXS* psMot, unsigned int uiLineNo)
{
    int iErr;
    struct itimerspec sItval;
    memset((void*)&sItval, 0, sizeof(struct itimerspec));
    sItval.it_value.tv_sec = 0;
    sItval.it_value.tv_nsec = 0;
    if (timer_settime(psMot->timer_id, 0, &sItval, NULL) == -1)
    {
        iErr = errno;
        CV_LOG_A_ERROR(("%u: %s\n", uiLineNo, strerror(iErr)));
    }
}

/*=========================================================================*/
static VC_BOOL MotNTLxS_Start(MOTNT* psMotNT, VC_UINT16 ui16Speed,
                              VC_UINT16 ui16Dist, MOTNT_DIR sDir,
                              const char* pcFile, int iLine)
{
    return MotNTLxS_StartFlex(psMotNT, ui16Speed, ui16Dist, sDir,
                              VC_TRUE, NULL, pcFile, iLine);
}


/*=========================================================================*/
static VC_BOOL MotNTLxS_StartFlex(MOTNT* psMotNT, VC_UINT16 ui16Speed, VC_UINT16 ui16Dist,
                                  MOTNT_DIR sDir, VC_BOOL bExecAddToFunctions,
                                  MOTNT_STARTFLEX_PARM* psParm, const char* pcFile, int iLine)
{
    MOTNTLXS* psMot = (MOTNTLXS*)psMotNT;
    UNSIGNED uiActualEvents;
    unsigned int uiStatus;

    MOTNT_STARTFLEX_PARM sParm;
    VC_UINT16 ui16StartSpeed;
    MOTNT_SPEEDUNIT sSpeedUnit;
    MOTNT_DISTUNIT sDistUnit;
    VC_BOOL bAllowStartInFalseDir;

    unsigned int uiTimerValue_us;

    if (psParm == NULL)
    {
        MotNT_InitStartFlexParm(&sParm);
        psParm = &sParm;
    }

    ui16StartSpeed        = psParm->ui16StartSpeed;
    sSpeedUnit            = psParm->sSpeedUnit;
    sDistUnit             = psParm->sDistUnit;
    bAllowStartInFalseDir = psParm->bAllowStartInFalseDir;

    MotNTLxS_PauseStateMachineChange(psMot, VC_TRUE);

#ifdef VALENTIN
    // Event loeschen
    uiStatus =
            tx_event_flags_get(&psMot->sMotNTItf.sEvent, MOTNT_EVENT_STOPPED,
                               TX_OR_CLEAR, &uiActualEvents, TX_NO_WAIT);
    if ((uiStatus != TX_SUCCESS) && (uiStatus != TX_NO_EVENTS))
    {
        CV_HANDLE_STATUS_A(uiStatus);
    }
#endif

    //. Wenn Motor laeuft und Richtungswechsel: return Fehler
    if ( (psMot->sCurState != MOTNT_STATE_STOPPED)
         && (psMot->sMotNTItf.motDirection != sDir) )
    {
        if (!bAllowStartInFalseDir)
        {
            MotNTLxS_PauseStateMachineChange(psMot, VC_FALSE);
            return VC_FALSE;
        }
        else
        {
            // Stop
            if (!psMot->bImmediateRestartInOtherDir)
            {
                psMot->bDoStart = VC_FALSE;
                SetCondDecrease(psMot, 0xFFFF, VC_TRUE);
                psMot->bImmediateRestartInOtherDir = VC_TRUE;
            }
        }
    }
    else
    {
        psMot->bImmediateRestartInOtherDir = VC_FALSE;
    }

    if (!psMot->bImmediateRestartInOtherDir)
    {
        psMot->sMotNTItf.bIsRunning = VC_TRUE;
        psMot->sMotNTItf.motDirection = sDir;
    }

    if (ui16Speed > 0)
    {
        switch (sSpeedUnit)
        {
            case MOTNT_SPEEDUNIT_MM_S:
                psMot->ui16DoStart_SpeedInTimerVal
                    = MotNT_Speed2TimerVal((MOTNT*)psMot, ui16Speed);
                break;
            case MOTNT_SPEEDUNIT_TIMERVAL:
                psMot->ui16DoStart_SpeedInTimerVal = ui16Speed;
                break;
        }
    }
    else
    {
        psMot->ui16DoStart_SpeedInTimerVal = 0;
    }
    if (ui16Dist > 0)
    {
        switch (sDistUnit)
        {
            case MOTNT_DISTUNIT_MM:
                psMot->ui16DoStart_DistInSteps
                    = MotNT_Dist2Steps((MOTNT*)psMot, ui16Dist);
                break;
            case MOTNT_DISTUNIT_STEPS:
                psMot->ui16DoStart_DistInSteps = ui16Dist;
                break;
        }
    }
    else
    {
        psMot->ui16DoStart_DistInSteps = 0;
    }
    if (ui16StartSpeed > 0)
    {
        switch (sSpeedUnit)
        {
            case MOTNT_SPEEDUNIT_MM_S:
                psMot->ui16DoStart_StartSpeedInTimerVal
                    = MotNT_Speed2TimerVal((MOTNT*)psMot, ui16StartSpeed);
                break;
            case MOTNT_SPEEDUNIT_TIMERVAL:
                psMot->ui16DoStart_StartSpeedInTimerVal = ui16StartSpeed;
                break;
        }
    }
    else
    {
        psMot->ui16DoStart_StartSpeedInTimerVal = 0;
    }
    psMot->sDoStart_Dir = sDir;
    psMot->bExecAddToFunctions = bExecAddToFunctions;

    if (psMot->bImmediateRestartInOtherDir)
    {
        MotNTLxS_PauseStateMachineChange(psMot, VC_FALSE);

        if (pcFile != NULL)
        {
            CV_LOG(("%s, %i: StartF %s," VC_UINT32$ ",%u in other Dir! Speed:"
                    VC_UINT16$ "; Dist:" VC_UINT16$
                    "; Dir: %u\n", pcFile, iLine, psMot->sMotNTItf.pcName, psMot->sMotNTItf.ui32MotID,
                    psMot->sMotNTItf.eMotID, ui16Speed, ui16Dist, sDir));
        }
        else
        {
            CV_LOG(("StartF %s," VC_UINT32$ ",%u in other Dir! Speed:"
                    VC_UINT16$ "; Dist:" VC_UINT16$
                    "; Dir: %u\n", psMot->sMotNTItf.pcName, psMot->sMotNTItf.ui32MotID,
                    psMot->sMotNTItf.eMotID, ui16Speed, ui16Dist, sDir));
        }

        return VC_TRUE;
    }

    psMot->bDoStart = VC_TRUE;

    uiTimerValue_us = 0;
    if (psMot->sCurState == MOTNT_STATE_STOPPED)
    {
        // Da der Interrupt immer aktiv sein muesste (so zumindest der Plan)
        // ist das mehr oder weniger nur schoenes Beiwerk
        uiTimerValue_us = 1;
    }

    MotNTLxS_PauseStateMachineChange(psMot, VC_FALSE);

    if (!psMot->bDoNotStartInt)
    {
        psMot->bStopStateMachineExe = VC_FALSE;
        MotNTLxS_TimerValueSet(psMot, uiTimerValue_us, __LINE__);
    }

    if (pcFile != NULL)
    {
        CV_LOG(("%s, %i: StartF! %s," VC_UINT32$ ",%u Speed:" VC_UINT16$ "; Dist:"
                VC_UINT16$ "; Dir: %u\n",
                pcFile, iLine, psMot->sMotNTItf.pcName, psMot->sMotNTItf.ui32MotID,
                psMot->sMotNTItf.eMotID, ui16Speed, ui16Dist, sDir));
    }
    else
    {
        CV_LOG(("StartF! %s," VC_UINT32$ ",%u Speed:" VC_UINT16$ "; Dist:"
                VC_UINT16$ "; Dir: %u\n",
                psMot->sMotNTItf.pcName, psMot->sMotNTItf.ui32MotID,
                psMot->sMotNTItf.eMotID, ui16Speed, ui16Dist, sDir));
    }

    return VC_TRUE;
}

/*=========================================================================*/
static void
DoStart(MOTNTLXS* psMot, VC_UINT16 ui16SpeedInTimerVal,
        VC_UINT16 ui16DistInSteps, MOTNT_DIR sDir,
        VC_UINT16 ui16StartSpeedInTimerVal)
{
    VC_UINT16 ui16EndSpeedTimerVal;

    MotNTLxS_PauseStateMachineChange(psMot, VC_TRUE);

    if (sDir == MOTNT_DIR_FW)
    {
        psMot->psCurveObjCurInc = psMot->psCurveObjFwInc;
        psMot->psCurveObjCurDec = psMot->psCurveObjFwDec;
        psMot->pui32StepsDone =
                (VC_UINT32*)&psMot->sMotNTItf.ui32StepsDoneForward;
    }
    else
    {
        psMot->psCurveObjCurInc = psMot->psCurveObjBwInc;
        psMot->psCurveObjCurDec = psMot->psCurveObjBwDec;
        psMot->pui32StepsDone =
                (VC_UINT32*)&psMot->sMotNTItf.ui32StepsDoneBackward;
    }

    psMot->sMotNTItf.motDirection = sDir;

    psMot->sCurIncCurve = Curve_Get(psMot->psCurveObjCurInc);
    psMot->sCurDecCurve = Curve_Get(psMot->psCurveObjCurDec);

    if (psMot->sCurState == MOTNT_STATE_STOPPED)
    {
        psMot->uiStepTimerValue_us = 0xFFFF;
    }

    if (ui16SpeedInTimerVal == 0)
    {
        ui16EndSpeedTimerVal = MotNTLxS_GetFollowTimerVal4Mot(psMot);
    }
    else
    {
        ui16EndSpeedTimerVal = ui16SpeedInTimerVal;
    }

    // Stepping
    if (ui16DistInSteps > 0)
    {
        VC_UINT32 ui32Steps4Accel = 0;
        VC_UINT32 ui32Steps4Break = 0;
        VC_UINT32 ui32Steps4AccelCalc = 0;
        VC_UINT32 ui32S4Ai;
        VC_UINT32 ui32S4Bi;
        VC_UINT32 ui32S4BiOld = 0;
        unsigned int uiCurCurveTableIndex = 0;

        psMot->ui32StepsToDo = ui16DistInSteps;

        // StepsToDo um Rampen und StepsDone korrigieren
        if ( (ui16EndSpeedTimerVal > psMot->uiStepTimerValue_us)
             || (psMot->sCurState == MOTNT_STATE_STOPPED) )
        {
            SetCurveTabelIndexInc(psMot);
        }
        else
        {
            SetCurveTabelIndexDec(psMot);
        }

        if (psMot->psCurveObjCurInc != NULL)
        {
            ui32Steps4Accel =
                Curve_GetIndex(psMot->psCurveObjCurInc,
                               ui16EndSpeedTimerVal);
            if ( (ui16StartSpeedInTimerVal != 0xFFFF)
                 && (ui16StartSpeedInTimerVal != 0) )
            {
                ui32Steps4AccelCalc =
                    Curve_GetIndex(psMot->psCurveObjCurInc,
                                   ui16StartSpeedInTimerVal);
            }
        }
        if ( (ui16StartSpeedInTimerVal != 0xFFFF)
             && (ui16StartSpeedInTimerVal != 0) )
        {
            if (ui32Steps4Accel > ui32Steps4AccelCalc)
            {
                ui32Steps4Accel -= ui32Steps4AccelCalc;
            }
            else
            {
                ui32Steps4Accel = 0;
            }
        }
        if (psMot->psCurveObjCurDec != NULL)
        {
            ui32Steps4Break =
                Curve_GetIndex(psMot->psCurveObjCurDec,
                               ui16EndSpeedTimerVal) + 1; // Er braucht
            // 1 Step um in RunSpeed zu merken, dass er nach Decrease
            // wechseln muss
        }

        switch (psMot->sCurState)
        {
            case MOTNT_STATE_NO:
            case MOTNT_STATE_STOPPED:
                uiCurCurveTableIndex = 0;
                if ( (ui16StartSpeedInTimerVal != 0xFFFF)
                     && (ui16StartSpeedInTimerVal != 0) )
                {
                    if (psMot->psCurveObjCurInc != NULL)
                    {
                        uiCurCurveTableIndex =
                            Curve_GetIndex(psMot->psCurveObjCurInc,
                                           ui16StartSpeedInTimerVal);
                    }
                }
                ui32Steps4AccelCalc = ui32Steps4Accel;
                break;

            case MOTNT_STATE_DECREASE:
                if (psMot->psCurveObjCurInc != NULL)
                {
                    uiCurCurveTableIndex =
                        Curve_GetIndex(psMot->psCurveObjCurInc,
                                       psMot->uiStepTimerValue_us);
                }
                if (ui32Steps4Accel > uiCurCurveTableIndex)
                {
                    ui32Steps4AccelCalc = ui32Steps4Accel
                                          - uiCurCurveTableIndex;
                }
                else
                {
                    ui32Steps4AccelCalc = 0;
                }
                break;

            case MOTNT_STATE_RUNSPEED:
            case MOTNT_STATE_INCREASE:
                uiCurCurveTableIndex = psMot->uiCurCurveTableIndex;
                if (ui32Steps4Accel > uiCurCurveTableIndex)
                {
                    ui32Steps4AccelCalc = ui32Steps4Accel
                                          - uiCurCurveTableIndex;
                }
                else
                {
                    ui32Steps4AccelCalc = 0;
                }
                break;
        }

        if (psMot->ui32StepsToDo >= ui32Steps4Break + ui32Steps4AccelCalc)
        {
            psMot->ui32StepsToDo -= ui32Steps4Break;
        }
        else
        {
            for (ui32S4Ai=uiCurCurveTableIndex;
                 ui32S4Ai<ui32Steps4Accel; ui32S4Ai++)
            {
                if (psMot->psCurveObjCurDec != NULL)
                {
                    ui32S4Bi =
                        Curve_GetIndex(psMot->psCurveObjCurDec,
                                       psMot->sCurIncCurve.puiCurveTable
                                       [ui32S4Ai]) + 1; // Er braucht
                    // 1 Step um in RunSpeed zu merken, dass er nach Decrease
                    // wechseln muss
                }
                else {
                    ui32S4Bi = 0;
                }
                if (ui32S4Ai-uiCurCurveTableIndex + ui32S4Bi
                    == psMot->ui32StepsToDo)
                {
                    psMot->ui32StepsToDo -= ui32S4Bi;
                    ui16EndSpeedTimerVal =
                        psMot->sCurIncCurve.puiCurveTable[ui32S4Ai];
                    break;
                }
                else if (ui32S4Ai-uiCurCurveTableIndex + ui32S4Bi
                         > psMot->ui32StepsToDo)
                {
                    psMot->ui32StepsToDo -= ui32S4BiOld;
                    if (ui32S4Ai > 0) ui32S4Ai--;
                    ui16EndSpeedTimerVal =
                        psMot->sCurIncCurve.puiCurveTable[ui32S4Ai];
                    break;
                }
                ui32S4BiOld = ui32S4Bi;
            }

            if (ui32S4Ai >= ui32Steps4Accel)
            {
                if (psMot->ui32StepsToDo >= ui32Steps4Break)
                {
                    psMot->ui32StepsToDo -= ui32Steps4Break;
                }
                else
                {
                    psMot->ui32StepsToDo = 0;
                }
            }
        }

        psMot->ui32StepsToDo += *psMot->pui32StepsDone;
    }
    else
    {
        psMot->ui32StepsToDo = 0;
        if ( (psMot->sCurState == MOTNT_STATE_STOPPED)
             && (ui16StartSpeedInTimerVal != 0xFFFF)
             && (ui16StartSpeedInTimerVal != 0) )
        {
            SetCurveTabelIndexInc(psMot);
        }
    }

    if (ui16EndSpeedTimerVal > psMot->uiStepTimerValue_us)
    {
        SetCondDecrease(psMot, ui16EndSpeedTimerVal, VC_FALSE);
    }
    else
    {
        SetCondIncrease(psMot, ui16EndSpeedTimerVal);
    }

    MotNTLxS_PauseStateMachineChange(psMot, VC_FALSE);
}

/*=========================================================================*/
static void MotNTLxS_Stop(MOTNT* psMotNT, const char* pcFile, int iLine)
{
    MOTNTLXS* psMot = (MOTNTLXS*)psMotNT;

    if ( (psMot->bStopAfterDec)
         && (psMot->sCurState == MOTNT_STATE_DECREASE) )
    {
        if (pcFile != NULL)
        {
            CV_LOG(("%s, %i: Stop Unreal %s," VC_UINT32$ ",%u!\n", pcFile, iLine,
                    psMot->sMotNTItf.pcName, psMot->sMotNTItf.ui32MotID,
                    psMot->sMotNTItf.eMotID));
        }
        else
        {
            CV_LOG(("Stop Unreal %s," VC_UINT32$ ",%u!\n",
                    psMot->sMotNTItf.pcName, psMot->sMotNTItf.ui32MotID,
                    psMot->sMotNTItf.eMotID));
        }
        return;
    }

    MotNTLxS_PauseStateMachineChange(psMot, VC_TRUE);
    psMot->bDoStart = VC_FALSE;
    psMot->bDoStartFastInt = VC_FALSE;
    psMot->bImmediateRestartInOtherDir = VC_FALSE;
    SetCondDecrease(psMot, 0xFFFF, VC_TRUE);
    MotNTLxS_PauseStateMachineChange(psMot, VC_FALSE);

    if (pcFile != NULL)
    {
        CV_LOG(("%s, %i: Stop %s," VC_UINT32$ ",%u!\n", pcFile, iLine,
                psMot->sMotNTItf.pcName, psMot->sMotNTItf.ui32MotID,
                psMot->sMotNTItf.eMotID));
    }
    else
    {
        CV_LOG(("Stop %s," VC_UINT32$ ",%u!\n",
                psMot->sMotNTItf.pcName, psMot->sMotNTItf.ui32MotID,
                psMot->sMotNTItf.eMotID));
    }
}


/*=========================================================================*/
static unsigned int MotNTLxS_Speed2TimerVal(MOTNT* psMotNT,
                                            VC_UINT16 ui16Speed)
{
    MOTNTLXS* psMot = (MOTNTLXS*)psMotNT;

    if (ui16Speed == 0) return 0xFFFF;

    return (
        (1000 * 1000)
        * psMot->ui16FeedPer360Degs / (psMot->ui16Resolution * ui16Speed)
    );
}


/*=========================================================================*/
static VC_UINT16 MotNTLxS_TimerVal2Speed(MOTNT* psMotNT,
                                         unsigned int uiTimerVal)
{
    MOTNTLXS* psMot = (MOTNTLXS*)psMotNT;

    if (uiTimerVal == 0xFFFF)
        return 0;

    if (uiTimerVal == 0)
        return 0;

    return ( 1000
             * (1000 * psMot->ui16FeedPer360Degs)
             / (psMot->ui16Resolution * uiTimerVal)
    );
}


/*=========================================================================*/
static VC_UINT32 MotNTLxS_Dist2Steps(MOTNT* psMotNT, VC_UINT16 ui16Dist)
{
    MOTNTLXS* psMot = (MOTNTLXS*)psMotNT;

    return (ui16Dist * psMot->sMotNTItf.ui16Res)
            / psMot->sMotNTItf.ui16Mm360Deg;
}

/*=========================================================================*/
static VC_UINT32 MotNTLxS_Steps2Dist(MOTNT* psMotNT, VC_UINT32 ui32Steps)
{
    MOTNTLXS* psMot = (MOTNTLXS*)psMotNT;

    return (ui32Steps * psMot->sMotNTItf.ui16Mm360Deg)
            / psMot->sMotNTItf.ui16Res;
}

/*=========================================================================*/
static VC_UINT32 MotNTLxS_GetSteps2Stop(MOTNT* psMotNT)
{
    MOTNTLXS* psMot = (MOTNTLXS*)psMotNT;

    if ( (psMot->bStopAfterDec)
         && (psMot->sCurState == MOTNT_STATE_DECREASE) )
    {
        return psMot->uiCurCurveTableIndex + 1;
    }

    return 0;
}

/*=========================================================================*/
static VC_STATUS MotNTLxS_CurveObjCreate(MOTNT* psMotNT, CURVEOBJ* psCurveObj,
                                char* pcName, VC_BOOL bIncrease,
                                CURVE_BASE_POINTS* psCurveBasePoints)
{
    return Curve_Create(psCurveObj, pcName, bIncrease, psCurveBasePoints,
                        &MotNTLxS_GetNextX, &MotNTLxS_ChangeIdealY2TabelVal, 0,
                        (const char**)psMotNT, VC_TRUE, NULL);
}


/*=========================================================================*/
static void
MotNTLxS_CurveObjSet(MOTNT* psMotNT, CURVEOBJ* psCurveObj,
                     MOTNT_MOVETYPE sMoveType)
{
    MOTNTLXS* psMot = (MOTNTLXS*)psMotNT;

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
MotNTLxS_CurveObjGet(MOTNT* psMotNT, MOTNT_MOVETYPE sMoveType)
{
    MOTNTLXS* psMot = (MOTNTLXS*)psMotNT;

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


/*=========================================================================*/
/**
 * Liefert den bereits Motortimer gerecht umgerechneten Followtimerwert
 * zurueck.
*/
static VC_UINT16
MotNTLxS_GetFollowTimerVal4Mot(MOTNTLXS* psMot)
{
    VC_UINT16 ui16FollowTimerVal;

    if (psMot->GetFollowTimerValue == NULL) return 0xFFFF;

    ui16FollowTimerVal = psMot->GetFollowTimerValue(
            psMot->pvFollowTimerValObj);

    if ( ui16FollowTimerVal >= psMot->ui16FollowTimerMinSpeedVal )
    {
        if (ui16FollowTimerVal == 0xFFFF)
        {
            return 0xFFFF;
        }
        return ((U32)(psMot->ui16FollowTimerMinSpeedVal) *
                psMot->ui32FollowTimerFac) >> 16;
    }
    return (ui16FollowTimerVal * psMot->ui32FollowTimerFac) >> 16;

}

/*=========================================================================*/
static VC_BOOL
MotNTLxS_ExecAtDist(MOTNT* psMotNT, VC_UINT32 ui32Dist, MOTNT_DISTUNIT sDistUnit,
                     VC_BOOL bRelative, MOTNT_DIR sDir4Relative, void
                     (*ExecAtDistFunc)(MOTNT* psMot, int argc, const char* argv[]),
                     int argc, const char* argv[])
{
    MOTNTLXS* psMot = (MOTNTLXS*)psMotNT;
    unsigned int ui;
    VC_UINT32 ui32CurDistC;

    for (ui=0; ui<MOTNTLXS_NO_OF_ADD_TO_FUNCS; ui++)
    {
        if (psMot->pExecAtDistFuncs[ui] == NULL)
        {
            psMot->pExecAtDistFuncs[ui] = ExecAtDistFunc;
            psMot->pExecAtDistFuncsArgc[ui] = argc;
            psMot->pppExecAtDistFuncsArgv[ui] = argv;
            if (sDistUnit == MOTNT_DISTUNIT_MM)
            {
                ui32Dist = MotNTLxS_Dist2Steps(psMotNT, ui32Dist);
            }
            if (!bRelative)
            {
                ui32CurDistC = MotNT_GetDist(psMotNT, MOTNT_DISTUNIT_STEPS);
                if (ui32Dist >= ui32CurDistC)
                {
                    sDir4Relative = MOTNT_DIR_FW;
                    ui32Dist = ui32Dist - ui32CurDistC;
                }
                else
                {
                    sDir4Relative = MOTNT_DIR_BW;
                    ui32Dist = ui32CurDistC - ui32Dist;
                }
            }
            if (sDir4Relative == MOTNT_DIR_FW)
            {
                psMot->pui32Dist4ExecAtDistBw[ui] = 0;
                psMot->pui32Dist4ExecAtDistFw[ui]
                    = psMotNT->ui32StepsDone + ui32Dist;
            }
            if (sDir4Relative == MOTNT_DIR_BW)
            {
                psMot->pui32Dist4ExecAtDistFw[ui] = 0xFFFFFFFF;
                psMot->pui32Dist4ExecAtDistBw[ui]
                    = psMotNT->ui32StepsDone - ui32Dist;
            }

            psMot->bExecAtDistTablesUpdated = VC_TRUE;

            return VC_TRUE;
        }
    }
    return VC_FALSE;
}

/*=========================================================================*/
static VC_BOOL
MotNTLxS_AddToStep(MOTNT* psMotNT, void
(*AddToStepFunc)(struct SMOTNT* psMotNT, int argc, const char* argv[]),
                    int argc, const char* argv[])
{
    MOTNTLXS* psMot = (MOTNTLXS*)psMotNT;

    if (psMot->ui8NoOfAddToStepFuncs == MOTNTLXS_NO_OF_ADD_TO_FUNCS)
    {
        return VC_FALSE;
    }

    psMot->pAddToStepFuncs[psMot->ui8NoOfAddToStepFuncs]
        = AddToStepFunc;
    psMot->pAddToStepFuncsArgc[psMot->ui8NoOfAddToStepFuncs]
        = argc;
    psMot->pppAddToStepFuncsArgv[psMot->ui8NoOfAddToStepFuncs]
        = argv;

    psMot->ui8NoOfAddToStepFuncs++;

    return VC_TRUE;
}


/*=========================================================================*/
static VC_BOOL
MotNTLxS_AddToStateSwitch(MOTNT* psMotNT,
       void (*AddToStateSwitchFunc)(struct SMOTNT* psMotNT, MOTNT_STATE sCurState,
                               MOTNT_STATE sNextState, VC_BOOL bStopAfterDec,
                                       int argc, const char* argv[]),
       int argc, const char* argv[])
{
    MOTNTLXS* psMot = (MOTNTLXS*)psMotNT;

    if (psMot->ui8NoOfAddToStateSwitchFuncs == MOTNTLXS_NO_OF_ADD_TO_FUNCS)
    {
        return VC_FALSE;
    }

    psMot->pAddToStateSwitchFuncs[psMot->ui8NoOfAddToStateSwitchFuncs]
        = AddToStateSwitchFunc;
    psMot->pAddToStateSwitchFuncsArgc[psMot->ui8NoOfAddToStateSwitchFuncs]
        = argc;
    psMot->pppAddToStateSwitchFuncsArgv[psMot->ui8NoOfAddToStateSwitchFuncs]
       = argv;

    psMot->ui8NoOfAddToStateSwitchFuncs++;

    return VC_TRUE;
}


/*=========================================================================*/
static VC_BOOL MotNTLxS_AddToStop(MOTNT* psMotNT,
                            void (*AddToStopFunc)(
                                   MOTNT* psMot,
                                   int argc, const char* argv[]),
                            int argc, const char* argv[])
{
    MOTNTLXS* psMot = (MOTNTLXS*)psMotNT;

    if (psMot->ui8NoOfAddToStopFuncs == MOTNTLXS_NO_OF_ADD_TO_FUNCS)
    {
        return VC_FALSE;
    }

    psMot->pAddToStopFuncs[psMot->ui8NoOfAddToStopFuncs]
        = AddToStopFunc;
    psMot->pAddToStopFuncsArgc[psMot->ui8NoOfAddToStopFuncs]
        = argc;
    psMot->pppAddToStopFuncsArgv[psMot->ui8NoOfAddToStopFuncs]
        = argv;

    psMot->ui8NoOfAddToStopFuncs++;

    return VC_TRUE;
}

/*=========================================================================*/
static VC_UINT32
MotNTLxS_GetPositionFromMotor(MOTNT* psMotNT)
{
    MOTNTLXS* psMot = (MOTNTLXS*)psMotNT;
    return psMot->sMotNTItf.ui32StepsDone;
}

/*=========================================================================*/
static void
MotNTLxS_StopAtPosition(struct SMOTNT* psMotNT, unsigned int position)
{
    MOTNT_STARTFLEX_PARM sParm;

    MotNT_InitStartFlexParm(&sParm);
    sParm.sDistUnit  = MOTNT_DISTUNIT_STEPS;

    MotNT_StartFlex(psMotNT, MotNT_GetCurSpeed(psMotNT, MOTNT_SPEEDUNIT_MM_S),
                    position-MotNT_GetDist(psMotNT, MOTNT_DISTUNIT_STEPS),
                    MotNT_GetCurDir(psMotNT), VC_TRUE, &sParm);
}

/*=========================================================================*/
static VC_BOOL
MotNTLxS_SetPositionCallback(struct SMOTNT* psMotNT, unsigned int uiPosition, void* pvCbParms)
{

#ifdef VCHALGEN5_SUPPORT
    MOTNTLXS* psMot = (MOTNTLXS*)psMotNT;
    unsigned int ui;
    VC_UINT32 ui32CurDistC;
    VC_UINT32 ui32Dist = uiPosition;
    MOTNT_DIR sDir4Relative;

    for (ui=0; ui<MOTNTLXS_NO_OF_ADD_TO_FUNCS; ui++)
    {
        if (psMot->pExecAtDistFuncs[ui] == NULL)
        {
            psMot->pExecAtDistFuncs[ui] = (void*)VCHALGen5DeviceMotors_callbackPosition;
            psMot->pppExecAtDistFuncsArgv[ui] = (const char**)pvCbParms;

            ui32CurDistC = MotNT_GetDist(psMotNT, MOTNT_DISTUNIT_STEPS);
            if (ui32Dist >= ui32CurDistC)
            {
                sDir4Relative = MOTNT_DIR_FW;
                ui32Dist = ui32Dist - ui32CurDistC;
            }
            else
            {
                sDir4Relative = MOTNT_DIR_BW;
                ui32Dist = ui32CurDistC - ui32Dist;
            }

            if (sDir4Relative == MOTNT_DIR_FW)
            {
                psMot->pui32Dist4ExecAtDistBw[ui] = 0;
                psMot->pui32Dist4ExecAtDistFw[ui]
                    = psMotNT->ui32StepsDone + ui32Dist;
            }
            if (sDir4Relative == MOTNT_DIR_BW)
            {
                psMot->pui32Dist4ExecAtDistFw[ui] = 0xFFFFFFFF;
                psMot->pui32Dist4ExecAtDistBw[ui]
                    = psMotNT->ui32StepsDone - ui32Dist;
            }

            psMot->bExecAtDistTablesUpdated = VC_TRUE;

            return VC_TRUE;
        }
    }
#endif

    return VC_FALSE;
}

/*=========================================================================*/
static VC_BOOL
MotNTLxS_ClearPositionCallbacks(struct SMOTNT* psMotNT){

#ifdef VCHALGEN5_SUPPORT
    MOTNTLXS* psMot = (MOTNTLXS*)psMotNT;
    unsigned int ui;

    for (ui=0; ui<MOTNTLXS_NO_OF_ADD_TO_FUNCS; ui++)
    {
        if (psMot->pExecAtDistFuncs[ui] == (void*)VCHALGen5DeviceMotors_callbackPosition)
        {
            psMot->pExecAtDistFuncs[ui] = NULL;
            psMot->pppExecAtDistFuncsArgv[ui] = NULL; // muss ggf. in uebergeordneter Instanz geloescht werden

            psMot->pui32Dist4ExecAtDistBw[ui] = 0;
            psMot->pui32Dist4ExecAtDistFw[ui] = 0;

            psMot->bExecAtDistTablesUpdated = VC_TRUE;
        }
    }

    return(VC_TRUE);
#endif

    return VC_FALSE;
}

/*=========================================================================*/
static void
SwitchState(MOTNTLXS* psMot, MOTNT_STATE sNextState)
{
    VC_UINT8 ui8;
#ifdef VALENTIN
    CVL_LEVEL sCvlLevel;

    if (CV_LOG_ISACTIVATED())
    {
        CV_LOG_GETLEVEL(&sCvlLevel);
        if (sCvlLevel == CVL_MORE)
        {
            switch (sNextState)
            {
                case MOTNT_STATE_STOPPED:
                    CV_LOG_MORE(("MOTNT_STATE_STOPPED\n"))
                    ;
                    break;
                case MOTNT_STATE_INCREASE:
                    CV_LOG_MORE(("MOTNT_STATE_INCREASE\n"))
                    ;
                    break;
                case MOTNT_STATE_RUNSPEED:
                    CV_LOG_MORE(("MOTNT_STATE_RUNSPEED\n"))
                    ;
                    break;
                case MOTNT_STATE_DECREASE:
                    CV_LOG_MORE(("MOTNT_STATE_DECREASE\n"))
                    ;
                    break;
                case MOTNT_STATE_NO:
                    CV_LOG_MORE(("MOTNT_NO_STATE\n"))
                    ;
                    break;
            }
            CV_LOG_MORE(("TimerVal: %hu; Steps: %lu\n", psMot->uiStepTimerValue_us,
                         abs(psMot->sMotNTItf.ui32StepsDoneForward
                                 - psMot->sMotNTItf.ui32StepsDoneBackward) ));
        }
    }
#endif

    if (psMot->AddToStateSwitchNative != NULL)
    {
        psMot->AddToStateSwitchNative((MOTNT*)psMot,
                (MOTNT_STATE)psMot->sCurState,
                (MOTNT_STATE)sNextState,
                psMot->bStopAfterDec,
                psMot->iArgcAddToState,
                psMot->ppcArgvAddToState);
    }

    // AddToStateSwitch
    if (psMot->bExecAddToFunctions)
    {
        for (ui8=0; ui8<psMot->ui8NoOfAddToStateSwitchFuncs; ui8++)
        {
            psMot->pAddToStateSwitchFuncs[ui8]((MOTNT*)psMot,
                    (MOTNT_STATE)psMot->sCurState,
                    (MOTNT_STATE)sNextState,
                    psMot->bStopAfterDec,
                    psMot->pAddToStateSwitchFuncsArgc[ui8],
                    psMot->pppAddToStateSwitchFuncsArgv[ui8]);
        }
    }

    psMot->sCurState = sNextState;
}


/*=========================================================================*/
static void
SetCondIncrease(MOTNTLXS* psMot, U16 ui16EndSpeedTimerVal)
{
    MotNTLxS_PauseStateMachineChange(psMot, VC_TRUE);
    psMot->ui16DoCondEndSpeedTimerVal = ui16EndSpeedTimerVal;
    psMot->sCurCond = MOTNTLXS_COND_INC;
    MotNTLxS_PauseStateMachineChange(psMot, VC_FALSE);
}

/*=========================================================================*/
static void
DoCondIncrease(MOTNTLXS* psMot, U16 ui16EndSpeedTimerVal)
{
    MOTNT_STATE sNextState;

    MotNTLxS_PauseStateMachineChange(psMot, VC_TRUE);

    sNextState = lk_sStateChart[psMot->sCurState][MOTNTLXS_COND_INC];

    if (sNextState == MOTNT_STATE_NO)
    {
        MotNTLxS_PauseStateMachineChange(psMot, VC_FALSE);
        return;
    }

    SetCurveTabelIndexInc(psMot);

    psMot->uiEndSpeedTimerVal_us = ui16EndSpeedTimerVal;
    SwitchState(psMot, sNextState);

    MotNTLxS_PauseStateMachineChange(psMot, VC_FALSE);
}


/*=========================================================================*/
static void
SetCondDecrease(MOTNTLXS* psMot, U16 ui16EndSpeedTimerVal,
                            VC_BOOL bStopAfterDec)
{
    MotNTLxS_PauseStateMachineChange(psMot, VC_TRUE);
    psMot->bDoCondStopAfterDec = bStopAfterDec;
    psMot->ui16DoCondEndSpeedTimerVal = ui16EndSpeedTimerVal;
    psMot->sCurCond = MOTNTLXS_COND_DEC;
    MotNTLxS_PauseStateMachineChange(psMot, VC_FALSE);
}

/*=========================================================================*/
static void
DoCondDecrease(MOTNTLXS* psMot, U16 ui16EndSpeedTimerVal,
               VC_BOOL bStopAfterDec)
{
    MOTNT_STATE sNextState;

    MotNTLxS_PauseStateMachineChange(psMot, VC_TRUE);

    sNextState = lk_sStateChart[psMot->sCurState][MOTNTLXS_COND_DEC];

    if (sNextState == MOTNT_STATE_NO)
    {
        MotNTLxS_PauseStateMachineChange(psMot, VC_FALSE);
        return;
    }

    SetCurveTabelIndexDec(psMot);

    psMot->uiEndSpeedTimerVal_us = ui16EndSpeedTimerVal;
    psMot->bStopAfterDec = bStopAfterDec;
    SwitchState(psMot, sNextState);

    MotNTLxS_PauseStateMachineChange(psMot, VC_FALSE);
}


/*=========================================================================*/
static void SetCondEndSpeedReached(MOTNTLXS* psMot)
{
    MOTNT_STATE sNextState;

    MotNTLxS_PauseStateMachineChange(psMot, VC_TRUE);

    sNextState =
            lk_sStateChart[psMot->sCurState][MOTNTLXS_COND_ENDSPEED];

    if (sNextState == MOTNT_STATE_NO)
    {
        MotNTLxS_PauseStateMachineChange(psMot, VC_FALSE);
        return;
    }

    psMot->uiStepTimerValue_us = psMot->uiEndSpeedTimerVal_us;

    SwitchState(psMot, sNextState);

    MotNTLxS_PauseStateMachineChange(psMot, VC_FALSE);
}


/*=========================================================================*/
static void SetCondStopSpeedReached(MOTNTLXS* psMot)
{
    MOTNT_STATE sNextState;

    MotNTLxS_PauseStateMachineChange(psMot, VC_TRUE);

    sNextState =
            lk_sStateChart[psMot->sCurState][MOTNTLXS_COND_STOPSPEED];

    if (sNextState == MOTNT_STATE_NO)
    {
        MotNTLxS_PauseStateMachineChange(psMot, VC_FALSE);
        return;
    }

    MotNTLxS_TimerValueStop(psMot, __LINE__);
    psMot->bStopStateMachineExe = VC_TRUE;

    psMot->sMotNTItf.bIsRunning = VC_FALSE;

    SwitchState(psMot, sNextState);

    MotNTLxS_PauseStateMachineChange(psMot, VC_FALSE);
}


/*=========================================================================*/
static void MotNTLxS_PauseStateMachineChange(MOTNTLXS* psMot, VC_BOOL bPause)
{
    if (bPause)
    {
        psMot->bAllowStateMachineChange = VC_FALSE;
        psMot->uiDoStateMachinePauseCounter++;
    }
    else
    {
        psMot->uiDoStateMachinePauseCounter--;
        if (psMot->uiDoStateMachinePauseCounter == 0)
        {
            psMot->bAllowStateMachineChange = VC_TRUE;
        }
    }
}

/*=========================================================================*/
static inline void
MotNTLxS_DoExecAtDist(MOTNTLXS* psMot)
{
    unsigned int ui;

    for (ui=0; ui<MOTNTLXS_NO_OF_ADD_TO_FUNCS; ui++)
    {
        if (psMot->sMotNTItf.motDirection == MOTNT_DIR_FW)
        {
            if (psMot->pExecAtDistFuncs[ui] != NULL)
            {
                if (psMot->sMotNTItf.ui32StepsDone >= psMot->pui32Dist4ExecAtDistFw[ui])
                {
                    psMot->pExecAtDistFuncs[ui]((MOTNT*)psMot,
                            psMot->pExecAtDistFuncsArgc[ui],
                            psMot->pppExecAtDistFuncsArgv[ui]);
                    psMot->pExecAtDistFuncs[ui] = NULL;
                    psMot->bExecAtDistTablesUpdated = VC_TRUE;
                }
            }
        }

        if (psMot->sMotNTItf.motDirection == MOTNT_DIR_BW)
        {
            if (psMot->pExecAtDistFuncs[ui] != NULL)
            {
                if (psMot->sMotNTItf.ui32StepsDone <= psMot->pui32Dist4ExecAtDistBw[ui])
                {
                    psMot->pExecAtDistFuncs[ui]((MOTNT*)psMot,
                            psMot->pExecAtDistFuncsArgc[ui],
                            psMot->pppExecAtDistFuncsArgv[ui]);
                    psMot->pExecAtDistFuncs[ui] = NULL;
                    psMot->bExecAtDistTablesUpdated = VC_TRUE;
                }
            }
        }
    }
}

/*=========================================================================*/
static inline void
MotNTLxS_ExecAtDistUpdate(MOTNTLXS* psMot)
{
    unsigned int ui;

    psMot->bExecAtDistTablesUpdated = VC_FALSE;

    psMot->ui32NextDist4ExecAtDistFw = 0xFFFFFFFF;
    psMot->ui32NextDist4ExecAtDistBw = 0xFFFFFFFF;
    for (ui=0; ui<MOTNTLXS_NO_OF_ADD_TO_FUNCS; ui++)
    {
        if (psMot->pExecAtDistFuncs[ui] != NULL)
        {
            if (psMot->pui32Dist4ExecAtDistFw[ui]
                < psMot->ui32NextDist4ExecAtDistFw)
            {
                psMot->ui32NextDist4ExecAtDistFw
                    = psMot->pui32Dist4ExecAtDistFw[ui];
            }
            if (psMot->pui32Dist4ExecAtDistBw[ui]
                > psMot->ui32NextDist4ExecAtDistBw)
            {
                psMot->ui32NextDist4ExecAtDistBw
                    = psMot->pui32Dist4ExecAtDistBw[ui];
            }
        }
    }
}

/*=========================================================================*/
static void* MotNTLxS_StepTimerThread(void* pvObject)
{
    int sig;
    MOTNTLXS* psMot = (MOTNTLXS*)pvObject;

    while (1)
    {
        sigwait (&(psMot->alarm_sig), &sig);
        psMot->wakeups_missed += timer_getoverrun (psMot->timer_id);

        MotNTLxS_StateMachineExe(psMot);
    }

    return NULL;
}


/*=========================================================================*/
static void MotNTLxS_StateMachineExe(MOTNTLXS* psMot)
{
    VC_BOOL bNeedSMExeProntoAgain = VC_TRUE;
    VC_UINT8 ui8;
    VC_BOOL bRestartInOtherDir = VC_FALSE;

    if (psMot == NULL) return;

    if ( (! (psMot->bDoStart && psMot->bAllowStateMachineChange ) )
         && (! ((psMot->bAllowStateMachineChange)
                && (psMot->sCurState == MOTNT_STATE_DECREASE)) )
       )
    {
        bNeedSMExeProntoAgain = VC_FALSE;
    }

    if (!psMot->bDisableStateMachine)
    {
        //. Interrupthandling
        if (psMot->bAllowStateMachineChange)
        {
            if (psMot->bFollowTimerNewCalc)
            {
                psMot->ui16FollowTimerMinSpeedVal =
                    psMot->ui16FollowTimerMinSpeedValNewCalc;
                psMot->ui32FollowTimerFac =
                    psMot->ui32FollowTimerFacNewCalc;
                psMot->bFollowTimerNewCalc = VC_FALSE;
            }
        }

        switch (psMot->sCurState)
        {
            case MOTNT_STATE_NO:
            case MOTNT_STATE_STOPPED:
                if (psMot->bAllowStateMachineChange)
                {
                    MotNTLxS_TimerValueStop(psMot, __LINE__);
                    if (psMot->bStopStateMachineExe) return;
                    psMot->bStopStateMachineExe = VC_TRUE;
                }
                break;

            case MOTNT_STATE_INCREASE:
                if (psMot->uiCurCurveTableIndex
                    >= psMot->sCurIncCurve.uiCurveTableSize-1)
                {
                    psMot->uiCurCurveTableIndex
                        = psMot->sCurIncCurve.uiCurveTableSize-1;
                    psMot->uiStepTimerValue_us =
                        psMot->sCurIncCurve.puiCurveTable
                            [psMot->uiCurCurveTableIndex];
                    if (psMot->bAllowStateMachineChange)
                    {
                        SetCondEndSpeedReached(psMot);
                    }
                }
                else
                {
                    psMot->uiStepTimerValue_us =
                        psMot->sCurIncCurve.puiCurveTable
                            [psMot->uiCurCurveTableIndex++];
                }

                if (psMot->uiStepTimerValue_us <= psMot->uiEndSpeedTimerVal_us)
                {
                    psMot->uiStepTimerValue_us = psMot->uiEndSpeedTimerVal_us;
                    if (psMot->bAllowStateMachineChange)
                    {
                        SetCondEndSpeedReached(psMot);
                    }
                }

                if (psMot->ui32StepsToDo > 0)
                {
                    if (*psMot->pui32StepsDone >= psMot->ui32StepsToDo)
                    {
                        SetCondDecrease(psMot, 0xFFFF, VC_TRUE);
                    }
                }
                break;

            case MOTNT_STATE_RUNSPEED:
                if (psMot->ui32StepsToDo > 0)
                {
                    if (*psMot->pui32StepsDone >= psMot->ui32StepsToDo)
                    {
                        if (psMot->bAllowStateMachineChange)
                        {
                            SetCondDecrease(psMot, 0xFFFF, VC_TRUE);
                        }
                    }
                }
                if (psMot->ui16DoStart_SpeedInTimerVal == 0)
                {
                    psMot->uiStepTimerValue_us =
                        MotNTLxS_GetFollowTimerVal4Mot(psMot);
                }
                break;

            case MOTNT_STATE_DECREASE:
#ifdef VALENTIN
                if (psMot->sMotNTItf.ui8DbgMode > 0)
                {
                    CVLog_Out4Measure_On(CVLOG_OUT4MEASURE_CUT1);
                }
#endif

                if (psMot->uiCurCurveTableIndex <= 0)
                {
                    psMot->uiCurCurveTableIndex = 0;
                    psMot->uiStepTimerValue_us =
                        psMot->sCurDecCurve.puiCurveTable
                            [psMot->uiCurCurveTableIndex];
                    if (psMot->bStopAfterDec)
                    {
                        if (psMot->bAllowStateMachineChange)
                        {
                            SetCondStopSpeedReached(psMot);
                            if (psMot->bImmediateRestartInOtherDir)
                            {
                                bRestartInOtherDir = VC_TRUE;
                            }
                            else
                            {
                                bNeedSMExeProntoAgain = VC_FALSE;
                            }
                        }
                    }
                    else
                    {
                        if (psMot->bAllowStateMachineChange)
                        {
                            bNeedSMExeProntoAgain = VC_FALSE;
                            SetCondEndSpeedReached(psMot);
                        }
                    }
                }
                else
                {
                    bNeedSMExeProntoAgain = VC_FALSE;
                    psMot->uiStepTimerValue_us =
                        psMot->sCurDecCurve.puiCurveTable
                            [psMot->uiCurCurveTableIndex--];
                }

                if (psMot->uiStepTimerValue_us >= psMot->uiEndSpeedTimerVal_us)
                {
                    psMot->uiStepTimerValue_us = psMot->uiEndSpeedTimerVal_us;
                    if (psMot->bAllowStateMachineChange)
                    {
                        if (psMot->bStopAfterDec)
                        {
                            SetCondStopSpeedReached(psMot);
                            if (psMot->bImmediateRestartInOtherDir)
                            {
                                bRestartInOtherDir = VC_TRUE;
                            }
                        }
                        else
                        {
                            bNeedSMExeProntoAgain = VC_FALSE;
                            SetCondEndSpeedReached(psMot);
                        }
                    }
                }
                break;
        }

        if (psMot->bAllowStateMachineChange)
        {
            if (psMot->bDoStart)
            {
                DoStart(psMot, psMot->ui16DoStart_SpeedInTimerVal,
                        psMot->ui16DoStart_DistInSteps, psMot->sDoStart_Dir,
                        psMot->ui16DoStart_StartSpeedInTimerVal);
            }
            if ( (psMot->bDoStart) || (psMot->bDoStartFastInt) )
            {
                MotNTLxS_TimerValueSet(psMot, psMot->uiStepTimerValue_us, __LINE__);
                psMot->bStopStateMachineExe = VC_FALSE;
            }

            switch (psMot->sCurCond)
            {
                case MOTNTLXS_COND_INC:
                    DoCondIncrease(psMot, psMot->ui16DoCondEndSpeedTimerVal);
                    psMot->sCurCond = MOTNTLXS_COND_NO;
                    break;

                case MOTNTLXS_COND_ENDSPEED:
                    psMot->sCurCond = MOTNTLXS_COND_NO;
                    break;

                case MOTNTLXS_COND_DEC:
                    DoCondDecrease(psMot, psMot->ui16DoCondEndSpeedTimerVal,
                                   psMot->bDoCondStopAfterDec);
                    psMot->sCurCond = MOTNTLXS_COND_NO;
                    break;

                case MOTNTLXS_COND_STOPSPEED:
                    psMot->sCurCond = MOTNTLXS_COND_NO;
                    break;

                case MOTNTLXS_COND_NO:
                    break;
            }

            if (psMot->bDoStartFastInt)
            {
                psMot->bDoStartFastInt = VC_FALSE;
            }
            if (psMot->bDoStart)
            {
                psMot->bDoStart = VC_FALSE;
                MotNTLxS_TimerValueSet(psMot, 1, __LINE__);
                return;
            }

            if (bRestartInOtherDir)
            {
                psMot->bDoStart = VC_TRUE;
                psMot->sMotNTItf.motDirection = psMot->sDoStart_Dir;
                MotNTLxS_TimerValueSet(psMot, psMot->uiStepTimerValue_us, __LINE__);
                psMot->bStopStateMachineExe = VC_FALSE;
            }
        }
    }

    if (!psMot->bStopStateMachineExe)
    {
        if (psMot->uiStepTimerValue_us != 0xFFFF)
        {
            if (psMot->DoStepNative != NULL)
            {
                psMot->DoStepNative((MOTNT*)psMot, psMot->iArgcDoStepNative, psMot->ppcArgvDoStepNative);
            }

            // AddToStep
            if (psMot->bExecAddToFunctions)
            {
                for (ui8=0; ui8<psMot->ui8NoOfAddToStepFuncs; ui8++)
                {
                    psMot->pAddToStepFuncs[ui8]((MOTNT*)psMot, psMot->pAddToStepFuncsArgc[ui8],
                                               psMot->pppAddToStepFuncsArgv[ui8]);
                }
            }

            // ExecAtDist
            if (psMot->sMotNTItf.motDirection == MOTNT_DIR_FW)
            {
                if (psMot->sMotNTItf.ui32StepsDone >= psMot->ui32NextDist4ExecAtDistFw)
                {
                    MotNTLxS_DoExecAtDist(psMot);
                }
            }
            else
            {
                if (psMot->sMotNTItf.ui32StepsDone <= psMot->ui32NextDist4ExecAtDistBw)
                {
                    MotNTLxS_DoExecAtDist(psMot);
                }
            }

            if (psMot->bExecAtDistTablesUpdated)
            {
                MotNTLxS_ExecAtDistUpdate(psMot);
            }

            *psMot->pui32StepsDone = *psMot->pui32StepsDone + 1;
            if (psMot->sMotNTItf.motDirection == MOTNT_DIR_FW)
            {
                psMot->sMotNTItf.ui32StepsDone++;
            }
            else
            {
                psMot->sMotNTItf.ui32StepsDone--;
            }
        }
    }

    if (psMot->sMotNTItf.bDbgTimerVals)
    {
        psMot->sMotNTItf.pui16DbgMem[(*psMot->sMotNTItf.puiDbgMemIndex)++]
            = psMot->uiCurCurveTableIndex;
        psMot->sMotNTItf.pui16DbgMem[(*psMot->sMotNTItf.puiDbgMemIndex)++]
            = psMot->uiStepTimerValue_us;

        if (*psMot->sMotNTItf.puiDbgMemIndex >= psMot->sMotNTItf.uiDbgMemSize)
        {
            *psMot->sMotNTItf.puiDbgMemIndex = 0;
        }
    }

    if (bNeedSMExeProntoAgain)
    {
        MotNTLxS_TimerValueSet(psMot, 1, __LINE__);
    }

    if (psMot->bStopStateMachineExe)
    {
        psMot->sMotNTItf.bIsRunning = VC_FALSE;
        if (psMot->bExecAddToFunctions)
        {
            for (ui8=0; ui8<psMot->ui8NoOfAddToStopFuncs; ui8++)
            {
#ifdef VCHALGEN5_SUPPORT
                if (psMot->pAddToStopFuncs[ui8] == VCHALGen5DeviceMotors_callbackMotorStopped) {
                    psMot->pAddToStopFuncsArgc[ui8] = psMot->sMotNTItf.ui32StepsDone;
                }
#endif
                psMot->pAddToStopFuncs[ui8]((MOTNT*)psMot, psMot->pAddToStopFuncsArgc[ui8],
                                           psMot->pppAddToStopFuncsArgv[ui8]);
            }
        }

#ifdef VALENTIN
        CV_HANDLE_STATUS_A_F( tx_event_flags_set(&psMot->sMotNTItf.sEvent,
                                                 MOTNT_EVENT_STOPPED, TX_OR) );
#endif
    }
    else
    {
        if (!bNeedSMExeProntoAgain)
        {
            MotNTLxS_TimerValueSet(psMot, psMot->uiStepTimerValue_us, __LINE__);
        }
    }
}

#endif // LINUX_SUPPORT
