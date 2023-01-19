/*
 * Copyright (c) 2015, Texas Instruments Incorporated
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
 *  ======== USBMSCH.h ========
 */

#ifndef USBMSCH_H_
#define USBMSCH_H_

#ifdef __cplusplus
extern "C" {
#endif

#define WAIT_FOREVER    (~(0))



/*!
 *  ======== USBMSCH_init ========
 *  Function to initialize the USB MSC reference module.
 *
 *  Note: This function is not reentrant safe.
 *
 *  @param(usbInternal)     board indicator for executing the
 *                          the correct API calls - host, device
 *                          or high speed
 *
 *  @return
 *
 */
extern void USBMSCH_init(bool usbInternal);

/*!
 *  ======== USBMSCH_ReadLine ========
 *  This function reads a line of text from the UART console.  The USB host main
 *  function is called throughout this process to keep USB alive and well.
 *
 *  @param
 *
 *  @return
 *
 */
extern void USBMSCH_ReadLine(void);

/*!
 *  ======== USBKBH_waitForConnect ========
 *  This function blocks while the USB is not connected
 */
extern bool USBMSCH_waitForConnect(unsigned int timeout);

#ifdef __cplusplus
}
#endif

#endif /* USBMSCH_H_ */
