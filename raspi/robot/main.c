#include <stdio.h>
#include <unistd.h> // usleep

#include "wrgpio.h"

#include <vccomponents.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <errno.h>

#define ms *1000

static MOTNTLXS lk_sMotNT1;

// static MOTNT_CALC lk_sMotNT1Calc;
static MOTNT_CURVES_CONTAINER lk_sCurvesContainer;

static int lk_iSig;
static VC_BOOL lk_bDoOnly2ndStep;

/*=========================================================================*/
static void
MotNT1_DoStepNative(struct SMOTNT* psMotNT,
                    int argc, const char* argv[])
{
    if (lk_bDoOnly2ndStep)
    {
        printf( "CarS: %u\n",
            abs(MotNT_GetInf(psMotNT, MOTNT_INF_STEPS_DONE, MOTNT_DIR_FW)
                - MotNT_GetInf(psMotNT, MOTNT_INF_STEPS_DONE, MOTNT_DIR_BW)) );
    }
    lk_bDoOnly2ndStep = !lk_bDoOnly2ndStep;
}

/*=========================================================================*/
static void
MotNT1_StateSwitchNative(MOTNT* psMotNT, MOTNT_STATE sCurState,
                                  MOTNT_STATE sNextState,
                                  VC_BOOL bStopAfterDec,
                                  int argc, const char* argv[])
{
    if ( (sCurState == MOTNT_STATE_STOPPED)
         && (sNextState == MOTNT_STATE_INCREASE) )
    {
        lk_bDoOnly2ndStep = VC_TRUE;
        printf("StateSwitch: Mot1 Started\n");
    }

    switch (sNextState)
    {
        case MOTNT_STATE_STOPPED:
            printf("StateSwitch: Mot1 Stopped\n");
            break;

        case MOTNT_STATE_INCREASE:
            printf("StateSwitch: Mot1 Inc\n");
            break;

        case MOTNT_STATE_RUNSPEED:
            printf("StateSwitch: Mot1 Run\n");
            break;

        case MOTNT_STATE_DECREASE:
            printf("StateSwitch: Mot1 Dec\n");
            break;

        case MOTNT_STATE_NO:
            break;
    }
}

/*=========================================================================*/
void
MotNT1_AddToStep(MOTNT* psMotNT,
                          int argc, const char* argv[])
{
    printf("Step\n");
}

/*=========================================================================*/
void
MotNT1_AddToStop(MOTNT* psMotNT,
                          int argc, const char* argv[])
{
    printf("AddToStop: Mot1 Stopped\n");
}

/*=========================================================================*/
static VC_STATUS
MotNT1_CurvesInit(MOTNT* psMotNT)
{
    VC_STATUS sStatus;
#define CURVES_INIT_maxSpeed 1000
//    const unsigned int maxSpeed = 600;

    static unsigned int puiCurveXFwIncTimeUs[] =
        { 0,  60 ms};
    static unsigned int puiCurveYFwIncSpeedMmS[] =
        {50, CURVES_INIT_maxSpeed   };

    static unsigned int puiCurveXFwDecTimeUs[] =
        { 0,  40 ms};
    static unsigned int puiCurveYFwDecSpeedMmS[] =
        {50, CURVES_INIT_maxSpeed   };

    static unsigned int puiCurveXBwIncTimeUs[] =
        { 0,  80 ms};
    static unsigned int puiCurveYBwIncSpeedMmS[] =
        {50, CURVES_INIT_maxSpeed   };
    static unsigned int puiCurveXBwDecTimeUs[] =
        { 0,  80 ms};
    static unsigned int puiCurveYBwDecSpeedMmS[] =
        {50, CURVES_INIT_maxSpeed   };

    sStatus =
        MotNT_CurvesCreate(psMotNT, &lk_sCurvesContainer,
                           CREATE_CURVE_BASE_POINTS(puiCurveXFwIncTimeUs,
                                                    puiCurveYFwIncSpeedMmS),
                           CREATE_CURVE_BASE_POINTS(puiCurveXFwDecTimeUs,
                                                    puiCurveYFwDecSpeedMmS),
                           CREATE_CURVE_BASE_POINTS(puiCurveXBwIncTimeUs,
                                                    puiCurveYBwIncSpeedMmS),
                           CREATE_CURVE_BASE_POINTS(puiCurveXBwDecTimeUs,
                                                    puiCurveYBwDecSpeedMmS),
                           "MotNTCar");

    return sStatus;
}

/*=========================================================================*/
MOTNT_CURVES
MotNT1_CurvesGet(void)
{
    return MotNT_CurvesGet(&lk_sCurvesContainer);
}

/*=========================================================================*/
void
MotNT1_Init(/*MOTNT** ppsMotNT, MOTNT_CALC** ppsMotNTCalc,*/void)
{
    CURVEOBJ_PARM_REG sCurveObjParmReg;
    int iRet;
    struct sigevent sSigev;

    MotNTLxS_InitStruct(&lk_sMotNT1);

    lk_sMotNT1.CurvesInit = &MotNT1_CurvesInit;
    lk_sMotNT1.CurvesGetFirst = &MotNT1_CurvesGet;

    lk_sMotNT1.ui16FeedPer360Degs = 20;
    lk_sMotNT1.ui16Resolution = 120;

    /* Create the signal mask that will be used in wait_period */
    lk_iSig = SIGRTMIN;
    sigemptyset(&(lk_sMotNT1.alarm_sig));
    sigaddset(&(lk_sMotNT1.alarm_sig), lk_iSig);
    pthread_sigmask (SIG_BLOCK, &(lk_sMotNT1.alarm_sig), NULL);

    /* Create a timer that will generate the signal we have chosen */
    sSigev.sigev_notify = SIGEV_SIGNAL;
    sSigev.sigev_signo = lk_iSig;
    sSigev.sigev_value.sival_ptr = (void *)&lk_sMotNT1.timer_id;
    iRet = timer_create (CLOCK_MONOTONIC, &sSigev, &lk_sMotNT1.timer_id);
    if (iRet == -1)
    {
        CV_HANDLE_STATUS_A(-1);
        return;
    }

    lk_sMotNT1.DoStepNative = &MotNT1_DoStepNative;
    lk_sMotNT1.AddToStateSwitchNative = &MotNT1_StateSwitchNative;

    lk_sMotNT1.sMotNTItf.pcName = "MotNT1";

    MotNTLxS_Init(&lk_sMotNT1);

    MotNT_AddToStep((MOTNT*)&lk_sMotNT1,
                    &MotNT1_AddToStep, 0, NULL);
    MotNT_AddToStop((MOTNT*)&lk_sMotNT1,
                    &MotNT1_AddToStop, 0, NULL);

/*
    CurveObjParm_InitReg(&sCurveObjParmReg);
    sCurveObjParmReg.pcCurveName = "MotNTCarFwInc";
    sCurveObjParmReg.pcXName     = "Time";
    sCurveObjParmReg.pcYName     = "Speed";
    sCurveObjParmReg.uiScaleX    = 1000;
    sCurveObjParmReg.pcXUnit     = "ms";
    sCurveObjParmReg.pcYUnit     = "mm/s";
    sCurveObjParmReg.ePrinterCfg = CFG_DC_CMOTNT_CURVE_FWINC;
    sCurveObjParmReg.bSaveWithPmPara = VC_TRUE;
    CurveObjParm_RegTablePoints(&sCurveObjParmReg,
            MotNT_CurveObjGet((MOTNT*)&lk_sMotNT1, MOTNT_MOVETYPE_FWINC),
            NULL);

    CurveObjParm_InitReg(&sCurveObjParmReg);
    sCurveObjParmReg.pcCurveName = "MotNTCarFwDec";
    sCurveObjParmReg.pcXName     = "Time";
    sCurveObjParmReg.pcYName     = "Speed";
    sCurveObjParmReg.uiScaleX    = 1000;
    sCurveObjParmReg.pcXUnit     = "ms";
    sCurveObjParmReg.pcYUnit     = "mm/s";
    sCurveObjParmReg.ePrinterCfg = CFG_DC_CMOTNT_CURVE_FWDEC;
    sCurveObjParmReg.bSaveWithPmPara = VC_TRUE;
    CurveObjParm_RegTablePoints(&sCurveObjParmReg,
            MotNT_CurveObjGet((MOTNT*)&lk_sMotNT1, MOTNT_MOVETYPE_FWDEC),
            NULL);

    CurveObjParm_InitReg(&sCurveObjParmReg);
    sCurveObjParmReg.pcCurveName = "MotNTCarBwInc";
    sCurveObjParmReg.pcXName     = "Time";
    sCurveObjParmReg.pcYName     = "Speed";
    sCurveObjParmReg.uiScaleX    = 1000;
    sCurveObjParmReg.pcXUnit     = "ms";
    sCurveObjParmReg.pcYUnit     = "mm/s";
    sCurveObjParmReg.ePrinterCfg = CFG_DC_CMOTNT_CURVE_BWINC;
    sCurveObjParmReg.bSaveWithPmPara = VC_TRUE;
    CurveObjParm_RegTablePoints(&sCurveObjParmReg,
            MotNT_CurveObjGet((MOTNT*)&lk_sMotNT1, MOTNT_MOVETYPE_BWINC),
                              NULL);

    CurveObjParm_InitReg(&sCurveObjParmReg);
    sCurveObjParmReg.pcCurveName = "MotNTCarBwDec";
    sCurveObjParmReg.pcXName     = "Time";
    sCurveObjParmReg.pcYName     = "Speed";
    sCurveObjParmReg.uiScaleX    = 1000;
    sCurveObjParmReg.pcXUnit     = "ms";
    sCurveObjParmReg.pcYUnit     = "mm/s";
    sCurveObjParmReg.ePrinterCfg = CFG_DC_CMOTNT_CURVE_BWDEC;
    sCurveObjParmReg.bSaveWithPmPara = VC_TRUE;
    CurveObjParm_RegTablePoints(&sCurveObjParmReg,
            MotNT_CurveObjGet((MOTNT*)&lk_sMotNT1, MOTNT_MOVETYPE_BWDEC),
                              NULL);

    MotNTCalcCurve_InitStruct(&lk_sMotNT1Calc);
    lk_sMotNT1Calc.pMot = (MOTNT*)&lk_sMotNT1;
    MotNTCalcCurve_Constructor(&lk_sMotNT1Calc, VC_FALSE);

    if (ppsMotNTCalc != NULL)
    {
        *ppsMotNTCalc = &lk_sMotNT1Calc;
    }

    MotNTTstParm_Init((MOTNT*)&lk_sMotNT1, "MotNTCar", &lk_sMotNT1Calc,
                      CFG_UNKNOWN, CFG_DC_CMOTNT_TESTFWBW, NULL, NULL, CFG_UNKNOWN, NULL);
//    MotNTTstParm_Debug((MOTNT*)&lk_sMotNT1, "MotNTCar", CFG_DC_CMOTNT_DEBUG);

    *ppsMotNT = (MOTNT*)&lk_sMotNT1;
*/
}


int main(void)
{
    unsigned int gpioNum = 18;

    wrgpio_init("robot");
    wrgpio_set_pinmode2outp(gpioNum);

    MotNT1_Init();

    while (1) {
        wrgpio_set(gpioNum);
        usleep(500*1000);
        wrgpio_del(gpioNum);
        usleep(500*1000);
    }
    wrgpio_close();

    return 0;
}
