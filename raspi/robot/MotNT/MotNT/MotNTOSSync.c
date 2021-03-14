/***************************************************************************/
/*                             Druckersoftware                             */
/*                    Copyright (C) Carl Valentin GmbH                     */
/***************************************************************************/


/*. IMPORT ================================================================*/

#include <vcplatform.h>


#define CV_LOG_GROUP "Components_Motor"
#include <cvlog.h>

#include <vccomponents.h>

/*. ENDIMPORT =============================================================*/


/*. EXPORT ================================================================*/

/*. ENDEXPORT =============================================================*/


/*. LOCAL =================================================================*/

/*-------------------------------- Makros ---------------------------------*/

/*--------------------------- Typdeklarationen ----------------------------*/

/*------------------------------ Prototypen -------------------------------*/

/*------------------------ Konstantendeklarationen ------------------------*/

/*------------------------- modulglobale Variable -------------------------*/

/*. ENDLOCAL ==============================================================*/

/*=========================================================================*/
void
MotNTOSSync_Create(MOTNT* psMot)
{
    char* pcName;
    if (psMot == NULL)
        return;
    pcName = AllocMemory(strlen(psMot->pcName)+10);
    sprintf(pcName, "%sStrtStop", psMot->pcName);
    CV_HANDLE_STATUS_A_F( tx_semaphore_create(&psMot->sSemaStartStop, pcName, 1) );
#ifdef VALENTIN
    CV_HANDLE_STATUS_A_F( tx_event_flags_create(&psMot->sEvent, psMot->pcName) );
#endif
}

/*=========================================================================*/
VC_BOOL
MotNTOSSync_StartReal(MOTNT* psMot, VC_UINT16 ui16Speed,
                      VC_UINT16 ui16Dist, MOTNT_DIR motntDirection,
                      const char* pcFile, int iLine)
{
    VC_BOOL bRet;
    if (psMot == NULL)
        return VC_FALSE;
    tx_semaphore_get(&psMot->sSemaStartStop, TX_WAIT_FOREVER);
    bRet = MotNT_StartReal(psMot, ui16Speed, ui16Dist, motntDirection,
                           (char*)pcFile, iLine);

    tx_semaphore_put(&psMot->sSemaStartStop);
    return bRet;
}

/*=========================================================================*/
VC_BOOL
MotNTOSSync_StartFlexReal(MOTNT* psMot, VC_UINT16 ui16Speed, VC_UINT16 ui16Dist,
                      MOTNT_DIR motntDirection, VC_BOOL bExecAddToFunctions,
                      MOTNT_STARTFLEX_PARM* psParm,
                      const char* pcFile, int iLine)
{
    VC_BOOL bRet = VC_TRUE;
    if (psMot == NULL)
        return VC_FALSE;
    tx_semaphore_get(&psMot->sSemaStartStop, TX_WAIT_FOREVER);
    bRet = MotNT_StartFlexReal(psMot, ui16Speed, ui16Dist, motntDirection,
                               bExecAddToFunctions, psParm,
                               pcFile, iLine);

    tx_semaphore_put(&psMot->sSemaStartStop);
    return bRet;
}

/*=========================================================================*/
void
MotNTOSSync_StopReal(MOTNT* psMot, const char* pcFile, int iLine)
{
    if (psMot == NULL)
        return;
    tx_semaphore_get(&psMot->sSemaStartStop, TX_WAIT_FOREVER);
    MotNT_StopReal(psMot, (char*)pcFile, iLine);
    tx_semaphore_put(&psMot->sSemaStartStop);
}

/*=========================================================================*/
VC_STATUS
MotNTOSSync_WaitTillStoppedTimeout(MOTNT* psMot, unsigned int uiTimeout)
{
    UNSIGNED uiActualEvents;
    unsigned int uiStatus;
    unsigned int uiTimeoutInTicks;

    if (psMot == NULL)
        return VC_ERROR;

#ifdef VALENTIN
    if ( (uiTimeout != TX_NO_WAIT) && (uiTimeout != TX_WAIT_FOREVER) )
    {
        uiTimeoutInTicks = MSECSTOTICKS(uiTimeout);
    }
    else
    {
        uiTimeoutInTicks = uiTimeout;
    }

    uiStatus = tx_event_flags_get(&psMot->sEvent, MOTNT_EVENT_STOPPED, TX_OR,
                                  &uiActualEvents, uiTimeoutInTicks);
    if ( (uiStatus != TX_SUCCESS) && (uiStatus != TX_NO_EVENTS) )
    {
        CV_HANDLE_STATUS_A(uiStatus);
        return VC_ERROR;
    }

    if (uiStatus == TX_NO_EVENTS)
    {
        return VC_TIMEOUT;
    }
#endif

    return VC_SUCCESS;
}

/*=========================================================================*/
VC_STATUS
MotNTOSSync_WaitTillStopped(MOTNT* psMot)
{
    return MotNTOSSync_WaitTillStoppedTimeout(psMot, TX_WAIT_FOREVER);
}
