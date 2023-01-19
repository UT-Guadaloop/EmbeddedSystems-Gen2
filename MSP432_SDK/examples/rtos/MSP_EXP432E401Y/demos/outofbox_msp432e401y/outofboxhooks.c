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
 *    ======== outofboxhooks.c ========
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include <pthread.h>
#include <mqueue.h>

#include <ti/ndk/inc/netmain.h>

#include <ti/ndk/slnetif/slnetifndk.h>
#include <ti/net/slnet.h>
#include <ti/net/slnetif.h>
#include <ti/net/slnetutils.h>

/* Example/Board Header files */
#include "cloud_task.h"
#include "command_task.h"

extern mqd_t mqCmdQueueNoBlock;

/*
 *  ======== netIPAddrHook ========
 *  user defined network IP address hook
 */
void netIPAddrHook(uint32_t IPAddr, unsigned int IfIdx, unsigned int fAdd)
{
    pthread_t          thread;
    pthread_attr_t     attrs;
    struct sched_param priParam;
    int                retc;
    int                detachState;
    uint32_t           hostByteAddr;
    static bool createTask = true;
    int32_t status = 0;
    tMailboxMsg sDebug;

    if (fAdd) {
        snprintf(sDebug.pcBuf, TX_BUF_SIZE, "Network Added: ");
    }
    else {
        snprintf(sDebug.pcBuf, TX_BUF_SIZE, "Network Removed: ");
    }
    mq_send(mqCmdQueueNoBlock, (char*)(&sDebug), 0, 0);

    /* print the IP address that was added/removed */
    hostByteAddr = NDK_ntohl(IPAddr);
    snprintf(sDebug.pcBuf, TX_BUF_SIZE, "If-%d:%d.%d.%d.%d\n", IfIdx,
             (uint8_t)(hostByteAddr>>24)&0xFF, (uint8_t)(hostByteAddr>>16)&0xFF,
             (uint8_t)(hostByteAddr>>8)&0xFF, (uint8_t)hostByteAddr&0xFF);
    mq_send(mqCmdQueueNoBlock, (char*)(&sDebug), 0, 0);

    /* initialize SlNet interface(s) */
    status = ti_net_SlNet_initConfig();
    if (status < 0)
    {
        snprintf(sDebug.pcBuf, TX_BUF_SIZE, "Failed to initialize SlNet "
                 "interface(s)- status (%d)\n", (int)status);
        mq_send(mqCmdQueueNoBlock, (char*)(&sDebug), 0, 0);
        while (1);
    }

    if (fAdd && createTask) {
        /*
         *  Create the Task that farms out incoming TCP connections.
         *  arg0 will be the port that this task listens to.
         */

        /* Set priority and stack size attributes */
        pthread_attr_init(&attrs);
        priParam.sched_priority = 1;

        detachState = PTHREAD_CREATE_DETACHED;
        retc = pthread_attr_setdetachstate(&attrs, detachState);
        if (retc != 0) {
            snprintf(sDebug.pcBuf, TX_BUF_SIZE,
                     "netIPAddrHook: pthread_attr_setdetachstate() failed\n");
            mq_send(mqCmdQueueNoBlock, (char*)(&sDebug), 0, 0);
            while (1);
        }

        pthread_attr_setschedparam(&attrs, &priParam);

        retc |= pthread_attr_setstacksize(&attrs, CLOUDTASKSTACKSIZE);
        if (retc != 0) {
            snprintf(sDebug.pcBuf, TX_BUF_SIZE,
                     "netIPAddrHook: pthread_attr_setstacksize() failed\n");
            mq_send(mqCmdQueueNoBlock, (char*)(&sDebug), 0, 0);
            while (1);
        }

        retc = pthread_create(&thread, &attrs, cloudTask, 0);
        if (retc != 0) {
            snprintf(sDebug.pcBuf, TX_BUF_SIZE,
                     "netIPAddrHook: pthread_create() failed\n");
            mq_send(mqCmdQueueNoBlock, (char*)(&sDebug), 0, 0);
            while (1);
        }

        createTask = false;
    }
}

/*
 *  ======== serviceReportHook ========
 *  NDK service report hook
 */
void serviceReport(uint32_t item, uint32_t status, uint32_t report, void *h)
{
    static char *taskName[] = {"Telnet", "", "NAT", "DHCPS", "DHCPC", "DNS"};
    static char *reportStr[] = {"", "Running", "Updated", "Complete", "Fault"};
    static char *statusStr[] =
        {"Disabled", "Waiting", "IPTerm", "Failed","Enabled"};
    tMailboxMsg sDebug;

    snprintf(sDebug.pcBuf, TX_BUF_SIZE, "Service Status: %-9s: %-9s: %-9s: %03d\n",
             taskName[item - 1], statusStr[status], reportStr[report / 256],
             (int)(report & 0xFF));
    mq_send(mqCmdQueueNoBlock, (char*)(&sDebug), 0, 0);
	
	/* report common system issues */
    if ((item == CFGITEM_SERVICE_DHCPCLIENT) &&
            (status == CIS_SRV_STATUS_ENABLED) &&
            (report & NETTOOLS_STAT_FAULT)) {
		snprintf(sDebug.pcBuf, TX_BUF_SIZE,
                     "DHCP Client initialization failed; check your network.\n");
		mq_send(mqCmdQueueNoBlock, (char*)(&sDebug), 0, 0);

        while (1);
    }
}
