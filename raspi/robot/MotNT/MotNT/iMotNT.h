/***************************************************************************/
/*                             Druckersoftware                             */
/*                    Copyright (C) Carl Valentin GmbH                     */
/***************************************************************************/

#ifndef _VC_IMOTNT_H_
#define _VC_IMOTNT_H_

/*. IMPORT ================================================================*/

#include "MotNT.h"
#include "MotNTStub.h"
#include "MotNTLxS.h"
#include "MotNTCalc.h"
#ifdef VALENTIN
#include "MotNTTPUE.h"
#include "MotNTTPUX.h"
#include "MotNTLxFG5.h"
#include "MotNTJobApi.h"
#include "MotNTTPUSyncStep.h"
#include "MotNTCtrlInt.h"
#endif

/*. ENDIMPORT =============================================================*/


/*. EXPORT ================================================================*/

/*-------------------------------- Makros ---------------------------------*/

/*--------------------------- Typdeklarationen ----------------------------*/

typedef struct
{
    MOTNT*  psMot;
    MOTNT_CALC* psMotNTCalc;
    VC_UINT16 ui16Speed;
    MOTNT_DIR sMotNTDir;
    VC_UINT16 ui16Dist;
    MOTNT_SPEEDUNIT sSpeedUnit;
    MOTNT_DISTUNIT sDistUnit;
} MOTNTTSTPARM_STARTSTOP_PARMS;

typedef struct
{
    MOTNT*  psMot;
    MOTNT_CALC* psMotNTCalc;
    VC_UINT16 ui16FwSpeed;
    VC_UINT16 ui16BwSpeed;
    VC_UINT16 ui16Dist;
    MOTNT_SPEEDUNIT sSpeedUnit;
    MOTNT_DISTUNIT sDistUnit;
    VC_UINT16 ui16CycleC;
} MOTNTTSTPARM_FWBW_PARMS;

typedef struct
{
#ifdef VALENTIN
    LATMOTID sLatMotID;
#endif
    VC_UINT8 ui8MaxCur;
    VC_UINT8 ui8CurHold;
    CURVEOBJ* psCurveObjCurInc;
    VC_UINT8 ui8CurInc;
    CURVEOBJ* psCurveObjCurRun;
    VC_UINT8 ui8CurRun;
    CURVEOBJ* psCurveObjCurDec;
    VC_UINT8 ui8CurDec;
    VC_BOOL  bDir;
    // Interne Verwenung
    VC_BOOL bSetRunCurInDoStep;
} MOTNTLATMOTPARM;

/*------------------------------ Prototypen -------------------------------*/

#ifdef VALENTIN
void
MotNTLatMot_MicroStepCpuSet(VC_BOOL b);
VC_STATUS
MotNTLatMot_ParmReg(char* pcMotName, MOTNTLATMOTPARM* psParmDflt,
                   MOTNTLATMOTPARM* psParmMem, EPRINTER_CFG ePrinterCfg,
                   VC_BOOL bSetDefault);
void
MotNTLatMot_DoStepNative(struct SMOTNT* pMotNT,
                                int argc, const char* argv[]);
void
MotNTLatMot_MicroStepCpuDoStepNative(MOTNT* psMotNT,
                                     int argc, const char* argv[]);
void
MotNTLatMot_StateSwitchNative(MOTNT* psMotNT, MOTNT_STATE sCurState,
                           MOTNT_STATE sNextState, VC_BOOL bStopAfterDec,
                           int argc, const char* argv[]);
void
MotNTLatMot_MicroStepCpuStateSwitchNative(MOTNT* psMotNT, MOTNT_STATE sCurState,
                              MOTNT_STATE sNextState, VC_BOOL bStopAfterDec,
                              int argc, const char* argv[]);
#endif

VC_STATUS
MotNT_CurvesCreate(MOTNT* psMotNT, MOTNT_CURVES_CONTAINER* psCurvesContainer,
                   CURVE_BASE_POINTS sCurveBasePointsFwInc,
                   CURVE_BASE_POINTS sCurveBasePointsFwDec,
                   CURVE_BASE_POINTS sCurveBasePointsBwInc,
                   CURVE_BASE_POINTS sCurveBasePointsBwDec,
                   char* pcName);
VC_STATUS
MotNT_CurvesDelete(MOTNT_CURVES_CONTAINER* psCurvesContainer);
MOTNT_CURVES
MotNT_CurvesGet(MOTNT_CURVES_CONTAINER* psCurvesContainer);

#ifdef VALENTIN
void
MotNTTstParm_Init(MOTNT* psMot, char* pcMotName, MOTNT_CALC* psMotNTCalc,
                  EPRINTER_CFG ePrtCfgStartStop, EPRINTER_CFG ePrtCfgFwBw,
                  MOTNTTSTPARM_STARTSTOP_PARMS* psStartStopParms,
                  MOTNTTSTPARM_FWBW_PARMS* psFwBwParms,
                  EPRINTER_CFG ePrtCfgFollowMode,
                  MOTNT_FOLLOWMODE* peFollowMode);
void
MotNTTstParm_Debug(MOTNT* psMot, char* pcMotName, EPRINTER_CFG ePrinterCfg);
void
MotNTTstParm_LatMotCurInfo(MOTNT* psMot, char* pcMotName, CURVEOBJ* psCurveObjCurInc,
                           CURVEOBJ* psCurveObjCurRun, CURVEOBJ* psCurveObjCurDec,
                           MOTNTTSTPARM_STARTSTOP_PARMS* psStartStopParms);
#endif

/*------------------------ Konstantendeklarationen ------------------------*/

/*. ENDEXPORT =============================================================*/

#endif // _VC_IMOTNT_H_
