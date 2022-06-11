#include <stdio.h>

#include <ti/ipc/Std.h>
#include <ti/ipc/Ipc.h>
#include <ti/ipc/transports/TransportRpmsg.h>
#include <ti/ipc/MessageQ.h>

#include <ti/ipc/MultiProc.h>
#include <ti/ipc/Notify.h>

// Don't forget to start as root (or with sudo) !!!

#define IPU1MSGQNAME "Ipu1:MsgQ:1"
#define MSGHEAPID 0

typedef struct {
    MessageQ_MsgHeader  reserved;
    UInt32              cmd;
} MyMsg;

int main(int argc, char* argv[])
{
    int status;

    MessageQ_QueueId    ipu1msgQ = MessageQ_INVALIDMESSAGEQ;
    MyMsg*              pMyMsg;
    int i;

    printf("--> main:\n");

    Ipc_transportConfig(&TransportRpmsg_Factory);
    status = Ipc_start();
    if (status < 0) {
        printf("Ipc_start failed: status = %d\n", status);
        return status;
    }

    pMyMsg = (MyMsg*)MessageQ_alloc(MSGHEAPID, sizeof(MyMsg));
    if (pMyMsg == NULL) {
        printf("MessageQ_alloc failed\n");
        goto main_end;
    }

    i = 0;
    do {
        status = MessageQ_open(IPU1MSGQNAME, &ipu1msgQ);
        sleep(1);
        i++;
    } while (status==MessageQ_E_NOTFOUND && i<10);
    if (status < 0) {
        printf("MessageQ_open failed\n");
        goto main_end;
    }

    pMyMsg->cmd = 5;
    MessageQ_put(ipu1msgQ, (MessageQ_Msg)pMyMsg);

    status = MessageQ_close(&ipu1msgQ);
    if (status < 0) {
        printf("MessageQ_close failed\n");
    }

main_end:
    Ipc_stop();

    printf("<-- main:\n");

    return 0;
}
