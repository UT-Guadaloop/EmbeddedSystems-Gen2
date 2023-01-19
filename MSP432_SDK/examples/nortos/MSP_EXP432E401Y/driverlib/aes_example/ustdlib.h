//*****************************************************************************
//
// ustdlib.h - Prototypes for simple standard library functions.
//
// Copyright (c) 2007 Texas Instruments Incorporated.  All rights reserved.
// TI Information - Selective Disclosure
//
//*****************************************************************************

#ifndef __USTDLIB_H__
#define __USTDLIB_H__

//*****************************************************************************
//
// Include the standard C headers upon which these replacements are based.
//
//*****************************************************************************
#include <stdarg.h>
#include <time.h>

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
// Prototypes for the APIs.
//
//*****************************************************************************
extern void ulocaltime(time_t timer, struct tm *tm);
extern time_t umktime(struct tm *timeptr);
extern int urand(void);
extern int usnprintf(char * restrict s, size_t n, const char * restrict format,
                     ...);
extern int usprintf(char * restrict s, const char * restrict format, ...);
extern void usrand(unsigned int seed);
extern int ustrcasecmp(const char *s1, const char *s2);
extern int ustrcmp(const char *s1, const char *s2);
extern size_t ustrlen(const char *s);
extern int ustrncasecmp(const char *s1, const char *s2, size_t n);
extern int ustrncmp(const char *s1, const char *s2, size_t n);
extern char *ustrncpy(char * restrict s1, const char * restrict s2, size_t n);
extern char *ustrstr(const char *s1, const char *s2);
extern float ustrtof(const char * restrict nptr,
                     const char ** restrict endptr);
extern unsigned long int ustrtoul(const char * restrict nptr,
                                  const char ** restrict endptr, int base);
extern int uvsnprintf(char * restrict s, size_t n,
                      const char * restrict format, va_list arg);

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif // __USTDLIB_H__
