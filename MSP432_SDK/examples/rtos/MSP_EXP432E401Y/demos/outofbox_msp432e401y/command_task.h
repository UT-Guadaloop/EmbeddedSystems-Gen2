/*
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
 */
 
 /*
 *  ======== command_task.h ========
 */

#ifndef __COMMAND_TASK_H__
#define __COMMAND_TASK_H__

/* Defines the size of the buffer used to store the command. */
#define RX_BUF_SIZE             128

/* Defines the size of the buffer used to store the data to be displayed on
 * UART console. */
#define TX_BUF_SIZE             RX_BUF_SIZE

/* Defines the value that is returned on success. */
#define CMDLINE_SUCCESS         (0)

/* Defines the value that is returned if the command is not found. */
#define CMDLINE_BAD_CMD         (-1)

/* Defines the value that is returned if there are too many arguments. */
#define CMDLINE_TOO_MANY_ARGS   (-2)

/* Defines the value that is returned if there are too few arguments. */
#define CMDLINE_TOO_FEW_ARGS    (-3)

/* Defines the value that is returned if an argument is invalid. */
#define CMDLINE_INVALID_ARG     (-4)

/* Defines the value that is returned if unable to retreive command from UART
 * buffer. */
#define CMDLINE_UART_ERROR      (-5)

/* Defines that inform if command was fully received or partially. */
#define CMD_INCOMPLETE          (1)
#define CMD_RECEIVED            (0)

/* Defines the maximum number of arguments that can be parsed. */
#define CMDLINE_MAX_ARGS        8

/* Command line function callback type. */
typedef int (*pfnCmdLine)(int argc, char *argv[]);

/* Structure for an entry in the command list table. */
typedef struct
{
    //
    //! A pointer to a string containing the name of the command.
    //
    const char *pcCmd;

    //
    //! A function pointer to the implementation of the command.
    //
    pfnCmdLine pfnCmd;

    //
    //! A pointer to a string of brief help text for the command.
    //
    const char *pcHelp;
}
tCmdLineEntry;

/* A structure to pass requests between the cloud task and command task using
 * Mailbox.  The ui32Request element should be one of the pre-defined requests.
 * The buffer can be used to pass data. */
typedef struct sMailboxMsg
{
    //
    // The request identifier for this message.
    //
    uint32_t ui32Request;

    //
    // A message buffer to hold additional message data.
    //
    char pcBuf[RX_BUF_SIZE];
} tMailboxMsg;

/* States to manage printing command prompt. */
enum
{
    Cmd_Prompt_Print,
    Cmd_Prompt_No_Print,
    Cmd_Prompt_No_Erase
};

/* This is the command table that must be provided by the application.  The
 * last element of the array must be a structure whose pcCmd field contains
 * a NULL pointer. */
extern tCmdLineEntry g_psCmdTable[];
extern int32_t CommandTaskInit(void);

#endif // __COMMAND_TASK_H__
