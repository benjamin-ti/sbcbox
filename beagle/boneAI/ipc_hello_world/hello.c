#include <xdc/runtime/Log.h>
#include <xdc/runtime/Error.h>
#include <ti/drv/gpio/GPIO.h>
#include <ti/drv/gpio/test/led_blink/src/GPIO_board.h>

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>

static Void smain(UArg arg0, UArg arg1);

// ti_ipc_remoteproc_ResourceTable
// processor_sdk_rtos_am57xx_06_03_02_08/ipc_3_50_04_08/packages/ti/ipc/remoteproc/rsc_table_vayu_ipu.h

Int main(Int argc, Char* argv[])
{
    Task_Params     taskParams;

    Log_info0("--> main:");

    /* GPIO initialization */
    GPIO_init();

    /* Set the callback function */
    //GPIO_setCallback(USER_LED0, AppGpioCallbackFxn);
    /* Enable GPIO interrupt on the specific gpio pin */
    //GPIO_enableInt(USER_LED0);

    // GPIO 5_0 -> P9-12
    GPIO_write((USER_LED0), GPIO_PIN_VAL_HIGH);
    // *(uint32_t*)0x6805B194 = 0x00000001;

/*
    // create main thread (interrupts not enabled in main on BIOS)
    Task_Params_init(&taskParams);
    taskParams.instance->name = "smain";
    taskParams.arg0 = (UArg)argc;
    taskParams.arg1 = (UArg)argv;
    taskParams.stackSize = 0x1000;
    Task_create(smain, &taskParams, NULL);

    // start scheduler, this never returns
    BIOS_start();
*/
    // should never get here
    Log_info0("<-- main:");
}


// /home/user/ti/processor_sdk_rtos_am57xx_06_03_02_08/pdk_am57xx_1_0_18/packages/ti/drv/gpio/test/led_blink/am572x/m4/bios
Void smain(UArg arg0, UArg arg1)
{
    // cat /sys/kernel/debug/remoteproc/remoteproc0/trace0
    Log_info0("hello world");

    /* GPIO initialization */
    GPIO_init();
    // *(uint32_t*)0x6805B194 = 0x00000001;

    /* Set the callback function */
    //GPIO_setCallback(USER_LED0, AppGpioCallbackFxn);
    /* Enable GPIO interrupt on the specific gpio pin */
    //GPIO_enableInt(USER_LED0);
    GPIO_write((USER_LED0), GPIO_PIN_VAL_HIGH);

    Log_info1("bye world: %u", 5);

    return;
}
