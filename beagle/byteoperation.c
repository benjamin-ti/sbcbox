/***************************************************************************/
/*                             Druckersoftware                             */
/*                    Copyright (C) Carl Valentin GmbH                     */
/***************************************************************************/


/*. IMPORT ================================================================*/

#include "vcplatform.h" // INSERTED BY checkIncludes.sh

#include "byteoperation.h"
//#include <VCStdLib/VCEndian/VCEndian.h>

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
/**
 * Spiegelt den übergebenen 4-Byte-Wert byteweise über  eine Tabelle
 * und liefert das Resultat zurück         |
 */
unsigned int mirror(unsigned int uiData)
{
  unsigned int uiMirrorData;
                                                  /*. Wert byteweise spiegeln */
  uiMirrorData = ((unsigned int) gl_aucMirrorByte[uiData & 0xFF]) << 24;
  uiMirrorData |= ((unsigned int) gl_aucMirrorByte[(uiData >> 8) & 0xFF]) << 16;
  uiMirrorData |= ((unsigned int) gl_aucMirrorByte[(uiData >> 16) & 0xFF]) << 8;
  uiMirrorData |= (unsigned int) gl_aucMirrorByte[(uiData >> 24) & 0xFF];

  return(uiMirrorData);                     /*. gespiegelten Wert zurückgeben */
}

/*=========================================================================*/
/**
 * Spiegelt eine 32-Bit-Variable
 * \param ulVal zu spiegelnde Wert
 * \return gespiegelter Wert
 */
unsigned long mirrorUL(unsigned long ulVal)
{
    return((ulVal >>24) | ((ulVal & 0x00FF0000) >>8) |
            ((ulVal & 0x0000FF00) <<8) | ((ulVal & 0x000000FF) <<24));
}

/*=========================================================================*/
unsigned int swapUI(register unsigned int x)
{
#if VC_ENDIAN == VC_BIG_ENDIAN
    if(x)
    {
        return (((x & 0xff000000) >> 24) |
        ((x & 0x00ff0000) >> 8) |
        ((x & 0x0000ff00) << 8) |
        ((x & 0x000000ff) << 24));
    }
    return 0;
#else
    return x;
#endif
}

/*=========================================================================*/
/**
 *  Umgekehrte Endian Logik als swapUI
 *  (Momentan nur Kruecke, sollte natuerlich vereinheitlicht werden)
 *  Benoetigt wird diese Funktion bei Bitmapfonts und 1D Barcodes, da
 *  hier die restliche Generierlogik davon ausgeht, dass bei LittleEndian
 *  getauscht wird
 */
unsigned int swapUI_LE(register unsigned int x)
{
#if VC_ENDIAN == VC_LITTLE_ENDIAN
    if(x)
    {
        return (((x & 0xff000000) >> 24) |
        ((x & 0x00ff0000) >> 8) |
        ((x & 0x0000ff00) << 8) |
        ((x & 0x000000ff) << 24));
    }
    return 0;
#else
    return x;
#endif
}


/*=========================================================================*/
unsigned short swapUS(register unsigned short x)
{
    if (x)
    {
        return (unsigned short) (((x & 0xff00) >> 8) | ((x & 0x00ff) << 8));
    }
    return 0;
}





/*=========================================================================*/
/**
 *  Zoomt den übergebenen Wert byteweise per Berechnung, der gezoomte Wert
 *  wird in den Zoombuffer geschrieben
 */
static void zoom_calc(unsigned int uiData, unsigned int uiZoomValue,
                      unsigned int *puiZoomBuf)
{
    unsigned int   uiCount    = 0x00000000;
    unsigned int   uiBitIn    = 0x80000000;      /* aktuell einzulesendes Bit */
    unsigned int   uiOutValue = 0x00000000;          /* aktueller Ausgabewert */
    unsigned int   uiBitOut   = 0x80000000;      /* aktuell auszugebendes Bit */

    while (uiBitIn)
    {
        for (uiCount=0; uiCount<uiZoomValue; uiCount++)         /* Bit zoomen */
        {
            if (uiData & uiBitIn)
            {
                uiOutValue |= uiBitOut;
            }

                             /* auf naechstes auszugebende Bit weiterschalten */
            uiBitOut >>= 1;                                  /* uiBitOut /= 2 */
            if (0x00000000 == uiBitOut)
            {
                uiBitOut = 0x80000000;

                                           /* aktuellen Ausgabewert speichern */
                *puiZoomBuf = vc_be32toh(uiOutValue);
                uiOutValue = 0x00000000;
                puiZoomBuf++;
            }
        }

                             /* auf naechstes einzulesende Bit weiterschalten */
        uiBitIn >>= 1;                                        /* uiBitIn /= 2 */
    }

                                           /* aktuellen Ausgabewert speichern */
    if (0x00000000 != uiBitOut)
    {
        while (uiBitOut)
        {
            uiBitOut >>= 1;                                  /* uiBitOut /= 2 */
            uiOutValue <<= 1;
        }
        *puiZoomBuf = vc_be32toh(uiOutValue);
    }

    return;
}

/*=========================================================================*/
/**
 *  Zoomt den übergebenen Wert byteweise über die entsprechende Tabelle bzw.
 *  wird ab einem Zoom-Wert von 10 berechnet.
 *  Der gezoomt Wert wird in den Zoombuffer geschrieben                               |
 */
void zoom(unsigned int uiData,unsigned int uiZoomValue,unsigned int *puiZoomBuf)
{
    unsigned char *pucZoomBuf   = 0x00;
    unsigned char *pucZoomTable = 0x00;

    if (uiZoomValue > 9)
    {
        // Ab Zoomfaktor 10 wird das Zoomen berechnet, nicht über Tabellen ermittelt
        zoom_calc(uiData, uiZoomValue, puiZoomBuf);
        return;
    }

    pucZoomBuf = (unsigned char *) puiZoomBuf;
                          /*. Zeiger auf die entsprechende Zoomtabelle setzen */
    pucZoomTable = (unsigned char *) lk_pucZoomTable[uiZoomValue];
                                          /*. HIGH-Byte der HIGH-Words zoomen */
    memcpy(pucZoomBuf,&pucZoomTable[(uiData >> 24) * uiZoomValue],uiZoomValue);
    pucZoomBuf += uiZoomValue;    /*. Zeiger auf den Zoombuffer aktualisieren */
                                           /*. LOW-Byte der HIGH-Words zoomen */
    memcpy(pucZoomBuf,&pucZoomTable[((uiData >> 16) & 0xff) * uiZoomValue],
           uiZoomValue);
    pucZoomBuf += uiZoomValue;    /*. Zeiger auf den Zoombuffer aktualisieren */
                                           /*. HIGH-Byte der LOW-Words zoomen */
    memcpy(pucZoomBuf,&pucZoomTable[((uiData >> 8) & 0xff) * uiZoomValue],
           uiZoomValue);
    pucZoomBuf += uiZoomValue;    /*. Zeiger auf den Zoombuffer aktualisieren */
                                            /*. LOW-Byte der LOW-Words zoomen */
    memcpy(pucZoomBuf,&pucZoomTable[(uiData & 0xff)  * uiZoomValue],uiZoomValue);

    return;
}

/*=========================================================================*/
void
memcpy32(unsigned int* pDest, unsigned int* pSrc, UINT uiSize)
{
    UINT i;

    for (i = 0; i < uiSize / 4; i++)
        *pDest++ = *pSrc++;
}

