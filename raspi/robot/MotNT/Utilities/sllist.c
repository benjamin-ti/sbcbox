/***************************************************************************/
/*                             Printerfirmware                             */
/*                    Copyright (C) Carl Valentin GmbH                     */
/*                       http://www.carl-valentin.de                       */
/*                  This code is licenced under the LGPL                   */
/***************************************************************************/

/**
 *\file
 * <!-- ################################################################# -->
 *\brief Simple Linked List (SLL)\n
 *       Modul zum Erzeugen und Verwalten von einfachen verketteten Listen.
 * <!-- ################################################################# -->
*/

/*. IMPORT ================================================================*/
#include <vcplatform.h> // INSERTED BY checkIncludes.sh

#include "sllist.h"
#include <cvlog.h>

/*. ENDIMPORT =============================================================*/


/*. EXPORT ================================================================*/

/*. ENDEXPORT =============================================================*/


/*. LOCAL =================================================================*/

/*------------------------------ Makros -----------------------------------*/

/*------------------------- Typdeklarationen ------------------------------*/

/*---------------------------- Prototypen ---------------------------------*/

/*---------------------- Konstantendeklarationen --------------------------*/

/*----------------------- modulglobale Variable ---------------------------*/

/*. ENDLOCAL ==============================================================*/


/*=========================================================================*/
/**
 *\param bThreadSave Wenn VC_TRUE, dann wird Liste durch Semaphore vor
 *           gleichzeitigem Zugriff zweier Threads gesch&uuml;tzt.\n
 *           Muss auf VC_FALSE stehen wenn Liste in einem Interrupt
 *           verwendet wird.
 *\param iSuspend Wenn bThreadSave VC_TRUE, dann gibt iSuspend an, ob bei
 *           verbotenem Zugriff die aufrufende Task angehalten werden soll
 *           oder nicht.
 *           - VC_SUSPEND: Task wird angehalten\n
 *           - VC_NO_SUSPEND: Task wird nicht angehalten\n
 *           - 1 - 2147483647 (0x7FFFFFFF): Timeout-Value
 *           .
 *           Muss auf VC_NO_SUSPEND stehen wenn Liste in einem Interrupt oder
 *           Timer verwendet wird und ThreadSave genutzt wird.
 *\param Compare Zeiger auf eine Funktion, mit welcher festgestellt werden
 *           kann, ob zwei Elemente identisch sind. Wird verwendet um
 *           zu verhindern, dass das gleiche Element zwei mal in der Liste
 *           vorkommt. Wenn das keine Rolle spielt, kann dieser Zeiger
 *           auch mit NULL belegt werden.
 *\attention Diese Funktion darf nicht in ISRs aufgerufen werden
*/
VC_STATUS SLL_Create(SLLIST** ppSLList, VC_BOOL bThreadSave, int iSuspend,
                     VC_BOOL (*Compare)(void* pvContentCur,
                                        void* pvContentNew))
{
    SLLIST* pSLList;

    //. Pruefe ob Uebergabeparameter korrekt
    if (ppSLList == NULL)
    {
        CV_HANDLE_STATUS(VC_ERROR);
        return VC_ERROR;
    }
    if (*ppSLList != NULL)
    {
        CV_HANDLE_STATUS(VC_ERROR);
        return VC_ERROR;
    }
    if ( bThreadSave && (iSuspend != VC_SUSPEND) &&
        (iSuspend != VC_NO_SUSPEND) && (iSuspend < 1) )
    {
        CV_HANDLE_STATUS(VC_ERROR);
        return VC_ERROR;
    }

    //. Hole Speicher fuer Objekt
    *ppSLList = AllocMemory(sizeof(SLLIST));
    if (*ppSLList == NULL)
    {
        CV_HANDLE_STATUS(VC_ERROR);
        return VC_ERROR;
    }
    // Der Uebersichtlichkeit halber:
    pSLList = *ppSLList;

    //. Init Objekt
    memset(pSLList, 0, sizeof(SLLIST));

    if (bThreadSave)
    {
        pSLList->pnuSemaphore = AllocMemory(sizeof(TX_SEMAPHORE));
        if (pSLList->pnuSemaphore == NULL)
        {
            CV_HANDLE_STATUS(VC_ERROR);
            SLL_Delete(ppSLList);
            return VC_ERROR;
        }
        memset(pSLList->pnuSemaphore, 0, sizeof(TX_SEMAPHORE));

        IF_CHECK_STATUS_FAILED(
               tx_semaphore_create(pSLList->pnuSemaphore, "SLList", 1) )
        {
            SLL_Delete(ppSLList);
            return VC_ERROR;
        }
    }
    else
    {
        pSLList->pnuSemaphore = NULL;
    }

    pSLList->pssllNodeFirst = NULL;
    pSLList->pssllNodeLast = NULL;
    pSLList->ulNoOfNodes = 0;
    pSLList->Compare = Compare;
    pSLList->iSuspend = iSuspend;

    return VC_SUCCESS;
}


/*=========================================================================*/
/**
 *\return
 *        - VC_SUCCESS bei Erfolg
 *        - VC_ERROR bei Fehler
 *        - VC_TIMEOUT wenn bThreadSave bei SLL_Create und Timeout
 *\attention Diese Funktion darf nicht in ISRs aufgerufen werden
*/
VC_STATUS SLL_Delete(SLLIST** ppSLList)
{
    STATUS status;
    VC_STATUS cvRetStatus = VC_SUCCESS;
    SLLIST* pSLList;
    struct SSLL_Node* pssllNode;
    struct SSLL_Node* pssllNodeNext = NULL;
    unsigned long ul;
    TX_SEMAPHORE* pnuSemaphore = NULL;

    if (ppSLList == NULL)
    {
        CV_HANDLE_STATUS(VC_ERROR);
        return VC_ERROR;
    }
    if (*ppSLList == NULL)
    {
        CV_HANDLE_STATUS(VC_ERROR);
        return VC_ERROR;
    }

    pSLList = *ppSLList;

    if (pSLList->pnuSemaphore != NULL)
    {
        status = tx_semaphore_get(pSLList->pnuSemaphore,
                                     pSLList->iSuspend);
    }
    else
    {
        status = TX_SUCCESS;
    }
    if (status != TX_SUCCESS)
    {
        if (status == TX_NO_INSTANCE)
        {
            cvRetStatus = VC_TIMEOUT;
        }
        else
        {
            CV_HANDLE_STATUS(status);
            cvRetStatus = VC_ERROR;
        }
    }
    else
    {
        //. Speicher der Nodes freigeben
        pssllNode = pSLList->pssllNodeFirst;
        for (ul=0; ul<pSLList->ulNoOfNodes; ul++)
        {
            if (pssllNode == NULL)
            {
                cvRetStatus = VC_ERROR;
                break;
            }
            pssllNodeNext = pssllNode->pssllNodeNext;
            FreeMemory(pssllNode);
            pssllNode = pssllNodeNext;
        }

        //. Speicher des Objekts freigeben
        pnuSemaphore = pSLList->pnuSemaphore;
        FreeMemory(*ppSLList);

        *ppSLList = NULL;
    }

    if (pnuSemaphore != NULL)
    {
        CV_HANDLE_STATUS_F( tx_semaphore_put(pnuSemaphore) );
    }

    if (cvRetStatus == VC_SUCCESS)
    {
        tx_semaphore_delete(pnuSemaphore);
        FreeMemory(pnuSemaphore);
    }

    return cvRetStatus;
}


/*=========================================================================*/
/**
 * Sucht nach Item pvContent und gibt Zeiger auf den Node vor dem gefundenen
 * Item zur&uuml;ck. Falls das gefundene Item FirstNode ist wird allerdings
 * FirstNode-1 zur&uuml;ckgegeben. Dies muss nach Aufruf dieser Funktion
 * gepr&uuml;ft werden!\n
 * Falls die Suche erfolglos war ist der R&uuml;ckgabewert NULL.\n
*/
static struct SSLL_Node* SLL_SearchItem(SLLIST* pSLList, void* pvContent)
{
    unsigned long ul;
    struct SSLL_Node* pssllNode;
    struct SSLL_Node* pssllNodePrev = NULL;

    if (pSLList == NULL) return NULL;
    if (pvContent == NULL) return NULL;
    if (pSLList->Compare == NULL) return NULL;

    pssllNode = pSLList->pssllNodeFirst;
    for (ul=0; ul<pSLList->ulNoOfNodes; ul++)
    {
        if (pssllNode == NULL) return NULL;
        if ( pSLList->Compare(pssllNode->pvContent, pvContent) )
        {
            if (pssllNodePrev == NULL) return pssllNode-1;
            return pssllNodePrev;
        }
        pssllNodePrev = pssllNode;
        pssllNode = pssllNode->pssllNodeNext;
    }

    return NULL;
}


/*=========================================================================*/
/**
 * Fügt Item pvContent der Liste pSLList hinzu.
 *\return
 *        - VC_SUCCESS bei Erfolg
 *        - VC_ERROR bei Fehler
 *        - VC_TIMEOUT wenn bThreadSave bei SLL_Create und Timeout
*/
VC_STATUS SLL_Append(SLLIST* pSLList, void* pvContent)
{
    STATUS status;
    VC_STATUS cvRetStatus = VC_SUCCESS;
    struct SSLL_Node* pssllNode;

    //. Check if List is valid
    if (pSLList == NULL) return VC_ERROR;

    //. Check if pvContent is valid
    if (pvContent == NULL) return VC_ERROR;

    if (pSLList->pnuSemaphore != NULL)
    {
        status = tx_semaphore_get(pSLList->pnuSemaphore,
                                       pSLList->iSuspend);
    }
    else
    {
        status = TX_SUCCESS;
    }
    if (status != TX_SUCCESS)
    {
        if (status == TX_NO_INSTANCE)
        {
            cvRetStatus = VC_TIMEOUT;
        }
        else
        {
            CV_HANDLE_STATUS(status);
            cvRetStatus = VC_ERROR;
        }
    }
    else
    {
        //  Check again if List is valid
        //  Muss nach Obtain_Sema stehen weg. SLL_Delete
        if (pSLList == NULL) return VC_ERROR;

        if ( SLL_SearchItem(pSLList, pvContent) != NULL )
        {
            cvRetStatus = VC_ERROR;
        }
        else
        {
            //. Add Node to List
            pssllNode = AllocMemory(sizeof(struct SSLL_Node));
            if (pssllNode == NULL)
            {
                cvRetStatus = VC_ERROR;
            }
            else
            {
                if (pSLList->pssllNodeLast != NULL)
                {
                    pSLList->pssllNodeLast->pssllNodeNext = pssllNode;
                }
                pSLList->pssllNodeLast = pssllNode;

                if (pSLList->pssllNodeFirst == NULL)
                {
                    pSLList->pssllNodeFirst = pSLList->pssllNodeLast;
                }

                pSLList->ulNoOfNodes++;

                //. Init Node
                pssllNode->pssllNodeNext = NULL;
                pssllNode->pvContent = pvContent;
            }
        }
    }

    if (pSLList->pnuSemaphore != NULL)
    {
        CV_HANDLE_STATUS_F( tx_semaphore_put(pSLList->pnuSemaphore) );
    }

    return cvRetStatus;
}


/*=========================================================================*/
/**
 * L&ouml;scht Item pvContent aus der Liste pSLList.
 *\return
 *        - VC_SUCCESS bei Erfolg
 *        - VC_ERROR bei Fehler
 *        - VC_TIMEOUT wenn bThreadSave bei SLL_Create und Timeout
*/
VC_STATUS SLL_Drop(SLLIST* pSLList, void* pvContent)
{
    STATUS status;
    VC_STATUS cvRetStatus = VC_SUCCESS;
    struct SSLL_Node* pssllNodePrev;
    struct SSLL_Node* pssllNode;

    //. Check if List is valid
    if (pSLList == NULL) return VC_ERROR;

    //. Check if pvContent is valid
    if (pvContent == NULL) return VC_ERROR;

    if (pSLList->pnuSemaphore != NULL)
    {
        status = tx_semaphore_get(pSLList->pnuSemaphore,
                                       pSLList->iSuspend);
    }
    else
    {
        status = TX_SUCCESS;
    }
    if (status != TX_SUCCESS)
    {
        if (status == TX_NO_INSTANCE)
        {
            cvRetStatus = VC_TIMEOUT;
        }
        else
        {
            CV_HANDLE_STATUS(status);
            cvRetStatus = VC_ERROR;
        }
    }
    else
    {
        //. Check again if List is valid
        //  nach ObSema weg SLL_Delete
        if (pSLList == NULL) return VC_ERROR;

        //. Search for Content
        pssllNodePrev = SLL_SearchItem(pSLList, pvContent);
        if (pssllNodePrev == NULL)
        {
            cvRetStatus = VC_ERROR;
        }
        else
        {
            //. Drop from List
            if (pssllNodePrev == pSLList->pssllNodeFirst-1)
            {
                pssllNodePrev = NULL;
                pssllNode = pSLList->pssllNodeFirst;
                pSLList->pssllNodeFirst = pssllNode->pssllNodeNext;
            }
            else
            {
                pssllNode = pssllNodePrev->pssllNodeNext;
                pssllNodePrev->pssllNodeNext = pssllNode->pssllNodeNext;
            }
            if (pssllNode == pSLList->pssllNodeLast)
            {
                pSLList->pssllNodeLast = pssllNodePrev;
            }

            FreeMemory(pssllNode);

            pSLList->ulNoOfNodes--;
        }
    }

    if (pSLList->pnuSemaphore != NULL)
    {
        CV_HANDLE_STATUS_F( tx_semaphore_put(pSLList->pnuSemaphore) );
    }

    return cvRetStatus;
}

/*=========================================================================*/
/**
 * Entfernt das erste Item aus der Liste pSLList.
 *\return
 *        - VC_SUCCESS bei Erfolg
 *        - VC_ERROR bei Fehler
 *        - VC_TIMEOUT wenn bThreadSave bei SLL_Create und Timeout
 *\attention Es w
 *           Die aufrufende Funktion muß das managen.
*/
VC_STATUS SLL_RemoveFirst(SLLIST* pSLList, void** ppvContent)
{
    STATUS status = TX_SUCCESS;;
    VC_STATUS cvRetStatus = VC_SUCCESS;
    struct SSLL_Node* pssllNode;

    //. Check if List is valid
    if (pSLList == NULL) return VC_ERROR;

    //. Check if pvContent is valid
    if (ppvContent == NULL) return VC_ERROR;
    *ppvContent = NULL;  // Rückgabewert initialisieren

    if (pSLList->pnuSemaphore != NULL)
    {
        status = tx_semaphore_get(pSLList->pnuSemaphore,
                                       pSLList->iSuspend);
    }

    if (status != TX_SUCCESS)
    {
        if (status == TX_NO_INSTANCE)
        {
            cvRetStatus = VC_TIMEOUT;
        }
        else
        {
            CV_HANDLE_STATUS(status);
            cvRetStatus = VC_ERROR;
        }
    }
    else
    {
        //. Check again if List is valid
        //  nach ObSema weg SLL_Delete
        if (pSLList == NULL) return VC_ERROR;


        pssllNode = pSLList->pssllNodeFirst;
        if (pssllNode)
        {
            // Den ersten Node aus der Liste heraus nehmen
            pSLList->pssllNodeFirst = pssllNode->pssllNodeNext;
            if (pssllNode == pSLList->pssllNodeLast)
            {
                // Wenn es der letzte Node war, Zeiger anpasssen
                pSLList->pssllNodeLast = NULL;
            }
            pSLList->ulNoOfNodes--;

            // Den Zeiger auf das Item retten
            *ppvContent = pssllNode->pvContent;

            FreeMemory(pssllNode);
        }
        else
        {
            // Kein Eintrag in Liste -> Fehler
            cvRetStatus = VC_ERROR;
        }
    }

    if (pSLList->pnuSemaphore != NULL)
    {
        CV_HANDLE_STATUS_F( tx_semaphore_put(pSLList->pnuSemaphore) );
    }

    return cvRetStatus;
}



/*=========================================================================*/
/**
 *\return
 *        - > 0 bei Erfolg
 *        - 0 bei Fehler
 *        - -1 wenn bThreadSave bei SLL_Create und Timeout
*/
unsigned long SLL_GetNoOfItems(SLLIST* pSLList)
{
    STATUS status;
    unsigned long ulNoOfNodes = 0;

    //. Check if List is valid
    if (pSLList == NULL) return 0;

    if (pSLList->pnuSemaphore != NULL)
    {
        status = tx_semaphore_get(pSLList->pnuSemaphore,
                                       pSLList->iSuspend);
    }
    else
    {
        status = TX_SUCCESS;
    }
    if (status != TX_SUCCESS)
    {
        if (status == TX_NO_INSTANCE)
        {
            ulNoOfNodes = -1;
        }
        else
        {
            CV_HANDLE_STATUS(status);
            ulNoOfNodes = 0;
        }
    }
    else
    {
        //. Check again if List is valid
        //  nach ObSema weg SLL_Delete
        if (pSLList == NULL) return VC_ERROR;

        ulNoOfNodes = pSLList->ulNoOfNodes;
    }

    if (pSLList->pnuSemaphore != NULL)
    {
        CV_HANDLE_STATUS_F( tx_semaphore_put(pSLList->pnuSemaphore) );
    }

    return ulNoOfNodes;
}


/*=========================================================================*/
/**
 * Gibt das Item ulItem aus der Liste pSLList zur&uuml;ck. Falls sich an der
 * Position ulItem kein Item befindet wird NULL zur&uuml;ckgegeben.\n
 *
 *\param psllLast Zeiger auf eine Struktur in der das letzte Ergebniss
 *           festgehalten wird. Dies dient zur Optimierung der Laufzeit,
 *           da sonst die komplette Liste bei jedem Funktionsaufruf
 *           aufs neue Durchlaufen werden m&uuml;sste.\n
 *           Spielt die Laufzeit keine Rolle, so kann hier auch NULL
 *           angegeben werden (siehe Beispiele).
 *\return
 *        - Zeiger auf Item bei Erfolg
 *        - NULL bei Fehler
 *        - NULL-1 wenn bThreadSave bei SLL_Create und Timeout
 *
 * Aufrufbeispiel zum durchsuchen der kompletten Liste:
 *\verbatim
  SLL_Last sllLast;
  void* pvContent;
  unsigned long ul;
  unsigned long ulNoOfItems;
  ulNoOfItems = SLL_GetNoOfItems(pSLList);
  memset(&sllLast, 0, sizeof(SLL_Last));
  for (ul=0; ul<ulNoOfItems; ul++)
  {
      pvContent = SLL_GetItem(pSLList, ul, &sllLast);
      DoSthWithContent(pvContent);
  }
  \endverbatim
 *\attention Vor der ersten Verwendung sollte sllLast immer auf 0 gesetzt
 * sein!
 *
 *\verbatim
  void* pvContent;
  unsigned long ul;
  for (ul=0; ul<SLL_GetNoOfItems(pSLList); ul++)
  {
      pvContent = SLL_GetItem(pSLList, ul, NULL);
      DoSthWithContent(pvContent);
  }
  \endverbatim
*/
void* SLL_GetItem(SLLIST* pSLList, unsigned long ulItem,
                  SLL_Last* psllLast)
{
    STATUS status;
    unsigned long ul;
    unsigned long ulLast = 0;
    struct SSLL_Node* pssllNode = NULL;
    void* pvRet = NULL;

    //. Check if List is valid
    if (pSLList == NULL) return NULL;

    if (pSLList->pnuSemaphore != NULL)
    {
        status = tx_semaphore_get(pSLList->pnuSemaphore,
                                     pSLList->iSuspend);
    }
    else
    {
        status = TX_SUCCESS;
    }
    if (status != TX_SUCCESS)
    {
        if (status == TX_NO_INSTANCE)
        {
            pvRet = (void*)(NULL-1);
        }
        else
        {
            CV_HANDLE_STATUS(status);
            pvRet = NULL;
        }
    }
    else
    {
        //. Check again if List is valid
        //  nach ObSema weg SLL_Delete
        if (pSLList == NULL) return NULL;

        //. Check if there is a info of last Call and handle it
        if (psllLast != NULL)
        {
            if (psllLast->pssllNodeLast != NULL)
            {
                pssllNode = psllLast->pssllNodeLast;
                ulLast = psllLast->ulLast;
                // if we have it allready, return it.
                if (ulLast == ulItem)
                {
                    if (pSLList->pnuSemaphore != NULL)
                    {
                        CV_HANDLE_STATUS_F(
                            tx_semaphore_put(pSLList->pnuSemaphore) );
                    }
                    return pssllNode;
                }
            }
        }
        if ( (pssllNode == NULL) || (ulLast > ulItem) )
        {
            pssllNode = pSLList->pssllNodeFirst;
            ulLast = 0;
        }

        //. Go to Item
        for (ul=ulLast; ul<ulItem; ul++)
        {
            if (pssllNode == NULL)
            {
                if (pSLList->pnuSemaphore != NULL)
                {
                    CV_HANDLE_STATUS_F( tx_semaphore_put(pSLList->pnuSemaphore) );
                }
                return NULL;
            }
            pssllNode = pssllNode->pssllNodeNext;
        }

        //. Return Item
        psllLast->pssllNodeLast = pssllNode;
        psllLast->ulLast = ul;

        pvRet = pssllNode->pvContent;
    }

    if (pSLList->pnuSemaphore != NULL)
    {
        CV_HANDLE_STATUS_F( tx_semaphore_put(pSLList->pnuSemaphore) );
    }

    return pvRet;
}
