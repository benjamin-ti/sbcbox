/***************************************************************************/
/*                             Printerfirmware                             */
/*                    Copyright (C) Carl Valentin GmbH                     */
/*                       http://www.carl-valentin.de                       */
/*                  This code is licenced under the LGPL                   */
/***************************************************************************/

#ifndef _VC_PLATFORM_H_
#define _VC_PLATFORM_H_

#include <stddef.h>

#define LINUX_SUPPORT
#define CV_ARM_SUPPORT

typedef int             TYerrno;    /* Errorhandling */
#define RET_OK                0     /* Return OK */

typedef unsigned long   U32;        /* 32-Bit vorzeichenlos      */
typedef   signed long   I32;        /* 32-Bit vorzeichenbehaftet */
typedef unsigned short  U16;        /* 16-Bit vorzeichenlos      */
typedef   signed short  I16;        /* 16-Bit vorzeichenbehaftet */
typedef unsigned char    U8;        /* 8-Bit vorzeichenlos       */
typedef   signed char    I8;        /* 8-Bit vorzeichenbehaftet  */

typedef unsigned int    UINT;       /* unsigned int 16/32 Bit */
typedef unsigned char   UCHAR;      /* 8-Bit vorzeichenlos */
typedef unsigned short  USHORT;     /* 16-Bit vorzeichenlos */
typedef unsigned long   ULONG;      /* 32-Bit vorzeichenlos */

typedef int             BOOL;       /* 8-Bit vorzeichenlos  */

#if 1 // ausnahmsweise hier besser als Makro da sonst Eclipse Code Analyzer Problem meldet bei einem return, wenn VOID als rueckgabewert einer funktion verwendet wird
#define VOID    void
#else
typedef void            VOID;
#endif

typedef char *          HPCHAR;     /* Zeiger auf char  */

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

#if 1
typedef int VC_BOOL;
#define VC_FALSE 0
#define VC_TRUE 1
#else
#ifdef LINUX_SUPPORT
typedef int VC_BOOL;
#define VC_FALSE 0
#define VC_TRUE 1
#else
typedef enum {VC_FALSE, VC_TRUE} VC_BOOL; //!< Boolscher Wert
#endif
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#define VCHAL_msDelay(ARG) usleep(ARG*1000)

#endif
