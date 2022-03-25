/**
 *  \file   am572x/GPIO_soc.c
 *
 *  \brief  AM572x SOC specific GPIO hardware attributes.
 *
 *   This file contains the GPIO hardware attributes like base address and
 *   interrupt ids.
 */

/*
 * Copyright (C) 2014-2015 Texas Instruments Incorporated - http://www.ti.com/
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the
 * distribution.
 *
 * Neither the name of Texas Instruments Incorporated nor the names of
 * its contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <ti/csl/csl_utils.h>
#include <ti/csl/soc/am572x/src/cslr_soc.h>
#include <ti/csl/csl_types.h>
#include <ti/drv/gpio/GPIO.h>
#include <ti/drv/gpio/soc/GPIO_soc.h>

/*
typedef struct GPIO_v1_HwAttrs_s {
    // GPIO Peripheral base address
    uint32_t baseAddr;
    // GPIO Peripheral interrupt vector
    uint32_t line1IntNum;
    // GPIO Peripheral interrupt vector
    uint32_t line2IntNum;
    // GPIO Peripheral interrupt vector
    uint32_t line1EventId;
    // GPIO Peripheral interrupt vector
    uint32_t line2EventId;
} GPIO_v1_HwAttrs;
*/

/* GPIO Driver hardware attributes */
GPIO_v1_hwAttrs_list GPIO_v1_hwAttrs = {
/*
    {
       CSL_IPU_GPIO1_REGS+0x20000000,
       51,
       0,
       55,
       0
    },
    {
       CSL_IPU_GPIO2_REGS+0x20000000,
       52,
       0,
       56,
       0
    },
    {
       CSL_IPU_GPIO3_REGS+0x20000000,
       49,
       0,
       57,
       0
    },

    {
       CSL_IPU_GPIO4_REGS+0x20000000,
       49,
       0,
       58,
       0
    },
*/
    {
       0x6805B000,
       49,
       0,
       59,
       0
    },
/*
    {
       CSL_IPU_GPIO6_REGS+0x20000000,
       49,
       0,
       60,
       0
    },
    {
       CSL_IPU_GPIO7_REGS+0x20000000,
       49,
       0,
       61,
       0
    },
    {
       CSL_IPU_GPIO8_REGS+0x20000000,
       49,
       0,
       0,
       0
    },
*/
};

/* GPIO configuration structure */
CSL_PUBLIC_CONST GPIOConfigList GPIO_config =
{
    {
        &GPIO_FxnTable_v1,
        NULL,
        NULL
    },
    /* "pad to full predefined length of array" */
    {
        NULL,
        NULL,
        NULL
    },
    {
        NULL,
        NULL,
        NULL
    }
};
