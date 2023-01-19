//*****************************************************************************
//
// flash_pb.h - Prototypes for the flash parameter block functions.
//
// Copyright (c) 2008 Texas Instruments Incorporated.  All rights reserved.
// TI Information - Selective Disclosure
//
//*****************************************************************************

#ifndef __FLASH_PB_H__
#define __FLASH_PB_H__

//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// Prototype for the flash parameter block functions.
//
//*****************************************************************************
extern uint8_t *FlashPBGet(void);
extern void FlashPBSave(uint8_t *pui8Buffer);
extern void FlashPBInit(uint32_t ui32Start, uint32_t ui32End,
                        uint32_t ui32Size);

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif // __FLASH_PB_H__
