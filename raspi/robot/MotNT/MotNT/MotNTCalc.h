/***************************************************************************/
/*                             Druckersoftware                             */
/*                    Copyright (C) Carl Valentin GmbH                     */
/***************************************************************************/

#ifndef _VC_MOTNTCALC_H_
#define _VC_MOTNTCALC_H_

/*. IMPORT ================================================================*/

#include <vcplatform.h>
#include "../vccomponents_intern.h"
#include "../Utilities/curve.h"

#include "MotNT.h"

/*. ENDIMPORT =============================================================*/


/*. EXPORT ================================================================*/

/*-------------------------------- Makros ---------------------------------*/

/*--------------------------- Typdeklarationen ----------------------------*/

typedef struct SMOTNT_CALC
{
    // Init-Bereich
    MOTNT* pMot;

    // Private-Bereich
    const VC_UINT16 ui16ID;
    VC_UINT16 (*GetTime)(struct SMOTNT_CALC* psCalc, MOTNT_DIR motntDir,
                      VC_UINT16 ui16StartSpeed, VC_UINT16 ui16EndSpeed,
                      VC_UINT16 ui16Dist);
    VC_UINT16 (*GetDist)(struct SMOTNT_CALC* psCalc, MOTNT_DIR motntDir,
                      VC_UINT16 ui16StartSpeed, VC_UINT16 ui16EndSpeed,
                      MOTNT_DISTUNIT sDistUnitReturnValue);
    VC_UINT16 (*GetMaxSpeed)(struct SMOTNT_CALC* psCalc, MOTNT_DIR motntDir,
                          VC_UINT16 ui16StartSpeed, VC_UINT16 ui16EndSpeed,
                          VC_UINT16 ui16Dist);
    VC_UINT16 (*GetTime4CompleteCycle)(struct SMOTNT_CALC* psCalc, MOTNT_DIR motntDir,
                          VC_UINT16 ui16StartSpeed, VC_UINT16 ui16MaxSpeed,
                          VC_UINT16 ui16EndSpeed, VC_UINT16 ui16Dist);

    VC_UINT16* pui16MaxSpeed4DistForw; // Speed in TimerVal und Dist in Steps
    unsigned int uiMaxSpeed4DistForwSize;
    VC_UINT16* pui16MaxSpeed4DistBackw; // Speed in TimerVal und Dist in Steps
    unsigned int uiMaxSpeed4DistBackwSize;

    unsigned int* puiIncForwTimeTable;
    unsigned int* puiIncBackwTimeTable;
    unsigned int* puiDecForwTimeTable;
    unsigned int* puiDecBackwTimeTable;

    VC_BOOL bHighPrec;
} MOTNT_CALC;

/*------------------------------ Prototypen -------------------------------*/

void MotNTCalcCurve_InitStruct(MOTNT_CALC* psCalc);
VC_BOOL MotNTCalcCurve_Constructor(MOTNT_CALC* psCalc, VC_BOOL bHighPrec);
VC_BOOL MotNTCalcCurve_Update(MOTNT_CALC* psCalc,
                              MOTNT_CURVES_CONTAINER* psCurvesContainer);

/*------------------------ Konstantendeklarationen ------------------------*/

/*. ENDEXPORT =============================================================*/

#if (!defined AVOID_INLINE) || (defined MOTNTCALC_CFILE_THAT_DOES_THE_IMPLEMENT)
VC_INLINE VC_UINT16
MotNTCalc_GetTime(MOTNT_CALC* psCalc, MOTNT_DIR motntDir,
                         VC_UINT16 ui16StartSpeed, VC_UINT16 ui16EndSpeed,
                         VC_UINT16 ui16Dist)
{
    if ( (NULL != psCalc) && (NULL != psCalc->GetTime) )
    {
        return psCalc->GetTime(psCalc, motntDir, ui16StartSpeed, ui16EndSpeed,
                               ui16Dist);
    }
    return 0;
}

VC_INLINE VC_UINT16
MotNTCalc_GetDist(MOTNT_CALC* psCalc, MOTNT_DIR motntDir,
                         VC_UINT16 ui16StartSpeed, VC_UINT16 ui16EndSpeed,
                         MOTNT_DISTUNIT sDistUnitReturnValue)
{
    if ( (NULL != psCalc) && (NULL != psCalc->GetDist) )
    {
        return psCalc->GetDist(psCalc, motntDir, ui16StartSpeed, ui16EndSpeed,
                               sDistUnitReturnValue);
    }
    return 0;
}

VC_INLINE VC_UINT16
MotNTCalc_GetMaxSpeed(MOTNT_CALC* psCalc, MOTNT_DIR motntDir,
                         VC_UINT16 ui16StartSpeed, VC_UINT16 ui16EndSpeed,
                         VC_UINT16 ui16Dist)
{
    if ( (NULL != psCalc) && (NULL != psCalc->GetMaxSpeed) )
    {
        return psCalc->GetMaxSpeed(psCalc, motntDir, ui16StartSpeed,
                                   ui16EndSpeed, ui16Dist);
    }
    return 0;
}

VC_INLINE VC_UINT16
MotNTCalc_GetTime4CompleteCycle(MOTNT_CALC* psCalc, MOTNT_DIR motntDir,
                         VC_UINT16 ui16StartSpeed, VC_UINT16 ui16MaxSpeed,
                         VC_UINT16 ui16EndSpeed, VC_UINT16 ui16Dist)
{
    if ( (NULL != psCalc) && (NULL != psCalc->GetTime4CompleteCycle) )
    {
        return psCalc->GetTime4CompleteCycle(psCalc, motntDir, ui16StartSpeed,
                           ui16MaxSpeed, ui16EndSpeed, ui16Dist);
    }
    return 0;
}

#else

VC_UINT16
MotNTCalc_GetTime(MOTNT_CALC* psCalc, MOTNT_DIR motntDir,
                         VC_UINT16 ui16StartSpeed, VC_UINT16 ui16EndSpeed,
                         VC_UINT16 ui16Dist);
VC_UINT16
MotNTCalc_GetDist(MOTNT_CALC* psCalc, MOTNT_DIR motntDir,
                         VC_UINT16 ui16StartSpeed, VC_UINT16 ui16EndSpeed,
                         MOTNT_DISTUNIT sDistUnitReturnValue);
VC_UINT16
MotNTCalc_GetMaxSpeed(MOTNT_CALC* psCalc, MOTNT_DIR motntDir,
                         VC_UINT16 ui16StartSpeed, VC_UINT16 ui16EndSpeed,
                         VC_UINT16 ui16Dist);
VC_UINT16
MotNTCalc_GetTime4CompleteCycle(MOTNT_CALC* psCalc, MOTNT_DIR motntDir,
                         VC_UINT16 ui16StartSpeed, VC_UINT16 ui16MaxSpeed,
                         VC_UINT16 ui16EndSpeed, VC_UINT16 ui16Dist);

#endif

#endif // _VC_MOTNTCALC_H_
