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
 *  ======== command_task.c ========
 */

/* Standard header files */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <mqueue.h>
#include <unistd.h>

/* Driver header files */
#include <ti/drivers/UART.h>

/* Board Header files */
#include "ti_drivers_config.h"
#include "board_funcs.h"
#include "command_task.h"
#include "cloud_task.h"

#define COMMANDTASKSTACKSIZE 2048

/* The banner that is printed when the application starts */
#define BANNER                  "\n\tWelcome to the MSP-EXP432E401Y "         \
                                "LaunchPad's,\n\t\tOut of box Demo.\r\n\n"

/* UART console handle needed by the UART driver. */
UART_Handle g_psUARTHandle;

/* An array to hold the pointers to the command line arguments. */
static char *g_ppcArgv[CMDLINE_MAX_ARGS + 1];

/* An array to hold the command. */
static char g_pcRxBuf[RX_BUF_SIZE];

/* An array to hold the data to be displayed on UART console. */
char g_pcTXBuf[TX_BUF_SIZE];

extern tReadWriteType g_eLEDD1RW;
extern uint32_t g_ui32LEDD1;

/* Mailbox to post command task messages. */
extern mqd_t               mqCmdQueueBlock;
extern mqd_t               mqCmdQueueNoBlock;
extern mqd_t               mqCloudQueueBlock;
extern mqd_t               mqCloudQueueNoBlock;

/*
 *  ======== Cmd_help ========
 * This function implements the "help" command.  It prints a simple list of the
 * available commands with a brief description.
 */
int Cmd_help(int argc, char *argv[])
{
    tCmdLineEntry *pEntry;
    uint32_t ui32BufLen = 0;

    /* Print some header text. */
    ui32BufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE,"\nAvailable commands\r\n");
    UART_write(g_psUARTHandle, g_pcTXBuf, ui32BufLen);
    ui32BufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE,"------------------\r\n");
    UART_write(g_psUARTHandle, g_pcTXBuf, ui32BufLen);

    /* Point at the beginning of the command table. */
    pEntry = &g_psCmdTable[0];

    /* Enter a loop to read each entry from the command table.  The end of the
     * table has been reached when the command name is NULL. */
    while(pEntry->pcCmd)
    {
        /* Print the command name and the brief description. */
        ui32BufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE,"%15s%s\r\n",
                              pEntry->pcCmd, pEntry->pcHelp);
        UART_write(g_psUARTHandle, g_pcTXBuf, ui32BufLen);

        /* Advance to the next entry in the table. */
        pEntry++;
    }

    /* Return success. */
    return(CMDLINE_SUCCESS);
}

/*
 *  ======== Cmd_activate ========
 * If already connected to Exosite server, reconnects and requests a CIK.
 * If not already connected, will try to connect with Exosite server and
 * check for CIK in EEPROM.  If CIK is found, an attempt is made to POST data.
 * If that attempt fails, then CIK is requested.
 *
 * If failed to connect, failure is reported.  Use this command if CIK has not
 * been acquired.  Will replace any existing CIK with a new one if acquired.
 */
int Cmd_activate(int argc, char *argv[])
{
    tMailboxMsg sActivateRequest;

    /* Set the type of request. */
    sActivateRequest.ui32Request = Cloud_Activate_CIK;

    /* Send the request message. */
    mq_send(mqCloudQueueNoBlock, (char*)(&sActivateRequest), 0, 0);

    /* Return success. */
    return(CMDLINE_SUCCESS);
}

/*
 *  ======== Cmd_token ========
 * If already connected to Exosite server, reconnects and requests a CIK.
 * If not already connected, will try to connect with Exosite server and
 * check for CIK in EEPROM.  If CIK is found, an attempt is made to POST data.
 * If that attempt fails, then CIK is requested.
 *
 * If failed to connect, failure is reported.  Use this command if CIK has not
 * been acquired.  Will replace any existing CIK with a new one if acquired.
 */
int Cmd_token(int argc, char *argv[])
{
    uint32_t ui32BufLen = 0;
    tMailboxMsg sCIKRequest;

    /* Check the number of arguments. */
    if(argc == 2)
    {
        /* Correct number of arguments were entered.  Copy user provided token
         * into a buffer to pass to the cloud task. */
        strncpy(sCIKRequest.pcBuf, argv[1], EXOSITE_CIK_LENGTH + 1);

        /* Set the type of request. */
        sCIKRequest.ui32Request = Cloud_Update_Token;

        /* Send the request message. */
        mq_send(mqCloudQueueNoBlock, (char*)(&sCIKRequest), 0, 0);
    }
    else
    {
        /* Print help as invalid parameters were passed with this command. */
        ui32BufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE,"\nToken Command "
                                      "help:\r\n");
        UART_write(g_psUARTHandle, g_pcTXBuf, ui32BufLen);
        ui32BufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE,"    This command "
                              "updates a temporary token on the LaunchPad and "
                              "\r\n");
        UART_write(g_psUARTHandle, g_pcTXBuf, ui32BufLen);
        ui32BufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE,"    attempts to "
                              "re-provision the device with this token.\r\n");
        UART_write(g_psUARTHandle, g_pcTXBuf, ui32BufLen);

        ui32BufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE,"    token <token>\n\n");
        UART_write(g_psUARTHandle, g_pcTXBuf, ui32BufLen);
    }

    /* Return success. */
    return(CMDLINE_SUCCESS);
}

/*
 *  ======== Cmd_clear ========
 * The "clear" command sends an ascii control code to the UART that should
 * clear the screen for most PC-side terminals.
 */
int Cmd_clear(int argc, char *argv[])
{
    uint32_t ui32BufLen = 0;

    ui32BufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE,"\033[2J\033[H");
    UART_write(g_psUARTHandle, g_pcTXBuf, ui32BufLen);

    /* Return success. */
    return(CMDLINE_SUCCESS);
}

/*
 *  ======== Cmd_led ========
 * The "led" command can be used to manually set the state of the two on-board
 * LEDs. The new LED state will also be transmitted back to the exosite server,
 * so the cloud representation of the LEDs should stay in sync with the board's
 * actual behavior.
 */
int Cmd_led(int argc, char *argv[])
{
    uint32_t ui32BufLen;

    /* If we have too few arguments, or the second argument starts with 'h'
     * (like the first character of help), print out information about the
     * usage of this command. */
    if((argc == 2) && (argv[1][0] == 'o'))
    {
        if((argv[1][1] == 'n') || (argv[1][1] == 'f'))
        {
            g_ui32LEDD1 = (argv[1][1] == 'n') ? 1 : 0;
            g_eLEDD1RW = READ_WRITE;

            return 0;
        }
    }

    /* The required arguments were not passed.  So print this command/s help. */
    ui32BufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE,"\nLED command usage:\n\n"
                          "    led <on|off>\n");
    UART_write(g_psUARTHandle, g_pcTXBuf, ui32BufLen);

    return 0;
}

/*
 *  ======== Cmd_connect ========
 * The "connect" command attempts to re-establish a link with the cloud server.
 * This command can be used to connect/re-connect after a cable unplug or other
 * loss of internet connectivity.  Will use the existing CIK if valid, will
 * acquire a new CIK as needed.
 */
int Cmd_connect(int argc, char *argv[])
{
    tMailboxMsg sConnectRequest;

    /* Set the type of request. */
    sConnectRequest.ui32Request = Cloud_Server_Connect;

    /* Send the request message. */
    mq_send(mqCloudQueueNoBlock, (char*)(&sConnectRequest), 0, 0);

    /* Return success. */
    return(CMDLINE_SUCCESS);
}

/*
 *  ======== Cmd_getmac ========
 * The "getmac" command prints the user's current MAC address to the UART.
 */
int Cmd_getmac(int argc, char *argv[])
{
    uint32_t ui32BufLen;

    ui32BufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE, "MAC Address: %s\n",
                          g_pcMACAddress);
    UART_write(g_psUARTHandle, g_pcTXBuf, ui32BufLen);

    return 0;
}

/*
 *  ======== Cmd_proxy ========
 * The proxy command accepts a URL string as a parameter. This string is then
 * used as a HTTP proxy for all future Internet communications.
 */
int Cmd_proxy(int argc, char *argv[])
{
    uint32_t ui32BufLen = 0;
    tMailboxMsg sProxyRequest;

    /* Check the number of arguments. */
    if(argc == 3)
    {
        /* Set the type of request. */
        sProxyRequest.ui32Request = Cloud_Proxy_Set;

        /* Copy the proxy address provided by the user to the message buffer
         * which is actually sent to the other task. Also merges the port and
         * the proxy server into a single string. */
        snprintf(sProxyRequest.pcBuf, 128, "%s:%s", argv[1], argv[2]);

        /* Send the request message. */
        mq_send(mqCloudQueueNoBlock, (char*)(&sProxyRequest), 0, 0);
    }
    else
    {
        ui32BufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE,"\nProxy configuration "
                              "help:\r\n");
        UART_write(g_psUARTHandle, g_pcTXBuf, ui32BufLen);
        ui32BufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE,"    The proxy command "
                              "changes the proxy behavior of this board.\r\n");
        UART_write(g_psUARTHandle, g_pcTXBuf, ui32BufLen);

        ui32BufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE,"    To enable the proxy "
                              "with a specific proxy name and port, type\r\n");
        UART_write(g_psUARTHandle, g_pcTXBuf, ui32BufLen);
        ui32BufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE,"    proxy "
                              "<proxyaddress> <portnumber>. For example:\n\n");
        UART_write(g_psUARTHandle, g_pcTXBuf, ui32BufLen);

        ui32BufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE,"    proxy "
                              "www.mycompanyproxy.com 80\n\n");
        UART_write(g_psUARTHandle, g_pcTXBuf, ui32BufLen);
    }

    /* Return success. */
    return(CMDLINE_SUCCESS);
}

/*
 *  ======== Cmd_pid ========
 * The pid command allows users to provide a product id.
 */
int Cmd_pid(int argc, char *argv[])
{
    uint32_t ui32BufLen;
    tMailboxMsg sPIDMsg;

    /* Check the number of arguments. */
    if(argc == 2)
    {
        /* Correct number of arguments were entered.  Check if length of PID
         * entered is within bounds. */
        if(strlen(argv[1]) > EXOSITE_PID_LENGTH)
        {
            ui32BufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE,"\nPID value entered "
                                  "is incorrect.\r\n");
            UART_write(g_psUARTHandle, g_pcTXBuf, ui32BufLen);
            return(CMDLINE_SUCCESS);
        }

        /* Set the type of request. */
        sPIDMsg.ui32Request = Cloud_Update_PID;

        /* PID value is within bounds.  Store PID in non-volatile memory. */
        strncpy(sPIDMsg.pcBuf, argv[1], EXOSITE_PID_LENGTH + 1);

        /* Send the request message. */
        mq_send(mqCloudQueueNoBlock, (char*)(&sPIDMsg), 0, 0);
    }
    else
    {
        /* Print help as invalid parameters were passed with this command. */
        ui32BufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE,"\nPID Command "
                                      "help:\r\n");
        UART_write(g_psUARTHandle, g_pcTXBuf, ui32BufLen);
        ui32BufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE,"    This command "
                              "updates the Product ID in the non-volatile "
                              "\r\n");
        UART_write(g_psUARTHandle, g_pcTXBuf, ui32BufLen);
        ui32BufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE,"    memory and attempts "
                              "to re-connect the device with this PID.\r\n");
        UART_write(g_psUARTHandle, g_pcTXBuf, ui32BufLen);

        ui32BufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE,"    pid <pid>\n\n");
        UART_write(g_psUARTHandle, g_pcTXBuf, ui32BufLen);
    }

    /* Return success. */
    return(CMDLINE_SUCCESS);
}

/*
 *  ======== Cmd_erase ========
 * The erase command erases the NVS where the PID and token/cik are stored.
 * If this command is used then, you must restart the MCU and enter the PID and
 * token using the "pid" and "token" commands.
 */
int Cmd_erase(int argc, char *argv[])
{
    uint32_t ui32BufLen;

    if(argc == 1)
    {
        /* Correct number of arguments were entered.  Erase EEPROM. */
        EraseEEPROM();
    }
    else
    {
        /* Print help as invalid parameters were passed with this command. */
        ui32BufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE,"\nNVS Erase Command "
                                              "help:\r\n");
        UART_write(g_psUARTHandle, g_pcTXBuf, ui32BufLen);
        ui32BufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE,"    This command "
                              "erases the non-volatile storage where the\r\n");
        UART_write(g_psUARTHandle, g_pcTXBuf, ui32BufLen);
        ui32BufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE,"    PID and token are "
                              "stored. Restart the MCU after using this "
                              "command.\r\n");
        UART_write(g_psUARTHandle, g_pcTXBuf, ui32BufLen);
        ui32BufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE,"    Enter PID and "
                              "optionally token using the respective commands"
                              "\r\n");
        UART_write(g_psUARTHandle, g_pcTXBuf, ui32BufLen);
        ui32BufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE,"    erasenvs\n\n");
        UART_write(g_psUARTHandle, g_pcTXBuf, ui32BufLen);

    }

    /* Return success. */
    return(CMDLINE_SUCCESS);
}

/*
 * This is the table that holds the command names, implementing functions, and
 * brief description. */
tCmdLineEntry g_psCmdTable[] =
{
    { "help",      Cmd_help,      ": Display list of commands" },
    { "h",         Cmd_help,      ": alias for help" },
    { "?",         Cmd_help,      ": alias for help" },
    { "activate",  Cmd_activate,  ": Get a CIK from exosite" },
    { "clear",     Cmd_clear,     ": Clear the display " },
    { "connect",   Cmd_connect,   ": Tries to establish a connection with"
                                  " exosite." },
    { "getmac",    Cmd_getmac,    ": Prints the current MAC address."},
    { "led",       Cmd_led,       ": Toggle LEDs. Type \"led help\" for more "
                                  "info." },
    { "proxy",     Cmd_proxy,     ": Set or disable a HTTP proxy server." },
    { "token",     Cmd_token,     ": Enter Token for re-provisioning"},
    { "pid",       Cmd_pid,       ": Enter product ID"},
    { "erasenvs",  Cmd_erase,     ": Erase NVS"},
    { 0, 0, 0 }
};

/*
 *  ======== CommandReceived ========
 * This function reads the available characters from the UART buffer and checks
 * if a command is received.
 *
 * If CR or LF is received then it is assumend that a command is received, it
 * is copied into a global RX buffer and the flag "CMD_RECEIVED" is returned.
 *
 * If CR or LF is not found then the flag "CMD_INCOMPLETE" is sent to
 * indicate that command might be incomplete.
 *
 * If data is received continuously without a CR or LF and exceeds the length
 * of the global RX buffer, then it is deemed as bad command and the flag
 * "CMDLINE_BAD_CMD" is returned. If the UART driver reports an error then the
 * flag "CMDLINE_UART_ERROR" is is returned.
 */
int32_t CommandReceived(uint32_t ui32RxData)
{
    int32_t i32Ret;
    static uint32_t ui32Idx = 0;

    /* Read data till . */
    while(ui32RxData)
    {
        /* Read one character at a time. */
        i32Ret = UART_read(g_psUARTHandle, g_pcRxBuf + ui32Idx, 1);
        if(i32Ret == UART_ERROR)
        {
            return CMDLINE_UART_ERROR;
        }

        /* Did we get CR or LF? */
        if((*(g_pcRxBuf + ui32Idx) == '\r') ||
           (*(g_pcRxBuf + ui32Idx) == '\n'))
        {
            /* Yes - Copy null character to help in string operations and
             * return success. */
            *(g_pcRxBuf + ui32Idx) = '\0';
            ui32Idx = 0;
            return CMD_RECEIVED;
        }

        ui32RxData--;
        ui32Idx++;

        if(ui32Idx > RX_BUF_SIZE)
        {
            ui32Idx = 0;
            memset(g_pcRxBuf, 0, RX_BUF_SIZE);
            return CMDLINE_BAD_CMD;
        }
    }

    /* We did not get CR or LF.  So return failure to find . */
    return CMD_INCOMPLETE;
}

/*
 *  ======== CmdLineProcess ========
 * This function processes a command line string into arguments and executes
 * the command.
 *
 * This function will take the supplied command line string from the global
 * RX buffer and break it up into individual arguments.  The first argument is
 * treated as a command and is searched for in the command table.  If the
 * command is found, then the command function is called and all of the command
 * line arguments are passed in the normal argc, argv form.
 *
 * The command table is contained in an array named "g_psCmdTable" containing
 * "tCmdLineEntry" structures.  The array must be terminated with an entry
 * whose "pcCmd" field contains a NULL pointer.
 *
 * This function returns either "CMDLINE_BAD_CMD" if the command is not found
 * or "CMDLINE_TOO_MANY_ARGS" if there are more arguments than can be parsed.
 * Otherwise it returns the code that was returned by the command function.
 */
int32_t CmdLineProcess(uint32_t ui32DataLen)
{
    char *pcChar;
    uint_fast8_t ui8Argc;
    bool bFindArg = true;
    tCmdLineEntry *psCmdEntry;

    /* Initialize the argument counter, and point to the beginning of the
     * command line string. */
    ui8Argc = 0;
    pcChar = g_pcRxBuf;

    /* Advance through the command line until a zero character is found. */
    while(*pcChar)
    {
        /* If there is a space, then replace it with a zero, and set the flag
         * to search for the next argument. */
        if(*pcChar == ' ')
        {
            *pcChar = 0;
            bFindArg = true;
        }

        /* Otherwise it is not a space, so it must be a character that is part
         * of an argument. */
        else
        {
            /* If bFindArg is set, then that means we are looking for the start
             * of the next argument. */
            if(bFindArg)
            {
                /* As long as the maximum number of arguments has not been
                 * reached, then save the pointer to the start of this new arg
                 * in the argv array, and increment the count of args, argc. */
                if(ui8Argc < CMDLINE_MAX_ARGS)
                {
                    g_ppcArgv[ui8Argc] = pcChar;
                    ui8Argc++;
                    bFindArg = false;
                }

                /* The maximum number of arguments has been reached so return
                 * the error. */
                else
                {
                    return(CMDLINE_TOO_MANY_ARGS);
                }
            }
        }

        /* Advance to the next character in the command line. */
        pcChar++;
    }

    /* If one or more arguments was found, then process the command. */
    if(ui8Argc)
    {
        /* Start at the beginning of the command table, to look for a matching
         * command. */
        psCmdEntry = &g_psCmdTable[0];

        /* Search through the command table until a null command string is
         * found, which marks the end of the table. */
        while(psCmdEntry->pcCmd)
        {
            /* If this command entry command string matches argv[0], then call
             * the function for this command, passing the command line
             * arguments. */
            if(!strcmp(g_ppcArgv[0], psCmdEntry->pcCmd))
            {
                return(psCmdEntry->pfnCmd(ui8Argc, g_ppcArgv));
            }

            /* Not found, so advance to the next entry. */
            psCmdEntry++;
        }
    }

    /* Fall through to here means that no matching command was found, so return
     * an error. */
    return(CMDLINE_BAD_CMD);
}

/*
 *  ======== CmdLineErrorHandle ========
 * This function reports an error to the user of the command prompt.
 */
void CmdLineErrorHandle(int32_t i32Ret)
{
    uint32_t ui32BufLen;

    /* Handle the case of bad command. */
    if(i32Ret == CMDLINE_BAD_CMD)
    {
        ui32BufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE, "Bad command! Type "
                              "\"help\" for a list of commands.\r\n");
        UART_write(g_psUARTHandle, g_pcTXBuf, ui32BufLen);
    }

    /* Handle the case of too many arguments. */
    else if(i32Ret == CMDLINE_TOO_MANY_ARGS)
    {
        ui32BufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE, "Too many arguments for "
                              "command processor!\r\n");
        UART_write(g_psUARTHandle, g_pcTXBuf, ui32BufLen);
    }

    /* Handle the case of too few arguments. */
    else if(i32Ret == CMDLINE_TOO_FEW_ARGS)
    {
        ui32BufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE, "Too few arguments for "
                              "command processor!\r\n");
        UART_write(g_psUARTHandle, g_pcTXBuf, ui32BufLen);
    }

    /* Handle the case of UART read error. */
    else if(i32Ret == CMDLINE_UART_ERROR)
    {
        ui32BufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE, "UART Read error!\r\n");
        UART_write(g_psUARTHandle, g_pcTXBuf, ui32BufLen);
    }
}

/*
 *  ======== commandTask ========
 * This task manages the COM port.  This function is created statically using
 * the project's .cfg file.
 */
void *commandTask(void *arg0)
{
    UART_Params sUARTParams;
    int32_t i32RxCount;
    int32_t i32Ret;
    uint32_t ui32BufLen;
    tMailboxMsg sCloudMbox;

    /* Create a UART parameters instance.  The default parameter are 115200
     * baud, 8 data bits, 1 stop bit and no parity. */
    UART_Params_init(&sUARTParams);

    /* Modify some of the default parameters for this application. */
    sUARTParams.readReturnMode = UART_RETURN_FULL;
    sUARTParams.readEcho = UART_ECHO_ON;

    /* Configure UART0 with the above parameters. */
    g_psUARTHandle = UART_open(CONFIG_UART0, &sUARTParams);
    if (g_psUARTHandle == NULL)
    {
        while(1);
    }

    /* Print the banner to the debug console */
    UART_write(g_psUARTHandle, BANNER, sizeof(BANNER));

    /* Get MAC address.  If unsuccessful, exit. */
    if(GetMacAddress(g_pcMACAddress, sizeof(g_pcMACAddress)) == false)
    {
        /* Return error. */
        ui32BufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE, "Failed to get MAC "
                              "address.  Looping forever.\n");
        UART_write(g_psUARTHandle, g_pcTXBuf, ui32BufLen);

        while(1);
    }

    /* Print Mac Address. */
    ui32BufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE, "MAC Address: %s\n",
                          g_pcMACAddress);
    UART_write(g_psUARTHandle, g_pcTXBuf, ui32BufLen);

    /* Print help instructions.  These will be erased when we have new data
     * from cloud task to print. */
    ui32BufLen = snprintf(g_pcTXBuf, TX_BUF_SIZE, "Acquiring IP address...");
    UART_write(g_psUARTHandle, g_pcTXBuf, ui32BufLen);

    /* Loop forever receiving commands. */
    while(1)
    {
        /* Check if data is available to be read from UART. */
        UART_control(g_psUARTHandle, UART_CMD_GETRXCOUNT, (void *)&i32RxCount);
        if(i32RxCount > 0)
        {
            /* Yes - See if we got a command. */
            i32Ret = CommandReceived((uint32_t)i32RxCount);
            if(i32Ret == CMD_RECEIVED)
            {
                /* Yes - Process it. */
                i32Ret = CmdLineProcess(i32Ret);
                memset(g_pcRxBuf, 0, RX_BUF_SIZE);
            }

            /* Did we receive any error? */
            if((i32Ret != CMD_INCOMPLETE) || (i32Ret != CMDLINE_SUCCESS))
            {
                /* Yes - Handle the error. */
                CmdLineErrorHandle(i32Ret);
            }

            /* Print prompt except when full command is not received. */
            if(i32Ret != CMD_INCOMPLETE)
            {
                UART_write(g_psUARTHandle, "> ", 2);
            }
        }

        /* Check if we received any data to be printed to UART from Cloud
         * task. */
        if(mq_receive(mqCmdQueueNoBlock, (char*)(&sCloudMbox), sizeof(tMailboxMsg), 0) != -1)
        {
            /* Yes -  Get the length of the data to be printed and pass this
             * information to the UART driver to print.  Don't forget to erase
             * the command prompt before printing the information received. */
            ui32BufLen = strlen(sCloudMbox.pcBuf);
            if(sCloudMbox.ui32Request != Cmd_Prompt_No_Erase)
            {
                UART_write(g_psUARTHandle, "\033[1K\r", 5);
            }
            UART_write(g_psUARTHandle, sCloudMbox.pcBuf, ui32BufLen);

            /* Print prompt for all cases except when the Cloud task wants to
             * indicate progress by printing dots. */
            if(sCloudMbox.ui32Request == Cmd_Prompt_Print)
            {
                /* Print prompt. */
                UART_write(g_psUARTHandle, "> ", 2);
            }
        }
        else
        {
            usleep(100000);
        }
    }
}

/*
 *  ======== CommandTaskInit ========
 * Initialize the CommandTask which manages the UART interface along with
 * command line.
 */
int32_t CommandTaskInit(void)
{
    pthread_t           thread;
    pthread_attr_t      attrs;
    struct sched_param  priParam;
    int                 retc;
    int                 detachState;

    /* Set priority and stack size attributes */
    pthread_attr_init(&attrs);
    priParam.sched_priority = 1;

    detachState = PTHREAD_CREATE_DETACHED;
    retc = pthread_attr_setdetachstate(&attrs, detachState);
    if (retc != 0) {
        /* pthread_attr_setdetachstate() failed */
        return (-1);
    }

    pthread_attr_setschedparam(&attrs, &priParam);

    retc |= pthread_attr_setstacksize(&attrs, COMMANDTASKSTACKSIZE);
    if (retc != 0) {
        /* pthread_attr_setstacksize() failed */
        return (-1);
    }

    retc = pthread_create(&thread, &attrs, commandTask, NULL);
    if (retc != 0) {
        /* pthread_create() failed */
        return (-1);
    }

    return 0;
}
