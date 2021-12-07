/***************************************************************************/
/*                             Printerfirmware                             */
/*                    Copyright (C) Carl Valentin GmbH                     */
/*                       http://www.carl-valentin.de                       */
/*                  This code is licenced under the LGPL                   */
/***************************************************************************/

#ifndef VC_VCENDIAN_H_
#define VC_VCENDIAN_H_

#include "vcplatform.h"

#include <stdint.h>

#define VCENDIAN(function, variable) \
    (variable) = (function)(variable)

#ifdef __cplusplus
extern "C"
{
#endif

    uint16_t
    vc_htobe16(uint16_t host_16bits);

    uint16_t
    vc_htole16(uint16_t host_16bits);

    uint16_t
    vc_be16toh(uint16_t big_endian_16bits);

    uint16_t
    vc_le16toh(uint16_t little_endian_16bits);


    uint32_t
    vc_htobe32(uint32_t host_32bits);

    uint32_t
    vc_htole32(uint32_t host_32bits);

    uint32_t
    vc_be32toh(uint32_t big_endian_32bits);

    uint32_t
    vc_le32toh(uint32_t little_endian_32bits);




#ifdef __cplusplus
} // extern "C"
#endif

#endif /* VC_VCENDIAN_H_ */
