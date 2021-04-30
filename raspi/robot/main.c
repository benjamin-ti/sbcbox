#include <stdio.h>
#include <unistd.h> // usleep

#include <wiringPi.h>
#include <softPwm.h>
#include "wrgpio.h"

#include <vccomponents.h>
#include <cvlog.h>
#include "MotNT/MotNT/iMotNT.h"

#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <errno.h>

#define ms *1000

// M1
// static const unsigned int lk_gpioNumStep = 14;
// static const unsigned int lk_gpioNumCur = 2;
// static const unsigned int lk_gpioNumDir = 4;
// M2
static const unsigned int lk_gpioNumStep = 18;
static const unsigned int lk_gpioNumCur = 15;
static const unsigned int lk_gpioNumDir = 17;

static MOTNTLXS lk_sMotNT1;
static MOTNT*   lk_psMotNT1 = (MOTNT*)&lk_sMotNT1;

// static MOTNT_CALC lk_sMotNT1Calc;
static MOTNT_CURVES_CONTAINER lk_sCurvesContainer;

static int lk_iSig;
static VC_BOOL lk_bDoOnly2ndStep;

static int lk_wakeups_missed;
static sigset_t   lk_pwm_sig;
static timer_t    lk_pwm_timer_id;
static pthread_t  lk_pwm_tid;

static sigset_t   lk_pwm2_sig;
static timer_t    lk_pwm2_timer_id;
static pthread_t  lk_pwm2_tid;

/*=========================================================================*/
static void
MotNT1_DoStepNative(struct SMOTNT* psMotNT,
                    int argc, const char* argv[])
{
/*
    if (lk_bDoOnly2ndStep)
    {
        printf( "CarS: %u\n",
            abs(MotNT_GetInf(psMotNT, MOTNT_INF_STEPS_DONE, MOTNT_DIR_FW)
                - MotNT_GetInf(psMotNT, MOTNT_INF_STEPS_DONE, MOTNT_DIR_BW)) );
    }
    lk_bDoOnly2ndStep = !lk_bDoOnly2ndStep;
*/
    static VC_BOOL b;
    if (b)
        wrgpio_set(lk_gpioNumStep);
    else
        wrgpio_del(lk_gpioNumStep);
    b = !b;
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
            if (MotNT_GetCurDir(psMotNT) == MOTNT_DIR_FW)
            {
                wrgpio_set(lk_gpioNumDir);
            }
            else
            {
                wrgpio_del(lk_gpioNumDir);
            }
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
//    printf("Step\n");
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

    lk_sMotNT1.ui16FeedPer360Degs = 400;
    lk_sMotNT1.ui16Resolution = 800;

    wrgpio_set_pinmode2outp(lk_gpioNumStep);
    wrgpio_set_pinmode2outp(lk_gpioNumCur);
    wrgpio_set_pinmode2outp(lk_gpioNumDir);

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

/*=========================================================================*/
static void* PWMTimerThread(void* pvObject)
{
    static VC_BOOL b;
    int sig;

    while (1)
    {
        sigwait (&(lk_pwm_sig), &sig);
        lk_wakeups_missed += timer_getoverrun (lk_pwm_timer_id);

        if (b)
            wrgpio_set(lk_gpioNumCur);
        else
            wrgpio_del(lk_gpioNumCur);
        b = !b;
    }

    return NULL;
}

/*=========================================================================*/
static void* PWM2TimerThread(void* pvObject)
{
    static VC_BOOL b;
    int sig;

    while (1)
    {
        sigwait (&(lk_pwm2_sig), &sig);
        printf("PWM2\n");
    }

    return NULL;
}

/*=========================================================================*/
static void PWMTimerValueSet(unsigned int uiTimerValue_us)
{
    int iErr;
    unsigned int iNs;
    unsigned int iSec;
    struct itimerspec sItval;

    if (uiTimerValue_us == 0)
    {
        if (timer_gettime(lk_pwm_timer_id, &sItval) == -1)
        {
            iErr = errno;
            CV_LOG_A_ERROR(("%s\n", strerror(iErr)));
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
    if (timer_settime(lk_pwm_timer_id, 0, &sItval, NULL) == -1)
    {
        iErr = errno;
        CV_LOG_A_ERROR(("%s\n", strerror(iErr)));
    }
}

/*=========================================================================*/
static void PWM2TimerValueSet(unsigned int uiTimerValue_us)
{
    int iErr;
    unsigned int iNs;
    unsigned int iSec;
    struct itimerspec sItval;

    if (uiTimerValue_us == 0)
    {
        if (timer_gettime(lk_pwm2_timer_id, &sItval) == -1)
        {
            iErr = errno;
            CV_LOG_A_ERROR(("%s\n", strerror(iErr)));
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
    if (timer_settime(lk_pwm2_timer_id, 0, &sItval, NULL) == -1)
    {
        iErr = errno;
        CV_LOG_A_ERROR(("%s\n", strerror(iErr)));
    }
}

/*=========================================================================*/
int main(void)
{
//    unsigned int gpioNum = 18;
    unsigned int gpioMotorReset = 3;
    int iRet;
    static struct sigevent sSigev2;
    static struct sigevent sSigev3;
    int iSigPWM;

    sigset_t alarm_sig;
    int i;
    /* Block all real time signals so they can be used for the timers.
       Note: this has to be done in main() before any threads are created
       so they all inherit the same mask. Doing it later is subject to
       race conditions */
    sigemptyset (&alarm_sig);
    for (i=SIGRTMIN; i<=SIGRTMAX; i++)
        sigaddset (&alarm_sig, i);
    sigprocmask (SIG_BLOCK, &alarm_sig, NULL);

    wrgpio_init("robot");
//    wrgpio_set_pinmode2outp(gpioNum);
    wrgpio_set_pinmode2outp(gpioMotorReset);

    if (wiringPiSetup() == -1)
        return 1;
/*
    pinMode(9, OUTPUT);
    softPwmCreate(9, 1, 100);
    softPwmWrite(9, 50);
*/

    iSigPWM = SIGRTMIN+1;
    sigemptyset(&(lk_pwm_sig));
    sigaddset(&(lk_pwm_sig), iSigPWM);
    pthread_sigmask (SIG_BLOCK, &(lk_pwm_sig), NULL);

    /* Create a timer that will generate the signal we have chosen */
    sSigev2.sigev_notify = SIGEV_SIGNAL;
    sSigev2.sigev_signo = iSigPWM;
    sSigev2.sigev_value.sival_ptr = (void *)&lk_pwm_timer_id;
    iRet = timer_create (CLOCK_MONOTONIC, &sSigev2, &lk_pwm_timer_id);
    if (iRet == -1)
    {
        CV_HANDLE_STATUS_A(-1);
        return VC_FALSE;
    }

    iRet = pthread_create(&lk_pwm_tid, NULL, &PWMTimerThread, NULL);
    if (iRet != 0)
    {
        CV_HANDLE_STATUS_A(-1);
        return VC_FALSE;
    }
    else
    {
        struct sched_param param;
        param.sched_priority = 80;
        iRet = pthread_setschedparam(lk_pwm_tid, SCHED_FIFO, &param);
        if (iRet != 0)
        {
            CV_HANDLE_STATUS_A(-1);
        }

        iRet = pthread_detach(lk_pwm_tid);
        if (iRet)
        {
            CV_HANDLE_STATUS_A(-1); // wird eh nicht passieren, wenn vorangegangenes pthread_create erfolgreich war, was wahrscheinlich ist, wenn es 0 zurueckgab ;-)
        }
    }

/*
    iSigPWM = SIGRTMIN+2;
    sigemptyset(&(lk_pwm2_sig));
    sigaddset(&(lk_pwm2_sig), iSigPWM);
    pthread_sigmask (SIG_BLOCK, &(lk_pwm2_sig), NULL);

    // Create a timer that will generate the signal we have chosen
    sSigev3.sigev_notify = SIGEV_SIGNAL;
    sSigev3.sigev_signo = iSigPWM;
    sSigev3.sigev_value.sival_ptr = (void *)&lk_pwm2_timer_id;
    iRet = timer_create (CLOCK_MONOTONIC, &sSigev3, &lk_pwm2_timer_id);
    if (iRet == -1)
    {
        CV_HANDLE_STATUS_A(-1);
        return VC_FALSE;
    }

    iRet = pthread_create(&lk_pwm2_tid, NULL, &PWM2TimerThread, NULL);
    if (iRet != 0)
    {
        CV_HANDLE_STATUS_A(-1);
        return VC_FALSE;
    }
    else
    {
        struct sched_param param;
        param.sched_priority = 80;
        iRet = pthread_setschedparam(lk_pwm2_tid, SCHED_FIFO, &param);
        if (iRet != 0)
        {
            CV_HANDLE_STATUS_A(-1);
        }

        iRet = pthread_detach(lk_pwm2_tid);
        if (iRet)
        {
            CV_HANDLE_STATUS_A(-1); // wird eh nicht passieren, wenn vorangegangenes pthread_create erfolgreich war, was wahrscheinlich ist, wenn es 0 zurueckgab ;-)
        }
    }
*/

    wrgpio_set(gpioMotorReset);

    MotNT1_Init();

    PWMTimerValueSet(1*1000);
//    PWM2TimerValueSet(500*1000);

/*
    while (1)
    {
        wrgpio_set(gpioNum);
        usleep(500*1000);
        wrgpio_del(gpioNum);
        usleep(500*1000);
    }
*/

    // PWM-Frequenz in Hz = 19.2 MHz / Clock / Range
    pinMode(1, PWM_OUTPUT);
    pwmSetMode(PWM_MODE_MS);
/*
    pwmSetClock(1920); // 1920: PWM-Takt beträgt 19.2 MHz / 1920 = 10 KHz
    pwmSetRange(200); // PWM-Range (PWMR): Die 10 kHz werden durch 200 Einheiten geteilt, d.h. die PWM-Frequenz beträgt10 KHz / 200 = 50 Hz
    pwmWrite(1, 100); // Das Ausgangssignal wird für 100 Einheiten auf High geschaltet. Bei einer PWM-Range (PWMR) von 200 Einheiten ergibt das einen Tastgrad von 50%
*/
    pwmSetClock(1920);
    pwmSetRange(10);
    pwmWrite(1, 5);

//    MotNT_Start(lk_psMotNT1, 1200, 0, MOTNT_DIR_FW);
    usleep(5000*1000);
//    MotNT_Stop(lk_psMotNT1);
    usleep(1000*1000);

    pinMode(1, OUTPUT);

    wrgpio_del(gpioMotorReset);

    wrgpio_close();

    return 0;
}
