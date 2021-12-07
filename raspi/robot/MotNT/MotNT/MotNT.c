/***************************************************************************/
/*                             Printerfirmware                             */
/*                    Copyright (C) Carl Valentin GmbH                     */
/*                       http://www.carl-valentin.de                       */
/*                  This code is licenced under the LGPL                   */
/***************************************************************************/


/* IMPORT =================================================================*/

#include <vcplatform.h>


#define CV_LOG_GROUP "Components_Motor"
#include <cvlog.h>

#include <vccomponents.h>

/* ENDIMPORT ==============================================================*/


/* EXPORT =================================================================*/

/* ENDEXPORT ==============================================================*/


/* LOCAL ==================================================================*/

/*-------------------------------- macros ---------------------------------*/

/*--------------------------- type declaration ----------------------------*/

/*------------------------------ prototypes -------------------------------*/

/*------------------------- constant declarations -------------------------*/

/*---------------------------- local variables ----------------------------*/

/* ENDLOCAL ===============================================================*/

/*=========================================================================*/
VC_STATUS
MotNT_CurvesCreate(MOTNT* psMotNT, MOTNT_CURVES_CONTAINER* psCurvesContainer,
                   CURVE_BASE_POINTS sCurveBasePointsFwInc,
                   CURVE_BASE_POINTS sCurveBasePointsFwDec,
                   CURVE_BASE_POINTS sCurveBasePointsBwInc,
                   CURVE_BASE_POINTS sCurveBasePointsBwDec,
                   char* pcName)
{
    unsigned int uiSize;

    uiSize = strlen(pcName);

    if (psCurvesContainer == NULL)
        return VC_ERROR;

    // FwInc
    psCurvesContainer->pcNameFwInc = AllocMemory(uiSize+6);
    snprintf(psCurvesContainer->pcNameFwInc, uiSize+6, "%sFwInc", pcName);
    RETURN_IF_CHECK_STATUS_A_FAILED(VC_ERROR,
        MotNT_CurveObjCreate(psMotNT,
                &psCurvesContainer->sCurveObjFwInc,
                psCurvesContainer->pcNameFwInc, VC_TRUE,
                &sCurveBasePointsFwInc) );

    // FwDec
    psCurvesContainer->pcNameFwDec = AllocMemory(uiSize+6);
    snprintf(psCurvesContainer->pcNameFwDec, uiSize+6, "%sFwDec", pcName);
    RETURN_IF_CHECK_STATUS_A_FAILED(VC_ERROR,
        MotNT_CurveObjCreate(psMotNT,
                &psCurvesContainer->sCurveObjFwDec,
                psCurvesContainer->pcNameFwDec, VC_FALSE,
                &sCurveBasePointsFwDec) );

    // BwInc
    psCurvesContainer->pcNameBwInc = AllocMemory(uiSize+6);
    snprintf(psCurvesContainer->pcNameBwInc, uiSize+6, "%sBwInc", pcName);
    RETURN_IF_CHECK_STATUS_A_FAILED(VC_ERROR,
        MotNT_CurveObjCreate(psMotNT,
                &psCurvesContainer->sCurveObjBwInc,
                psCurvesContainer->pcNameBwInc, VC_TRUE,
                &sCurveBasePointsBwInc) );

    // BwDec
    psCurvesContainer->pcNameBwDec = AllocMemory(uiSize+6);
    snprintf(psCurvesContainer->pcNameBwDec, uiSize+6, "%sBwDec", pcName);
    RETURN_IF_CHECK_STATUS_A_FAILED(VC_ERROR,
        MotNT_CurveObjCreate(psMotNT,
                &psCurvesContainer->sCurveObjBwDec,
                psCurvesContainer->pcNameBwDec, VC_FALSE,
                &sCurveBasePointsBwDec) );

    return VC_SUCCESS;
}

/*=========================================================================*/
VC_STATUS
MotNT_CurvesDelete(MOTNT_CURVES_CONTAINER* psCurvesContainer)
{
    VC_STATUS sStatus = VC_SUCCESS;

    if (psCurvesContainer == NULL)
        return VC_ERROR;

    IF_CHECK_STATUS_A_FAILED(
            Curve_Delete(&psCurvesContainer->sCurveObjFwInc) )
    {
        sStatus = VC_ERROR;
    }
    IF_CHECK_STATUS_A_FAILED(
            Curve_Delete(&psCurvesContainer->sCurveObjFwDec) )
    {
        sStatus = VC_ERROR;
    }
    IF_CHECK_STATUS_A_FAILED(
            Curve_Delete(&psCurvesContainer->sCurveObjBwInc) )
    {
        sStatus = VC_ERROR;
    }
    IF_CHECK_STATUS_A_FAILED(
            Curve_Delete(&psCurvesContainer->sCurveObjBwDec) )
    {
        sStatus = VC_ERROR;
    }

    if (sStatus != VC_SUCCESS) return sStatus;

    FreeMemory(psCurvesContainer->pcNameFwInc);
    FreeMemory(psCurvesContainer->pcNameFwDec);
    FreeMemory(psCurvesContainer->pcNameBwInc);
    FreeMemory(psCurvesContainer->pcNameBwDec);

    return sStatus;
}

/*=========================================================================*/
MOTNT_CURVES
MotNT_CurvesGet(MOTNT_CURVES_CONTAINER* psCurvesContainer)
{
    MOTNT_CURVES sCurves;

    memset(&sCurves, 0, sizeof(MOTNT_CURVES));

    if (psCurvesContainer != NULL)
    {
        sCurves.psCurveObjFwInc = &psCurvesContainer->sCurveObjFwInc;
        sCurves.psCurveObjFwDec = &psCurvesContainer->sCurveObjFwDec;
        sCurves.psCurveObjBwInc = &psCurvesContainer->sCurveObjBwInc;
        sCurves.psCurveObjBwDec = &psCurvesContainer->sCurveObjBwDec;
    }

    return sCurves;
}
