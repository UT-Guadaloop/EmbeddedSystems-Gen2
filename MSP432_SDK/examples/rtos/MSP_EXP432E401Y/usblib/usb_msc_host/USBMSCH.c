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

/*
 *  ======== USBMSCH.c ========
 */
/* Standard C header files*/
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

/* POSIX Header files */
#include <ti/drivers/dpl/HwiP.h>

/*Header file for UART */
#include <ti/drivers/UART.h>

/* For usleep() */
#include <unistd.h>

/* driverlib Header files */
#include "ti/devices/msp432e4/driverlib/driverlib.h"

/* usblib Header files */
#include <ti/usblib/msp432e4/usb-ids.h>
#include <ti/usblib/msp432e4/usblib.h>
#include <ti/usblib/msp432e4/usbhid.h>
#include <ti/usblib/msp432e4/host/usbhost.h>
#include <ti/usblib/msp432e4/host/usbhmsc.h>
#include "cmdline.h"
#include "third_party/fatfs/ff.h"
#include "third_party/fatfs/diskio.h"
#include "fat_usbmsc.h"

/* POSIX Header files */
#include <pthread.h>

/* Example/Board Header files */
#include <USBMSCH.h>
#include "ti_drivers_config.h"
#include "ti_usblib_config.h"

/* Defines */
#define HCDMEMORYPOOLSIZE   128 /* Memory for the Host Class Driver */
#define MSCMEMORYPOOLSIZE    128 /* Memory for the MSC host driver */

//*****************************************************************************
//
// Defines the size of the buffer used to store the data to be displayed on
// UART console.
//
//*****************************************************************************
#define TX_BUF_SIZE 128

//*****************************************************************************
//
// Defines the size of the buffers that hold the path, or temporary data from
// the memory card.  There are two buffers allocated of this size.  The buffer
// size must be large enough to hold the longest expected full path name,
// including the file name, and a trailing null character.
//
//*****************************************************************************
#define PATH_BUF_SIZE   80

//*****************************************************************************
//
// Defines the size of the buffer that holds the command line.
//
//*****************************************************************************
#define CMD_BUF_SIZE    64

//*****************************************************************************
//
// Defines the number of times to call to check if the attached device is
// ready.
//
//*****************************************************************************
#define USBMSC_DRIVE_RETRY      4

//*****************************************************************************
//
// A macro that holds the number of result codes.
//
//*****************************************************************************
#define NUM_FRESULT_CODES (sizeof(fresultStrings) / sizeof(tFresultString))

//*****************************************************************************
//
// A macro to make it easy to add result codes to the table.
//
//*****************************************************************************
#define FRESULT_ENTRY(f)        { (f), (#f) }

/* Typedefs */
//*****************************************************************************
//
// A structure that holds a mapping between an FRESULT numerical code,
// and a string representation.  FRESULT codes are returned from the FatFs
// FAT file system driver.
//
//*****************************************************************************
typedef struct
{
    FRESULT fresult;
    char *pcResultStr;
}
tFresultString;

//*****************************************************************************
//
// A structure that holds a the current state of the MSC device
//
//*****************************************************************************
typedef volatile enum {
    /*  No device is present.*/
    USBMSC_NO_DEVICE = 0,
    /* Mass storage device is being enumerated.*/
    USBMSC_DEVICE_ENUM,
    /* Mass storage device is ready.*/
    USBMSC_DEVICE_READY,
    /* An unsupported device has been attached.*/
    USBMSC_UNKNOWN_DEVICE,
    /* A mass storage device was connected but failed to ever report ready.*/
    USBMSC_TIMEOUT_DEVICE,
    /* A power fault has occurred.*/
    USBMSC_POWER_FAULT,
} USBMSCH_USBState;

//*****************************************************************************
//
// Instance of the MSC Host
//
//*****************************************************************************
typedef tUSBHMSCInstance         *USBMscHType;

//*****************************************************************************
//
// A table that holds a mapping between the numerical FRESULT code and
// it's name as a string.  This is used for looking up error codes for
// printing to the console.
//
//*****************************************************************************
tFresultString fresultStrings[] =
{
    FRESULT_ENTRY(FR_OK),
    FRESULT_ENTRY(FR_DISK_ERR),
    FRESULT_ENTRY(FR_INT_ERR),
    FRESULT_ENTRY(FR_NOT_READY),
    FRESULT_ENTRY(FR_NO_FILE),
    FRESULT_ENTRY(FR_NO_PATH),
    FRESULT_ENTRY(FR_INVALID_NAME),
    FRESULT_ENTRY(FR_DENIED),
    FRESULT_ENTRY(FR_EXIST),
    FRESULT_ENTRY(FR_INVALID_OBJECT),
    FRESULT_ENTRY(FR_WRITE_PROTECTED),
    FRESULT_ENTRY(FR_INVALID_DRIVE),
    FRESULT_ENTRY(FR_NOT_ENABLED),
    FRESULT_ENTRY(FR_NO_FILESYSTEM),
    FRESULT_ENTRY(FR_MKFS_ABORTED),
    FRESULT_ENTRY(FR_TIMEOUT),
    FRESULT_ENTRY(FR_LOCKED),
    FRESULT_ENTRY(FR_NOT_ENOUGH_CORE),
    FRESULT_ENTRY(FR_TOO_MANY_OPEN_FILES),
    FRESULT_ENTRY(FR_INVALID_PARAMETER)
};

//*****************************************************************************
//
// UART console handle needed by the UART driver.
//
//*****************************************************************************
UART_Handle uartHandle;

//*****************************************************************************
//
// UART Parameters
//
//*****************************************************************************
 UART_Params uartParams;

 //*****************************************************************************
 //
 // Current state of the MSC device
 //
 //*****************************************************************************
 static volatile USBMSCH_USBState state;
 static volatile USBMSCH_USBState UIState;

//*****************************************************************************
//
// The instance data for the MSC driver.
//
//*****************************************************************************
tUSBHMSCInstance *g_psMSCInstance = 0;

//*****************************************************************************
//
// Buffer length
//
//*****************************************************************************
uint32_t uartBufLen = 0;

//*****************************************************************************
//
// An array to hold the data to be displayed on UART console.
//
//*****************************************************************************
char g_pcTXBuf[TX_BUF_SIZE];

//*****************************************************************************
//
// The class of the unknown device.
//
//*****************************************************************************
uint32_t unknownClass;

//*****************************************************************************
//
// This buffer holds the full path to the current working directory.  Initially
// it is root ("/").
//
//*****************************************************************************
static char cwdBuf[PATH_BUF_SIZE] = "/";

//*****************************************************************************
//
// A temporary data buffer used when manipulating file paths, or reading data
// from the memory card.
//
//*****************************************************************************
static char tmpBuf[PATH_BUF_SIZE];

//*****************************************************************************
//
// The buffer that holds the command line arguments.
//
//*****************************************************************************
static char cmdBuf[CMD_BUF_SIZE];

//*****************************************************************************
//
// The buffer that holds the descriptors.
//
//*****************************************************************************
static unsigned char    memPoolHCD[HCDMEMORYPOOLSIZE];

//*****************************************************************************
//
// System clock
//
//*****************************************************************************
static uint32_t sysClock;

//*****************************************************************************
//
// Current FAT fs state.
//
//*****************************************************************************
static FATFS fatFs;
static DIR dirObject;
static FILINFO fileInfo;
static FIL fileObject;

/* Function prototypes */
/*
 *  ======== USBMSCH_cbMSCHandler ========
 *  Callback handler for the USB stack.
 *
 *  Callback handler call by the USB stack to notify us on what has happened in
 *  regards to the MSC device.
 *
 *  @param(cbData)          A callback pointer provided by the client.
 *
 *  @param(event)           Identifies the event that occurred in regards to
 *                          this device.
 *
 *  @param(eventMsgData)    A data value associated with a particular event.
 *
 *  @param(eventMsgPtr)     A data pointer associated with a particular event.
 *
 */
static void USBMSCH_cbMscHandler(USBMscHType instance, uint32_t event,
                                 void *eventMsgPtr);

/*
 *  ======== USBMSCH_hwiHander ========
 *  This function calls the USB library's host interrupt handler.
 *
 *  @param(arg0)          Argument passed in
 *
 *  @return
 *
 */
static void USBMSCH_hwiHandler(uintptr_t arg0);

extern void USB0_IRQHostHandler(void);

/*
 *  ======== USBMSCH_hwiHandler ========
 */
static void USBMSCH_hwiHandler(uintptr_t arg0)
{
    USB0_IRQHostHandler();
}

//*****************************************************************************
//
// This function returns a string representation of an error code that was
// returned from a function call to FatFs.  It can be used for printing human
// readable error messages.
//
//*****************************************************************************
const char *USBMSCH_StringFromFresult(FRESULT fresult)
{
    uint32_t index;

    //
    // Enter a loop to search the error code table for a matching error code.
    //
    for(index = 0; index < NUM_FRESULT_CODES; index++)
    {
        //
        // If a match is found, then return the string name of the error code.
        //
        if(fresultStrings[index].fresult == fresult)
        {
            return(fresultStrings[index].pcResultStr);
        }
    }

    //
    // At this point no matching code was found, so return a string indicating
    // unknown error.
    //
    return("UNKNOWN ERROR CODE");
}

//*****************************************************************************
//
// This function implements the "ls" command.  It opens the current directory
// and enumerates through the contents, and prints a line for each item it
// finds.  It shows details such as file attributes, time and date, and the
// file size, along with the name.  It shows a summary of file sizes at the end
// along with free space.
//
//*****************************************************************************
int Cmd_ls(int argc, char *argv[])
{
    uint32_t totalSize;
    uint32_t fileCount;
    uint32_t dirCount;
    FRESULT fresult;
    FATFS *pFatFs;

    //
    // Do not attempt to do anything if there is not a drive attached.
    //
    if(state != USBMSC_DEVICE_READY)
    {
        return(FR_NOT_READY);
    }

    //
    // Open the current directory for access.
    //
    fresult = f_opendir(&dirObject, cwdBuf);

    //
    // Check for error and return if there is a problem.
    //
    if(fresult != FR_OK)
    {
        return(fresult);
    }

    totalSize = 0;
    fileCount = 0;
    dirCount = 0;

    //
    // Enter loop to enumerate through all directory entries.
    //
    while(1)
    {
        //
        // Read an entry from the directory.
        //
        fresult = f_readdir(&dirObject, &fileInfo);

        //
        // Check for error and return if there is a problem.
        //
        if(fresult != FR_OK)
        {
            return(fresult);
        }

        //
        // If the file name is blank, then this is the end of the listing.
        //
        if(!fileInfo.fname[0])
        {
            break;
        }

        //
        // If the attribute is directory, then increment the directory count.
        //
        if(fileInfo.fattrib & AM_DIR)
        {
            dirCount++;
        }

        //
        // Otherwise, it is a file.  Increment the file count, and add in the
        // file size to the total.
        //
        else
        {
            fileCount++;
            totalSize += fileInfo.fsize;
        }

        //
        // Print the entry information on a single line with formatting to show
        // the attributes, date, time, size, and name.
        //
        uartBufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE, "%c%c%c%c%c %u/%02u/%02u %02u:%02u %9u  %s\n",
                               (fileInfo.fattrib & AM_DIR) ? 'D' : '-',
                               (fileInfo.fattrib & AM_RDO) ? 'R' : '-',
                               (fileInfo.fattrib & AM_HID) ? 'H' : '-',
                               (fileInfo.fattrib & AM_SYS) ? 'S' : '-',
                               (fileInfo.fattrib & AM_ARC) ? 'A' : '-',
                               (fileInfo.fdate >> 9) + 1980,
                               (fileInfo.fdate >> 5) & 15,
                                fileInfo.fdate & 31,
                               (fileInfo.ftime >> 11),
                               (fileInfo.ftime >> 5) & 63,
                                (unsigned int)fileInfo.fsize,
                                fileInfo.fname);
        UART_writePolling(uartHandle, g_pcTXBuf, uartBufLen);
    }

    //
    // Print summary lines showing the file, dir, and size totals.
    //
    uartBufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE,"\n%4u File(s),%10u bytes total\n%4u Dir(s)", (unsigned int)fileCount, (unsigned int)totalSize, (unsigned int)dirCount);
    UART_writePolling(uartHandle, g_pcTXBuf, uartBufLen);

    //
    // Get the free space.
    //
    fresult = f_getfree("/", (DWORD *)&totalSize, &pFatFs);

    //
    // Check for error and return if there is a problem.
    //
    if(fresult != FR_OK)
    {
        return(fresult);
    }

    //
    // Display the amount of free space that was calculated.
    //
    uartBufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE,", %10uK bytes free\n", (unsigned int)(totalSize * pFatFs->csize / 2));
    UART_writePolling(uartHandle, g_pcTXBuf, uartBufLen);

    //
    // Made it to here, return with no errors.
    //
    return(0);
}

//*****************************************************************************
//
// This function implements the "cd" command.  It takes an argument that
// specifies the directory to make the current working directory.  Path
// separators must use a forward slash "/".  The argument to cd can be one of
// the following:
//
// * root ("/")
// * a fully specified path ("/my/path/to/mydir")
// * a single directory name that is in the current directory ("mydir")
// * parent directory ("..")
//
// It does not understand relative paths, so don't try something like this:
// ("../my/new/path")
//
// Once the new directory is specified, it attempts to open the directory to
// make sure it exists.  If the new path is opened successfully, then the
// current working directory (cwd) is changed to the new path.
//
//*****************************************************************************
int Cmd_cd(int argc, char *argv[])
{
    uint32_t index;
    FRESULT fresult;

    //
    // Do not attempt to do anything if there is not a drive attached.
    //
    if(state != USBMSC_DEVICE_READY)
    {
        return(FR_NOT_READY);
    }

    //
    // Copy the current working path into a temporary buffer so it can be
    // manipulated.
    //
    strcpy(tmpBuf, cwdBuf);

    //
    // If the first character is /, then this is a fully specified path, and it
    // should just be used as-is.
    //
    if(argv[1][0] == '/')
    {
        //
        // Make sure the new path is not bigger than the cwd buffer.
        //
        if(strlen(argv[1]) + 1 > sizeof(cwdBuf))
        {

            uartBufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE,"Resulting path name is too long\n");
            UART_writePolling(uartHandle, g_pcTXBuf, uartBufLen);

            return(0);
        }

        //
        // If the new path name (in argv[1])  is not too long, then copy it
        // into the temporary buffer so it can be checked.
        //
        else
        {
            strncpy(tmpBuf, argv[1], sizeof(tmpBuf));
        }
    }

    //
    // If the argument is .. then attempt to remove the lowest level on the
    // CWD.
    //
    else if(!strcmp(argv[1], ".."))
    {
        //
        // Get the index to the last character in the current path.
        //
        index = strlen(tmpBuf) - 1;

        //
        // Back up from the end of the path name until a separator (/) is
        // found, or until we bump up to the start of the path.
        //
        while((tmpBuf[index] != '/') && (index > 1))
        {
            //
            // Back up one character.
            //
            index--;
        }

        //
        // Now we are either at the lowest level separator in the current path,
        // or at the beginning of the string (root).  So set the new end of
        // string here, effectively removing that last part of the path.
        //
        tmpBuf[index] = 0;
    }

    //
    // Otherwise this is just a normal path name from the current directory,
    // and it needs to be appended to the current path.
    //
    else
    {
        //
        // Test to make sure that when the new additional path is added on to
        // the current path, there is room in the buffer for the full new path.
        // It needs to include a new separator, and a trailing null character.
        //
        if(strlen(tmpBuf) + strlen(argv[1]) + 1 + 1 > sizeof(cwdBuf))
        {

            uartBufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE,"Resulting path name is too long\n");
            UART_writePolling(uartHandle, g_pcTXBuf, uartBufLen);
            return(0);
        }

        //
        // The new path is okay, so add the separator and then append the new
        // directory to the path.
        //
        else
        {
            //
            // If not already at the root level, then append a /
            //
            if(strcmp(tmpBuf, "/"))
            {
                strcat(tmpBuf, "/");
            }

            //
            // Append the new directory to the path.
            //
            strcat(tmpBuf, argv[1]);
        }
    }

    //
    // At this point, a candidate new directory path is in chTmpBuf.  Try to
    // open it to make sure it is valid.
    //
    fresult = f_opendir(&dirObject, tmpBuf);

    //
    // If it can't be opened, then it is a bad path.  Inform user and return.
    //
    if(fresult != FR_OK)
    {

        uartBufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE,"cd: %s\n", tmpBuf);
        UART_writePolling(uartHandle, g_pcTXBuf, uartBufLen);
        return(fresult);
    }

    //
    // Otherwise, it is a valid new path, so copy it into the CWD.
    //
    else
    {
        strncpy(cwdBuf, tmpBuf, sizeof(cwdBuf));
    }

    //
    // Return success.
    //
    return(0);
}

//*****************************************************************************
//
// This function implements the "pwd" command.  It simply prints the current
// working directory.
//
//*****************************************************************************
int Cmd_pwd(int argc, char *argv[])
{
    //
    // Do not attempt to do anything if there is not a drive attached.
    //
    if(state != USBMSC_DEVICE_READY)
    {
        return(FR_NOT_READY);
    }

    //
    // Print the CWD to the console.
    //

    uartBufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE,"%s\n", cwdBuf);
    UART_writePolling(uartHandle, g_pcTXBuf, uartBufLen);

    //
    // Return success.
    //
    return(0);
}

//*****************************************************************************
//
// This function implements the "cat" command.  It reads the contents of a file
// and prints it to the console.  This should only be used on text files.  If
// it is used on a binary file, then a bunch of garbage is likely to printed on
// the console.
//
//*****************************************************************************
int Cmd_cat(int argc, char *argv[])
{
    FRESULT fresult;
    uint32_t bytesRead;

    //
    // Do not attempt to do anything if there is not a drive attached.
    //
    if(state != USBMSC_DEVICE_READY)
    {
        return(FR_NOT_READY);
    }

    //
    // First, check to make sure that the current path (CWD), plus the file
    // name, plus a separator and trailing null, will all fit in the temporary
    // buffer that will be used to hold the file name.  The file name must be
    // fully specified, with path, to FatFs.
    //
    if(strlen(cwdBuf) + strlen(argv[1]) + 1 + 1 > sizeof(tmpBuf))
    {
        uartBufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE,"Resulting path name is too long\n");
        UART_writePolling(uartHandle, g_pcTXBuf, uartBufLen);
        return(0);
    }

    //
    // Copy the current path to the temporary buffer so it can be manipulated.
    //
    strcpy(tmpBuf, cwdBuf);

    //
    // If not already at the root level, then append a separator.
    //
    if(strcmp("/", cwdBuf))
    {
        strcat(tmpBuf, "/");
    }

    //
    // Now finally, append the file name to result in a fully specified file.
    //
    strcat(tmpBuf, argv[1]);

    //
    // Open the file for reading.
    //
    fresult = f_open(&fileObject, tmpBuf, FA_READ);

    //
    // If there was some problem opening the file, then return an error.
    //
    if(fresult != FR_OK)
    {
        return(fresult);
    }

    //
    // Enter a loop to repeatedly read data from the file and display it, until
    // the end of the file is reached.
    //
    do
    {
        //
        // Read a block of data from the file.  Read as much as can fit in the
        // temporary buffer, including a space for the trailing null.
        //
        fresult = f_read(&fileObject, tmpBuf, sizeof(tmpBuf) - 1,
                         (UINT *)&bytesRead);

        //
        // If there was an error reading, then print a newline and return the
        // error to the user.
        //
        if(fresult != FR_OK)
        {

            uartBufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE,"\n");
            UART_writePolling(uartHandle, g_pcTXBuf, uartBufLen);
            return(fresult);
        }

        //
        // Null terminate the last block that was read to make it a null
        // terminated string that can be used with printf.
        //
        tmpBuf[bytesRead] = 0;

        //
        // Print the last chunk of the file that was received.
        //

        uartBufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE,"%s", tmpBuf);
        UART_writePolling(uartHandle, g_pcTXBuf, uartBufLen);

        //
        // Continue reading until less than the full number of bytes are read.
        // That means the end of the buffer was reached.
        //
    }
    while(bytesRead == sizeof(tmpBuf) - 1);

    //
    // Return success.
    //
    return(0);
}

//*****************************************************************************
//
// This function implements the "help" command.  It prints a simple list of the
// available commands with a brief description.
//
//*****************************************************************************
int Cmd_help(int argc, char *argv[])
{
    tCmdLineEntry *pEntry;

    //
    // Print some header text.
    //

    uartBufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE,"Available commands\r\n");
    UART_writePolling(uartHandle, g_pcTXBuf, uartBufLen);
    uartBufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE,"------------------\n" );
    UART_writePolling(uartHandle, g_pcTXBuf, uartBufLen);

    //
    // Point at the beginning of the command table.
    //
    pEntry = &g_psCmdTable[0];

    //
    // Enter a loop to read each entry from the command table.  The end of the
    // table has been reached when the command name is NULL.
    //
    while(pEntry->pcCmd)
    {
        //
        // Print the command name and the brief description.
        //

        uartBufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE,"%s%s\n", pEntry->pcCmd, pEntry->pcHelp);
        UART_writePolling(uartHandle, g_pcTXBuf, uartBufLen);

        //
        // Advance to the next entry in the table.
        //
        pEntry++;
    }

    //
    // Return success.
    //
    return(0);
}

//*****************************************************************************
//
// This is the table that holds the command names, implementing functions, and
// brief description.
//
//*****************************************************************************
tCmdLineEntry g_psCmdTable[] =
{
    { "help",   Cmd_help,      " : Display list of commands" },
    { "h",      Cmd_help,   "    : alias for help" },
    { "?",      Cmd_help,   "    : alias for help" },
    { "ls",     Cmd_ls,      "   : Display list of files" },
    { "chdir",  Cmd_cd,         ": Change directory" },
    { "cd",     Cmd_cd,      "   : alias for chdir" },
    { "pwd",    Cmd_pwd,      "  : Show current working directory" },
    { "cat",    Cmd_cat,      "  : Show contents of a text file" },
    { 0, 0, 0 }
};

/*
 *  ======== USBMSCH_cbMSCHandler ========
 *
 *  This is the callback from the MSC driver.
 *
 *  \param instance is the driver instance which is needed when
 *  communicating with the driver.
 *  \param event is one of the events defined by the driver.
 *  \param eventMsgPtr is a pointer to data passed into the initial call to register
 *  the callback.
 *
 *  This function handles callback events from the MSC driver.  The only events
 *  currently handled are the MSC_EVENT_OPEN and MSC_EVENT_CLOSE.  This allows
 *  the main routine to know when a MSC device has been detected and
 *  enumerated and when a MSC device has been removed from the system.
 *
 *  \return None
 *
 */
static void USBMSCH_cbMscHandler(USBMscHType instance, uint32_t event,
                                              void *eventMsgPtr)
{
    /* Determine what event has happened */
    switch (event) {
        case MSC_EVENT_OPEN:

            /* Set the MSC state for initialization */
            state = USBMSC_DEVICE_ENUM;

            break;

         case MSC_EVENT_CLOSE:
             /* Go back to the "no device" state and wait for a new connection*/
             state = USBMSC_NO_DEVICE;

            break;

        default:
            break;
    }
}

/*
 *  ======== USBHCDEvents ========
 *
 * This is the generic callback from host stack.
 *
 * \param cbData is actually a pointer to a tEventInfo structure.
 *
 * This function will be called to inform the application when a USB event has
 * occurred that is outside those related to the mass storage device.  At this
 * point this is used to detect unsupported devices being inserted and removed.
 * It is also used to inform the application when a power fault has occurred.
 * This function is required when the g_USBGenericEventDriver is included in
 * the host controller driver array that is passed in to the
 * USBHCDRegisterDrivers() function.
 *
 * \return None.
 *
 */
void USBHCDEvents(void *cbData)
{
    tEventInfo *pEventInfo;

    /* Cast this pointer to its actual type. */
    pEventInfo = (tEventInfo *)cbData;

    switch (pEventInfo->ui32Event) {

        /* Unknown device connected*/
        case USB_EVENT_UNKNOWN_CONNECTED:

            /* Save the unknown class.*/
            unknownClass = pEventInfo->ui32Instance;
            /* An unknown device was detected. */
            state = USBMSC_UNKNOWN_DEVICE;
            break;

        /* Unknown device is disconnected*/
        case USB_EVENT_DISCONNECTED:
            /* Unknown device has been removed. */
            state = USBMSC_NO_DEVICE;
            break;

        case USB_EVENT_POWER_FAULT:
            /* No power means no device is present. */
            state = USBMSC_POWER_FAULT;
            break;

        default:
            break;
    }
}

//*****************************************************************************
//
// This function reads a line of text from the UART console.  The USB host main
// function is called throughout this process to keep USB alive and well.
//
//*****************************************************************************
void USBMSCH_ReadLine(void)
{
    uint32_t prompt;

    uint32_t stateCopy;
    uint32_t driveTimeout;
    uint8_t tempIndex=0, charIndex, tempBufIdx = 0;
    uint32_t rxCharCount=0;
    char tmpBuffer[CMD_BUF_SIZE];

    /* Start reading at the beginning of the command buffer and print a prompt.*/
    cmdBuf[0] = '\0';
    charIndex = 0;
    prompt = 1;

    /* Initialize the drive timeout.*/
    driveTimeout = USBMSC_DRIVE_RETRY;

    /*
     * Loop forever.  This loop will be explicitly broken out of when the line
     * has been fully read.
     */
    while(1)
    {

        /* See if a mass storage device has been enumerated.*/
        if(state == USBMSC_DEVICE_ENUM)
        {
            /*
             * Take it easy on the Mass storage device if it is slow to
             * start up after connecting.
             */
            if(USBHMSCDriveReady(g_psMSCInstance) != 0)
            {
                /*
                 * Wait about 500ms before attempting to check if the
                 * device is ready again.
                 */
                SysCtlDelay(sysClock/(3*2));

                /* Decrement the retry count.*/
                driveTimeout--;

                /*
                 * If the timeout is hit then go to the
                 * STATE_TIMEOUT_DEVICE state.
                 */
                if(driveTimeout == 0)
                {
                    state = USBMSC_TIMEOUT_DEVICE;
                }

                break;
            }

            /* Reset the working directory to the root.*/
            cwdBuf[0] = '/';
            cwdBuf[1] = '\0';

            /*
             * Attempt to open the directory.  Some drives take longer to
             * start up than others, and this may fail (even though the USB
             * device has enumerated) if it is still initializing.
             */
            f_mount(&fatFs, "/", 1);

            if(f_opendir(&dirObject, cwdBuf) == FR_OK)
            {

                /* The drive is fully ready, so move to that state.*/
                state = USBMSC_DEVICE_READY;
            }

        }

        /*
         * See if the state has changed.  We make a copy of g_eUIState to
         * prevent a compiler warning about undefined order of volatile
         * accesses.
         */
        stateCopy = UIState;
        if(state != stateCopy)
        {

            /* Determine the new state.*/
            switch(state)
            {

                /* A previously connected device has been disconnected.*/
                case USBMSC_NO_DEVICE:
                {
                    if(UIState == USBMSC_UNKNOWN_DEVICE)
                    {

                        uartBufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE,"\nUnknown device disconnected.\n");
                        UART_writePolling(uartHandle, g_pcTXBuf, uartBufLen);

                    }
                    else
                    {

                        uartBufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE,"\nMass storage device disconnected.\n");
                        UART_writePolling(uartHandle, g_pcTXBuf, uartBufLen);

                    }
                    prompt = 1;
                    break;
                }


                /* A mass storage device is being enumerated.*/
                case USBMSC_DEVICE_ENUM:
                {
                    break;
                }

                /* A mass storage device has been enumerated and initialized.*/
                case USBMSC_DEVICE_READY:
                {

                    uartBufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE,"\nMass storage device connected.\n");
                    UART_writePolling(uartHandle, g_pcTXBuf, uartBufLen);

                    prompt = 1;
                    break;
                }

                /* An unknown device has been connected.*/
                case USBMSC_UNKNOWN_DEVICE:
                {

                    uartBufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE,"Unknown Device Class (0x%02x) Connected.\n",
                                                             (unsigned int)unknownClass);
                    UART_writePolling(uartHandle, g_pcTXBuf, uartBufLen);

                    prompt = 1;
                    break;
                }

                /* The connected mass storage device is not reporting ready.*/
                case USBMSC_TIMEOUT_DEVICE:
                {
                    /*
                     * If this is the first time in this state then print a
                     * message.
                     */
                    uartBufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE,"Device Timeout.\n");
                    UART_writePolling(uartHandle, g_pcTXBuf, uartBufLen);

                    prompt = 1;

                    break;
                }

                /* A power fault has occurred.*/
                case USBMSC_POWER_FAULT:
                {

                    uartBufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE,"\nPower fault.\n");
                    UART_writePolling(uartHandle, g_pcTXBuf, uartBufLen);

                    prompt = 1;
                    break;
                }
            }

            /* Save the current state.*/
            UIState = state;
        }

        /* Print a prompt if necessary.*/
        if(prompt)
        {

            /* Print the prompt based on the current state.*/
            if(state == USBMSC_DEVICE_READY)
            {

                uartBufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE,"%s> %s", cwdBuf, cmdBuf);
                UART_writePolling(uartHandle, g_pcTXBuf, uartBufLen);

            }
            else if(state == USBMSC_UNKNOWN_DEVICE)
            {

                uartBufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE,"UNKNOWN> %s", cmdBuf);
                UART_writePolling(uartHandle, g_pcTXBuf, uartBufLen);

            }
            else
            {

                uartBufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE,"NODEV> %s", cmdBuf);
                UART_writePolling(uartHandle, g_pcTXBuf, uartBufLen);

            }

            /* The prompt no longer needs to be printed.*/
            prompt = 0;
        }

        /*
         * See if there is a character that has been received from the
         * UART.
         */
        UART_control(uartHandle, UART_CMD_GETRXCOUNT, (void *)&rxCharCount);

        /* There is a character received from UART*/
        if(rxCharCount > 0)
        {
            /* Get the character*/
            UART_read(uartHandle, tmpBuffer + tempBufIdx, rxCharCount);

            /*
             * See if this character is a backspace (delete) and there is at least one
             * character in the input line
             */
            if((tmpBuffer[tempBufIdx] == 0x7F) && (charIndex != 0))
            {

                // Erase the last character from the input line.
                uartBufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE,"\b \b");
                UART_writePolling(uartHandle, g_pcTXBuf, uartBufLen);

                /* decrease the character index*/
                charIndex--;
                cmdBuf[charIndex] = '\0';

            }
            /* See if this character is a newline.*/
            else if((tmpBuffer[tempBufIdx] == '\r') || (tmpBuffer[tempBufIdx] == '\n'))
            {

                /* Return to the caller since user has finished entering characters  */
                uartBufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE,"\r\n");
                UART_writePolling(uartHandle, g_pcTXBuf, uartBufLen);

                return;

            }
            /*See if this character is an escape or Ctrl-U.*/
            else if((tmpBuffer[tempBufIdx] == 0x1b) || (tmpBuffer[tempBufIdx] == 0x15))
            {

                /* Erase all characters in the input buffer.*/
                while(charIndex)
                {

                    uartBufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE,"\b \b");
                    UART_writePolling(uartHandle, g_pcTXBuf, uartBufLen);

                    charIndex--;
                }
                cmdBuf[0] = '\0';
            }
            /* See if this is a printable ASCII character. */
            else if((tmpBuffer[tempBufIdx] >= ' ') && (tmpBuffer[tempBufIdx] <= '~') &&
                        (charIndex < (sizeof(cmdBuf) - 1)))
            {
                tempIndex = charIndex;

                /*Add this character to the input buffer.*/
                cmdBuf[charIndex++] = tmpBuffer[tempBufIdx];
                cmdBuf[charIndex] = '\0';

                uartBufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE,"%c", cmdBuf[tempIndex]);
                UART_writePolling(uartHandle, g_pcTXBuf, uartBufLen);

            }
        }
        /* Call the host library*/
        USBHCDMain();
    }
}

/*
 *  ======== USBMSCH_init ========
 */
void USBMSCH_init(bool usbInternal)
{
    HwiP_Handle hwi;
    uint32_t    PLLRate;
    uint32_t    ui32ULPI;

    /* Set initial state */
    state = USBMSC_NO_DEVICE;
    UIState = USBMSC_NO_DEVICE;


    /* Initialize the USB stack for host mode. */
    USBStackModeSet(0, eUSBModeHost, NULL);

    /* Register host class drivers */
    USBHCDRegisterDrivers(0, usbHostClassDrivers, numHostClassDrivers);

    /*register FAT file API calls*/
    disk_register(0,
                   FATUSBMSC_diskInitialize,
                   FATUSBMSC_diskStatus,
                   FATUSBMSC_diskRead,
                   FATUSBMSC_diskWrite,
                   FATUSBMSC_diskIOctl);

    /* Open an instance of the MSC host driver */
    g_psMSCInstance = USBHMSCDriveOpen(0, USBMSCH_cbMscHandler);

    /*instance of MSC host driver could not be created*/
    if(!g_psMSCInstance)
    {
        //Error initializing the MSC Host
        while(1);
    }

    /* Check if the ULPI mode is to be used or not */
    if(!(usbInternal)) {
        ui32ULPI = USBLIB_FEATURE_ULPI_HS;
        USBHCDFeatureSet(0, USBLIB_FEATURE_USBULPI, &ui32ULPI);
    } else {
		/* Initialize USB power configuration*/
		USBHCDPowerConfigInit(0, USBHCD_VBUS_AUTO_HIGH | USBHCD_VBUS_FILTER);
	}

    /*Run from the PLL */
    sysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                               SYSCTL_OSC_MAIN | SYSCTL_USE_PLL |
                                               SYSCTL_CFG_VCO_480), 120000000);

    /*Gets the VCO frequency of 120MHZ */
    SysCtlVCOGet(SYSCTL_XTAL_25MHZ, &PLLRate);

    /*Set the PLL*/
    USBHCDFeatureSet(0, USBLIB_FEATURE_USBPLL, &PLLRate);

    /* Enable the USB stack */
    USBHCDInit(0, memPoolHCD, HCDMEMORYPOOLSIZE);
	
	uartBufLen=snprintf(g_pcTXBuf, TX_BUF_SIZE, "\033[2J\033[H");
	UART_writePolling(uartHandle, g_pcTXBuf, uartBufLen);
	
    uartBufLen=snprintf(g_pcTXBuf, TX_BUF_SIZE, "USB Mass Storage Host program");
    uartBufLen+=snprintf(g_pcTXBuf + uartBufLen, TX_BUF_SIZE, "\r\nType \'help\' for help.\n\n");
    uartBufLen+=snprintf(g_pcTXBuf + uartBufLen, TX_BUF_SIZE, "\r");
    UART_writePolling(uartHandle, g_pcTXBuf, uartBufLen);

    /* Install interrupt handler */
    hwi = HwiP_create(INT_USB0, USBMSCH_hwiHandler, NULL);
    if (hwi == NULL)
    {
        //Can't create USB Hwi
        uartBufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE,"Can't create USB Hwi.\n");
        UART_writePolling(uartHandle, g_pcTXBuf, uartBufLen);

        while(1);
    }

}

/*
 *  ======== mscHostFxn ========
 *  Task for this function is created statically. See the project's .cfg file.
 */
void *mscHostFxn (void *arg0)
{
    int8_t status;

    bool *usbInternal = (bool *)arg0;

    UART_init();

    /*
     *  Initialize the UART parameters outside the loop. Let's keep
     *  most of the defaults (e.g. baudrate = 115200) and only change the
     *  following.
     *
     */
    UART_Params_init(&uartParams);

    /* Turn echo off*/
    uartParams.readEcho = UART_ECHO_OFF;

    /* Create a UART for the console */
    uartHandle = UART_open(CONFIG_UART_0, &uartParams);
    if (uartHandle == NULL)
    {
            while (1);
    }

    /* Call function to initialize the host*/
    USBMSCH_init(usbInternal);

    while (true)
    {
        /* Get a line of text from the user.*/
        USBMSCH_ReadLine();

        /*
         * Pass the line from the user to the command processor.
         * It will be parsed and valid commands executed.
         */
        status = CmdLineProcess(cmdBuf);

        /* Handle the case of bad command.*/
        if(status == CMDLINE_BAD_CMD)
        {
            uartBufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE,"Bad command!\n");
            UART_writePolling(uartHandle, g_pcTXBuf, uartBufLen);
        }

        /* Handle the case of too many arguments.*/
        else if(status == CMDLINE_TOO_MANY_ARGS)
        {

            uartBufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE,"Too many arguments for command processor!\n");
            UART_writePolling(uartHandle, g_pcTXBuf, uartBufLen);
        }

        /*
         * Otherwise the command was executed.  Print the error
         * code if one was returned.
         */
        else if(status != 0)
        {

            uartBufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE,"Command returned error code %s\n",
                                  USBMSCH_StringFromFresult((FRESULT)status));
            UART_writePolling(uartHandle, g_pcTXBuf, uartBufLen);

        }
    }
}
