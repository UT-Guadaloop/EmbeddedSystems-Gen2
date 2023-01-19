/* --COPYRIGHT--,BSD
 * Copyright (c) 2017, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/
/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
/* MSP432E4 USB MSC module                                               */
/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/

/*
 * This file was modified from a sample available from the FatFs
 * web site. It was modified to work with the MSP432E4 USB Library.
 */
#include <stdint.h>
#include <stdbool.h>
#include "ti/devices/msp432e4/driverlib/driverlib.h"
#include "ti/usblib/msp432e4/usblib.h"
#include "ti/usblib/msp432e4/usbmsc.h"
#include "ti/usblib/msp432e4/host/usbhost.h"
#include "ti/usblib/msp432e4/host/usbhmsc.h"
#include "fat_usbmsc.h"

#include "third_party/fatfs/diskio.h"
#include "third_party/fatfs/ff.h"

extern tUSBHMSCInstance *g_psMSCInstance;

static volatile
DSTATUS USBStat = STA_NOINIT;    /* Disk status */

/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS
FATUSBMSC_diskInitialize(
    BYTE bValue)                /* Physical drive number (0) */
{
    /* Set the not initialized flag again. If all goes well and the disk is */
    /* present, this will be cleared at the end of the function.            */
    USBStat |= STA_NOINIT;

    /* Find out if drive is ready yet. */
    if (USBHMSCDriveReady(g_psMSCInstance)) return(FR_NOT_READY);

    /* Clear the not init flag. */
    USBStat &= ~STA_NOINIT;

    return 0;
}

/*-----------------------------------------------------------------------*/
/* Returns the current status of a drive                                 */
/*-----------------------------------------------------------------------*/

DSTATUS FATUSBMSC_diskStatus (
    BYTE drv)                   /* Physical drive number (0) */
{
    if (drv) return STA_NOINIT;        /* Supports only single drive */
    return USBStat;
}

/*-----------------------------------------------------------------------*/
/* This function reads sector(s) from the disk drive                     */
/*-----------------------------------------------------------------------*/

DRESULT FATUSBMSC_diskRead (
    BYTE drv,               /* Physical drive number (0) */
    BYTE* buff,             /* Pointer to the data buffer to store read data */
    DWORD sector,           /* Physical drive nmuber (0) */
    UINT count)             /* Sector count (1..255) */

{
    if(USBStat & STA_NOINIT)
    {
        return(RES_NOTRDY);
    }

    /* READ BLOCK */
    if (USBHMSCBlockRead(g_psMSCInstance, sector, buff, count) == 0)
        return RES_OK;

    return RES_ERROR;
}

/*-----------------------------------------------------------------------*/
/* This function writes sector(s) to the disk drive                     */
/*-----------------------------------------------------------------------*/

#if _READONLY == 0
DRESULT FATUSBMSC_diskWrite (
    BYTE ucDrive,           /* Physical drive number (0) */
    const BYTE* buff,       /* Pointer to the data to be written */
    DWORD sector,           /* Start sector number (LBA) */
    UINT count)             /* Sector count (1..255) */
{
    if (ucDrive || !count) return RES_PARERR;
    if (USBStat & STA_NOINIT) return RES_NOTRDY;
    if (USBStat & STA_PROTECT) return RES_WRPRT;

    /* WRITE BLOCK */
    if(USBHMSCBlockWrite(g_psMSCInstance, sector, (unsigned char *)buff,
                         count) == 0)
        return RES_OK;

    return RES_ERROR;
}
#endif /* _READONLY */

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT FATUSBMSC_diskIOctl (
    BYTE drv,               /* Physical drive number (0) */
    BYTE ctrl,              /* Control code */
    void *buff)             /* Buffer to send/receive control data */
{
    if(USBStat & STA_NOINIT)
    {
        return(RES_NOTRDY);
    }

    switch(ctrl)
    {
        case CTRL_SYNC:
            return(RES_OK);

        default:
            return(RES_PARERR);
    }
}
