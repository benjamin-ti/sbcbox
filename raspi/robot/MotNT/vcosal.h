#ifndef VCOSAL_H
#define VCOSAL_H

#include <vcplatform.h>

#define TX_SEMAPHORE int

#define VC_NO_SUSPEND 0
#define VC_SUSPEND 1

#define TX_NO_INSTANCE -1

#define TX_WAIT_FOREVER 1

VC_STATUS tx_semaphore_create(TX_SEMAPHORE* sema, char* name, int initVal);
VC_STATUS tx_semaphore_get(TX_SEMAPHORE* sema, int suspend);
VC_STATUS tx_semaphore_put(TX_SEMAPHORE* sema);
void tx_semaphore_delete(TX_SEMAPHORE* sema);

#endif // VCOSAL_H
