/***************************************************************************/
/*                             Printerfirmware                             */
/*                    Copyright (C) Carl Valentin GmbH                     */
/*                       http://www.carl-valentin.de                       */
/*                  This code is licenced under the LGPL                   */
/***************************************************************************/

#ifndef _VC_MOTNT_H_
#define _VC_MOTNT_H_

/*. IMPORT ================================================================*/

#include "../vccomponents_intern.h"
#include "../Utilities/curve.h"

/*. ENDIMPORT =============================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*. EXPORT ================================================================*/

/*-------------------------------- Makros ---------------------------------*/

#define MOTNT_ID 0x4F98

#define MOTNT_EVENT_STOPPED 0x00000001

/*--------------------------- Typdeklarationen ----------------------------*/

typedef enum
{
    MOTNT_DIR_FW,
    MOTNT_DIR_BW
} MOTNT_DIR;

typedef enum
{
    MOTNT_STATE_STOPPED,
    MOTNT_STATE_INCREASE,
    MOTNT_STATE_RUNSPEED,
    MOTNT_STATE_DECREASE,

    MOTNT_STATE_NO,
} MOTNT_STATE;

typedef enum
{
    MOTNT_INF_MAX_SPEED,
    MOTNT_INF_MIN_SPEED,
    MOTNT_INF_STEPS_DONE,
    MOTNT_INF_STEPS_2STOP,
    MOTNT_INF_DIST_TODO_COMPLETE,
    MOTNT_INF_SPEED_TODO
} MOTNT_INF;

typedef enum
{
    MOTNT_SPEEDUNIT_MM_S,
    MOTNT_SPEEDUNIT_TIMERVAL
} MOTNT_SPEEDUNIT;

typedef enum
{
    MOTNT_DISTUNIT_MM,
    MOTNT_DISTUNIT_STEPS
} MOTNT_DISTUNIT;

typedef enum
{
    MOTNT_MOVETYPE_FWINC,
    MOTNT_MOVETYPE_FWDEC,
    MOTNT_MOVETYPE_BWINC,
    MOTNT_MOVETYPE_BWDEC
} MOTNT_MOVETYPE;

typedef enum
{
    MOTNT_FOLLOWMODE_SIMPLE,
    MOTNT_FOLLOWMODE_RAMP,
    MOTNT_FOLLOWMODE_RAMP_2NDSTEP_ONLY
} MOTNT_FOLLOWMODE;

typedef struct
{
    CURVEOBJ* psCurveObjFwInc;
    CURVEOBJ* psCurveObjFwDec;
    CURVEOBJ* psCurveObjBwInc;
    CURVEOBJ* psCurveObjBwDec;
} MOTNT_CURVES;

typedef struct
{
    CURVEOBJ sCurveObjFwInc;
    CURVEOBJ sCurveObjFwDec;
    CURVEOBJ sCurveObjBwInc;
    CURVEOBJ sCurveObjBwDec;
    char* pcNameFwInc;
    char* pcNameFwDec;
    char* pcNameBwInc;
    char* pcNameBwDec;
} MOTNT_CURVES_CONTAINER;

typedef struct
{
    VC_UINT16 ui16StartSpeed;
    MOTNT_SPEEDUNIT sSpeedUnit;
    MOTNT_DISTUNIT sDistUnit;
    VC_BOOL bAllowStartInFalseDir;
    VC_BOOL bAlwaysFollowMode;
    VC_UINT16 ui16SpeedToDo;
} MOTNT_STARTFLEX_PARM;

typedef enum
{
    MOTNT_MOTID_0,
    MOTNT_MOTID_1,
    MOTNT_MOTID_2,
    MOTNT_MOTID_3,
    MOTNT_MOTID_4,
    MOTNT_MOTID_5,
    MOTNT_MOTID_6,
    MOTNT_MOTID_7,
    MOTNT_MOTID_8,
    MOTNT_MOTID_9
} MOTNT_MOTID;

typedef enum
{
    MOTNT_EVENT_RECALC_CURVES,
    MOTNT_EVENT_RESETDISTCOUNTER
} MOTNT_EVENT;

typedef struct SMOTNT
{
    /* Private Common */
    VC_BOOL
    (*Start)(struct SMOTNT* psMot, VC_UINT16 ui16Speed, VC_UINT16 ui16Dist,
             MOTNT_DIR sDir, const char* pcFile, int iLine);
    VC_BOOL
    (*StartFlex)(struct SMOTNT* psMot, VC_UINT16 ui16Speed, VC_UINT16 ui16Dist,
             MOTNT_DIR sDir, VC_BOOL bExecAddToFunctions,
             MOTNT_STARTFLEX_PARM* psParm, const char* pcFile, int iLine);
    void
    (*Stop)(struct SMOTNT* psMot, const char* pcFile, int iLine);
    void
    (*StopAdv)(struct SMOTNT* psMot, VC_BOOL bDoCondDecImmediate,
               const char* pcFile, int iLine);

    VC_UINT16
    (*GetCurSpeed)(struct SMOTNT* psMot, MOTNT_SPEEDUNIT sSpeedUnit);
    MOTNT_STATE
    (*GetCurState)(struct SMOTNT* psMot);
    VC_UINT32
    (*GetSteps2Stop)(struct SMOTNT* psMot);

    unsigned int
    (*Speed2TimerVal)(struct SMOTNT* psMot, VC_UINT16 ui16Speed);
    VC_UINT16
    (*TimerVal2Speed)(struct SMOTNT* psMot, unsigned int uiTimerVal);
    VC_UINT32
    (*Dist2Steps)(struct SMOTNT* psMot, VC_UINT16 ui16Dist);
    VC_UINT32
    (*Steps2Dist)(struct SMOTNT* psMot, VC_UINT32 ui32Steps);
    unsigned int
    (*TimerVal2Time)(struct SMOTNT* psMot, unsigned int uiTimerVal,
                     unsigned int uiFac);
    unsigned int
    (*Time2TimerVal)(struct SMOTNT* psMot, unsigned int uiTimer,
                     unsigned int uiFac);

    VC_BOOL
    (*ExecAtDist)(struct SMOTNT* psMotNT, VC_UINT32 ui32Dist, MOTNT_DISTUNIT sDistUnit,
                     VC_BOOL bRelative, MOTNT_DIR sDir4Relative, void
                     (*ExecAtDistFunc)(struct SMOTNT* psMotNT, int argc, const char* argv[]),
                     int argc, const char* argv[]);
    VC_BOOL
    (*AddToStep)(struct SMOTNT* psMotNT, void
    (*AddToStepFunc)(struct SMOTNT* psMotNT, int argc, const char* argv[]),
                 int argc, const char* argv[]);
    VC_BOOL
    (*AddToStepDelete)(struct SMOTNT* psMotNT, void
    (*AddToStepFunc)(struct SMOTNT* psMotNT, int argc, const char* argv[]));
    VC_BOOL
    (*AddToStateSwitch)(
            struct SMOTNT* psMotNT,
            void
            (*AddToStateSwitchFunc)(struct SMOTNT* psMotNT,
                                    MOTNT_STATE sCurState,
                                    MOTNT_STATE sNextState,
                                    VC_BOOL bStopAfterDec, int argc,
                                    const char* argv[]),
            int argc, const char* argv[]);
    VC_BOOL
    (*AddToStop)(struct SMOTNT* psMotNT, void
    (*AddToStopFunc)(struct SMOTNT* psMotNT, int argc, const char* argv[]),
                 int argc, const char* argv[]);
    VC_UINT32
    (*GetDistCounter)(struct SMOTNT* psMotNT); /* optional */
    void
    (*ResetDistCounter)(struct SMOTNT* psMotNT); /* optional */
    void
    (*RecalcFollowTimer)(struct SMOTNT* psMotNT); /* optional */
    VC_STATUS
    (*SetRes)(struct SMOTNT* psMotNT, VC_UINT16 ui16Res); /* optional */
    VC_STATUS
    (*SetMm360Deg)(struct SMOTNT* psMotNT, VC_UINT16 ui16Mm360Deg); /* optional */

    VC_STATUS
    (*CurveObjCreate)(struct SMOTNT* psMotNT, CURVEOBJ* psCurveObj,
                      char* pcName, VC_BOOL bIncrease,
                      CURVE_BASE_POINTS* psCurveBasePoints);
    void
    (*CurveObjSet)(struct SMOTNT* psMotNT, CURVEOBJ* psCurveObj,
                   MOTNT_MOVETYPE sMoveType);
    CURVEOBJ*
    (*CurveObjGet)(struct SMOTNT* psMotNT, MOTNT_MOVETYPE sMoveType);

    void (*MotEvent)(struct SMOTNT* psMot, MOTNT_EVENT eMotEvent);

    VC_UINT32 ui32StepsDoneForward;
    VC_UINT32 ui32StepsDoneBackward;
    VC_UINT32 ui32StepsDone;

    VC_BOOL bIsRunning;
    MOTNT_DIR motDirection;
    VC_UINT16 ui16DistToDoComplete;
    VC_UINT16 ui16SpeedToDo;

    VC_UINT16* pui16DbgMem;
    unsigned int* puiDbgMemIndex;
    unsigned int uiDbgMemSize;
    VC_BOOL  bDbgTimerVals;
    VC_UINT8 ui8DbgMode;
    VC_BOOL  bOutpDbgInfAtStop;
    VC_BOOL  bOutpDbgInfSerialInsteadLog;
    void
    (*Debug)(struct SMOTNT* psMotNT);
    void
    (*DebugOutp)(struct SMOTNT* psMotNT);

    TX_SEMAPHORE sSemaStartStop;
#ifdef VALENTIN
    TX_EVENT_FLAGS_GROUP sEvent;
#endif

    const VC_UINT16 ui16ID;

    /* Common Init */
    char* pcName;
    VC_UINT32 ui32MotID; // Just 4 Info
    MOTNT_MOTID eMotID;  // Debug

    VC_UINT16 ui16Res;
    VC_UINT16 ui16Mm360Deg;
    VC_BOOL bFollowUseRamp;
    MOTNT_FOLLOWMODE eFollowMode;

    VC_UINT16 ui16MaxSpeedForward;
    VC_UINT16 ui16MinSpeedForward;
    VC_UINT16 ui16MaxSpeedBackward;
    VC_UINT16 ui16MinSpeedBackward;

    void ( * startCallBack ) ( MOTNT_DIR );

    void
    (*SetMotorPosition)(struct SMOTNT* psMotNT, unsigned int position);
    VC_UINT32
    (*GetPositionFromMotor)(struct SMOTNT* psMotNT);
    void
    (*StopAtPosition)(struct SMOTNT* psMotNT, unsigned int position);
    VC_BOOL
    (*SetPositionCallback)(struct SMOTNT* psMotNT, unsigned int uiPosition, void* pCbParms);
    VC_BOOL
    (*ClearPositionCallbacks)(struct SMOTNT* psMotNT);
} MOTNT;

/*------------------------------ Prototypen -------------------------------*/

/*------------------------ Konstantendeklarationen ------------------------*/

/*. ENDEXPORT =============================================================*/

#ifdef PJDEBUG

    #define MotNT_Start(MOT, SPEED, DIST, DIR) \
            MotNT_StartReal(MOT, SPEED, DIST, DIR, __FILE__, __LINE__)

    #define MotNT_StartFlex(MOT, SPEED, DIST, DIR, EXECADDTOFUNC, PARM) \
            MotNT_StartFlexReal(MOT, SPEED, DIST, DIR, EXECADDTOFUNC, PARM, \
                                __FILE__, __LINE__)

    #define MotNT_Stop(MOT) \
            MotNT_StopReal(MOT, __FILE__, __LINE__)

    #define MotNT_StopAdv(MOT, DODECIMMEDIATE) \
            MotNT_StopAdvReal(MOT, DODECIMMEDIATE, __FILE__, __LINE__)

    #define MotNTOSSync_Start(MOT, SPEED, DIST, DIR) \
            MotNTOSSync_StartReal(MOT, SPEED, DIST, DIR, __FILE__, __LINE__)

    #define MotNTOSSync_StartFlex(MOT, SPEED, DIST, DIR, EXECADDTOFUNC, PARM) \
            MotNTOSSync_StartFlexReal(MOT, SPEED, DIST, DIR, EXECADDTOFUNC, PARM, \
                                      __FILE__, __LINE__)

    #define MotNTOSSync_Stop(MOT) \
            MotNTOSSync_StopReal(MOT, __FILE__, __LINE__)

#else

    #define MotNT_Start(MOT, SPEED, DIST, DIR) \
            MotNT_StartReal(MOT, SPEED, DIST, DIR, NULL, 0)

    #define MotNT_StartFlex(MOT, SPEED, DIST, DIR, EXECADDTOFUNC, PARM) \
            MotNT_StartFlexReal(MOT, SPEED, DIST, DIR, EXECADDTOFUNC, PARM, NULL, 0)

    #define MotNT_Stop(MOT) \
            MotNT_StopReal(MOT, NULL, 0)

    #define MotNT_StopAdv(MOT, DODECIMMEDIATE) \
            MotNT_StopAdvReal(MOT, DODECIMMEDIATE, NULL, 0)

    #define MotNTOSSync_Start(MOT, SPEED, DIST, DIR) \
            MotNTOSSync_StartReal(MOT, SPEED, DIST, DIR, NULL, 0)

    #define MotNTOSSync_StartFlex(MOT, SPEED, DIST, DIR, EXECADDTOFUNC, PARM) \
            MotNTOSSync_StartFlexReal(MOT, SPEED, DIST, DIR, EXECADDTOFUNC, PARM, NULL, 0)

    #define MotNTOSSync_Stop(MOT) \
            MotNTOSSync_StopReal(MOT, NULL, 0)

#endif

void
MotNTOSSync_Create(MOTNT* psMot); // Nur interne Benutzung

VC_BOOL
MotNTOSSync_StartReal(MOTNT* psMot, VC_UINT16 ui16Speed, VC_UINT16 ui16Dist,
                  MOTNT_DIR sDir, const char* pcFile, int iLine);
VC_BOOL
MotNTOSSync_StartFlexReal(MOTNT* psMot, VC_UINT16 ui16Speed, VC_UINT16 ui16Dist,
                      MOTNT_DIR sDir, VC_BOOL bExecAddToFunctions,
                      MOTNT_STARTFLEX_PARM* psParm, const char* pcFile, int iLine);
void
MotNTOSSync_StopReal(MOTNT* psMot, const char* pcFile, int iLine);
VC_STATUS
MotNTOSSync_WaitTillStopped(MOTNT* psMot);
VC_STATUS
MotNTOSSync_WaitTillStoppedTimeout(MOTNT* psMot, unsigned int uiTimeout);

#if (!defined AVOID_INLINE) || (defined MOTNT_CFILE_THAT_DOES_THE_IMPLEMENT)

/*=========================================================================*/
VC_INLINE void
MotNT_InitStartFlexParm(MOTNT_STARTFLEX_PARM* psParm)
{
    if (psParm == NULL) return;

    psParm->ui16StartSpeed = 0;
    psParm->ui16SpeedToDo = 0;
    psParm->sSpeedUnit = MOTNT_SPEEDUNIT_MM_S;
    psParm->sDistUnit = MOTNT_DISTUNIT_MM;
    psParm->bAllowStartInFalseDir = VC_FALSE;
    psParm->bAlwaysFollowMode = VC_FALSE;
}

/*=========================================================================*/
VC_INLINE VC_BOOL
MotNT_StartReal(MOTNT* psMot, VC_UINT16 ui16Speed, VC_UINT16 ui16Dist,
                MOTNT_DIR sDir, const char* pcFile, int iLine)
{
    if ((psMot != NULL) && (psMot->Start != NULL))
    {
        psMot->ui16DistToDoComplete = ui16Dist;
        psMot->ui16SpeedToDo = ui16Speed;
        if (psMot->startCallBack != NULL)
        {
            psMot->startCallBack(sDir);
        }
        return psMot->Start(psMot, ui16Speed, ui16Dist, sDir, pcFile, iLine);
    }

    return VC_FALSE;
}

/*=========================================================================*/
/**
 * \param ui16StartSpeed: Bei 0 wird Wert ignoriert und MinSpeed verwendet
 */
VC_INLINE VC_BOOL
MotNT_StartFlexReal(MOTNT* psMot, VC_UINT16 ui16Speed, VC_UINT16 ui16Dist,
                MOTNT_DIR sDir, VC_BOOL bExecAddToFunctions,
                MOTNT_STARTFLEX_PARM* psParm, const char* pcFile, int iLine)
{
    MOTNT_STARTFLEX_PARM sParm;
    if ((psMot != NULL) && (psMot->StartFlex != NULL))
    {
        if (psParm == NULL)
        {
            psParm = &sParm;
            MotNT_InitStartFlexParm(psParm);
        }

        psMot->ui16DistToDoComplete = ui16Dist;
        psMot->ui16SpeedToDo = ui16Speed;
        if (psParm->ui16SpeedToDo != 0)
        {
            psMot->ui16SpeedToDo = psParm->ui16SpeedToDo;
        }
        if (psMot->startCallBack != NULL)
        {
            psMot->startCallBack(sDir);
        }
        return psMot->StartFlex(psMot, ui16Speed, ui16Dist, sDir,
                                bExecAddToFunctions, psParm, pcFile, iLine);
    }

    return VC_FALSE;
}

/*=========================================================================*/
VC_INLINE void
MotNT_StopReal(MOTNT* psMot, const char* pcFile, int iLine)
{
    if ((psMot != NULL) && (psMot->Stop != NULL))
    {
        return psMot->Stop(psMot, pcFile, iLine);
    }
}

/*=========================================================================*/
VC_INLINE void
MotNT_StopAdvReal(MOTNT* psMot, VC_BOOL bDoCondDecImmediate, const char* pcFile, int iLine)
{
    if ((psMot != NULL) && (psMot->StopAdv != NULL))
    {
        return psMot->StopAdv(psMot, bDoCondDecImmediate, pcFile, iLine);
    }
}

/*=========================================================================*/
VC_INLINE VC_BOOL
MotNT_IsRunning(MOTNT* psMot)
{
    if (psMot != NULL)
    {
        return *(volatile VC_BOOL*)&(psMot->bIsRunning);
    }

    return VC_FALSE;
}

/*=========================================================================*/
VC_INLINE VC_UINT16
MotNT_GetCurSpeed(MOTNT* psMot, MOTNT_SPEEDUNIT sSpeedUnit)
{
    if (psMot != NULL)
    {
        if (!psMot->bIsRunning)
        {
            if (sSpeedUnit == MOTNT_SPEEDUNIT_MM_S)
            {
                return 0;
            }
            else
            {
                return 0xFFFF;
            }
        }
        return psMot->GetCurSpeed(psMot, sSpeedUnit);
    }

    return 0;
}

/*=========================================================================*/
VC_INLINE MOTNT_DIR
MotNT_GetCurDir(MOTNT* psMot)
{
    if (psMot != NULL)
    {
        return psMot->motDirection;
    }

    return MOTNT_DIR_FW;
}

/*=========================================================================*/
VC_INLINE MOTNT_STATE
MotNT_GetCurState(MOTNT* psMot)
{
    if ((psMot != NULL) && (psMot->GetCurState != NULL))
    {
        return psMot->GetCurState(psMot);
    }

    return MOTNT_STATE_STOPPED;
}

/*=========================================================================*/
VC_INLINE VC_UINT32
MotNT_GetInf(MOTNT* psMot, MOTNT_INF sInf, MOTNT_DIR sDir)
{
    if (psMot != NULL)
    {
        switch (sInf)
        {
            case MOTNT_INF_MAX_SPEED:
                if (sDir == MOTNT_DIR_FW)
                {
                    return psMot->ui16MaxSpeedForward;
                }
                else
                {
                    return psMot->ui16MaxSpeedBackward;
                }
                break;

            case MOTNT_INF_MIN_SPEED:
                if (sDir == MOTNT_DIR_FW)
                {
                    return psMot->ui16MinSpeedForward;
                }
                else
                {
                    return psMot->ui16MinSpeedBackward;
                }
                break;

            case MOTNT_INF_STEPS_DONE:
                if (sDir == MOTNT_DIR_FW)
                {
                    return psMot->ui32StepsDoneForward;
                }
                else
                {
                    return psMot->ui32StepsDoneBackward;
                }
                break;

            case MOTNT_INF_DIST_TODO_COMPLETE:
                return (VC_UINT32)psMot->ui16DistToDoComplete;
                break;

            case MOTNT_INF_SPEED_TODO:
                return (VC_UINT32)psMot->ui16SpeedToDo;
                break;

            case MOTNT_INF_STEPS_2STOP:
                if ((psMot != NULL) && (psMot->GetSteps2Stop != NULL))
                {
                    return psMot->GetSteps2Stop(psMot);
                }
                break;
        }
    }

    return 0;
}

/*=========================================================================*/
VC_INLINE VC_STATUS
MotNT_CurveObjCreate(MOTNT* psMot, CURVEOBJ* psCurveObj, char* pcName,
                     VC_BOOL bIncrease, CURVE_BASE_POINTS* psCurveBasePoints)
{
    if ((psMot != NULL) && (psMot->CurveObjCreate != NULL))
    {
        return psMot->CurveObjCreate(psMot, psCurveObj, pcName, bIncrease,
                psCurveBasePoints);
    }

    return VC_ERROR;
}

/*=========================================================================*/
VC_INLINE void
MotNT_CurveObjSet(MOTNT* psMot, CURVEOBJ* psCurveObj,
                  MOTNT_MOVETYPE sMoveType)
{
    if ((psMot != NULL) && (psMot->CurveObjSet != NULL))
    {
        psMot->CurveObjSet(psMot, psCurveObj, sMoveType);
    }
}

/*=========================================================================*/
VC_INLINE CURVEOBJ*
MotNT_CurveObjGet(MOTNT* psMot, MOTNT_MOVETYPE sMoveType)
{
    if ((psMot != NULL) && (psMot->CurveObjGet != NULL))
    {
        return psMot->CurveObjGet(psMot, sMoveType);
    }

    return NULL;
}

/*=========================================================================*/
VC_INLINE CURVE
MotNT_CurveGet(MOTNT* psMot, MOTNT_MOVETYPE sMoveType)
{
    CURVEOBJ* psCurveObj;
    CURVE sCurve =
        {NULL, 0, 0};

    if ((psMot != NULL) && (psMot->CurveObjGet != NULL))
    {
        psCurveObj = psMot->CurveObjGet(psMot, sMoveType);
        sCurve = Curve_Get(psCurveObj);
        return sCurve;
    }

    return sCurve;
}

/*=========================================================================*/
VC_INLINE unsigned int
MotNT_Speed2TimerVal(MOTNT* psMot, VC_UINT16 ui16Speed)
{
    if ((psMot != NULL) && (psMot->Speed2TimerVal != NULL))
    {
        return psMot->Speed2TimerVal(psMot, ui16Speed);
    }

    return 0;
}

/*=========================================================================*/
VC_INLINE VC_UINT16
MotNT_TimerVal2Speed(MOTNT* psMot, unsigned int uiTimerVal)
{
    if ((psMot != NULL) && (psMot->TimerVal2Speed != NULL))
    {
        return psMot->TimerVal2Speed(psMot, uiTimerVal);
    }

    return 0;
}

/*=========================================================================*/
VC_INLINE VC_UINT32
MotNT_Dist2Steps(MOTNT* psMot, VC_UINT16 ui16Dist)
{
    if ((psMot != NULL) && (psMot->Dist2Steps != NULL))
    {
        return psMot->Dist2Steps(psMot, ui16Dist);
    }

    return 0;
}

/*=========================================================================*/
VC_INLINE VC_UINT32
MotNT_Steps2Dist(MOTNT* psMot, VC_UINT32 ui32Steps)
{
    if ((psMot != NULL) && (psMot->Steps2Dist != NULL))
    {
        return psMot->Steps2Dist(psMot, ui32Steps);
    }

    return 0;
}

/*=========================================================================*/
/**
 * uiFac = 1: Zeit in ms
 * uiFac = 1000: Zeit in us
 */
VC_INLINE unsigned int
MotNT_TimerVal2Time(MOTNT* psMot, unsigned int uiTimerVal, unsigned int uiFac)
{
    if ((psMot != NULL) && (psMot->TimerVal2Time != NULL))
    {
        return psMot->TimerVal2Time(psMot, uiTimerVal, uiFac);
    }

    return 0;
}

/*=========================================================================*/
VC_INLINE unsigned int
MotNT_Time2TimerVal(MOTNT* psMot, unsigned int uiTime, unsigned int uiFac)
{
    if ((psMot != NULL) && (psMot->Time2TimerVal != NULL))
    {
        return psMot->Time2TimerVal(psMot, uiTime, uiFac);
    }

    return 0;
}

/*=========================================================================*/
VC_INLINE VC_BOOL
MotNT_ExecAtDist(MOTNT* psMot, VC_UINT32 ui32Dist, MOTNT_DISTUNIT sDistUnit,
                 VC_BOOL bRelative, MOTNT_DIR sDir4Relative, void
                 (*ExecAtDistFunc)(MOTNT* psMot, int argc, const char* argv[]),
                 int argc, const char* argv[])
{
    if ((psMot != NULL) && (psMot->ExecAtDist != NULL))
    {
        return psMot->ExecAtDist(psMot, ui32Dist, sDistUnit, bRelative,
                                 sDir4Relative, ExecAtDistFunc, argc, argv);
    }

    return VC_FALSE;
}

/*=========================================================================*/
VC_INLINE VC_BOOL
MotNT_AddToStep(MOTNT* psMot, void
                (*AddToStepFunc)(MOTNT* psMot, int argc, const char* argv[]),
                int argc, const char* argv[])
{
    if ((psMot != NULL) && (psMot->AddToStep != NULL))
    {
        return psMot->AddToStep(psMot, AddToStepFunc, argc, argv);
    }

    return VC_FALSE;
}

/*=========================================================================*/
VC_INLINE VC_BOOL
MotNT_AddToStepDelete(MOTNT* psMot, void
                (*AddToStepFunc)(MOTNT* psMot, int argc, const char* argv[]))
{
    if ((psMot != NULL) && (psMot->AddToStepDelete != NULL))
    {
        return psMot->AddToStepDelete(psMot, AddToStepFunc);
    }

    return VC_FALSE;
}

/*=========================================================================*/
VC_INLINE VC_BOOL
MotNT_AddToStateSwitch(
        MOTNT* psMot,
        void
        (*AddToStateSwitchFunc)(MOTNT* psMot, MOTNT_STATE sCurState,
                                MOTNT_STATE sNextState, VC_BOOL bStopAfterDec,
                                int argc, const char* argv[]),
        int argc, const char* argv[])
{
    if ((psMot != NULL) && (psMot->AddToStateSwitch != NULL))
    {
        return psMot->AddToStateSwitch(psMot, AddToStateSwitchFunc, argc,
                argv);
    }

    return VC_FALSE;
}

/*=========================================================================*/
VC_INLINE VC_BOOL
MotNT_AddToStop(MOTNT* psMot, void
                (*AddToStopFunc)(MOTNT* psMot, int argc, const char* argv[]),
                int argc, const char* argv[])
{
    if ((psMot != NULL) && (psMot->AddToStop != NULL))
    {
        return psMot->AddToStop(psMot, AddToStopFunc, argc, argv);
    }

    return VC_FALSE;
}

/*=========================================================================*/
VC_INLINE VC_BOOL
MotNT_RegEventCallback(MOTNT* psMot,
                       void (*MotEvent)(MOTNT* psMot, MOTNT_EVENT eMotEvent))
{
    if (psMot->MotEvent != NULL) return VC_FALSE;
    psMot->MotEvent = MotEvent;
    return VC_TRUE;
}

/*=========================================================================*/
VC_INLINE VC_UINT32
MotNT_GetDist(MOTNT* psMot, MOTNT_DISTUNIT sDistUnit)
{
    VC_UINT32 ui32Fw;
    VC_UINT32 ui32Bw;
    VC_UINT32 ui32Dist;
    VC_UINT32 ui32Dist_mm;

    if (psMot == NULL) return 0;

    if (psMot->GetDistCounter != NULL)
    {
        ui32Dist = psMot->GetDistCounter(psMot);
    }
    else
    {
        ui32Fw = MotNT_GetInf(psMot, MOTNT_INF_STEPS_DONE, MOTNT_DIR_FW);
        ui32Bw = MotNT_GetInf(psMot, MOTNT_INF_STEPS_DONE, MOTNT_DIR_BW);
        if (ui32Fw > ui32Bw)
            ui32Dist = ui32Fw - ui32Bw;
        else
            ui32Dist = ui32Bw - ui32Fw;
    }

    switch (sDistUnit)
    {
        case MOTNT_DISTUNIT_MM:
            ui32Dist_mm = MotNT_Steps2Dist(psMot, ui32Dist);
            return ui32Dist_mm;
            break;

        case MOTNT_DISTUNIT_STEPS:
            return ui32Dist;
            break;
    }

    return 0;
}

/*=========================================================================*/
VC_INLINE void
MotNT_ResetDistCounter(MOTNT* psMot)
{
    if (psMot == NULL) return;

    psMot->ui32StepsDoneBackward = 0;
    psMot->ui32StepsDoneForward = 0;
    psMot->ui32StepsDone = 0;

    if (psMot->MotEvent != NULL)
    {
        psMot->MotEvent((MOTNT*)psMot, MOTNT_EVENT_RESETDISTCOUNTER);
    }

    if ((psMot->ResetDistCounter != NULL))
    {
        return psMot->ResetDistCounter(psMot);
    }
}

/*=========================================================================*/
VC_INLINE VC_UINT16
MotNT_GetRes(MOTNT* psMot)
{
    if (psMot == NULL) return 0;
    return psMot->ui16Res;
}

/*=========================================================================*/
VC_INLINE VC_STATUS
MotNT_SetRes(MOTNT* psMot, VC_UINT16 ui16Res)
{
    if ((psMot != NULL) && (psMot->SetRes != NULL))
    {
        return psMot->SetRes(psMot, ui16Res);
    }
    return VC_ERROR;
}

/*=========================================================================*/
VC_INLINE VC_UINT16
MotNT_GetMm360Deg(MOTNT* psMot)
{
    if (psMot == NULL) return 0;
    return psMot->ui16Mm360Deg;
}

/*=========================================================================*/
VC_INLINE VC_STATUS
MotNT_SetMm360Deg(MOTNT* psMot, VC_UINT16 ui16Mm360Deg)
{
    if ((psMot != NULL) && (psMot->SetMm360Deg != NULL))
    {
        return psMot->SetMm360Deg(psMot, ui16Mm360Deg);
    }
    return VC_ERROR;
}

/*=========================================================================*/
/**
 * Diese Funktion sollte immer niedriger priorisiert sein wie der Motor-Int
 */
VC_INLINE void
MotNT_RecalcFollowTimer(MOTNT* psMot)
{
    if ((psMot != NULL) && (psMot->RecalcFollowTimer != NULL))
    {
        psMot->RecalcFollowTimer(psMot);
    }
}

/*=========================================================================*/
VC_INLINE VC_STATUS
MotNT_FollowModeSet(MOTNT* psMot, MOTNT_FOLLOWMODE eFollowMode)
{
    if (psMot == NULL) return VC_NULLPTR;
    psMot->eFollowMode = eFollowMode;
    return VC_SUCCESS;
}

/*=========================================================================*/
VC_INLINE MOTNT_FOLLOWMODE
MotNT_FollowModeGet(MOTNT* psMot)
{
    if (psMot == NULL) return MOTNT_FOLLOWMODE_SIMPLE;
    return psMot->eFollowMode;
}

/*=========================================================================*/
VC_INLINE void
MotNT_DebugSet(MOTNT* psMot, VC_UINT16* pui16DbgMem,
               unsigned int* puiDbgMemIndex, unsigned int uiDbgMemSize,
               VC_BOOL bDbgTimerVals, VC_UINT8 ui8DbgMode,
               VC_BOOL bOutpDbgInfAtStop, VC_BOOL bOutpDbgInfSerialInsteadLog)
{
    if (psMot != NULL)
    {
        psMot->pui16DbgMem = pui16DbgMem;
        psMot->puiDbgMemIndex = puiDbgMemIndex;
        psMot->uiDbgMemSize = uiDbgMemSize;
        psMot->bDbgTimerVals = bDbgTimerVals;
        psMot->ui8DbgMode = ui8DbgMode;
        psMot->bOutpDbgInfAtStop = bOutpDbgInfAtStop;
        psMot->bOutpDbgInfSerialInsteadLog = bOutpDbgInfSerialInsteadLog;

        if (psMot->Debug != NULL)
        {
            psMot->Debug(psMot);
        }
    }
}

/*=========================================================================*/
VC_INLINE void
MotNT_DebugGet(MOTNT* psMot, unsigned int* puiDbgMemSize,
               VC_BOOL* pbDbgTimerVals, VC_UINT8* pui8DbgMode,
               VC_BOOL* pbOutpDbgInfAtStop, VC_BOOL* pbOutpDbgInfSerialInsteadLog)
{
    if (psMot != NULL)
    {
        if (puiDbgMemSize != NULL) *puiDbgMemSize = psMot->uiDbgMemSize;
        if (pbDbgTimerVals != NULL) *pbDbgTimerVals = psMot->bDbgTimerVals;
        if (pui8DbgMode != NULL) *pui8DbgMode = psMot->ui8DbgMode;
        if (pbOutpDbgInfAtStop != NULL)
            *pbOutpDbgInfAtStop = psMot->bOutpDbgInfAtStop;
        if (pbOutpDbgInfSerialInsteadLog != NULL)
            *pbOutpDbgInfSerialInsteadLog = psMot->bOutpDbgInfSerialInsteadLog;
    }
}

/*=========================================================================*/
VC_INLINE void
MotNT_DebugOutp(MOTNT* psMot)
{
    if (psMot->DebugOutp != NULL)
    {
        psMot->DebugOutp(psMot);
    }
}

#else

void
MotNT_InitStartFlexParm(MOTNT_STARTFLEX_PARM* psParm);
VC_BOOL
MotNT_StartReal(MOTNT* psMot, VC_UINT16 ui16Speed,
                VC_UINT16 ui16Dist, MOTNT_DIR sDir, const char* pcFile, int iLine);
VC_BOOL
MotNT_StartFlexReal(MOTNT* psMot, VC_UINT16 ui16Speed, VC_UINT16 ui16Dist,
                MOTNT_DIR sDir, VC_BOOL bExecAddToFunctions,
                MOTNT_STARTFLEX_PARM* psParm, const char* pcFile, int iLine);
void
MotNT_StopReal(MOTNT* psMot, const char* pcFile, int iLine);
void
MotNT_StopAdvReal(MOTNT* psMot, VC_BOOL bDoCondDecImmediate, const char* pcFile, int iLine);

VC_BOOL
MotNT_IsRunning(MOTNT* psMot);
VC_UINT16
MotNT_GetCurSpeed(MOTNT* psMot, MOTNT_SPEEDUNIT sSpeedUnit);
MOTNT_DIR
MotNT_GetCurDir(MOTNT* psMot);
MOTNT_STATE
MotNT_GetCurState(MOTNT* psMot);
VC_UINT32
MotNT_GetInf(MOTNT* psMot, MOTNT_INF sInf, MOTNT_DIR sDir);

VC_STATUS
MotNT_CurveObjCreate(MOTNT* psMot, CURVEOBJ* psCurveObj,
                     char* pcName, VC_BOOL bIncrease,
                     CURVE_BASE_POINTS* psCurveBasePoints);
void
MotNT_CurveObjSet(MOTNT* psMot, CURVEOBJ* psCurveObj,
                  MOTNT_MOVETYPE sMoveType);
CURVEOBJ*
MotNT_CurveObjGet(MOTNT* psMot, MOTNT_MOVETYPE sMoveType);
CURVE
MotNT_CurveGet(MOTNT* psMot, MOTNT_MOVETYPE sMoveType);

unsigned int
MotNT_Speed2TimerVal(MOTNT* psMot, VC_UINT16 ui16Speed);
VC_UINT16
MotNT_TimerVal2Speed(MOTNT* psMot, unsigned int uiTimerVal);
VC_UINT32
MotNT_Dist2Steps(MOTNT* psMot, VC_UINT16 ui16Dist);
VC_UINT16
MotNT_Steps2Dist(MOTNT* psMot, VC_UINT32 ui32Steps);
unsigned int
MotNT_TimerVal2Time(MOTNT* psMot, unsigned int uiTimerVal,
                    unsigned int uiFac);
unsigned int
MotNT_Time2TimerVal(MOTNT* psMot, unsigned int uiTimer,
                    unsigned int uiFac);

VC_BOOL
MotNT_ExecAtDist(MOTNT* psMot, VC_UINT32 ui32Dist, MOTNT_DISTUNIT sDistUnit,
                 VC_BOOL bRelative, MOTNT_DIR sDir4Relative, void
                 (*ExecAtDistFunc)(MOTNT* psMot, int argc, const char* argv[]),
                 int argc, const char* argv[]);
VC_BOOL
MotNT_AddToStep(MOTNT* psMot, void (*AddToStepFunc)(MOTNT* psMot,
                int argc, const char* argv[]),
                int argc, const char* argv[]);
VC_BOOL
MotNT_AddToStateSwitch(MOTNT* psMot,
                        void (*AddToStateSwitchFunc)(
                                MOTNT* psMot,
                                MOTNT_STATE sCurState,
                                MOTNT_STATE sNextState,
                                VC_BOOL bStopAfterDec,
                                int argc, const char* argv[]),
                        int argc, const char* argv[]);
VC_BOOL
MotNT_AddToStop(MOTNT* psMot, void (*AddToStopFunc)(MOTNT* psMot,
                int argc, const char* argv[]),
                int argc, const char* argv[]);
VC_BOOL
MotNT_RegEventCallback(MOTNT* psMot,
                       void (*MotEvent)(MOTNT* psMot, MOTNT_EVENT eMotEvent));

VC_UINT32
MotNT_GetDist(MOTNT* psMot, MOTNT_DISTUNIT sDistUnit );
void
MotNT_ResetDistCounter(MOTNT* psMot);
VC_UINT16
MotNT_GetRes(MOTNT* psMot);
VC_STATUS
MotNT_SetRes(MOTNT* psMot, VC_UINT16 ui16Res);
VC_UINT16
MotNT_GetMm360Deg(MOTNT* psMot);
VC_STATUS
MotNT_SetMm360Deg(MOTNT* psMot, VC_UINT16 ui16Mm360Deg);

void
MotNT_RecalcFollowTimer(MOTNT* psMot);

VC_STATUS
MotNT_FollowModeSet(MOTNT* psMot, MOTNT_FOLLOWMODE eFollowMode);
MOTNT_FOLLOWMODE
MotNT_FollowModeGet(MOTNT* psMot);

void
MotNT_DebugSet(MOTNT* psMot, VC_UINT16* pui16DbgMem,
               unsigned int* puiDbgMemIndex, unsigned int uiDbgMemSize,
               VC_BOOL bDbgTimerVals, VC_UINT8 ui8DbgMode,
               VC_BOOL bOutpDbgInfAtStop, VC_BOOL bOutpDbgInfSerialInsteadLog);
void
MotNT_DebugGet(MOTNT* psMot, unsigned int* puiDbgMemSize,
               VC_BOOL* pbDbgTimerVals, VC_UINT8* pui8DbgMode,
               VC_BOOL* pbOutpDbgInfAtStop, VC_BOOL* pbOutpDbgInfSerialInsteadLog);
void
MotNT_DebugOutp(MOTNT* psMot);
#endif

#ifdef __cplusplus
} // extern "C" {
#endif


#endif // _VC_MOTNT_H_
