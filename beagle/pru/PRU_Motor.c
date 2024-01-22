#include <stdint.h>
#include <stdio.h>
#include <pru_cfg.h>
#include <pru_intc.h>
#include <rsc_types.h>
#include <pru_rpmsg.h>
#include <pru_iep.h>
#include "resource_table_1.h"

volatile register uint32_t __R30;
volatile register uint32_t __R31;

/* Host-1 Interrupt sets bit 31 in register R31 */
#define HOST_INT                        ((uint32_t) 1 << 31)

/* The PRU-ICSS system events used for RPMsg are defined in the Linux device tree
 * PRU0 uses system event 16 (To ARM) and 17 (From ARM)
 * PRU1 uses system event 18 (To ARM) and 19 (From ARM)
 */
#define TO_ARM_HOST                     18
#define FROM_ARM_HOST                   19

/*
 * Using the name 'rpmsg-pru' will probe the rpmsg_pru driver found
 * at linux-x.y.z/drivers/rpmsg/rpmsg_pru.c
 */
#define CHAN_NAME                       "rpmsg-pru"
#define CHAN_DESC                       "Channel 31"
#define CHAN_PORT                       31

typedef enum
{
    MOTNT_STATE_STOPPED,
    MOTNT_STATE_INCREASE,
    MOTNT_STATE_RUNSPEED,
    MOTNT_STATE_DECREASE,

    MOTNT_STATE_NO,
} MOTNT_STATE;

/*
 * Used to make sure the Linux drivers are ready for RPMsg communication
 * Found at linux-x.y.z/include/uapi/linux/virtio_config.h
 */
#define VIRTIO_CONFIG_S_DRIVER_OK       4

uint8_t payload[RPMSG_MESSAGE_SIZE+1];
uint8_t send[RPMSG_MESSAGE_SIZE+1];

uint8_t bRunPWM;

volatile unsigned int *ram = (volatile unsigned int *)0x9ED00000;

// M1_STEP:    LCD_DATA3 / GPIO_73 / P8_44 / pr1_pru1_pru_r31_3
// M1_CURRENT: LCD_DATA1 / GPIO_71 / P8_46 / pr1_pru1_pru_r31_1
volatile uint32_t gpioStep = 0x00000008;
volatile uint32_t gpioCurr = 0x00000002;

MOTNT_STATE sCurState;
unsigned int uiStepTimerVal, uiEndSpeedTimerVal;
unsigned int uiCurCurveTableIndex;

/*=========================================================================*/
void reset_iep(void)
{
    // Set counter to 0
    CT_IEP.TMR_CNT = 0x0;
    // Enable counter
    // (*(volatile unsigned int *) 0x0002E000 ) = 0x0011;
    CT_IEP.TMR_GLB_CFG = 0x11;
}

/*=========================================================================*/
static inline int VC_SPrintfNumHelpPos(char* pc8Char,
                          uint32_t ui32Value, uint32_t ui32PosNum,
                          uint32_t* pui32CurPosNumSum)
{
    uint32_t ui32;

    if (ui32Value >= ui32PosNum)
    {
        ui32 = ((ui32Value-*pui32CurPosNumSum) / ui32PosNum);
        *pc8Char = '0' + (char)ui32;
        *pui32CurPosNumSum += ui32 * ui32PosNum;
        return 1;
    }

    return 0;
}

/*=========================================================================*/
/**
 * \param pc8String Abgeschlossener String mit genuegend Platz fuer Zahl
 *                  Beispiel: "Bytes Received\n\0          "
 *                  Daraus wird dann z.B. Folgendes gemacht (ui32Value=1024):
 *                            "Bytes Received\n1024\0      "
 */
static unsigned int VC_SPrintfNum(char* pc8String, uint32_t ui32Value)
{
    unsigned int uiI;
    uint32_t ui32CurPosNumSum = 0;
    uint32_t ui32;

    uiI = 0;

    if (ui32Value == 0) {
        pc8String[0] = '0';
        pc8String[1] = '\0';
        return 1;
    }

    for (ui32=1000000000; ui32>0; ui32 /= 10)
    {
        if (VC_SPrintfNumHelpPos(pc8String+uiI, ui32Value, ui32,
                                 &ui32CurPosNumSum)) uiI++;
    }

    pc8String[uiI] = '\0';

    return uiI;
}

/*=========================================================================*/
void Mot_Step(int doStep)
{
    switch (sCurState)
    {
        case MOTNT_STATE_NO:
        case MOTNT_STATE_STOPPED:
            break;

        case MOTNT_STATE_INCREASE:
            uiStepTimerVal = *(ram+uiCurCurveTableIndex);
            uiCurCurveTableIndex++;
            if (uiStepTimerVal < uiEndSpeedTimerVal) {
                uiStepTimerVal = uiEndSpeedTimerVal;
                sCurState = MOTNT_STATE_RUNSPEED;
            }
            break;

        case MOTNT_STATE_RUNSPEED:
            break;

        case MOTNT_STATE_DECREASE:
            break;
    }

    if (doStep) {
        __R30 |= gpioStep;
    }
}

/*=========================================================================*/
void main(void)
{
    char pcRet[20];
    unsigned int uiRetLen;

    struct pru_rpmsg_transport transport;
    uint16_t src, dst, len;
    volatile uint8_t *status;
    int      bStep;
    int      bCurr;

    unsigned int uiTimerVal;
    unsigned int uiStepTimerValReal;
    unsigned int uiCurTimerVal, uiTimerValDiff;
    unsigned int uiStepTimerValOld;
    unsigned int uiCurrTimerVal, uiCurrTimerValOld;

    /* Allow OCP master port access by the PRU so the PRU can read external memories */
    CT_CFG.SYSCFG_bit.STANDBY_INIT = 0;

    /* Clear the status of the PRU-ICSS system event that the ARM will use to 'kick' us */
    CT_INTC.SICR_bit.STS_CLR_IDX = FROM_ARM_HOST;

    /* Make sure the Linux drivers are ready for RPMsg communication */
    status = &resourceTable.rpmsg_vdev.status;
    while (!(*status & VIRTIO_CONFIG_S_DRIVER_OK));

    /* Initialize the RPMsg transport structure */
    pru_rpmsg_init(&transport, &resourceTable.rpmsg_vring0, &resourceTable.rpmsg_vring1, TO_ARM_HOST, FROM_ARM_HOST);

    reset_iep();
    uiCurrTimerValOld = 0;
    uiCurrTimerVal = 200000/2; // Bei 200Mhz 1ms oder 2x 500us

    // Create the RPMsg channel between the PRU and ARM user space using the transport structure.
    while (pru_rpmsg_channel(RPMSG_NS_CREATE, &transport, CHAN_NAME, CHAN_DESC, CHAN_PORT) != PRU_RPMSG_SUCCESS);

    while (1)
    {
        // Check bit 31 of register R31 to see if the ARM has kicked us
        if (__R31 & HOST_INT) {
            // Clear the event status
            CT_INTC.SICR_bit.STS_CLR_IDX = FROM_ARM_HOST;
            // Receive all available messages, multiple messages can be sent per kick
            while (pru_rpmsg_receive(&transport, &src, &dst, payload, &len) == PRU_RPMSG_SUCCESS)
            {
                if (len>=6 && payload[0]=='S' && payload[1]=='T')  {
                    uiCurCurveTableIndex = 0;
                    uiEndSpeedTimerVal = (payload[2]<<24) | (payload[3]<<16) | (payload[4]<<8) | payload[5];
                    sCurState = MOTNT_STATE_INCREASE;
                    bRunPWM = 1;
                    uiStepTimerValOld = CT_IEP.TMR_CNT;
                    bStep = 0;
//                    uiTimerVal = *(ram+4);
                    uiRetLen = VC_SPrintfNum(pcRet, uiEndSpeedTimerVal);
                    pru_rpmsg_send(&transport, dst, src, pcRet, uiRetLen);
                    Mot_Step(0);
                    uiStepTimerValReal = uiStepTimerVal/2;
                }

                if (len>=2 && payload[0]=='S' && payload[1]=='P')  {
                    bRunPWM = 0;
                    pru_rpmsg_send(&transport, dst, src, "Off!", 4);
                }
            }
        }

        uiCurTimerVal = CT_IEP.TMR_CNT;

        if (bRunPWM)
        {
            if (uiCurTimerVal >= uiStepTimerValOld)
                uiTimerValDiff = uiCurTimerVal-uiStepTimerValOld;
            else
                uiTimerValDiff = 0xFFFFFFFF-uiStepTimerValOld+uiCurTimerVal;
            if (uiTimerValDiff >= uiStepTimerValReal) {
                if (!bStep)
                    __R30 &= ~gpioStep;
                if (bStep) {
                    Mot_Step(bStep);
                    uiStepTimerValReal = uiStepTimerVal/2;
                }
                bStep = !bStep;
                uiStepTimerValOld = uiCurTimerVal;
            }

            if (uiCurTimerVal >= uiCurrTimerValOld)
                uiTimerValDiff = uiCurTimerVal-uiCurrTimerValOld;
            else
                uiTimerValDiff = 0xFFFFFFFF-uiCurrTimerValOld+uiCurTimerVal;
            if (uiTimerValDiff >= uiCurrTimerVal) {
                if (bCurr)
                    __R30 |= gpioCurr;
                else
                    __R30 &= ~gpioCurr;
                bCurr = !bCurr;
                uiCurrTimerValOld = uiCurTimerVal;
            }
        } // if (bRunPWM)

    } // while (1)
}
