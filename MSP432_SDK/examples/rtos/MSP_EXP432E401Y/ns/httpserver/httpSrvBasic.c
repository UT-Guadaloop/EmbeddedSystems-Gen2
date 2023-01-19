/*
 * Copyright (c) 2015-2020, Texas Instruments Incorporated
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
 *    ======== httpSrvBasic.c ========
 */

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* BSD support */
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <ti/net/http/httpserver.h>
#include <ti/net/http/urlhandler.h>
#include <ti/net/slnetif.h>
#include <ti/net/slnetsock.h>

#include <ti/display/Display.h>

#include "urlsimple.h"
#include "urlsimpleput.h"
#include "urlmemzip.h"

extern Display_Handle display;
extern void startSNTP(void);
extern void fdOpenSession();
extern void fdCloseSession();
extern void *TaskSelf();

#define SECURE_PORT             443
#define NUM_URLHANDLERS         3
#define SERVER_BACKLOG_COUNT    2

/*
 * ADD_ROOT_CA should be set to 1 if you want to verify client certificates with
 * a root certificate. A root certificate is not provided with this example. You
 * will also need to add a certificate chain to your client that your chosen
 * root certificate trusts. Our python-based client included with this example
 * does not provide a certificate for verification.
 */
#define ADD_ROOT_CA             0

/*
 * Secure object names.
 * Reflects the "Name" field for a Secure Object in SysConfig
 */
#define ROOT_CA_CERT_FILE     "DummyCA"
#define PRIVATE_KEY_FILE      "DummyKey"
#define TRUSTED_CERT_FILE     "DummyTrustedCert"

/*
 * Structure that is passed to HTTPServer_create. It defines the callbacks
 * used by the server as it parses requests.
 */
URLHandler_Setup handlerTable[] =
{
    {
        NULL,
        URLSimple_create,
        URLSimple_delete,
        URLSimple_process,
        NULL,
        NULL
    },
    {
        NULL,
        NULL,
        NULL,
        URLSimplePut_process,
        NULL,
        NULL
    },
    {
        NULL,
        NULL,
        NULL,
        URLMemzip_process,
        NULL,
        NULL
    }
};

/*
 * ======== serverFxn ========
 *
 * Thread started by netIPAddrHook in httpSrvBasicHooks.c once the NDK has
 * acquired an IP address. This thread initializes and starts a simple HTTP
 * server.
 */
void *serverFxn(void *arg)
{
    HTTPServer_Handle       srv;
    struct                  sockaddr_in addr;
    int                     status = 0;
    uint16_t                port = *(uint16_t *)arg;
    SlNetSockSecAttrib_t    *secAttribs;

    fdOpenSession(TaskSelf());

    Display_printf(display, 0, 0, "*** Initializing HTTP Server ***\n");
    HTTPServer_init();

    /* Create the HTTPServer and pass in your array of handlers */
    if ((srv = HTTPServer_create(handlerTable, NUM_URLHANDLERS, NULL)) == NULL)
    {
        Display_printf(display, 0, 0,
                       "Failed to create HTTPServer> 0x%p\n", srv);
        exit(1);
    }

    /*
     * Create a connection point for the server.
     */
    memset((char *)&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(0);
    addr.sin_port = htons(port);

    if (port == SECURE_PORT)
    {
        startSNTP();

        secAttribs = SlNetSock_secAttribCreate();

        #if ADD_ROOT_CA
            /*
             *  Set the ROOT CA for the example client - this enables/enforces
             *  client authentication
             */
            status |= SlNetSock_secAttribSet(secAttribs,
                                            SLNETSOCK_SEC_ATTRIB_PEER_ROOT_CA,
                                            ROOT_CA_CERT_FILE,
                                            sizeof(ROOT_CA_CERT_FILE));
        #endif
        /* Set the server's personal identification certificate chain */
        status |= SlNetSock_secAttribSet(secAttribs,
                                         SLNETSOCK_SEC_ATTRIB_LOCAL_CERT,
                                         TRUSTED_CERT_FILE,
                                         sizeof(TRUSTED_CERT_FILE));
        /* Set the server's private key */
        status |= SlNetSock_secAttribSet(secAttribs,
                                         SLNETSOCK_SEC_ATTRIB_PRIVATE_KEY,
                                         PRIVATE_KEY_FILE,
                                         sizeof(PRIVATE_KEY_FILE));
        if (status < 0)
        {
            Display_printf(display, 0, 0, "Failed to set security attributes "
                           "- status (%d)\n", status);
            exit(1);
        }

        HTTPServer_enableSecurity(srv, secAttribs, true);
    }

    Display_printf(display, 0, 0, "... Starting HTTP Server on port %d ...\n",
                   port);
    HTTPServer_serveSelect(srv, (struct sockaddr *)&addr, sizeof(addr),
                           SERVER_BACKLOG_COUNT);
    HTTPServer_delete(&srv);

    fdCloseSession(TaskSelf());

    return (NULL);
}
