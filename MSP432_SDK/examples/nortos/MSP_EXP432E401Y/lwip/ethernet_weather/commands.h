//*****************************************************************************
//
// commands.h - Header file for commandline functions in qs-cloud
//
// Copyright (c) 2014 Texas Instruments Incorporated.  All rights reserved.
// TI Information - Selective Disclosure
//
//*****************************************************************************

#ifndef __COMMANDS_H__
#define __COMMANDS_H__

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
// Forward declarations for command-line operations.
//
//*****************************************************************************
extern int Cmd_help(int argc, char *argv[]);
extern int Cmd_cityscroll(int argc, char *argv[]);
extern int Cmd_setproxy(int argc, char *argv[]);

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif // __COMMANDS_H__
