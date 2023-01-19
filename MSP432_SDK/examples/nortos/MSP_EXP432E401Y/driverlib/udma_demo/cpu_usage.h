//*****************************************************************************
//
// cpu_usage.h - Prototypes for the CPU utilization routines.
//
// Copyright (c) 2007 Texas Instruments Incorporated.  All rights reserved.
// TI Information - Selective Disclosure
//
//*****************************************************************************

#ifndef __CPU_USAGE_H__
#define __CPU_USAGE_H__

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
// Prototypes for the CPU utilization routines.
//
//*****************************************************************************
extern uint32_t CPUUsageTick(void);
extern void CPUUsageInit(uint32_t ui32ClockRate, uint32_t ui32Rate,
                         uint32_t ui32Timer);

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif // __CPU_USAGE_H__
