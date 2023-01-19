/*
 * Copyright (c) 2015-2019, Texas Instruments Incorporated
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
 *    ======== tcpEchoHooks.c ========
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <pthread.h>

#include <ti/ndk/inc/netmain.h>

#include <ti/ndk/slnetif/slnetifndk.h>
#include <ti/net/slnet.h>
#include <ti/net/slnetif.h>
#include <ti/net/slnetutils.h>

#include <ti/display/Display.h>
#include <ti/drivers/emac/EMACMSP432E4.h>

#define TCPPORT 1000

#define TCPHANDLERSTACK 8192
#define IFPRI  4   /* Ethernet interface priority */

/* Prototypes */
extern void *tcpHandler(void *arg0);

extern Display_Handle display;

/* CA cert of client (clientCaCert.pem) */
uint8_t cliCAPem[] =
"-----BEGIN CERTIFICATE-----\r\n"
"MIICizCCAfQCCQDjeEKNLQFj2jANBgkqhkiG9w0BAQsFADCBiTELMAkGA1UEBhMC\r\n"
"VVMxEzARBgNVBAgMCkNhbGlmb3JuaWExDzANBgNVBAcMBkdvbGV0YTELMAkGA1UE\r\n"
"CgwCVEkxEzARBgNVBAsMCmR1bW15X3Jvb3QxEzARBgNVBAMMCmR1bW15X3Jvb3Qx\r\n"
"HTAbBgkqhkiG9w0BCQEWDnJvb3RAZHVtbXkuY29tMB4XDTE4MDUxMDAwMzY0N1oX\r\n"
"DTM4MDUwNTAwMzY0N1owgYkxCzAJBgNVBAYTAlVTMRMwEQYDVQQIDApDYWxpZm9y\r\n"
"bmlhMQ8wDQYDVQQHDAZHb2xldGExCzAJBgNVBAoMAlRJMRMwEQYDVQQLDApkdW1t\r\n"
"eV9yb290MRMwEQYDVQQDDApkdW1teV9yb290MR0wGwYJKoZIhvcNAQkBFg5yb290\r\n"
"QGR1bW15LmNvbTCBnzANBgkqhkiG9w0BAQEFAAOBjQAwgYkCgYEA51oMh3E5O0rL\r\n"
"74096ErsyhtJQnhscRlIMhmFvb1pa6tPF3OAre+qKWhQTG1o0SXoITiNfp71nSwM\r\n"
"+JO6uC7G5L6GEr6ruvwq7lnEvKCXEU8nBz0H8qWVcZDHlls8FsVsXohOsVa8eyQM\r\n"
"7Nant7BteV82IbLTzfc7OAWcFxIIY1UCAwEAATANBgkqhkiG9w0BAQsFAAOBgQAC\r\n"
"qKus1ZUh2hcQWL+3ASAkb56uKWjo4No5xMlx1nyF652BN/u0kh3QVqh53txXAgv5\r\n"
"QIwHdbPT+YmHqnxtmKXwbDWWuJiuB7A/OYEYmHzsIc5TxboNWXlj/xG3C1XLYDxM\r\n"
"IkHxye6pRJ5ByqlHhAyGRNcN1PkHHdUrL/ywvAxW6A==\r\n"
"-----END CERTIFICATE-----";

uint16_t cliCAPemLen = sizeof(cliCAPem);

/*
 * Full chain with server cert followed by CA cert
 *
 * Note that the CA cert is optional as per RFC 5246, but we just
 * want to show an example of how a certificate chain can be supplied.
 */
uint8_t srvCertPem[] =
/* server cert (serverCert.pem) */
"-----BEGIN CERTIFICATE-----\r\n"
"MIICkTCCAfoCCQCppMM/xH86ITANBgkqhkiG9w0BAQUFADCBiTELMAkGA1UEBhMC\r\n"
"VVMxEzARBgNVBAgMCkNhbGlmb3JuaWExDzANBgNVBAcMBkdvbGV0YTELMAkGA1UE\r\n"
"CgwCVEkxEzARBgNVBAsMCmR1bW15X3Jvb3QxEzARBgNVBAMMCmR1bW15X3Jvb3Qx\r\n"
"HTAbBgkqhkiG9w0BCQEWDnJvb3RAZHVtbXkuY29tMB4XDTE3MTEyNzE4MTE1NFoX\r\n"
"DTI3MTEyNTE4MTE1NFowgY8xCzAJBgNVBAYTAlVTMRMwEQYDVQQIDApDYWxpZm9y\r\n"
"bmlhMQ8wDQYDVQQHDAZHb2xldGExCzAJBgNVBAoMAlRJMRUwEwYDVQQLDAxkdW1t\r\n"
"eV9zZXJ2ZXIxFTATBgNVBAMMDGR1bW15X3NlcnZlcjEfMB0GCSqGSIb3DQEJARYQ\r\n"
"c2VydmVyQGR1bW15LmNvbTCBnzANBgkqhkiG9w0BAQEFAAOBjQAwgYkCgYEAz0EQ\r\n"
"B30L687NYQj7NOMbf9O1gauVQSqc3NLdYhSiGQ8QnUvX28GXrABl63s1N49XPaze\r\n"
"MoC/XX1IZRplVFYnJNHGMt0koe2arWYpWqsUAvNJpZW8X5M3SCdxOW2v3aF/SO6B\r\n"
"1F+CSYdyeaxGwed1KyNXd48BfzvBzLGKA/SG7oUCAwEAATANBgkqhkiG9w0BAQUF\r\n"
"AAOBgQAiZM4W81a8h5iGxjOoNgux3iqhJD8RvcewwXrloIpREbqqoZVGFL7xfKFo\r\n"
"b7U+bTinyRHmjwcYbAzY74g3R+IdB9/2fSdP4sz2CgXcNGewn4FhGOb4G6lG8Eev\r\n"
"I7WJ4t/HQGW7+SnA990ZYL/5l3d21UFTpAmFk9Co+oMtq6ONqg==\r\n"
"-----END CERTIFICATE-----\r\n"
/* CA cert of server (serverCaCert.pem) */
"-----BEGIN CERTIFICATE-----\r\n"
"MIICizCCAfQCCQC9wRaASarXVzANBgkqhkiG9w0BAQUFADCBiTELMAkGA1UEBhMC\r\n"
"VVMxEzARBgNVBAgMCkNhbGlmb3JuaWExDzANBgNVBAcMBkdvbGV0YTELMAkGA1UE\r\n"
"CgwCVEkxEzARBgNVBAsMCmR1bW15X3Jvb3QxEzARBgNVBAMMCmR1bW15X3Jvb3Qx\r\n"
"HTAbBgkqhkiG9w0BCQEWDnJvb3RAZHVtbXkuY29tMB4XDTE3MTEyNzE4MDcwOFoX\r\n"
"DTI3MTEyNTE4MDcwOFowgYkxCzAJBgNVBAYTAlVTMRMwEQYDVQQIDApDYWxpZm9y\r\n"
"bmlhMQ8wDQYDVQQHDAZHb2xldGExCzAJBgNVBAoMAlRJMRMwEQYDVQQLDApkdW1t\r\n"
"eV9yb290MRMwEQYDVQQDDApkdW1teV9yb290MR0wGwYJKoZIhvcNAQkBFg5yb290\r\n"
"QGR1bW15LmNvbTCBnzANBgkqhkiG9w0BAQEFAAOBjQAwgYkCgYEAttRn99pMw3no\r\n"
"PZzoWjwRO1KMVFztsYHvC06Pct753DKtMn4Zc4F3YIfIDzPx80ZOkX2oMj2RHTKI\r\n"
"2hzXOp7qHGC8fnFPebguyESkXWtPLGzmt7vQfCNeLKAKclOtoKK6vs98mqtQjxZP\r\n"
"H3V89cvdV6Lanzw2KhvHSYv+qiPzoc8CAwEAATANBgkqhkiG9w0BAQUFAAOBgQB+\r\n"
"osjP1mDGxcRN6ZtptXI6Ocq6mL7YusoOiBUmcnjqzBruotcPz+ofbSl58J3twEKG\r\n"
"HdGDz1CTFEo6h5pgsaYDa66LjCnkInqfPI5+RcmwZ6sULNfNAPL4V40L+KnyP+hd\r\n"
"U2JLkdWgnZAEsb1S5DZEsFzz+Eq/nWLslFdhbwWgZw==\r\n"
"-----END CERTIFICATE-----";

uint16_t srvCertPemLen = sizeof(srvCertPem);

/* server private key (serverKey.pem) */
uint8_t srvKeyPem[] =
"-----BEGIN RSA PRIVATE KEY-----\r\n"
"MIICXQIBAAKBgQDPQRAHfQvrzs1hCPs04xt/07WBq5VBKpzc0t1iFKIZDxCdS9fb\r\n"
"wZesAGXrezU3j1c9rN4ygL9dfUhlGmVUVick0cYy3SSh7ZqtZilaqxQC80mllbxf\r\n"
"kzdIJ3E5ba/doX9I7oHUX4JJh3J5rEbB53UrI1d3jwF/O8HMsYoD9IbuhQIDAQAB\r\n"
"AoGBALQz9xL7yBy9UFf5ripa+XMYii34GatyuLdMZQ89r3+oWKLb6hD6b70vD29D\r\n"
"To9Dg/BtWTmCNmG8Z/D4lXhR/G93RPUWm6hQ88ol6TkqAPYWDgbUI7dCH4j82h6k\r\n"
"rnKAiCHBpH++KP1wxDw7Zr8BY/XBHLpCvgia0wKcLzZGKvjVAkEA8GpkQ8R8D8XD\r\n"
"RLl1Ujdb4u11zkaZGOmke0O4V6U6npk0EYgvquwyTnKipJGDfWXMflBYV04PG5rH\r\n"
"EkctJoudqwJBANywXniYLffn9UcsSXHrwHAXe3lCjcKCPgkUPqs694xOOz1d1uJq\r\n"
"iqNIESXJK8dMh+dwdR9Sq8dPI34I7F8plI8CQGpJvxYu0eJvPkst6u50Rw1mikSt\r\n"
"9ZWMBgnxAJjPFcF0Xg66NMjOL9d62ukC5C0WSng1sTi36/n6TbSI/y8hXo0CQFhH\r\n"
"lQLYnUrV1yApbxfLHqe4PQQ8w0hToU0wdAE7DVtqz/e0WgkoZVz7ryBWYNTQoGzM\r\n"
"Z42oHF8WITSBjUxj7bECQQDMlPPiBbVsH8SKTnXV+plBRfvhKIbfOdWyDFp1hP4x\r\n"
"VKlTiDGpKjMb7C++wlQlPVGTYKY7mWpC7b2jLZIIfDDp\r\n"
"-----END RSA PRIVATE KEY-----";

uint16_t srvKeyPemLen = sizeof(srvKeyPem);

/*
 *  ======== netIPAddrHook ========
 *  user defined network IP address hook
 */
void netIPAddrHook(uint32_t IPAddr, unsigned int IfIdx, unsigned int fAdd)
{
    pthread_t           thread;
    pthread_attr_t      attrs;
    struct sched_param  priParam;
    int                 retc;
    int                 detachState;
    uint32_t            hostByteAddr;
    static uint16_t     arg0 = TCPPORT;
    static bool         createTask = true;
    int32_t             status = 0;

    if (fAdd) {
        Display_printf(display, 0, 0, "Network Added: ");
    }
    else {
        Display_printf(display, 0, 0, "Network Removed: ");
    }

    /* print the IP address that was added/removed */
    hostByteAddr = NDK_ntohl(IPAddr);
    Display_printf(display, 0, 0, "If-%d:%d.%d.%d.%d\n", IfIdx,
            (uint8_t)(hostByteAddr>>24)&0xFF, (uint8_t)(hostByteAddr>>16)&0xFF,
            (uint8_t)(hostByteAddr>>8)&0xFF, (uint8_t)hostByteAddr&0xFF);

    /* initialize SlNet interface(s) */
    status = ti_net_SlNet_initConfig();
    if (status < 0)
    {
        Display_printf(display, 0, 0, "Failed to initialize SlNet interface(s)"
                       "- status (%d)\n", status);
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
            Display_printf(display, 0, 0,
                    "netIPAddrHook: pthread_attr_setdetachstate() failed\n");
            while (1);
        }

        pthread_attr_setschedparam(&attrs, &priParam);

        retc |= pthread_attr_setstacksize(&attrs, TCPHANDLERSTACK);
        if (retc != 0) {
            Display_printf(display, 0, 0,
                    "netIPAddrHook: pthread_attr_setstacksize() failed\n");
            while (1);
        }

        retc = pthread_create(&thread, &attrs, tcpHandler, (void *)&arg0);
        if (retc != 0) {
            Display_printf(display, 0, 0,
                    "netIPAddrHook: pthread_create() failed\n");
            while (1);
        }

        createTask = false;
    }
}

/*
 *  ======== serviceReport ========
 *  NDK service report.  Initially, just reports common system issues.
 */
void serviceReport(uint32_t item, uint32_t status, uint32_t report, void *h)
{
    static char *taskName[] = {"Telnet", "", "NAT", "DHCPS", "DHCPC", "DNS"};
    static char *reportStr[] = {"", "Running", "Updated", "Complete", "Fault"};
    static char *statusStr[] =
        {"Disabled", "Waiting", "IPTerm", "Failed","Enabled"};

    Display_printf(display, 0, 0, "Service Status: %-9s: %-9s: %-9s: %03d\n",
            taskName[item - 1], statusStr[status], reportStr[report / 256],
            report & 0xFF);

    /* report common system issues */
    if ((item == CFGITEM_SERVICE_DHCPCLIENT) &&
            (status == CIS_SRV_STATUS_ENABLED) &&
            (report & NETTOOLS_STAT_FAULT)) {
        Display_printf(display, 0, 0,
                "DHCP Client initialization failed; check your network.\n");

        while (1);
    }
}
