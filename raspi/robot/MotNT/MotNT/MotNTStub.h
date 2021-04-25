/***************************************************************************/
/*                             Druckersoftware                             */
/*                    Copyright (C) Carl Valentin GmbH                     */
/***************************************************************************/

#ifndef _VC_MOTNTSTUB_H_
#define _VC_MOTNTSTUB_H_

/*. IMPORT ================================================================*/

#include "../vccomponents_intern.h"
#include <vchal.h>
#include "../Utilities/curve.h"

#include "MotNT.h"

/*. ENDIMPORT =============================================================*/

/*. EXPORT ================================================================*/

/*-------------------------------- Makros ---------------------------------*/

/*--------------------------- Typdeklarationen ----------------------------*/

typedef struct
{
    MOTNT sMotNTItf;

    // Init-Bereich
    // -------------------------------
    // Noch von Common
    // char  pcName;
    // VC_UINT16 ui16MaxSpeedForward;  // Falls nicht angegeben werden der
    // VC_UINT16 ui16MinSpeedForward;  // absoluten Min.-Wert (basierend auf
    // VC_UINT16 ui16MaxSpeedBackward; // Timer Bit-Breite) und Max.-Wert aus
    // VC_UINT16 ui16MinSpeedBackward; // obigen Tabellen genommen

    VC_STATUS
    (*CurvesInit)(struct SMOTNT* pMotNT);
    MOTNT_CURVES
    (*CurvesGetFirst)(void);

    VC_UINT16 ui16FeedPer360Degs;
    VC_UINT16 ui16Resolution;

    // Private-Bereich
    // -------------------------------
    CURVEOBJ* psCurveObjFwInc;
    CURVEOBJ* psCurveObjFwDec;
    CURVEOBJ* psCurveObjBwInc;
    CURVEOBJ* psCurveObjBwDec;
} MOTNTSTUB;

/*------------------------------ Prototypen -------------------------------*/

void
MotNTStub_InitStruct(MOTNTSTUB* pMot);
VC_BOOL
MotNTStub_Init(MOTNTSTUB* pMot);

/*------------------------ Konstantendeklarationen ------------------------*/

/*. ENDEXPORT =============================================================*/

#endif // _VC_MOTNTSTUB_H_
