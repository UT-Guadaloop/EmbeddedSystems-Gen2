/*
 * Copyright (c) 2014-2018, Texas Instruments Incorporated
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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <ti/net/http/httpserver.h>
#include <ti/net/http/http.h>

#include "urlsimpleput.h"

/*
 *  ======== Ssock_recvall ========
 */
static ssize_t Ssock_recvall(int ssock, void * buf, size_t len, int flags)
{
    ssize_t nbytes = 0;

    while (len > 0) {
        nbytes = recv(ssock, buf, len, flags);
        if (nbytes > 0) {
            len -= nbytes;
            buf = (uint8_t *)buf + nbytes;
        }
        else {
            break;
        }
    }

    return (nbytes);
}

/*
 *  ======== URLSimplePut_process ========
 *
 *  Processes a request.
 */
int URLSimplePut_process(URLHandler_Handle urlHandler, int method,
                      const char * url, const char * urlArgs,
                      int contentLength, int ssock)
{
    int status = URLHandler_ENOTHANDLED;
    char *body = NULL;                  /* Body of HTTP response in process */
    int returnCode;                     /* HTTP response status code */
    char *contentType = "text/plain";   /* default, but can be overridden */

    /* Determine the request type */
    if (method == URLHandler_PUT)
    {
        returnCode = HTTP_SC_OK;
        body = "URLHandler #2 : /put This is the body of the resource you requested.";
        status = URLHandler_EHANDLED;
    }

    if (status != URLHandler_ENOTHANDLED)
    {
        if (contentLength > 0) {
            char *buf;

            buf = malloc(contentLength);
            if (buf == NULL)
            {
                /* Signals to the server that it should terminate this one session */
                status = URLHandler_EERRORHANDLED;
            }
            else
            {
                /* This is done to flush the socket */
                (void) Ssock_recvall(ssock, buf, contentLength, 0);
                free(buf);
            }
        }

        HTTPServer_sendSimpleResponse(ssock, returnCode, contentType,
                body ? strlen(body) : 0, body ? body : NULL);

    }

    return (status);
}
