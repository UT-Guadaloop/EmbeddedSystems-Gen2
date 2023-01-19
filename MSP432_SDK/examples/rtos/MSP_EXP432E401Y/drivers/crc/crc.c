/*
 * Copyright (c) 2019, Texas Instruments Incorporated
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
 *  ======== crc.c ========
 */
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

/* Driver Header files */
#include <ti/drivers/UART.h>
#include <ti/drivers/CRC.h>

/* Driver configuration */
#include "ti_drivers_config.h"

/* Expected CRC for CRC_32_IEEE with full endianness reversal */
static const uint32_t expectedCrc = 0x4C4B4461;

/* Example test vector */
static const size_t srcSize = 32;
static const uint8_t src [] = {
    0x12, 0x34, 0x56, 0x78, 0x12, 0x34, 0x56, 0x78,
    0x12, 0x34, 0x56, 0x78, 0x12, 0x34, 0x56, 0x78,
    0x12, 0x34, 0x56, 0x78, 0x12, 0x34, 0x56, 0x78,
    0x12, 0x34, 0x56, 0x78, 0x12, 0x34, 0x56, 0x78
};

static const char preOpMessage[] = "Performing CRC_32_IEEE with endianness reversal... ";
static const char postOpMessage[] = "Check successful.\r\n";

/* ======== mainThread ======== */
void *mainThread(void *arg0)
{
    int_fast16_t status;
    uint32_t result;

    CRC_Handle handle;
    CRC_Params params;

    UART_Handle uart;
    UART_Params uartParams;

    /* Initialize the drivers */
    CRC_init();
    UART_init();

    /* Create a UART with data processing off. */
    UART_Params_init(&uartParams);
    uartParams.writeDataMode = UART_DATA_BINARY;
    uartParams.readDataMode = UART_DATA_BINARY;
    uartParams.readReturnMode = UART_RETURN_FULL;
    uartParams.readEcho = UART_ECHO_OFF;
    uartParams.baudRate = 115200;

    uart = UART_open(CONFIG_UART_0, &uartParams);

    if (uart == NULL) {
        /* UART_open() failed */
        while (1);
    }

    UART_write(uart, preOpMessage, sizeof(preOpMessage));

    /* Set data processing options, including endianness control */
    CRC_Params_init(&params);
    params.byteSwapInput = CRC_BYTESWAP_BYTES_AND_HALF_WORDS;
    params.returnBehavior = CRC_RETURN_BEHAVIOR_BLOCKING;
    params.polynomial = CRC_POLYNOMIAL_CRC_32_IEEE;
    params.dataSize = CRC_DATA_SIZE_32BIT;
    params.seed = 0xFFFFFFFF;

    /* Open the driver using the settings above */
    handle = CRC_open(CONFIG_CRC_0, &params);
    if (handle == NULL)
    {
        /* If the handle is already open, execution will stop here */
        while(1);
    }

    /* Calculate the CRC of all 32 bytes in the source array */
    status = CRC_calculateFull(handle, src, srcSize, &result);
    if (status != CRC_STATUS_SUCCESS || result != expectedCrc)
    {
        /* If the CRC engine is busy or if an error occurs execution will stop here */
        while(1);
    }

    /* This is another way of achieving the same result; for instance
     * if data is arriving in blocks (over UART) this method may be preferable.
     * CRC_reset() may be used to clear an ongoing partial if the result is no
     * longer needed. CRC_finalise() also resets the state. */
    status = CRC_addData(handle, src, srcSize/2);
    if (status != CRC_STATUS_SUCCESS)
    {
        /* If the CRC engine is busy or if an error occurs execution will stop here */
        while(1);
    }

    status = CRC_addData(handle, &src[srcSize/2], srcSize/2);

    /* Extract the result from the internal state */
    CRC_finalize(handle, &result);

    if (status != CRC_STATUS_SUCCESS || result != expectedCrc)
    {
        /* If the CRC engine is busy or if an error occurs execution will stop here */
        while(1);
    }

    UART_write(uart, postOpMessage, sizeof(postOpMessage));

    /* Close the driver to allow other users to access this driver instance */
    CRC_close(handle);
    UART_close(uart);

    return NULL;
}
