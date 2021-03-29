/***************************************************************************/
/*                             Druckersoftware                             */
/*                    Copyright (C) Carl Valentin GmbH                     */
/***************************************************************************/

/*. IMPORT ================================================================*/

#include "vcplatform.h"
#include "byteoperation.h"
#include "atmega.h"

/*. ENDIMPORT =============================================================*/


/*. EXPORT ================================================================*/

volatile unsigned char gl_ucAtmelCtrl;

/*. ENDEXPORT =============================================================*/


/*. LOCAL =================================================================*/

/*-------------------------------- Makros ---------------------------------*/

/*--------------------------- Typdeklarationen ----------------------------*/

/*------------------------------ Prototypen -------------------------------*/

/*------------------------ Konstantendeklarationen ------------------------*/

// connected Atmel Device Objects
static const TYAtmelDevice oAtmelStepper1 =
{ .idAtmelDev   = ATMEL_STEPPER_1,
  .sciChannel   = CHANNEL_SCIF0,
  .deviceType   = ATMEGA32,
  .selectBit    = 0xEF,
  .fuseByteLow  = 0xFF,
  .fuseByteHigh = 0x81,
  .fuseByteExtd = 0x00,
  .pageSizeWord = 64
};

static const TYAtmelDevice oAtmelStepper2 =
{ .idAtmelDev   = ATMEL_STEPPER_2,
  .sciChannel   = CHANNEL_SCIF0,
  .deviceType   = ATMEGA32,
  .selectBit    = 0xBF,
  .fuseByteLow  = 0xFF,
  .fuseByteHigh = 0x81,
  .fuseByteExtd = 0x00,
  .pageSizeWord = 64
};

static const TYAtmelDevice oAtmelIO =
{ .idAtmelDev   = ATMEL_IO,
  .sciChannel   = CHANNEL_SCIF0,
  .deviceType   = ATMEGA32,
  .selectBit    = 0xFB,
  .fuseByteLow  = 0xFF,
  .fuseByteHigh = 0x81,
  .fuseByteExtd = 0x00,
  .pageSizeWord = 64
};

static const TYAtmelDevice oAtmelDisplay =
{ .idAtmelDev   = ATMEL_DISPLAY,
  .sciChannel   = CHANNEL_SCIF2,
  .deviceType   = ATMEGA64,
  .selectBit    = 0xFB,
  .fuseByteLow  = 0xFF,
  .fuseByteHigh = 0x99,
  .fuseByteExtd = 0xFF,
  .pageSizeWord = 128
};

static const TYAtmelDevice *gObjListAtmelDev[] =
{
    &oAtmelStepper1,
    &oAtmelStepper2,
    &oAtmelIO,
    &oAtmelDisplay
};

/*------------------------- modulglobale Variable -------------------------*/

// ATMEL - SPI Serial Programming Instruction Set Definition
static TYAtmelInstruction gATMEGA32Instruction[] =
{
  { 0xAC, 0x53, 0x00, 0x00 },  // ATMEL_WRITE_FUSEBIT

  { 0xAC, 0xA0, 0x00, 0x00 },  // ATMEL_WRITE_FUSEBIT
  { 0xAC, 0xA8, 0x00, 0x00 },  // ATMEL_WRITE_FUSEBIT_HIGH
  {0,0,0,0}, // ATMEL_WRITE_FUSEBIT_EXTD , NOT SUPPORTED FOR DEVICE
  { 0x50, 0x00, 0x00, 0x00 },  // ATMEL_READ_FUSEBIT
  { 0x58, 0x08, 0x00, 0x00 },  // ATMEL_READ_FUSEBIT_HIGH
  {0,0,0,0}, // ATMEL_READ_FUSEBIT_EXTD , NOT SUPPORTED FOR DEVICE
  { 0xAC, 0x80, 0x00, 0x00 },  // ATMEL_CHIP_ERASE
  { 0x40, 0x00, 0x00, 0x00 },  // ATMEL_LOAD_CMD_LOW
  { 0x48, 0x00, 0x00, 0x00 },  // ATMEL_LOAD_CMD_HIGH
  { 0x4C, 0x00, 0x00, 0x00 },  // ATMEL_WRITE_PROG_MEM_PAGE

  { 0x20, 0x00, 0x00, 0x00 },  // ATMEL_LOAD_CMD_LOW
  { 0x28, 0x00, 0x00, 0x00 },  // ATMEL_LOAD_CMD_HIGH
};

// ATMEL - SPI Serial Programming Instruction Set Definition
static TYAtmelInstruction gATMEGA64Instruction[] =
{
  { 0xAC, 0x53, 0x00, 0x00 },  // ATMEL_WRITE_FUSEBIT

  { 0xAC, 0xA0, 0x00, 0x00 },  // ATMEL_WRITE_FUSEBIT
  { 0xAC, 0xA8, 0x00, 0x00 },  // ATMEL_WRITE_FUSEBIT_HIGH
  { 0xAC, 0xA4, 0x00, 0x00 },  // ATMEL_WRITE_FUSEBIT_EXTD
  { 0x50, 0x00, 0x00, 0x00 },  // ATMEL_READ_FUSEBIT
  { 0x58, 0x08, 0x00, 0x00 },  // ATMEL_READ_FUSEBIT_HIGH
  { 0x50, 0x08, 0x00, 0x00 },  // ATMEL_READ_FUSEBIT_EXTD
  { 0xAC, 0x80, 0x00, 0x00 },  // ATMEL_CHIP_ERASE
  { 0x40, 0x00, 0x00, 0x00 },  // ATMEL_LOAD_CMD_LOW
  { 0x48, 0x00, 0x00, 0x00 },  // ATMEL_LOAD_CMD_HIGH
  { 0x4C, 0x00, 0x00, 0x00 },  // ATMEL_WRITE_PROG_MEM_PAGE

  { 0x20, 0x00, 0x00, 0x00 },  // ATMEL_LOAD_CMD_LOW
  { 0x28, 0x00, 0x00, 0x00 },  // ATMEL_LOAD_CMD_HIGH
};

/*. ENDLOCAL ==============================================================*/


/*=========================================================================*/
TYerrno Atmega_SelectCurrentAtmelDevice(TYAtmelDevice **pObjAtmelDev,
                                        AtmelDevices devAtmel)
{
    TYerrno errNo = RET_OK;
    if( devAtmel <= ATMEL_MAX_INDEX )
    {
        *pObjAtmelDev = (TYAtmelDevice*)gObjListAtmelDev[devAtmel];
    }
    else
    {
        errNo = ERR_ATMEL_ID;
    }

    return errNo;
}


/*=========================================================================*/
TYerrno  Atmega_GetInstruction(AtmelID idATMEL,
                               unsigned char idInstruction,
                               unsigned char *pInstruction)
{
    TYerrno errNo = RET_OK;
    TYAtmelInstruction *pInstructionSet = NULL;
    if( idInstruction < ATMEL_NUM_INSTRUCTION )
    {
        if( ATMEGA32 == idATMEL )
        {
            pInstructionSet = &gATMEGA32Instruction[idInstruction];
        }
        if( ATMEGA64 == idATMEL )
        {
            pInstructionSet = &gATMEGA64Instruction[idInstruction];
        }

        if( NULL != pInstructionSet )
        {
            if( 0 == pInstructionSet->byte1 )
            {
                errNo = ERR_ATMEL_CMD_NOT_SUPPORTED;
            }
            else
            {
#ifdef LINUX_SUPPORT
                *(pInstruction++) = pInstructionSet->byte1;
                *(pInstruction++) = pInstructionSet->byte2;
                *(pInstruction++) = pInstructionSet->byte3;
                *pInstruction     = pInstructionSet->byte4;
#else
                *(pInstruction++) = gl_aucMirrorByte[pInstructionSet->byte1];
                *(pInstruction++) = gl_aucMirrorByte[pInstructionSet->byte2];
                *(pInstruction++) = gl_aucMirrorByte[pInstructionSet->byte3];
                *pInstruction     = gl_aucMirrorByte[pInstructionSet->byte4];
#endif
            }
        }
        else
        {
            errNo =  ERR_ATMEL_CMD_NOT_SUPPORTED;
        }
    }
    else
    {
        errNo =  ERR_ATMEL_CMD_NOT_SUPPORTED;
    }

    return errNo;
}


/*=========================================================================*/
void Atmega_Start(AtmelDevices devAtmel)
{
    // Atmel wieder starten
    switch(devAtmel)
    {
        case ATMEL_STEPPER_1: SET_ATMEL_STEPPER_1_RESET_SIGNAL
                              break;
        case ATMEL_STEPPER_2: SET_ATMEL_STEPPER_2_RESET_SIGNAL
                              break;
        case ATMEL_IO:        SET_ATMEL_IO_RESET_SIGNAL
                              break;
        case ATMEL_DISPLAY:   SET_ATMEL_DISPLAY_RESET_SIGNAL
                              break;

        default:              break;
    }
}
// SCI Task nur bei Dynacode Application aktiv
#if !defined(CV_34_57_001_SUPPORT) && !defined(CV_34_57_004_SUPPORT)
void InitSCIForAtmelUpdate(unsigned int uiAtmel)
{
}
void RestoreSCIandAtmels(unsigned int uiAtmel)
{
}
#endif
#if defined(SPE_II_SUPPORT)
void InitSCIForAtmelUpdate(unsigned int uiAtmel){}
void RestoreSCIandAtmels(unsigned int uiAtmel){}
#endif
