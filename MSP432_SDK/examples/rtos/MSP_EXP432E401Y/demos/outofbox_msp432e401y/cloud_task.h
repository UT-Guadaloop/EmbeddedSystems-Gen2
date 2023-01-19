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
 *  ======== cloud_task.h ========
 */

#ifndef __CLOUD_TASK_H__
#define __CLOUD_TASK_H__

/*
 * ToDo USER STEP:
 * To configure proxy network during compile time, uncomment the label
 * "SET_PROXY" and "PROXY_ADDR".  Define the IP address and port number of the
 * desired proxy server by using the label "PROXY_ADDR" in the format
 * "<IP Address>:<Port No.>" as shown below. */
//#define SET_PROXY
//#define PROXY_ADDR              "192.168.1.20:80"

/* Exosite Port number. */
#define EXOSITE_HOSTNAME        "m2.exosite.io"
#define HTTP_PORT_NUMBER        "80"

/* Stack Size of the Cloud task */
#define CLOUDTASKSTACKSIZE 4096

/* Label that defines size of the MAC Address. */
#define MAC_ADDRESS_LENGTH      12

/* Labels that define size of the buffers that hold the provision request and
 * the CIK. */
#define EXOSITE_LENGTH          65
#define EXOSITE_CIK_LENGTH      40
#define EXOSITE_PID_LENGTH      20

/* EEPROM offsets for PID and CIK. */
#define EXOSITE_CIK_OFFSET      0
#define EXOSITE_PID_OFFSET      (EXOSITE_CIK_OFFSET + EXOSITE_CIK_LENGTH + 1)

/* Labels that define the number of Alias that will be received from the server
 * and size of the value returned. */
#define ALIAS_PROCESSING        3
#define VALUEBUF_SIZE           40

/* Cloud connection states. */
typedef enum
{
    Cloud_Server_Connect,
    Cloud_Activate_CIK,
    Cloud_Write,
    Cloud_Read,
    Cloud_Proxy_Set,
    Cloud_Update_Token,
    Cloud_Update_PID,
    Cloud_Idle
} tCloudState;

/* Write/Read status of an alias to/from the cloud server. */
typedef enum
{
    READ_ONLY,
    WRITE_ONLY,
    READ_WRITE,
    NONE
} tReadWriteType;

extern char g_pcMACAddress[MAC_ADDRESS_LENGTH + 1];
extern uint32_t g_ui32IPAddr;
extern bool g_bServerConnect;

/* Prototypes of the functions that are called from outside the cloud_task.c
 * module. */
extern void *cloudTask(void *arg0);

#endif // __CLOUD_TASK_H__
