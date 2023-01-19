/*
 * Copyright (c) 2013-2018, Texas Instruments Incorporated
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

#ifndef __DRIVERS_KENTEC_H__
#define __DRIVERS_KENTEC_H__

#ifdef __cplusplus
extern "C"
{
#endif

extern void Kentec_Init(uint32_t ui32SysClock);

/*
 * This driver operates in four different screen orientations.  They are:
 *
 * * Portrait - The screen is taller than it is wide, and the flex connector is
 *              on the left of the display.  This is selected by defining
 *              PORTRAIT.
 *
 * * Landscape - The screen is wider than it is tall, and the flex connector is
 *               on the bottom of the display.  This is selected by defining
 *               LANDSCAPE.
 *
 * * Portrait flip - The screen is taller than it is wide, and the flex
 *                   connector is on the right of the display.  This is
 *                   selected by defining PORTRAIT_FLIP.
 *
 * * Landscape flip - The screen is wider than it is tall, and the flex
 *                    connector is on the top of the display.  This is
 *                    selected by defining LANDSCAPE_FLIP.
 *
 * These can also be imagined in terms of screen rotation; if portrait mode is
 * 0 degrees of screen rotation, landscape is 90 degrees of counter-clockwise
 * rotation, portrait flip is 180 degrees of rotation, and landscape flip is
 * 270 degress of counter-clockwise rotation.
 *
 * If no screen orientation is selected, "landscape flip" mode will be used.
 *
 */
#if ! defined(PORTRAIT) && ! defined(PORTRAIT_FLIP) && \
    ! defined(LANDSCAPE) && ! defined(LANDSCAPE_FLIP)
#define LANDSCAPE
#endif

/*
 * Various definitions controlling coordinate space mapping and drawing
 * direction in the four supported orientations.
 */
#ifdef PORTRAIT
#define HORIZ_DIRECTION 0x28
#define VERT_DIRECTION 0x20
#define MAPPED_X(x, y) (319 - (y))
#define MAPPED_Y(x, y) (x)
#endif
#ifdef LANDSCAPE
#define HORIZ_DIRECTION 0x00
#define VERT_DIRECTION  0x08
#define MAPPED_X(x, y) (319 - (x))
#define MAPPED_Y(x, y) (239 - (y))
#endif
#ifdef PORTRAIT_FLIP
#define HORIZ_DIRECTION 0x18
#define VERT_DIRECTION 0x10
#define MAPPED_X(x, y) (y)
#define MAPPED_Y(x, y) (239 - (x))
#endif
#ifdef LANDSCAPE_FLIP
#define HORIZ_DIRECTION 0x30
#define VERT_DIRECTION  0x38
#define MAPPED_X(x, y) (x)
#define MAPPED_Y(x, y) (y)
#endif

/* Various internal SD2119 registers name labels */
#define SSD2119_DEVICE_CODE_READ_REG    0x00
#define SSD2119_OSC_START_REG           0x00
#define SSD2119_OUTPUT_CTRL_REG         0x01
#define SSD2119_LCD_DRIVE_AC_CTRL_REG   0x02
#define SSD2119_PWR_CTRL_1_REG          0x03
#define SSD2119_DISPLAY_CTRL_REG        0x07
#define SSD2119_FRAME_CYCLE_CTRL_REG    0x0b
#define SSD2119_PWR_CTRL_2_REG          0x0c
#define SSD2119_PWR_CTRL_3_REG          0x0d
#define SSD2119_PWR_CTRL_4_REG          0x0e
#define SSD2119_GATE_SCAN_START_REG     0x0f
#define SSD2119_SLEEP_MODE_1_REG        0x10
#define SSD2119_ENTRY_MODE_REG          0x11
#define SSD2119_SLEEP_MODE_2_REG        0x12
#define SSD2119_GEN_IF_CTRL_REG         0x15
#define SSD2119_PWR_CTRL_5_REG          0x1e
#define SSD2119_RAM_DATA_REG            0x22
#define SSD2119_FRAME_FREQ_REG          0x25
#define SSD2119_ANALOG_SET_REG          0x26
#define SSD2119_VCOM_OTP_1_REG          0x28
#define SSD2119_VCOM_OTP_2_REG          0x29
#define SSD2119_GAMMA_CTRL_1_REG        0x30
#define SSD2119_GAMMA_CTRL_2_REG        0x31
#define SSD2119_GAMMA_CTRL_3_REG        0x32
#define SSD2119_GAMMA_CTRL_4_REG        0x33
#define SSD2119_GAMMA_CTRL_5_REG        0x34
#define SSD2119_GAMMA_CTRL_6_REG        0x35
#define SSD2119_GAMMA_CTRL_7_REG        0x36
#define SSD2119_GAMMA_CTRL_8_REG        0x37
#define SSD2119_GAMMA_CTRL_9_REG        0x3a
#define SSD2119_GAMMA_CTRL_10_REG       0x3b
#define SSD2119_V_RAM_POS_REG           0x44
#define SSD2119_H_RAM_START_REG         0x45
#define SSD2119_H_RAM_END_REG           0x46
#define SSD2119_X_RAM_ADDR_REG          0x4e
#define SSD2119_Y_RAM_ADDR_REG          0x4f

#define GRLIB_DRIVER_FLAG_NEW_IMAGE     0x40000000

#define ENTRY_MODE_DEFAULT      0x6830

/* The dimensions of the LCD panel. */
#define LCD_VERTICAL_MAX 240
#define LCD_HORIZONTAL_MAX 320

#ifdef __cplusplus
}
#endif

#endif //__DRIVERS_KENTEC_H__
