#include <xdc/runtime/Error.h>
#include <xdc/runtime/Log.h>
#include <ti/ipc/MessageQ.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>

#define IPU1MSGQNAME "Ipu1:MsgQ:1"

typedef struct {
    MessageQ_MsgHeader  reserved;
    UInt32              cmd;
} MyMsg;


Void smain(UArg arg0, UArg arg1)
{
    int status;
    MessageQ_Params msgqParams;
    MessageQ_Handle ipu1msgQ;
    MyMsg*          pMyMsg;

    Log_info0("--> smain:");

    MessageQ_Params_init(&msgqParams);
    ipu1msgQ = MessageQ_create(IPU1MSGQNAME, &msgqParams);
    if (ipu1msgQ == NULL) {
        Log_info0("MessageQ_create failed");
        goto main_end;
    }

    status = MessageQ_get(ipu1msgQ, (MessageQ_Msg*)&pMyMsg,
        MessageQ_FOREVER);
    if (status < 0) {
        Log_info1("MessageQ_get failed: %i", status);
        goto main_end;
    }

    /* process the message */
    Log_info1("Msg: cmd=0x%x", pMyMsg->cmd);

    status = MessageQ_free((MessageQ_Msg)pMyMsg);
    if (status < 0) {
        Log_info1("MessageQ_free failed: %i", status);
    }

    status = MessageQ_delete(&ipu1msgQ);
    if (status < 0) {
        Log_info1("MessageQ_delete failed: %i", status);
    }

main_end:
    Log_info0("<-- smain:");
}

Int main(Int argc, Char* argv[])
{
    Error_Block     eb;
    Task_Params     taskParams;

    Log_info0("--> main:");

    Error_init(&eb);

    Task_Params_init(&taskParams);
    taskParams.instance->name = "smain";
    taskParams.arg0 = (UArg)argc;
    taskParams.arg1 = (UArg)argv;
    taskParams.stackSize = 0x1000;
    Task_create(smain, &taskParams, &eb);

    if (Error_check(&eb)) {
        Log_info0("main: failed to create application startup thread");
    }

    BIOS_start();

    Log_info0("<-- main:");

    return 0;
}
