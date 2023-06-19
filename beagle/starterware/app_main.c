/**
 * \file   led_blink_app_main.c
 *
 * \brief  Example application main source file, which configures the IP and
 *         executes the use-case.
 *
 *  \copyright Copyright (C) 2013 Texas Instruments Incorporated -
 *             http://www.ti.com/
 */

/*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*    Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
*    Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the
*    distribution.
*
*    Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */
#include "types.h"
#include "error.h"
#include "board.h"
#include "soc.h"
#include "device.h"
#include "chipdb.h"
#include "cache.h"
#include "mmu.h"
#include "example_utils_mmu.h"
#include "utils/console_utils.h"

#include <dma.h>
#include <dma_utils.h>

#include <interrupt.h>
#include <am335x/chipdb_defs.h>

/* Application header files */
#include "gpio_app.h"
#include "app_main.h"

/* ========================================================================== */
/*                                Macros                                      */
/* ========================================================================== */

/** \brief LED device instance number */
#define GPIO_LED_INST_NUM     (0U)

/* ========================================================================== */
/*                         Structures and Enums                               */
/* ========================================================================== */

/* None */


/* ========================================================================== */
/*                 Internal Function Declarations                             */
/* ========================================================================== */

/**
 * \brief   This API gets the board info like pin number and instance.
 *
 * \param   pObj        Pointer to the pin object.
 *
 * \retval  S_PASS      Board info successful retrieved. LED is connected to
 *                      GPIO on this board.
 * \retval  E_FAIL      This board does not support this application.
 *                      - LED device may not be available on this board.
 *                      - LED may not be connected to GPIO on this board.
 */
static int32_t GpioAppBoardInfoGet(gpioAppPinObj_t *pObj);

/**
 * \brief   This API gets the soc info - base address of gpio instance.
 *
 * \param   pObj        Pointer to the pin object.
 */
static void GpioAppSocInfoGet(gpioAppPinObj_t *pObj);

/**
 * \brief   GPIO led blink use case api.
 *
 * \param   delayValue  Delay value between led toggle.
 * \param   pPin        Pointer to the pin object.
 */
static void GpioAppLedBlink(uint32_t delayValue, gpioAppPinObj_t *pPin);

/**
 * \brief   This API implements counter based delay.
 *
 * \param   delayValue  Delay value between led toggle.
 */
static void GpioAppDelay(volatile uint32_t count);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/** \breif Structure holding the default values of use case structure. */
static const gpioAppLedCfg_t GPIOAPPLEDCFG_DEFAULT =
{
//    0x6FFFFFU     // delay with Cache
	0x1FFFF     // delay without Cache
};

/** \brief Global object for the LED use case information. */
static gpioAppLedCfg_t gLedAppCfg;

/** \brief Global object for the GPIO IP configuration. */
static gpioAppPinObj_t gGpioAppCfg;

static gpioAppPinObj_t gGpioAppPin0_7;
static gpioAppPinObj_t gGpioAppPin0_20;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

/** \brief DMA default data configuration */
static const dmaUtilsDataObj_t DMA_UTILS_DATA_DEFAULT =
{
    0U,                                        /* addr */
    DMA_XFER_DATA_ADDR_MODE_INC,               /* addrMode */
    0U,                                        /* fifoWidth */
    1U,                                        /* packetActSize */
    1U,                                        /* frameActSize */
    1U,                                        /* blockSize */
    0U,                                        /* packetInactSize */
    0U,                                        /* frameInactSize */
    DMA_UTILS_DATA_SYNC_BLOCK                  /* syncMode */
}; /* dmaUtilsDataObj_t */

/** \brief DMA default transfer configuration. */
static const dmaUtilsXferObj_t DMA_UTILS_XFER_DEFAULT =
{
    NULL,                                      /* pSrc */
    NULL,                                      /* pDest */
    DMA_UTILS_DATA_SYNC_MASK_NONE,             /* intrConfig */
    FALSE,                                     /* linkEnable */
    0U                                         /* nxtXferIdx */
}; /* dmaUtilsXferObj_t */

/** \brief DMA default channel configuration. */
static const dmaUtilsChObj_t DMA_UTILS_CH_DEFAULT =
{
    DMA_XFER_TRIGGER_TYPE_MANUAL,              /* triggerType */
    0U,                                        /* xferIdx */
    FALSE,                                     /* intrEnable */
    NULL,                                      /* callBack */
    0U                                         /* queueNum */
}; /* dmaUtilsChObj_t */

static volatile uint32_t bDMAUtilsMemCopyStatus = FALSE;

static uint32_t DMAMemCopyStatus(void)
{
    return bDMAUtilsMemCopyStatus;
}

static void DMAMemCopyCallBack(uint32_t chNum, uint32_t xferStatus)
{
    bDMAUtilsMemCopyStatus = TRUE;
}

static int32_t DMAMemCopy(uint32_t dmaType,
                        uint32_t instNum,
                        uint8_t *pSrcBuf,
                        uint8_t *pDstBuf,
                        uint32_t noOfBytes, uint32_t paRamMappingSet, uint32_t chNum)
{
    int32_t retStat = E_FAIL;
    dmaUtilsDataObj_t srcDataObj = DMA_UTILS_DATA_DEFAULT;
    dmaUtilsDataObj_t dstDataObj = DMA_UTILS_DATA_DEFAULT;
    dmaUtilsXferObj_t xferObj = DMA_UTILS_XFER_DEFAULT;
    dmaUtilsChObj_t chObj = DMA_UTILS_CH_DEFAULT;

    if((NULL != pSrcBuf) && (NULL != pDstBuf))
    {
        retStat = S_PASS;
        bDMAUtilsMemCopyStatus = FALSE;
    }

    if(S_PASS == retStat)
    {
        xferObj.pSrc = &srcDataObj;
        xferObj.pDst = &dstDataObj;

        xferObj.pSrc->addr = (uint32_t)pSrcBuf;
        xferObj.pSrc->packetActSize = noOfBytes;
        xferObj.pSrc->frameActSize = 1U;
        xferObj.pSrc->blockSize = 1U;
        xferObj.pSrc->addrMode = DMA_XFER_DATA_ADDR_MODE_INC;
        xferObj.pSrc->syncMode = DMA_UTILS_DATA_SYNC_BLOCK;

        xferObj.pDst->addr = (uint32_t)pDstBuf;
        xferObj.pDst->packetActSize = 1U;
        xferObj.pDst->frameActSize = 1U;
        xferObj.pDst->blockSize = 1U;
        xferObj.pDst->addrMode = DMA_XFER_DATA_ADDR_MODE_INC;
        xferObj.pDst->syncMode = DMA_UTILS_DATA_SYNC_BLOCK;

        xferObj.intrConfig = DMA_UTILS_DATA_SYNC_MASK_BLOCK;
        xferObj.linkEnable = TRUE;
        xferObj.nxtXferIdx = 0U;

        retStat = DMAUtilsDataXferConfig(dmaType, instNum, paRamMappingSet, &xferObj);
    }

    if(S_PASS == retStat)
    {
        chObj.xferIdx = paRamMappingSet;
        chObj.queueNum = 0U;
        chObj.intrEnable = TRUE;
        chObj.triggerType = DMA_XFER_TRIGGER_TYPE_EVENT;
        chObj.callBack = &DMAMemCopyCallBack;

        retStat = DMAUtilsChConfig(dmaType, instNum, chNum, &chObj);
    }

    if(S_PASS == retStat)
    {
        retStat = DMAUtilsXferStart(dmaType, instNum, chNum, chObj.triggerType);
    }

    return retStat;
}

static void GPIOIntr(uint32_t intrId, uint32_t cpuId, void* pUserParam)
{
    CONSOLEUtilsPrintf("GPIOIntr\n");
    GPIOIntrClear(gGpioAppPin0_20.instAddr, 0, gGpioAppPin0_20.pinNum);
#define AM335X_GPIO0_BASE 0x44E07000
    volatile unsigned* gpeoi;
    gpeoi = (volatile unsigned*)(AM335X_GPIO0_BASE+0x20);
    *gpeoi = 0;
}

static int32_t GPIOIntrConfig(void)
{
    /* Precondition : Enable ARM interrupt control and initialise the Interrupt
    Controller. */

    /* local Structure which holds the Interrupt parameters. */
    intcIntrParams_t intrParams;
    int32_t returnVal = S_PASS;
    intrParams.triggerType = INTC_TRIG_RISING_EDGE;

    /* Currently hard coded priority number */
    intrParams.priority = 0x20U;
    intrParams.pFnIntrHandler = GPIOIntr;
    /* Pointer to the app object being passed to handler */
    intrParams.pUserParam = NULL;
    /* Currently hard coded - non secure mode */
    intrParams.isIntrSecure = FALSE;

    returnVal = INTCConfigIntr(96, &intrParams, FALSE);

    return (returnVal);
}

static void XdmaEventIntr1(uint32_t intrId, uint32_t cpuId, void* pUserParam)
{
    CONSOLEUtilsPrintf("XdmaEventIntr1\n");
    GPIOPinWrite(gGpioAppPin0_7.instAddr, gGpioAppPin0_7.pinNum, GPIO_PIN_LOW);
    INTCClearIntr(124);
}



#define CTRLMOD_BASE    0x44e10000
#define TPPC_EVT_MUX_32_35  (volatile unsigned*)(CTRLMOD_BASE+0xFB0)


static int32_t XdmaEventIntr1Config(void)
{
    *TPPC_EVT_MUX_32_35 = 29;

    /* Precondition : Enable ARM interrupt control and initialise the Interrupt
    Controller. */

    /* local Structure which holds the Interrupt parameters. */
    intcIntrParams_t intrParams;
    int32_t returnVal = S_PASS;
    intrParams.triggerType = INTC_TRIG_RISING_EDGE;

    /* Currently hard coded priority number */
    intrParams.priority = 0x20U;
    intrParams.pFnIntrHandler = XdmaEventIntr1;
    /* Pointer to the app object being passed to handler */
    intrParams.pUserParam = NULL;
    /* Currently hard coded - non secure mode */
    intrParams.isIntrSecure = FALSE;

    returnVal = INTCConfigIntr(124, &intrParams, FALSE);

    return (returnVal);
}


int main()
{
    int32_t status = S_PASS;
    uint32_t ui32Pins;
    int32_t retStat = E_FAIL;

    /* Enable cache memory and MMU */
//    MMUConfigAndEnable();
//    CACHEEnable(CACHE_IDCACHE, CACHE_INNER_OUTER);

    /*
     * Initialize the global objects of use case and IP configuration
     * data structures with default values.
     */
    gLedAppCfg  = GPIOAPPLEDCFG_DEFAULT;
    gGpioAppCfg = GPIOAPPPINOBJ_DEFAULT;
    gGpioAppPin0_7 = GPIOAPPPINOBJ_DEFAULT;
    gGpioAppPin0_20 = GPIOAPPPINOBJ_DEFAULT;

    status = BOARDInit(NULL);

    /* Initialize the UART console */
    CONSOLEUtilsInit();

    /* Select the console type based on compile time check */
    CONSOLEUtilsSetType(CONSOLE_UTILS_TYPE_UART);

    CONSOLEUtilsPrintf("\n StarterWare GPIO Application!!\n");
    CONSOLEUtilsPrintf("BOARDInit status [0x%x]\n", status);

    /* Print SoC & Board Information. */
    SOCPrintInfo();
    BOARDPrintInfo();


    uint8_t* pui8SrcHelloBuf = "hello";
    uint8_t* pui8SrcByeBuf = "goodbye";
    uint8_t pui8DestBuf[10];

    memset(pui8DestBuf, 0, 10);

    DMAUtilsInit(0, 0, NULL);

    // Get board info
    status = GpioAppBoardInfoGet(&gGpioAppCfg);
    if (S_PASS == status)
    {
        // Get SoC info
        GpioAppSocInfoGet(&gGpioAppCfg);

        gGpioAppPin0_7.pinNum = 7;
        gGpioAppPin0_7.instNum = 0;
        gGpioAppPin0_7.instAddr = 0x44E07000;
        GPIOAppInit(&gGpioAppPin0_7);

        gGpioAppPin0_20.pinNum = 20;
        gGpioAppPin0_20.instNum = 0;
        gGpioAppPin0_20.instAddr = 0x44E07000;
        gGpioAppPin0_20.pinCfg.dir = GPIO_DIRECTION_INPUT;
//        gGpioAppPin0_20.pinCfg.debounceEnable;
//        gGpioAppPin0_20.pinCfg.debounceTime;
        gGpioAppPin0_20.pinCfg.intrEnable = TRUE;
        gGpioAppPin0_20.pinCfg.intrType = GPIO_INTR_MASK_RISE_EDGE;
        gGpioAppPin0_20.pinCfg.intrLine = 0;
//        gGpioAppPin0_20.pinCfg.wakeEnable;
//        gGpioAppPin0_20.pinCfg.wakeLine;
        GPIOAppInit(&gGpioAppPin0_20);

//        GPIOIntrConfig();
        XdmaEventIntr1Config();
                                      //             len, PaSet, ChNum
        DMAMemCopy(0, 0, pui8SrcHelloBuf, pui8DestBuf, 5,     1,    32); // 22
    //    DMAMemCopy(0, 0, pui8SrcByeBuf,   pui8DestBuf, 7,     2,    22);
    //    memcpy(pui8DestBuf, pui8SrcBuf, 5);

        GPIOPinWrite(gGpioAppPin0_7.instAddr, gGpioAppPin0_7.pinNum, GPIO_PIN_HIGH);
        ui32Pins = GPIOPinRead(gGpioAppPin0_20.instAddr, gGpioAppPin0_20.pinNum);
        GPIOPinWrite(gGpioAppPin0_7.instAddr, gGpioAppPin0_7.pinNum, GPIO_PIN_LOW);

//        if(S_PASS == retStat)
        {
            memset(pui8DestBuf, 0, 10);
            DMAMemCopy(0, 0, pui8SrcByeBuf,   pui8DestBuf, 7,     2,    32);
            GPIOPinWrite(gGpioAppPin0_7.instAddr, gGpioAppPin0_7.pinNum, GPIO_PIN_HIGH);
            ui32Pins = GPIOPinRead(gGpioAppPin0_20.instAddr, gGpioAppPin0_20.pinNum);
        }

        // GPIO clock/pin mux and IP configuration
        GPIOAppInit(&gGpioAppCfg);

        CONSOLEUtilsPrintf("\nNow blinking the LED ...\n");
        while(1)
        {
            // Led blink Use Case
            GpioAppLedBlink(gLedAppCfg.delay, &gGpioAppCfg);
        }
    }
    else
    {
        CONSOLEUtilsPrintf("This example is not supported on this board!\n");
    }

    return (S_PASS);
}


/* -------------------------------------------------------------------------- */
/*                 Internal Function Definitions                              */
/* -------------------------------------------------------------------------- */

static int32_t GpioAppBoardInfoGet(gpioAppPinObj_t *pObj)
{
    int32_t status = E_FAIL;
    chipdbModuleID_t modId;
    uint32_t  gpioInstNum;
    uint32_t  pinNum;

    /* Get the GPIO data for LED from the board data */
    modId = BOARDGetDeviceCtrlModId(DEVICE_ID_LED, GPIO_LED_INST_NUM);
    if (CHIPDB_MOD_ID_INVALID == modId)
    {
        CONSOLEUtilsPrintf("Device is not available on this board!\n");
    }
    else if (CHIPDB_MOD_ID_GPIO == modId)
    {
        gpioInstNum = BOARDGetDeviceCtrlModInstNum(DEVICE_ID_LED,
            GPIO_LED_INST_NUM);
        pinNum = BOARDGetDeviceCtrlInfo(DEVICE_ID_LED, GPIO_LED_INST_NUM);

        if ((INVALID_INST_NUM == gpioInstNum) || (INVALID_INFO == pinNum))
        {
            CONSOLEUtilsPrintf("Invalid GPIO board data!\n");
        }
        else
        {
            CONSOLEUtilsPrintf("GPIO Instance number: %d\n", gpioInstNum);
            CONSOLEUtilsPrintf("Pin number: %d\n", pinNum);
            pObj->instNum = gpioInstNum;
            pObj->pinNum  = pinNum;
            status = S_PASS;
        }
    }
    else
    {
        CONSOLEUtilsPrintf("Device is not connected to GPIO!\n");
    }

    return (status);
}

static void GpioAppSocInfoGet(gpioAppPinObj_t *pObj)
{
    if(TRUE == CHIPDBIsResourcePresent(CHIPDB_MOD_ID_GPIO, pObj->instNum))
    {
        pObj->instAddr = CHIPDBBaseAddress(CHIPDB_MOD_ID_GPIO, pObj->instNum);
    }
    else
    {
        /* GPIO instance number is not present. */
    }
}

/* -------------------------------------------------------------------------- */
/*              Use-case specific Internal Function Definitions               */
/* -------------------------------------------------------------------------- */

static void GpioAppLedBlink(uint32_t delayValue, gpioAppPinObj_t *pPin)
{
    /* Driving a logic high on the GPIO pin. */
    GPIOPinWrite(pPin->instAddr, pPin->pinNum, GPIO_PIN_HIGH);
    GpioAppDelay(delayValue);

    /* Driving a logic LOW on the GPIO pin. */
    GPIOPinWrite(pPin->instAddr, pPin->pinNum, GPIO_PIN_LOW);
    GpioAppDelay(delayValue);
}

static void __attribute__((optimize("O0")))GpioAppDelay(volatile uint32_t count)
{
    while(count--);
}
