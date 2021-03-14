/***************************************************************************/
/*                         Druckersoftware                                 */
/*                 Copyright (C) Carl Valentin GmbH                        */
/***************************************************************************/

#ifndef CV_SLLIST_H
#define CV_SLLIST_H

/*. IMPORT ================================================================*/

#include "../vccomponents_intern.h"

/*. ENDIMPORT =============================================================*/


/*. EXPORT ================================================================*/

/*------------------------------ Makros -----------------------------------*/

/*------------------------- Typdeklarationen ------------------------------*/

struct SSLL_Node
{
    struct SSLL_Node* pssllNodeNext;
    void*      pvContent;
};

typedef struct
{
    struct SSLL_Node* pssllNodeLast;
    unsigned long     ulLast;
} SLL_Last;

typedef struct
{
    struct SSLL_Node* pssllNodeFirst;
    struct SSLL_Node* pssllNodeLast;
    unsigned long     ulNoOfNodes;
    VC_BOOL           (*Compare)(void* pvContentCur, void* pvContentNew);
    TX_SEMAPHORE*     pnuSemaphore;
    int               iSuspend;
} SLLIST;

/*---------------------------- Prototypen ---------------------------------*/

VC_STATUS SLL_Create(SLLIST** ppSLList, VC_BOOL bThreadSave, int iSuspend,
                     VC_BOOL (*Compare)(void* pvContentCur,
                                        void* pvContentNew));
VC_STATUS SLL_Delete(SLLIST** ppSLList);
VC_STATUS SLL_Append(SLLIST* pSLList, void* pvContent);
VC_STATUS SLL_Drop(SLLIST* pSLList, void* pvContent);
VC_STATUS SLL_RemoveFirst(SLLIST* pSLList, void** ppvContent);
unsigned long SLL_GetNoOfItems(SLLIST* pSLList);
void* SLL_GetItem(SLLIST* pSLList, unsigned long ulItem,
                  SLL_Last* psllLast);

/*---------------------- Konstantendeklarationen --------------------------*/

/*. ENDEXPORT =============================================================*/

#endif // CV_SLLIST_H
