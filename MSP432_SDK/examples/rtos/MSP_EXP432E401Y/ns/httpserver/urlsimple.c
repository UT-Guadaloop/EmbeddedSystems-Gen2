/*
 * Copyright (c) 2014-2020, Texas Instruments Incorporated
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
 *  ======== urlsimple.c ========
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <ti/net/http/httpserver.h>
#include <ti/net/http/http.h>
#include <ti/net/http/logging.h>

#include "urlsimple.h"

/* Maximum length of a value in the database */
#define MAX_DB_ENTRY_LEN    (64)

/* Maximum number of key-value pairs the server can store */
#define DB_SIZE             (10)

#define ERR_BAD_INPUT       (-1)
#define ERR_DB_FULL         (-2)

/* Data type of this example server's elements */
typedef struct ServerStorage {
    char * key;
    char * value;
} ServerStorage;

/* Storage for the server */
static ServerStorage database[DB_SIZE];

int serverPost(const char * key, const char * value);
char * serverGet(const char * key);

/*
 *  ======== Ssock_recvall ========
 *
 *  Flush the socket buffer.
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
 *  ======== URLSimple_create ========
 *
 *  Typically, this function is used to allocate any structures needed by a
 *  URLHandler while it is processing requests. Here, we are simply using it to
 *  ensure that URLSimple_delete is called, which will free any memory allocated
 *  during this URLHandler's processing of requests. The HTTPServer does not
 *  call a URLHandler's URLHandler_DeleteFxn unless the URLHandler has a
 *  corresponding URLHandler_CreateFxn that returns a non-null entity.
 */
URLHandler_Handle URLSimple_create(void * params, URLHandler_Session session)
{
    return ((URLHandler_Handle)1);
}

/*
 *  ======== URLSimple_delete ========
 *
 *  Free any memory allocated with this URLHandler's creation, as well as
 *  perform any necessary clean up at the end of this URLHandler's processing
 *  session.
 */
void URLSimple_delete(URLHandler_Handle * u)
{
    int index = 0;
    for (; index < DB_SIZE; index++)
    {
        if (database[index].key != NULL)
        {
            free(database[index].key);
            database[index].key = NULL;
        }

        if (database[index].value != NULL)
        {
            free(database[index].value);
            database[index].value = NULL;
        }
    }
}

/*
 *  ======== URLSimple_process ========
 *
 *  Processes a request.
 */
int URLSimple_process(URLHandler_Handle urlHandler, int method,
                      const char * url, const char * urlArgs,
                      int contentLength, int ssock)
{
    int status          = URLHandler_ENOTHANDLED;
    char *body          = NULL;         /* Body of HTTP response in process */
    char *contentType   = "text/plain"; /* default, but can be overridden */
    char *retString     = NULL;         /* String retrieved from server */
    char *inputKey      = NULL;         /* Name of value to store in server */
    char *inputValue    = NULL;         /* Value to store in server */
    char *recvBuf       = NULL;         /* Body of received HTTP request */
    char argsToParse[MAX_DB_ENTRY_LEN];
    int returnCode;                     /* HTTP response status code */
    int retc;                           /* Error checking for internal funcs */

    /* Determine the corresponding URL on the server */
    if (strcmp(url, "/home.html") == 0)
    {
        if (method == URLHandler_GET)
        {
            body = "/get 'home.html': This is the resource requested.";
            returnCode = HTTP_SC_OK;
            status = URLHandler_EHANDLED;
        }
    }
    else if (strcmp(url, "/login.html") == 0)
    {
        if (method == URLHandler_GET)
        {
            body = "/get 'login.html': This is the login page.";
            returnCode = HTTP_SC_OK;
            status = URLHandler_EHANDLED;
        }
    }
    else if (strcmp(url, "/db.html") == 0)
    {
        if (method == URLHandler_GET)
        {
            /* Parse query string */
            strncpy(argsToParse, urlArgs, strlen(urlArgs) + 1);
            /* Ensure that only one uri arg was passed in the request */
            if (strchr(argsToParse, '=') == strrchr(argsToParse, '='))
            {
                inputKey = strtok(argsToParse, "=");
                retString = serverGet(inputKey);
                if (retString)
                {
                    body = retString;
                }
                else
                {
                    body = "String not found.";
                }
                returnCode = HTTP_SC_OK;
                status = URLHandler_EHANDLED;
            }
            else
            {
                body = "This handler does not support multiple parameter requests.";
                returnCode = HTTP_SC_BAD_REQUEST;
                status = URLHandler_EERRORHANDLED;
            }
        }
        else if (method == URLHandler_POST)
        {
            /* Retrieve the body of the HTTP message from the connected socket */
            recvBuf = calloc(1, contentLength + 1);
            if (recvBuf == NULL)
            {
                /* No memory available with which to store body of POST request */
                body = "The server cannot handle any further POST requests "
                       "without being restarted.";
                status = URLHandler_EERRORHANDLED;
                returnCode = HTTP_SC_SERVICE_UNAVAILABLE;
                /* Don't attempt to flush the socket, since we're out of memory */
                contentLength = 0;
            }
            else
            {
                retc = recv(ssock, recvBuf, contentLength + 1, 0);
                if (retc < 0)
                {
                    body = "No key-value pair found in body of POST request";
                    returnCode = HTTP_SC_BAD_REQUEST;
                    status = URLHandler_EERRORHANDLED;
                }
                else
                {
                    inputKey = strtok(recvBuf, "=");
                    inputValue = strtok(NULL, "=");
                    retc = serverPost(inputKey, inputValue);

                    /* Don't flush the socket afterwards */
                    contentLength = 0;

                    if (retc == ERR_BAD_INPUT)
                    {
                        body = "Failed to POST. The data string is invalid.";
                        returnCode = HTTP_SC_BAD_REQUEST;
                        status = URLHandler_EERRORHANDLED;
                    }
                    else if (retc == ERR_DB_FULL)
                    {
                        body = "Database is full";
                        returnCode = HTTP_SC_SERVICE_UNAVAILABLE;
                        status = URLHandler_EERRORHANDLED;
                    }
                    else
                    {
                        body = "Data string successfully posted to server.";
                        returnCode = HTTP_SC_OK;
                        status = URLHandler_EHANDLED;
                    }
                }
                free(recvBuf);
                recvBuf = NULL;
            }
        }
        else
        {
            body = "This method is not supported at this URL.";
            returnCode = HTTP_SC_METHOD_NOT_ALLOWED;
            /* Close the connection */
            status = URLHandler_EERRORHANDLED;
        }
    }
    else if(method == URLHandler_PATCH)
    {
        body = "PATCH is not handled by any handlers on this server.";
        returnCode = HTTP_SC_METHOD_NOT_ALLOWED;
        status = URLHandler_ENOTHANDLED;
    }
    else if(method == URLHandler_DELETE)
    {
        body = "DELETE is not handled by any handlers on this server.";
        returnCode = HTTP_SC_METHOD_NOT_ALLOWED;
        status = URLHandler_EERRORHANDLED;
    }

    if (status != URLHandler_ENOTHANDLED)
    {
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
                (void) Ssock_recvall(ssock, buf, contentLength, 0);
                free(buf);
                buf = NULL;
            }
        }

        HTTPServer_sendSimpleResponse(ssock, returnCode, contentType,
                body ? strlen(body) : 0, body ? body : NULL);
    }

    return (status);
}

/*
 *  ======== serverPost ========
 *
 *  Send a key-value pair to the database for storage.
 */
int serverPost(const char * key, const char * value)
{
    if ((key == NULL) || (value == NULL) || (strlen(key) > MAX_DB_ENTRY_LEN))
    {
        return (ERR_BAD_INPUT);
    }

    int index = 0;
    for(; index < DB_SIZE; index++)
    {
        if (database[index].key == NULL)
        {
            database[index].key = calloc(1, strlen(key) + 1);
            if (database[index].key == NULL)
            {
                return (ERR_DB_FULL);
            }

            database[index].value = calloc(1, strlen(value) + 1);
            if (database[index].value == NULL)
            {
                return (ERR_DB_FULL);
            }

            strncpy(database[index].key, key, strlen(key));
            strncpy(database[index].value, value, strlen(value));

            return (0);
        }
    }
    /* Database is full */
    return (ERR_DB_FULL);
}

/*
 *  ======== serverGet ========
 *
 *  Obtain a value from the server given a key.
 */
char * serverGet(const char * key)
{
    if (key == NULL)
    {
        return (NULL);
    }

    int index = 0;
    for (; index < DB_SIZE; index++)
    {
        char *serverData = database[index].key;
        if (strncmp(serverData, key, MAX_DB_ENTRY_LEN) == 0)
        {
            return (database[index].value);
        }
    }
    /* The key is not present in the database */
    return (NULL);
}
