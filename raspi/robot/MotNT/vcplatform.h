/***************************************************************************/
/*                             Printerfirmware                             */
/*                    Copyright (C) Carl Valentin GmbH                     */
/*                       http://www.carl-valentin.de                       */
/*                  This code is licenced under the LGPL                   */
/***************************************************************************/

#ifndef VCPLATFORM_H
#define VCPLATFORM_H

#include <stdlib.h>

#define LINUX_SUPPORT
#define VC_INLINE static

/**
 *   Funktionsrueckgabe Datentyp
 */
typedef enum vc_retval
{
    VC_ERROR = -101,
    VC_SUCCESS = 0,     //!< Funktion erfolgreich abgearbeitet
    VC_FAILED,          //!< Funktion fehlerhaft abgebrochen
    VC_NOT_AVAILABLE,   //!< Funktion ist fuer diese Applikation nicht verfuegbar
    VC_NULLPTR,         //!< Nullpointer Uebergabe an (Funktion)
    VC_TIMEOUT,         //!< Funktion wurde durch eine Zeitueberschreitung beendet
    VC_ALLOCMEM,        //!< Fehler bei Speicherallocierung
    VC_INV_PARAM,       //!< falscher Parameter(wert) in Uebergabe an Funktion
    VC_OS_ERR,          //!< OS Aufruf hat Fehler zurueckgeliefert
    VC_CONVERT,         //!< Fehler beim Umwandeln eines Parameters
    VC_MULTIDEFINITION, //!< Wert wurde mehrfach definiert
    VC_EXCEED_SIZE,     //!< Maximalgroesse ueberschritten
    VC_INV_CHAR,        //!< ungueltiges Zeichen
    VC_NOT_FOUND,       //!< gesuchtes Ergebnis nicht gefunden
    VC_ACCESS_DENIED,   //!< Zugriff verweigert
    VC_XMLTEL_DEF,      //!< Telegramm entspricht nicht der Definition
    VC_TEMP_CHANGED,    //!< Durchschnittstemperatur geaendert
    VC_BLOCKED,         //!< Ausfuehrung der Funktion wurde blockiert
    VC_INV_TYPE,        //!< Ungueltiger Typ
    VC_RESC_NOT_FOUND,  //!< erforderliche Resource nicht gefunden
    VC_QUEUE_FULL,      //!< Es konnte nichts mehr in eine Queue geschrieben werden
    VC_PARMVAL_INVALID, //!< Der angegeben Parameterwert ist ausserhalb des Gueltigkeitsbereichs
    VC_PARMVAL_2BIG,    //!< Der angegeben Parameterwert ist zu gross
    VC_PARMVAL_2SMALL,  //!< Der angegeben Parameterwert ist zu klein
    VC_PARMF_NOID,      //!< Parameterstruktur enthaelt keine gueltige ID
    VC_NEW,             //!< Funktion hat etwas neu angelegt
    VC_EXISTS,          //!< Anzulegendes Objekt existiert bereits
    VC_NO_INIT,         //!< Objekt/Modul wurde noch nicht initialisiert
    VC_INV_FCTCALL,      //!< Ungueltiger Funktionsaufruf
} VC_STATUS;

/**
 *   Datentypen mit fixer Bitbreite
 */
typedef unsigned long   VC_UINT32;       //!< 32-Bit vorzeichenlos
typedef   signed long   VC_INT32;        //!< 32-Bit vorzeichenbehaftet
typedef unsigned short  VC_UINT16;       //!< 16-Bit vorzeichenlos
typedef   signed short  VC_INT16;        //!< 16-Bit vorzeichenbehaftet
typedef unsigned char   VC_UINT8;        //!< 8-Bit vorzeichenlos
typedef   signed char   VC_INT8;         //!< 8-Bit vorzeichenbehaftet

typedef unsigned char  VC_BDATA8;        //!<  8-Bit Binary Data
typedef unsigned short VC_BDATA16;       //!< 16-Bit Binary Data
typedef unsigned long  VC_BDATA32;       //!< 32-Bit Binary Data

typedef unsigned char*  VC_PBDATA8;      //!<  8-Bit-Pointer auf Binary Data
typedef unsigned short* VC_PBDATA16;     //!< 16-Bit-Pointer auf Binary Data
typedef unsigned long*  VC_PBDATA32;     //!< 32-Bit-Pointer auf Binary Data

typedef short int       VC_WCHAR;        //!< Unicode character

typedef int VC_BOOL;
#define VC_FALSE 0
#define VC_TRUE 1

// Dienen zum Umgehen von const-Warnungen
#define _ACONST_P_(ARG)    *(void**)&(ARG)
#define _ACONST_PU16_(ARG)    *(VC_UINT16**)&(ARG)

// #define _ACONST_BOOL_(ARG) *(BOOL*)&(ARG)
#define _ACONST_VC_BOOL_(ARG) *(VC_BOOL*)&(ARG)

#define _ACONST_PCHAR_(ARG) *(char**)&(ARG)

#define _ACONST_U16_(ARG)  *(VC_UINT16*)&(ARG)
#define _ACONST_U32_(ARG)  *(VC_UINT32*)&(ARG)

#define _ACONST_SHORT_(ARG) *(short*)&(ARG)
#define _ACONST_INT_(ARG)   *(int*)&(ARG)
#define _ACONST_LONG_(ARG)  *(long*)&(ARG)
#define _ACONST_UINT_(ARG)   *(unsigned int*)&(ARG)

// ja auch das ;-)
#define TX_SUCCESS                      0x00
typedef int STATUS;
typedef unsigned short UINT16;
typedef unsigned short U16;
typedef unsigned long  UNSIGNED;
typedef unsigned long  U32;

/**
 *   Funktionen
 */
#define AllocMemory malloc
#define FreeMemory  free

static inline VC_STATUS MemFree( void *pMem )
{
    FreeMemory(pMem);

    return (VC_SUCCESS);
}

#endif // VCPLATFORM_H
