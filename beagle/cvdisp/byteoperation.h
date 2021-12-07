/***************************************************************************/
/*                             Printerfirmware                             */
/*                    Copyright (C) Carl Valentin GmbH                     */
/*                       http://www.carl-valentin.de                       */
/*                  This code is licenced under the LGPL                   */
/***************************************************************************/

#ifndef _VC_BYTEOPERATION_H_
#define _VC_BYTEOPERATION_H_

/*. IMPORT ================================================================*/

#include "vcplatform.h"

/*. ENDIMPORT =============================================================*/


/*. EXPORT ================================================================*/

/*-------------------------------- Makros ---------------------------------*/

/*--------------------------- Typdeklarationen ----------------------------*/

/*------------------------------ Prototypen -------------------------------*/

unsigned int mirror(unsigned int);
unsigned long mirrorUL(unsigned long ulVal);

unsigned int swapUI(register unsigned int x);
unsigned int swapUI_LE(register unsigned int x);
unsigned short swapUS(register unsigned short x);

void zoom(unsigned int uiData,unsigned int uiZoomValue,unsigned int *puiZoomBuf);

void memcpy32(unsigned int*, unsigned int*, UINT);

/*------------------------ Konstantendeklarationen ------------------------*/
extern const unsigned char gl_ucMirrorCharTable[256][8][2];
extern const unsigned char gl_aucMirrorByte[];

extern const unsigned char *lk_pucZoomTable[];

/*. ENDEXPORT =============================================================*/

#endif
