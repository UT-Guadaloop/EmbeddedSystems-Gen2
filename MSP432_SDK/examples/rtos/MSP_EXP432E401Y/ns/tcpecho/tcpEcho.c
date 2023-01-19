/*
 * Copyright (c) 2014-2019, Texas Instruments Incorporated
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
 *    ======== tcpEcho.c ========
 *    Contains BSD sockets code.
 */

#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include <pthread.h>
/* BSD support */
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>

#include <ti/net/slnetutils.h>

#include <ti/display/Display.h>

#define TCPPACKETSIZE 256
#define NUMTCPWORKERS 3
#define MAXPORTLEN    6

extern Display_Handle display;

extern void fdOpenSession();
extern void fdCloseSession();
extern void *TaskSelf();

/*
 *  ======== tcpWorker ========
 *  Task to handle TCP connection. Can be multiple Tasks running
 *  this function.
 */
void *tcpWorker(void *arg0)
{
    int  clientfd = *(int *)arg0;
    int  bytesRcvd;
    int  bytesSent;
    char buffer[TCPPACKETSIZE];

    fdOpenSession(TaskSelf());

    Display_printf(display, 0, 0, "tcpWorker: start clientfd = 0x%x\n",
            clientfd);

    while ((bytesRcvd = recv(clientfd, buffer, TCPPACKETSIZE, 0)) > 0) {
        bytesSent = send(clientfd, buffer, bytesRcvd, 0);
        if (bytesSent < 0 || bytesSent != bytesRcvd) {
            Display_printf(display, 0, 0, "send failed.\n");
            break;
        }
    }
    Display_printf(display, 0, 0, "tcpWorker stop clientfd = 0x%x\n", clientfd);

    close(clientfd);

    fdCloseSession(TaskSelf());

    return (NULL);
}

/*
 *  ======== tcpHandler ========
 *  Creates new Task to handle new TCP connections.
 */
void *tcpHandler(void *arg0)
{
    pthread_t          thread;
    pthread_attr_t     attrs;
    struct sched_param priParam;
    int                retc;
    int                detachState;
    int                status;
    int                server = -1;
    struct addrinfo    hints;
    struct addrinfo    *res, *p;
    struct sockaddr_in clientAddr;
    int                optval;
    static int         clientfd;
    int                optlen = sizeof(optval);
    socklen_t          addrlen = sizeof(clientAddr);
    char               portNumber[MAXPORTLEN];

    fdOpenSession(TaskSelf());

    Display_printf(display, 0, 0, "TCP Echo example started\n");

    sprintf(portNumber, "%d", *(uint16_t *)arg0);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags    = AI_PASSIVE;

    /* Obtain addresses suitable for binding to */
    status = getaddrinfo(NULL, portNumber, &hints, &res);
    if (status != 0) {
        Display_printf(display, 0, 0, "tcpHandler: getaddrinfo() failed: %s\n",
            gai_strerror(status));
        goto shutdown;
    }

    for (p = res; p != NULL; p = p->ai_next) {
        server = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (server == -1) {
            continue;
        }

        status = bind(server, p->ai_addr, p->ai_addrlen);
        if (status != -1) {
            break;
        }

        close(server);
    }

    if (server == -1) {
        Display_printf(display, 0, 0, "tcpHandler: failed to open socket\n");
        goto shutdown;
    } else if (p == NULL) {
        Display_printf(display, 0, 0, "tcpHandler: could not bind to socket: %d\n", status);
        goto shutdown;
    } else {
        freeaddrinfo(res);
        res = NULL;
    }

    status = listen(server, NUMTCPWORKERS);
    if (status == -1) {
        Display_printf(display, 0, 0, "tcpHandler: listen failed\n");
        goto shutdown;
    }

    optval = 1;
    status = setsockopt(server, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen);
    if (status == -1) {
        Display_printf(display, 0, 0, "tcpHandler: setsockopt failed\n");
        goto shutdown;
    }

    while ((clientfd =
            accept(server, (struct sockaddr *)&clientAddr, &addrlen)) != -1) {

        Display_printf(display, 0, 0,
                "tcpHandler: Creating thread clientfd = %x\n", clientfd);

        /* Set priority and stack size attributes */
        pthread_attr_init(&attrs);
        priParam.sched_priority = 3;

        detachState = PTHREAD_CREATE_DETACHED;
        retc = pthread_attr_setdetachstate(&attrs, detachState);
        if (retc != 0) {
            Display_printf(display, 0, 0,
                    "tcpHandler: pthread_attr_setdetachstate() failed");
            while (1);
        }

        pthread_attr_setschedparam(&attrs, &priParam);

        retc |= pthread_attr_setstacksize(&attrs, 2048);
        if (retc != 0) {
            Display_printf(display, 0, 0,
                    "tcpHandler: pthread_attr_setstacksize() failed");
            while (1);
        }

        retc = pthread_create(&thread, &attrs, tcpWorker, (void *)&clientfd);
        if (retc != 0) {
            Display_printf(display, 0, 0,
                    "tcpHandler: pthread_create() failed");
            while (1);
        }

        /* addrlen is a value-result param, must reset for next accept call */
        addrlen = sizeof(clientAddr);
    }

    Display_printf(display, 0, 0, "tcpHandler: accept failed.\n");

shutdown:
    if (res) {
        freeaddrinfo(res);
        res = NULL;
    }
    if (server != -1) {
        close(server);
    }

    fdCloseSession(TaskSelf());

    return (NULL);
}
