/***************************************************************************/
/*                             Printerfirmware                             */
/*                    Copyright (C) Carl Valentin GmbH                     */
/*                       http://www.carl-valentin.de                       */
/*                  This code is licenced under the LGPL                   */
/***************************************************************************/

#include "vcplatform.h"

#include "VCEndian.h"

#ifdef VC_LITTLE
#undef VC_LITTLE
#endif

#ifdef __BYTE_ORDER__ // wenn die vorliegende gcc version dies kann, ist dies am verlaesslichsten
# if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#  define VC_BIG 1
// VC_LITTLE nicht definieren
# elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#  define VC_LITTLE 1
# else
#  error unknown byte order
# endif
#elif __LITTLE_ENDIAN__ == 1 // SH3 compiler sagt little endian
# define VC_LITTLE 1
#elif __BIG_ENDIAN__ == 1 // SH3 compiler sagt big endian
#  define VC_BIG 1
// VC_LITTLE nicht definieren
#else
// weder gcc noch sh3 compiler sagt uns welche endian, also nehmen wir unsere eigene definition
// aus vcplatform.h
# if VC_ENDIAN == VC_LITTLE_ENDIAN
#  define VC_LITTLE 1
# elif VC_ENDIAN == VC_BIG_ENDIAN
#  define VC_BIG 1
// VC_LITTLE nichit definieren
# else
#  error Could not detect endianness
# endif
#endif


/*=========================================================================*/
/**
 *  Umsetzung Little Endian <-> Big Endian fuer 16bit-Werte
 */
static uint16_t swap16( uint16_t u16Value )
{
    return (((u16Value << 8) & 0xFF00) |
            ((u16Value >> 8) & 0x00FF));
}

/*=========================================================================*/
/**
 *  Umsetzung Little Endian <-> Big Endian fuer 32bit-Werte
 */
static uint32_t swap32( uint32_t u32Value )
{
    return (((uint32_t)swap16((U16)(u32Value & 0x0000FFFFUL)) << 16) |
            (uint32_t)swap16((U16)((u32Value >> 16) & 0x0000FFFFUL)));
}

uint16_t
vc_htobe16(uint16_t host_16bits)
{

#if VC_BIG == 1
    return host_16bits;
#elif VC_LITTLE == 1
    return swap16(host_16bits);
#else
#error unexpected byte order
    return -1;
#endif

}

uint16_t
vc_htole16(uint16_t host_16bits)
{

#if VC_BIG == 1
    return swap16(host_16bits);
#elif VC_LITTLE == 1
    return host_16bits;
#else
#error unexpected byte order
    return -1;
#endif

}

uint16_t
vc_be16toh(uint16_t big_endian_16bits)
{

#if VC_BIG == 1
    return big_endian_16bits;
#elif VC_LITTLE == 1
    return swap16(big_endian_16bits);
#else
#error unexpected byte order
    return -1;
#endif

}

uint16_t
vc_le16toh(uint16_t little_endian_16bits)
{

#if VC_BIG == 1
    return swap16(little_endian_16bits);
#elif VC_LITTLE == 1
    return little_endian_16bits;
#else
#error unexpected byte order
    return -1;
#endif

}

uint32_t
vc_htobe32(uint32_t host_32bits)
{

#if VC_BIG == 1
    return host_32bits;
#elif VC_LITTLE == 1
    return swap32(host_32bits);
#else
#error unexpected byte order
    return -1;
#endif

}

uint32_t
vc_htole32(uint32_t host_32bits)
{

#if VC_BIG == 1
    return swap32(host_32bits);
#elif VC_LITTLE == 1
    return host_32bits;
#else
#error unexpected byte order
    return -1;
#endif

}

uint32_t
vc_be32toh(uint32_t big_endian_32bits)
{

#if VC_BIG == 1
    return big_endian_32bits;
#elif VC_LITTLE == 1
    return swap32(big_endian_32bits);
#else
#error unexpected byte order
    return -1;
#endif

}

uint32_t
vc_le32toh(uint32_t little_endian_32bits)
{

#if VC_BIG == 1
    return swap32(little_endian_32bits);
#elif VC_LITTLE == 1
    return little_endian_32bits;
#else
#error unexpected byte order
    return -1;
#endif

}
