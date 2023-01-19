/*
 * Copyright (c) 2020, Texas Instruments Incorporated
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
 *  ======== urlmemzip.c ========
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <ti/net/http/httpserver.h>
#include <ti/net/http/http.h>

#include "memzip.h"

const char *CONTENT_TYPE_APPLET  = "application/octet-stream ";
const char *CONTENT_TYPE_AU      = "audio/au ";
const char *CONTENT_TYPE_CSS     = "text/css ";
const char *CONTENT_TYPE_DOC     = "application/msword ";
const char *CONTENT_TYPE_GIF     = "image/gif ";
const char *CONTENT_TYPE_HTML    = "text/html ";
const char *CONTENT_TYPE_JPG     = "image/jpeg ";
const char *CONTENT_TYPE_MPEG    = "video/mpeg ";
const char *CONTENT_TYPE_PDF     = "application/pdf ";
const char *CONTENT_TYPE_WAV     = "audio/wav ";
const char *CONTENT_TYPE_ZIP     = "application/zip ";
const char *CONTENT_TYPE_PLAIN   = "text/plain ";

/*
 * Fills in the supplied string with a content type string that
 * matches the supplied filename.
 */
static const char * getContentType(const char * fileName)
{
   const char * ext = fileName;

   /*  ext will point at the ".xxx" part of filename */
   /*  If fileName does not have an extension it will point to terminating char */
   while (*ext && *ext != '.') {
       ext++;
   }

   if      (!strcmp(".au",    ext)) return (CONTENT_TYPE_AU);
   else if (!strcmp(".class", ext)) return (CONTENT_TYPE_APPLET);
   else if (!strcmp(".css",   ext)) return (CONTENT_TYPE_CSS);
   else if (!strcmp(".doc",   ext)) return (CONTENT_TYPE_DOC);
   else if (!strcmp(".gif",   ext)) return (CONTENT_TYPE_GIF);
   else if (!strcmp(".htm",   ext)) return (CONTENT_TYPE_HTML);
   else if (!strcmp(".html",  ext)) return (CONTENT_TYPE_HTML);
   else if (!strcmp(".jpg",   ext)) return (CONTENT_TYPE_JPG);
   else if (!strcmp(".mpg",   ext)) return (CONTENT_TYPE_MPEG);
   else if (!strcmp(".mpeg",  ext)) return (CONTENT_TYPE_MPEG);
   else if (!strcmp(".pdf",   ext)) return (CONTENT_TYPE_PDF);
   else if (!strcmp(".wav",   ext)) return (CONTENT_TYPE_WAV);
   else if (!strcmp(".zip",   ext)) return (CONTENT_TYPE_ZIP);
   else if (!strcmp(".txt",   ext)) return (CONTENT_TYPE_PLAIN);
   else return (CONTENT_TYPE_APPLET);
}

/*
 *  ======== recvall ========
 *
 *  Flush the socket buffer.
 */
static ssize_t recvall(int ssock, void * buf, size_t len, int flags)
{
    ssize_t nbytes = 0;

    while (len > 0)
    {
        nbytes = recv(ssock, buf, len, flags);
        if (nbytes > 0)
        {
            len -= nbytes;
            buf = (uint8_t *)buf + nbytes;
        }
        else
        {
            break;
        }
    }

    return (nbytes);
}

/*
 *  ======== URLMemzip_process ========
 *
 *  Processes a request.
 */
int URLMemzip_process(URLHandler_Handle urlHandler, int method,
                      const char * url, const char * urlArgs,
                      int contentLength, int s)
{
    int status          = URLHandler_ENOTHANDLED;
    char *body          = NULL;         /* Body of HTTP response in process */
    const char *contentType   = "text/plain"; /* default, but can be overridden */
    int returnCode;
    size_t len;
    char * pData;

    /* Make default page "index.html" */
    if ((method == URLHandler_GET) && (strcmp(url, "/") == 0))
    {
        url = "index.html";
    }

    /* This URL handler only supports the GET method */
    if ((method == URLHandler_GET) &&
            (memzip_locate(url, (void **)&pData, &len) == MZ_OK))
    {
        body = pData;

        returnCode = HTTP_SC_OK;
        contentType = getContentType(url);
        status = URLHandler_EHANDLED;
    }

    if (status != URLHandler_ENOTHANDLED)
    {
        /* we're handling this URL request.  Read any data left in the socket */
        if (contentLength > 0)
        {
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
                (void) recvall(s, buf, contentLength, 0);
                free(buf);
            }
        }

        HTTPServer_sendSimpleResponse(s, returnCode, contentType,
                body ? len : 0, body ? body : NULL);
    }

    return (status);
}
