/***************************************************************************/
/*                             Printerfirmware                             */
/*                    Copyright (C) Carl Valentin GmbH                     */
/*                       http://www.carl-valentin.de                       */
/*                  This code is licenced under the LGPL                   */
/***************************************************************************/

#ifndef _VC_CURVE_H_
#define _VC_CURVE_H_

/*. IMPORT ================================================================*/

#include <vcplatform.h>


/*. ENDIMPORT =============================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*. EXPORT ================================================================*/

/*-------------------------------- Makros ---------------------------------*/

#define CURVE_ID 0xEA078078
#define CURVE_MAX_EXT_CONTENT 10

#define CREATE_CURVE_BASE_POINTS(PUI_CURVE_X, PUI_CURVE_Y)                  \
        CreateCurveBasePoints(PUI_CURVE_X, PUI_CURVE_Y,                     \
                              sizeof(PUI_CURVE_X) / sizeof(unsigned int))

/*--------------------------- Typdeklarationen ----------------------------*/

// Bei Aenderung hier lk_curveNull nicht vergessen anzupassen
typedef struct
{
    const unsigned int* puiCurveTable;
    unsigned int uiCurveTableSize;
    unsigned int uiStartX;
    unsigned int uiStartY;
} CURVE;

// Bei Aenderung hier lk_sCurveBasePointsNull nicht vergessen anzupassen
typedef struct
{
    unsigned int uiStartX;
    unsigned int uiStartY;
    unsigned short usNoOfBasePoints;
    unsigned int* puiBasePointsX;
    unsigned int* puiBasePointsY;
    unsigned int  uiEndX;
    unsigned int  uiEndY;
} CURVE_BASE_POINTS;

typedef struct
{
    void* pvPointer;
    unsigned int uiPointerInfo;
    VC_BOOL bFreeMemOnCurveDelete;
    VC_BOOL bValid; // Internal use only
} CURVE_EXTERNCONTENT;

typedef struct SCURVEOBJ
{
    char* pcName;
    VC_BOOL bIncrease;
    CURVE_BASE_POINTS sCurveBasePoints;
    CURVE_BASE_POINTS sCurveBasePointsOrg;
    unsigned int (*GetNextX)(unsigned int uiX, unsigned int uiY,
                             int argc, const char* argv[]);
    unsigned int (*ChangeIdealY2TabelVal)(unsigned int uiIdealY,
                                          int argc, const char* argv[]);
    unsigned int (*ChangeY2Ideal)(unsigned int uiY, int argc,
                                  const char* argv[]);
    int argc;
    const char** argv;

    VC_BOOL bSimpleNoBasepoints;
    VC_BOOL bEndYAsMaxVal;

    unsigned int* puiCurveTable;
    unsigned int  uiCurveTableSize;
    unsigned short usCurveGetIndexVal;
    unsigned int* puiCurveGetIndexTable;
    unsigned int  uiStartX;
    unsigned int  uiStartY;
    unsigned int  uiLockCounter;

    unsigned int* puiCurveTableFreeMem;
    unsigned int* puiCurveGetIndexTableFreeMem;

    unsigned int* puiCurveTableNew;
    unsigned int  uiCurveTableSizeNew;
    unsigned short usCurveGetIndexValNew;
    unsigned int* puiCurveGetIndexTableNew;
    unsigned int  uiStartXNew;
    unsigned int  uiStartYNew;

    VC_BOOL bAutoSwitch2UpdatedCurve;
    void (*InformOnUpdate)(struct SCURVEOBJ* psCurveObj, VC_BOOL bLocked,
            int argc, const char* argv[]);

    VC_BOOL bError;
    const VC_UINT32 ui32ID;

    CURVE_EXTERNCONTENT psExtContent[CURVE_MAX_EXT_CONTENT];
} CURVEOBJ;

typedef struct {
    char* pcCurveName;
    char* pcXName;
    char* pcYName;
    char* pcXUnit;
    char* pcYUnit;
    unsigned int uiScaleX;
    unsigned int uiScaleY;
    char* pcMaxX;
    char* pcMaxY;
#ifdef VALENTIN
    EPRINTER_CFG ePrinterCfg;
#endif
    VC_BOOL bSaveWithPmPara;
} CURVEOBJ_PARM_REG;

/*------------------------------ Prototypen -------------------------------*/

VC_STATUS Curve_Create(CURVEOBJ* psCurveObj, char* pcName, VC_BOOL bIncrease,
                       CURVE_BASE_POINTS* psCurveBasePoints,
                       unsigned int (*GetNextX)(unsigned int uiX,
                                                unsigned int uiY,
                                                int argc, const char* argv[]),
                       unsigned int (*ChangeIdealY2TabelVal)(unsigned int
                                                             uiIdealY,
                                                             int argc,
                                                             const char* argv[]),
                       int argc, const char* argv[],
                       VC_BOOL bAutoSwitch2UpdatedCurve,
                       void (*InformOnUpdate)(CURVEOBJ* psCurveObj,
                               VC_BOOL bLocked, int argc, const char* argv[]));
VC_STATUS Curve_CreateSimple(CURVEOBJ* psCurveObj, char* pcName, VC_BOOL bIncrease,
                             CURVE sCurve, VC_BOOL bAutoSwitch2UpdatedCurve,
                             void (*InformOnUpdate)(CURVEOBJ* psCurveObj,
                                     VC_BOOL bSwitchLocked, int argc, const char* argv[]),
                             int argc, const char* argv[]);
VC_STATUS    Curve_Delete(CURVEOBJ* psCurveObj);

VC_STATUS    Curve_SetAutoSwitch2UpdatedCurve(CURVEOBJ* psCurveObj, VC_BOOL bSet);
void         Curve_SetInformOnUpdate(CURVEOBJ* psCurveObj,
                                     void (*InformOnUpdate)(CURVEOBJ* psCurveObj,
                                             VC_BOOL bLocked,
                                             int argc, const char* argv[]));
VC_STATUS    Curve_Update(CURVEOBJ* psCurveObj,
                          CURVE_BASE_POINTS* psCurveBasePoints);
VC_STATUS    Curve_UpdateSimple(CURVEOBJ* psCurveObj,
                                CURVE sCurve);
void         Curve_InformUpdate(CURVEOBJ* psCurveObj);
VC_STATUS    Curve_Switch2UpdatedCurve(CURVEOBJ* psCurveObj,
                                       VC_BOOL bFreeMemOnNextUpd);
void         Curve_FreeUnusedMem(CURVEOBJ* psCurveObj);

CURVE        Curve_Get(CURVEOBJ* psCurveObj);
CURVE        Curve_GetAndLock(CURVEOBJ* psCurveObj);
void         Curve_Unlock(CURVEOBJ* psCurveObj);

CURVE_BASE_POINTS Curve_GetBasePoints(CURVEOBJ* psCurveObj);
unsigned int Curve_GetIndex(CURVEOBJ* psCurveObj, unsigned int uiY);
unsigned int Curve_GetIndexHiPrec(CURVEOBJ* psCurveObj, unsigned int uiY);

VC_STATUS    Curve_ExtContentAdd(CURVEOBJ* psCurveObj,
                                 CURVE_EXTERNCONTENT sExtContent,
                                 unsigned int uiID);
VC_STATUS    Curve_ExtContentGet(CURVEOBJ* psCurveObj,
                                 CURVE_EXTERNCONTENT* psExtContent,
                                 unsigned int uiID);
VC_STATUS    Curve_ExtContentReplace(CURVEOBJ* psCurveObj,
                                     CURVE_EXTERNCONTENT sExtContent,
                                     unsigned int uiID);
VC_STATUS    Curve_ExtContentDel(CURVEOBJ* psCurveObj, unsigned int uiID);

void CurveObjParm_InitReg(CURVEOBJ_PARM_REG* psCurveObjParmReg);
void CurveObjParm_RegTablePoints(CURVEOBJ_PARM_REG* psCurveObjParmReg,
                                 CURVEOBJ* psCurveObj, char* pcMenuID);

#ifdef VALENTIN
void Curve_Reg4CVPL(CURVEOBJ* psCurveObj,
                    unsigned int (*ChangeY2Ideal)(unsigned int uiY, int argc,
                                                  const char* argv[]));
void Curve_CVPL(char *pcData, unsigned long uiLength, unsigned char ucPort,
                SDCB* psDCB);
#endif

/*------------------------ Konstantendeklarationen ------------------------*/

/*. ENDEXPORT =============================================================*/

/*=========================================================================*/
static inline CURVE_BASE_POINTS
                  CreateCurveBasePoints(unsigned int* puiCurveX,
                                        unsigned int* puiCurveY,
                                        unsigned int uiSizeOfCurve)
{
    CURVE_BASE_POINTS curveBasePoints;

    curveBasePoints.uiStartX = 0;
    curveBasePoints.uiStartY = 0;
    curveBasePoints.uiEndX   = -1;
    curveBasePoints.uiEndY   = -1;
    curveBasePoints.usNoOfBasePoints = uiSizeOfCurve;
    curveBasePoints.puiBasePointsX = puiCurveX;
    curveBasePoints.puiBasePointsY = puiCurveY;

    return curveBasePoints;
}

#ifdef __cplusplus
} // extern "C" {
#endif

#endif // _VC_CURVE_H_
