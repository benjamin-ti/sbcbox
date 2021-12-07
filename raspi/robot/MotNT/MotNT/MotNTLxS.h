/***************************************************************************/
/*                             Printerfirmware                             */
/*                    Copyright (C) Carl Valentin GmbH                     */
/*                       http://www.carl-valentin.de                       */
/*                  This code is licenced under the LGPL                   */
/***************************************************************************/

#ifndef _VC_MOTNTLXS_H_
#define _VC_MOTNTLXS_H_

/*. IMPORT ================================================================*/

#include "../vccomponents_intern.h"
#include <vchal.h>
#include "../Utilities/curve.h"

#ifdef LINUX_SUPPORT
#include <time.h>
#endif

#include "MotNT.h"

/*. ENDIMPORT =============================================================*/


/*. EXPORT ================================================================*/

/*-------------------------------- Makros ---------------------------------*/

#define MOTNTLXS_NO_OF_ADD_TO_FUNCS 5

/*--------------------------- Typdeklarationen ----------------------------*/

typedef enum
{
    // !! Bei Aenderung hier StateChart nicht vergessen !!
    MOTNTLXS_COND_INC,
    MOTNTLXS_COND_ENDSPEED,
    MOTNTLXS_COND_DEC,
    MOTNTLXS_COND_STOPSPEED,

    MOTNTLXS_COND_NO,
} MOTNTLXS_COND;

typedef void (*MotNTLxS_ExecAtDistFuncs)(struct SMOTNT* pMotNT,
                  int argc, const char* argv[]);
typedef void (*MotNTLxS_AddToStepFuncs)(struct SMOTNT* pMotNT,
                               int argc, const char* argv[]);
typedef void (*MotNTLxS_AddToStateSwitchFuncs)(
                  struct SMOTNT* pMotNT, MOTNT_STATE sCurState,
                  MOTNT_STATE sNextState, VC_BOOL bStopAfterDec,
                  int argc, const char* argv[]);
typedef void (*MotNTLxS_AddToStopFuncs)(struct SMOTNT* pMotNT,
                               int argc, const char* argv[]);

typedef struct
{
    MOTNT sMotNTItf;

    // Init-Bereich
    // -------------------------------
    // Noch von Common
    // char  pcName;
    // VC_UINT16 ui16MaxSpeedForward;  // Falls nicht angegeben werden der
    // VC_UINT16 ui16MinSpeedForward;  // absoluten Min.-Wert (basierend auf
    // VC_UINT16 ui16MaxSpeedBackward; // Timer Bit-Breite) und Max.-Wert aus
    // VC_UINT16 ui16MinSpeedBackward; // obigen Tabellen genommen

    VC_STATUS    (*CurvesInit)(struct SMOTNT* pMotNT);
    MOTNT_CURVES (*CurvesGetFirst)(void);

    VC_UINT16  ui16FeedPer360Degs;
    VC_UINT16  ui16Resolution;

    void (*DoStepNative)(struct SMOTNT* pMotNT, int argc, const char* argv[]);
    int iArgcDoStepNative;
    const char** ppcArgvDoStepNative;

    void (*AddToStateSwitchNative)(struct SMOTNT* psMotNT,
                                   MOTNT_STATE sCurState,
                                   MOTNT_STATE sNextState,
                                   VC_BOOL bStopAfterDec,
                                   int argc, const char* argv[]);
    int iArgcAddToState;
    const char** ppcArgvAddToState;

    VC_UINT16 (*GetFollowTimerValue)(void* pvFollowTimerValObj);
    VC_UINT16 (*CalcFollowTimerVal)(void* pvFollowTimerValObj,
                                    VC_UINT16 ui16Speed);
    void* pvFollowTimerValObj;

#ifdef LINUX_SUPPORT
    sigset_t alarm_sig;
    timer_t timer_id;
#endif

    // Private-Bereich
    // -------------------------------
    CURVEOBJ* psCurveObjFwInc;
    CURVEOBJ* psCurveObjFwDec;
    CURVEOBJ* psCurveObjBwInc;
    CURVEOBJ* psCurveObjBwDec;

    char*           pcCurvesFwName;
    char*           pcCurvesBwName;
    unsigned int    uiStepTimerValue_us;
    CURVEOBJ*       psCurveObjCurInc;
    CURVEOBJ*       psCurveObjCurDec;
    CURVE           sCurIncCurve;
    CURVE           sCurDecCurve;
    unsigned int    uiEndSpeedTimerVal_us;
    VC_UINT32*      pui32StepsDone;
    VC_UINT32       ui32StepsToDo;
    unsigned int    uiCurCurveTableIndex;

    // AddToFuncs
    VC_UINT32       ui32NextDist4ExecAtDistFw;
    VC_UINT32       ui32NextDist4ExecAtDistBw;
    VC_BOOL         bExecAtDistTablesUpdated;
    VC_UINT32       pui32Dist4ExecAtDistFw[MOTNTLXS_NO_OF_ADD_TO_FUNCS];
    VC_UINT32       pui32Dist4ExecAtDistBw[MOTNTLXS_NO_OF_ADD_TO_FUNCS];
    MotNTLxS_ExecAtDistFuncs pExecAtDistFuncs[MOTNTLXS_NO_OF_ADD_TO_FUNCS];
    int             pExecAtDistFuncsArgc[MOTNTLXS_NO_OF_ADD_TO_FUNCS];
    const char**    pppExecAtDistFuncsArgv[MOTNTLXS_NO_OF_ADD_TO_FUNCS];

    VC_UINT8        ui8NoOfAddToStepFuncs;
    VC_UINT8        ui8NoOfAddToStateSwitchFuncs;
    VC_UINT8        ui8NoOfAddToStopFuncs;
    MotNTLxS_AddToStepFuncs pAddToStepFuncs[MOTNTLXS_NO_OF_ADD_TO_FUNCS];
    int             pAddToStepFuncsArgc[MOTNTLXS_NO_OF_ADD_TO_FUNCS];
    const char**    pppAddToStepFuncsArgv[MOTNTLXS_NO_OF_ADD_TO_FUNCS];
    MotNTLxS_AddToStateSwitchFuncs pAddToStateSwitchFuncs[MOTNTLXS_NO_OF_ADD_TO_FUNCS];
    int pAddToStateSwitchFuncsArgc[MOTNTLXS_NO_OF_ADD_TO_FUNCS];
    const char**    pppAddToStateSwitchFuncsArgv[MOTNTLXS_NO_OF_ADD_TO_FUNCS];
    MotNTLxS_AddToStopFuncs pAddToStopFuncs[MOTNTLXS_NO_OF_ADD_TO_FUNCS];
    int             pAddToStopFuncsArgc[MOTNTLXS_NO_OF_ADD_TO_FUNCS];
    const char**    pppAddToStopFuncsArgv[MOTNTLXS_NO_OF_ADD_TO_FUNCS];

    VC_BOOL         bStopStateMachineExe;
    // State Machine
    MOTNT_STATE     sCurState;
    VC_BOOL         bDisableStateMachine;
    VC_BOOL         bAllowStateMachineChange;
    unsigned int    uiDoStateMachinePauseCounter;
    VC_BOOL         bStopAfterDec;
    VC_BOOL         bExecAddToFunctions;
    // Befehle fuer den State-Machine-Ausfuehrungsinterrupt:
    // DoCond
    VC_UINT16       ui16DoCondEndSpeedTimerVal;
    VC_BOOL         bDoCondStopAfterDec;
    MOTNTLXS_COND   sCurCond;
    // DoStart
    VC_UINT16       ui16DoStart_SpeedInTimerVal;
    VC_UINT16       ui16DoStart_DistInSteps;
    MOTNT_DIR       sDoStart_Dir;
    VC_UINT16       ui16DoStart_StartSpeedInTimerVal;
    VC_BOOL         bDoStart;
    VC_BOOL         bDoStartFastInt;
    VC_BOOL         bImmediateRestartInOtherDir;
    // RecalcFollowTimer
    VC_UINT16       ui16FollowTimerMinSpeedVal;
    VC_UINT32       ui32FollowTimerFac;
    VC_UINT16       ui16FollowTimerMinSpeedValNewCalc;
    VC_UINT32       ui32FollowTimerFacNewCalc;
    VC_BOOL         bFollowTimerNewCalc;

#ifdef LINUX_SUPPORT
    int wakeups_missed;
    pthread_t tid;
#endif

    // UnitTest
    VC_BOOL bDoNotStartInt;
} MOTNTLXS;

/*------------------------------ Prototypen -------------------------------*/

void MotNTLxS_InitStruct(MOTNTLXS* pMot);
VC_BOOL MotNTLxS_Init(MOTNTLXS* pMot);

/*------------------------ Konstantendeklarationen ------------------------*/

/*. ENDEXPORT =============================================================*/

#endif // _VC_MOTNTLXS_H_
