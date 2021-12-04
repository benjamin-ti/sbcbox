/***************************************************************************/
/*                             Druckersoftware                             */
/*                    Copyright (C) Carl Valentin GmbH                     */
/***************************************************************************/

#ifndef _VC_ATMEGA_H_
#define _VC_ATMEGA_H_

/*. IMPORT ================================================================*/

#include "vcplatform.h"

#ifdef CV_34_57_004_SUPPORT // Neue Dynacode CPU
#include "../fpga/fpga.h"
#endif

extern volatile unsigned char gl_ucAtmelCtrl;

/*. ENDIMPORT =============================================================*/


/*. EXPORT ================================================================*/

/*-------------------------------- Makros ---------------------------------*/

#define CHANNEL_SCIF0   0
#define CHANNEL_SCIF2   2

#define ERR_ATMEL_OFFSET 100


#ifdef CV_34_57_004_SUPPORT // Neue Dynacode CPU
#define   SELECT_ATMEL_STEPPER_1            {*DYNA_ATMEL_CTRL = gl_ucAtmelCtrl &=~0x010;}
#define   DESELECT_ATMEL_STEPPER_1          {*DYNA_ATMEL_CTRL = gl_ucAtmelCtrl |= 0x10;}
#define   SET_ATMEL_STEPPER_1_RESET_SIGNAL  {*DYNA_ATMEL_CTRL = gl_ucAtmelCtrl |= 0x01;}
#define   CLEAR_ATMEL_STEPPER_1_RESET_SIGNAL {*DYNA_ATMEL_CTRL = gl_ucAtmelCtrl &= ~0x01;}

#define   SELECT_ATMEL_STEPPER_2            {*DYNA_ATMEL_CTRL= gl_ucAtmelCtrl &=~0x20;}
#define   DESELECT_ATMEL_STEPPER_2          {*DYNA_ATMEL_CTRL= gl_ucAtmelCtrl |=0x20;}
#define   SET_ATMEL_STEPPER_2_RESET_SIGNAL  {*DYNA_ATMEL_CTRL= gl_ucAtmelCtrl |=0x02;}
#define   CLEAR_ATMEL_STEPPER_2_RESET_SIGNAL {*DYNA_ATMEL_CTRL= gl_ucAtmelCtrl &=~0x02;}

#define   SELECT_ATMEL_IO                   {*DYNA_ATMEL_CTRL= gl_ucAtmelCtrl &=~0x40;}
#define   DESELECT_ATMEL_IO                 {*DYNA_ATMEL_CTRL= gl_ucAtmelCtrl |=0x40;}
#define   SET_ATMEL_IO_RESET_SIGNAL         {*DYNA_ATMEL_CTRL= gl_ucAtmelCtrl |=0x04;}
#define   CLEAR_ATMEL_IO_RESET_SIGNAL       {*DYNA_ATMEL_CTRL= gl_ucAtmelCtrl &=~0x04;}

#define   DESELECT_ALL_SCI_SLAVES           {*DYNA_ATMEL_CTRL= gl_ucAtmelCtrl =0x77;}

#else

#define   SELECT_ATMEL_STEPPER_1            {*((volatile VC_UINT8*)IO_ATMEGA_WRITE_ADDRESS)= gl_ucAtmelCtrl &=~0x10;}
#define   DESELECT_ATMEL_STEPPER_1          {*((volatile VC_UINT8*)IO_ATMEGA_WRITE_ADDRESS)= gl_ucAtmelCtrl |= 0x10;}
#define   SET_ATMEL_STEPPER_1_RESET_SIGNAL  {*((volatile VC_UINT8*)IO_ATMEGA_WRITE_ADDRESS)= gl_ucAtmelCtrl |= 0x20;}
#define   CLEAR_ATMEL_STEPPER_1_RESET_SIGNAL {*((volatile VC_UINT8*)IO_ATMEGA_WRITE_ADDRESS)= gl_ucAtmelCtrl &= ~0x20;}

#define   SELECT_ATMEL_STEPPER_2            {*((volatile VC_UINT8*)IO_ATMEGA_WRITE_ADDRESS)= gl_ucAtmelCtrl &=~0x40;}
#define   DESELECT_ATMEL_STEPPER_2          {*((volatile VC_UINT8*)IO_ATMEGA_WRITE_ADDRESS)= gl_ucAtmelCtrl |=0x40;}
#define   SET_ATMEL_STEPPER_2_RESET_SIGNAL  {*((volatile VC_UINT8*)IO_ATMEGA_WRITE_ADDRESS)= gl_ucAtmelCtrl |=0x80;}
#define   CLEAR_ATMEL_STEPPER_2_RESET_SIGNAL {*((volatile VC_UINT8*)IO_ATMEGA_WRITE_ADDRESS)= gl_ucAtmelCtrl &=~0x80;}

#define   SELECT_ATMEL_IO                   {*((volatile VC_UINT8*)IO_ATMEGA_WRITE_ADDRESS)= gl_ucAtmelCtrl &=~0x04;}
#define   DESELECT_ATMEL_IO                 {*((volatile VC_UINT8*)IO_ATMEGA_WRITE_ADDRESS)= gl_ucAtmelCtrl |=0x04;}
#define   SET_ATMEL_IO_RESET_SIGNAL         {*((volatile VC_UINT8*)IO_ATMEGA_WRITE_ADDRESS)= gl_ucAtmelCtrl |=0x08;}
#define   CLEAR_ATMEL_IO_RESET_SIGNAL       {*((volatile VC_UINT8*)IO_ATMEGA_WRITE_ADDRESS)= gl_ucAtmelCtrl &=~0x08;}


#define   DESELECT_ALL_SCI_SLAVES           {*((volatile VC_UINT8*)IO_ATMEGA_WRITE_ADDRESS)= gl_ucAtmelCtrl |= 0x54;}
#endif

#define IO_ATMEGA_WRITE_ADDRESS       0xB8000100

// ANM.PM. diese Werte entsprechend anpassen, wenn Display an endgueltige Hardware
// angeschlossen werden kann! Zu Testzwecken haengt Display am IO-Port
#define   SELECT_ATMEL_DISPLAY                   {*((volatile VC_UINT8*)IO_ATMEGA_WRITE_ADDRESS)= gl_ucAtmelCtrl &=~0x04;}
#define   DESELECT_ATMEL_DISPLAY                 {*((volatile VC_UINT8*)IO_ATMEGA_WRITE_ADDRESS)= gl_ucAtmelCtrl |=0x04;}
#define   SET_ATMEL_DISPLAY_RESET_SIGNAL         {*((volatile VC_UINT8*)IO_ATMEGA_WRITE_ADDRESS)= gl_ucAtmelCtrl |=0x08;}
#define   CLEAR_ATMEL_DISPLAY_RESET_SIGNAL       {*((volatile VC_UINT8*)IO_ATMEGA_WRITE_ADDRESS)= gl_ucAtmelCtrl &=~0x08;}

#define ATMEGA_SIZE_INSTRUCTION    4    // instruction length in byte

/*--------------------------- Typdeklarationen ----------------------------*/

typedef enum
{
    ATMEGA32,
    ATMEGA64
} AtmelID;

typedef enum atmelmem_err
{
    ERR_ATMEL_CMD_NOT_SUPPORTED = ERR_ATMEL_OFFSET,
    ERR_ATMEL_ID
} AtmelMemErr;

enum instruction_byte
{
    BYTE1,
    BYTE2,
    BYTE3,
    BYTE4
};

typedef enum atmel_devices
{
    ATMEL_STEPPER_1,
    ATMEL_STEPPER_2,
    ATMEL_IO,
    ATMEL_DISPLAY,
    ATMEL_MAX_INDEX = ATMEL_DISPLAY
} AtmelDevices;

typedef struct atmel_device
{
    unsigned char idAtmelDev;      // ATMEL ID ( old style support )
    unsigned char sciChannel;      // atmel connected to this channel
    unsigned char deviceType;      // ATMEGA family Device ATM32,64 etc.
    unsigned char selectBit;
    unsigned char fuseByteLow;
    unsigned char fuseByteHigh;
    unsigned char fuseByteExtd;
    unsigned char pageSizeWord;    // number programmable pages
}TYAtmelDevice;

// ATMEL Instruction
typedef struct atmel_instruction
{
    VC_UINT8 byte1;
    VC_UINT8 byte2;
    VC_UINT8 byte3;
    VC_UINT8 byte4;
}TYAtmelInstruction;

// SPI Serial Programming Instruction Set ( ATMEGA32, ATMEGA64 }
enum atmel_instruction_set
{
    ATMEL_PROG_ENABLE,
    ATMEL_WRITE_FUSEBYTE,
    ATMEL_WRITE_FUSEBYTE_HIGH,
    ATMEL_WRITE_FUSEBYTE_EXTD,
    ATMEL_READ_FUSEBYTE,
    ATMEL_READ_FUSEBYTE_HIGH,
    ATMEL_READ_FUSEBYTE_EXTD,
    ATMEL_CHIP_ERASE,
    ATMEL_LOAD_CMD_LOW,
    ATMEL_LOAD_CMD_HIGH,
    ATMEL_WRITE_PROG_MEM_PAGE,
    ATMEL_READ_CMD_LOW,
    ATMEL_READ_CMD_HIGH,
    ATMEL_NUM_INSTRUCTION
};

/*------------------------------ Prototypen -------------------------------*/

TYerrno Atmega_GetInstruction(AtmelID idATMEL,
                              unsigned char idInstruction,
                              unsigned char *pInstruction);

TYerrno Atmega_SelectCurrentAtmelDevice(TYAtmelDevice **pObjAtmelDev,
                                        AtmelDevices devAtmel);

void Atmega_Start(AtmelDevices devAtmel);


void RestoreSCIandAtmels(unsigned int);
void InitSCIForAtmelUpdate(unsigned int);

/*------------------------ Konstantendeklarationen ------------------------*/

/*. ENDEXPORT =============================================================*/

#endif // _VC_ATMEGA_H_
