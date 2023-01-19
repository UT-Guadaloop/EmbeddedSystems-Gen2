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
 *  ======== USBCDCMOUSE.h ========
 */

#ifndef USBCDCMOUSE_H_
#define USBCDCMOUSE_H_

#ifdef __cplusplus
extern "C" {
#endif

#define WAIT_FOREVER    (~(0))

/* Data structure used to specify the current state of USB mouse */
typedef struct {
    char deltaX;    /*! X position offset */
    char deltaY;    /*! Y position offset */
    bool button1;   /*! Button 1 */
    bool button2;   /*! Button 2 */
    bool button3;   /*! Button 3 */
} USBCDCMOUSE_State;
/*!
 *  ======== USBCDCMOUSE_init ========
 *  Function to initialize the USB serial reference module.
 *
 *  Note: This function is not reentrant safe.
 */
extern void USBCDCMOUSE_init(bool usbInternal);

/*!
 *  ======== USBCDCMOUSE_receiveData ========
 *  A blocking function to read from the USB Buffer
 *
 *  Function reads up to size number of bytes from the USB Buffer. The return
 *  value will indicate how many bytes were read from the USB Buffer. If there
 *  is no data in the USB Buffer, this function will block timeout number of
 *  ticks. Note: The WAIT_FOREVER macro can be used to block indefinitely.
 *
 *  @param(pStr)    Pointer to a buffer to which data is written to.
 *
 *  @param(size)    Maximum number of bytes to be read
 *
 *  @param(timeout) Number of tick to block if no data is currently available in
 *                  the USB Buffer.
 *
 *  @return         Returns the number of bytes actually read from the USB
 *                  Buffer.
 */
extern unsigned int USBCDCMOUSE_receiveData(unsigned char *pStr, unsigned int length, unsigned int timeout);

/*!
 *  ======== USBCDCMOUSE_sendData ========
 *  A blocking function that sends a specified amount of data.
 *
 *  Function sends size number of bytes from the buffer pointed by pStr.
 *
 *  @param(pStr)    Pointer to a buffer of data
 *
 *  @param(size)    Number of bytes to be sent
 *
 *  @return         Number of bytes added to the USB Buffer for transmission
 */
extern unsigned int USBCDCMOUSE_sendData(char *pStr, unsigned int length, unsigned int timeout);

/*!
 *  ======== USBCDCMOUSE_setState ========
 *  Function to set the most current mouse state.
 *
 *  Function updates the mouse state by supplying a structure that contains the
 *  most recent X and Y offset values along with the mouse button values.
 *
 *  @params(mouseState) A pointer to a structure that contains the latest state
 *                      values, such as the latest X, Y offsets and button
 *                      states.
 */
extern unsigned int USBCDCMOUSE_setState(USBCDCMOUSE_State *mouseState, unsigned int timeout);

/*!
 *  ======== USBCDCMOUSE_waitForConnect ========
 *  This function blocks while the USB is not connected
 */
extern bool USBCDCMOUSE_waitForConnect(unsigned int timeout);

#ifdef __cplusplus
}
#endif

#endif /* USBCDCMOUSE_H_ */
