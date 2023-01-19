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
 *  ======== USBSENSORD.h ========
 */

#ifndef USBSENSORD_H_
#define USBSENSORD_H_

#include <ti/usblib/msp432e4/usb-ids.h>
#include <ti/usblib/msp432e4/usblib.h>
#include <ti/usblib/msp432e4/usbhid.h>
#include <ti/usblib/msp432e4/device/usbdevice.h>
#include <ti/usblib/msp432e4/device/usbdhid.h>
#include <ti/usblib/msp432e4/device/usbdhidsensor.h>

#ifdef __cplusplus
extern "C" {
#endif


#define WAIT_FOREVER    (~(0))


/*!
 *  ======== USBSENSORD_init ========
 *  Function to initialize the USB sensor reference module.
 *
 *  Note: This function is not reentrant safe.
 */
extern void USBSENSORD_init(bool usbInternal);

/*!
 *  ======== USBSENSORD_sendData ========
 *  A blocking function that sends temperature
 *  related data to the host.
 *
 *  @param(*tempValueArray)     Pointer to the array storing all temperature data
 *
 *  @param(timeout) Number of ticks to wait for the data to be sent to the
 *                  host
 *
 *  @return         0
 */
extern int USBSENSORD_sendData(tSensorTemperatureReport *tempValueArray, unsigned int timeout);



#ifdef __cplusplus
}
#endif

#endif /* USBSENSORD_H_ */
