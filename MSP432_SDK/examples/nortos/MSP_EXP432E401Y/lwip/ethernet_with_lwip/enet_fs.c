//*****************************************************************************
//
// enet_fs.c - File System Processing for lwIP Web Server Apps.
//
// Copyright (c) 2013-2017 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
//
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
//
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
//
//
//*****************************************************************************
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "ti/devices/msp432e4/driverlib/driverlib.h"
#include "ustdlib.h"
#include "utils/lwiplib.h"
#include "lwip/apps/httpd.h"
#include "lwip/apps/fs.h"
#include "third_party/lwip/src/apps/httpd/fsdata.h"
#include "third_party/fatfs/ff.h"


//*****************************************************************************
//
// Include the file system data for this application.  This file is generated
// by the makefsfile utility, using the following command:
//
// ../../../../../tools/examples/makefsfile/makefsfile.exe -i fs -o enet_fsdata.h -r -h -q
//
// If any changes are made to the static content of the web pages served by the
// application, this command must be used to regenerate enet_fsdata.h in order
// for those changes to be picked up by the web server.
//
//*****************************************************************************
#include "enet_fsdata.h"

//*****************************************************************************
//
// The following are data structures used by FatFs.
//
//*****************************************************************************
static FATFS g_sFatFs;

//*****************************************************************************
//
// Initialize the file system.
//
//*****************************************************************************
void
fs_init(void)
{
    //
    // Initialize and mount the Fat File System.
    //
    f_mount(&g_sFatFs, 0, 0);
}

//*****************************************************************************
//
// Open a file and return a handle to the file, if found.  Otherwise,
// return NULL.
//
//*****************************************************************************
err_t fs_open(struct fs_file *file, const char *name)
{
    const struct fsdata_file *f;

      if ((file == NULL) || (name == NULL)) {
         return ERR_ARG;
      }

    #if LWIP_HTTPD_CUSTOM_FILES
      if (fs_open_custom(file, name)) {
        file->is_custom_file = 1;
        return ERR_OK;
      }
      file->is_custom_file = 0;
    #endif /* LWIP_HTTPD_CUSTOM_FILES */

      for (f = FS_ROOT; f != NULL; f = f->next) {
        if (!strcmp(name, (const char *)f->name)) {
          file->data = (const char *)f->data;
          file->len = f->len;
          file->index = f->len;
          file->pextension = NULL;
          file->flags = f->flags;
    #if HTTPD_PRECALCULATED_CHECKSUM
          file->chksum_count = f->chksum_count;
          file->chksum = f->chksum;
    #endif /* HTTPD_PRECALCULATED_CHECKSUM */
    #if LWIP_HTTPD_FILE_STATE
          file->state = fs_state_init(file, name);
    #endif /* #if LWIP_HTTPD_FILE_STATE */
          return ERR_OK;
        }
      }
      /* file not found */
      return ERR_VAL;
}

//*****************************************************************************
//
// Close an opened file designated by the handle.
//
//*****************************************************************************
void
fs_close(struct fs_file *psFile)
{
    //
    // If a Fat file was opened, free its object.
    //
    if(psFile->pextension)
    {
        mem_free(psFile->pextension);
    }

    //
    // Free the main file system object.
    //
    mem_free(psFile);
}

//*****************************************************************************
//
// Read the next chunck of data from the file.  Return the count of data
// that was read.  Return 0 if no data is currently available.  Return
// a -1 if at the end of file.
//
//*****************************************************************************
int
fs_read(struct fs_file *psFile, char *pcBuffer, int iCount)
{
    int iAvailable;
    UINT uiBytesRead;
    FRESULT fresult;

    //
    // Check to see if a Fat File was opened and process it.
    //
    if(psFile->pextension)
    {
        //
        // Read the data.
        //
        fresult = f_read(psFile->pextension, pcBuffer, iCount, &uiBytesRead);
        if((fresult != FR_OK) || (uiBytesRead == 0))
        {
            return(-1);
        }
        return((int)uiBytesRead);
    }

    //
    // Check to see if more data is available.
    //
    if(psFile->index == psFile->len)
    {
        //
        // There is no remaining data.  Return a -1 for EOF indication.
        //
        return(-1);
    }

    //
    // Determine how much data we can copy.  The minimum of the 'iCount'
    // parameter or the available data in the file system buffer.
    //
    iAvailable = psFile->len - psFile->index;
    if(iAvailable > iCount)
    {
        iAvailable = iCount;
    }

    //
    // Copy the data.
    //
    memcpy(pcBuffer, psFile->data + iAvailable, iAvailable);
    psFile->index += iAvailable;

    //
    // Return the count of data that we copied.
    //
    return(iAvailable);
}

//*****************************************************************************
//
// Determine the number of bytes left to read from the file.
//
//*****************************************************************************
int
fs_bytes_left(struct fs_file *psFile)
{
    //
    // Check to see if a Fat File was opened and process it.
    //
    if(psFile->pextension)
    {
        //
        // Return the number of bytes left to be read from the Fat File.
        //
        return(f_size((FIL *)psFile->pextension) -
               f_tell((FIL *)psFile->pextension));
    }

    //
    // Return the number of bytes left to be read from this file.
    //
    return(psFile->len - psFile->index);
}
