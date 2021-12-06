/***************************************************************************/
/*                             Printerfirmware                             */
/*                    Copyright (C) Carl Valentin GmbH                     */
/*                       http://www.carl-valentin.de                       */
/*                  This code is licenced under the LGPL                   */
/***************************************************************************/

#include <vcplatform.h>
#include <vcosal.h>

VC_STATUS tx_semaphore_create(TX_SEMAPHORE* sema, char* name, int initVal)
{
    return TX_SUCCESS;
}

VC_STATUS tx_semaphore_get(TX_SEMAPHORE* sema, int suspend)
{
    return TX_SUCCESS;
}

VC_STATUS tx_semaphore_put(TX_SEMAPHORE* sema)
{
    return TX_SUCCESS;
}

void tx_semaphore_delete(TX_SEMAPHORE* sema)
{

}
