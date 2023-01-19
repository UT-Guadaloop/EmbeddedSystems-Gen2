/*
 * Copyright (c) 2018, Texas Instruments Incorporated
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
 * THIS SOFTWARE IS POVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
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

#include <stdbool.h>
#include <stdint.h>
#include "kentec.h"
#include <ti/devices/msp432e4/driverlib/gpio.h>
#include <ti/devices/msp432e4/driverlib/pin_map.h>
#include <ti/devices/msp432e4/driverlib/sysctl.h>
#include <ti/devices/msp432e4/driverlib/lcd.h>
#include <ti/devices/msp432e4/inc/msp432.h>
#include <ti/grlib/grlib.h>

static void Kentec_PixelDraw(const Graphics_Display *displayData,
        int16_t xCoordinate, int16_t yCoordinate, uint16_t color);
static void Kentec_PixelDrawMultiple(const Graphics_Display *displayData,
        int16_t xCoordinate, int16_t yCoordinate, int16_t x0,
        int16_t numPixels, int16_t bitsPerPixel, const uint8_t *pixelData,
        const uint32_t *pixelPalette);
static void Kentec_LineDrawH(const Graphics_Display *displayData, int16_t x1,
        int16_t x2, int16_t yCoordinate, uint16_t color);
static void Kentec_LineDrawV(const Graphics_Display *displayData,
        int16_t xCoordinate, int16_t y1, int16_t y2, uint16_t color);
static void Kentec_RectFill(const Graphics_Display *displayData,
        const Graphics_Rectangle *rectangle, uint16_t color);
static uint32_t Kentec_ColorTranslate(const Graphics_Display *displayData,
        uint32_t color);
static void Kentec_Flush(const Graphics_Display *displayData);
static void Kentec_Clear(const Graphics_Display *displayData,
        uint16_t ulValue);

/*
 * The display structure that describes the driver for the Kentec
 * K350QVG-V2-F TFT panel with an SSD2119 controller.
 */

const Graphics_Display_Functions Kentec_fxns = {
    Kentec_PixelDraw,
    Kentec_PixelDrawMultiple,
    Kentec_LineDrawH,
    Kentec_LineDrawV,
    Kentec_RectFill,
    Kentec_ColorTranslate,
    Kentec_Flush,
    Kentec_Clear
};

Graphics_Display Kentec_GD = {
    sizeof(tDisplay),
    0,
    320,
    240,
    0
};

#define MAKE_ENTRY_MODE(x)      ((ENTRY_MODE_DEFAULT & 0xff00) | (x))

/*
 * Read Access Timing
 * ------------------
 *
 * Direction  OOOIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIOOOOOOOOOOOOO
 *
 * ~RD        -----                    --------------------------
 *                 \                  /                          |
 *                  ------------------
 *                 <       Trdl       ><        Trdh             >
 *                 <                  Tcycle                     >
 *                 < Tacc >
 *                         /------------------|
 * DATA       -------------                    ------------------
 *                         \------------------/
 *                                     < Tdh  >
 *
 * Delays          <   Trad  >< Tdhd ><    Trhd   ><  Trcd      >
 *
 * This design keeps CS tied low so pulse width constraints relating to CS
 * have been transfered to ~RD here.
 *
 * Tcycle Read Cycle Time  1000nS
 * Tacc   Data Access Time  100nS
 * Trdl   Read Data Low     500nS
 * Trdh   Read Data High    500nS
 * Tdh    Data Hold Time    100nS
 *
 * Trad (READ_DATA_ACCESS_DELAY) controls the delay between asserting ~RD and
 *       reading the data from the bus.
 * Tdhd (READ_DATA_HOLD_DELAY) controls the delay after reading the data and
 *       before deasserting ~RD.
 * Trhd (READ_HOLD_DELAY) controls the delay between deasserting ~RD and
 *       switching the data bus direction back to output.
 * Trcd (READ_DATA_CYCLE_DELAY) controls the delay after switching the
 *       direction of the data bus.
 */

/*
 * The delay to impose after setting the state of the read/write line and
 * before reading the data bus.  This is expressed in terms of cycles of a
 * tight loop whose body performs a single GPIO register access and needs to
 * comply with the 500nS read cycle pulse width constraint.
 */
#define READ_DATA_ACCESS_DELAY  5

/*
 * The delay to impose after reading the data and before resetting the state of
 * the read/write line during a read operation.
 */
#define READ_DATA_HOLD_DELAY    5

/*
 * The delay to impose after deasserting ~RD and before setting the bus back to
 * an output.
 */
#define READ_HOLD_DELAY         5

/*
 * The delay to impose after completing a read cycle and before returning to
 * the caller.
 */
#define READ_DATA_CYCLE_DELAY   5

 /* The dimensions of the LCD panel. */
#define LCD_HORIZONTAL_MAX      320
#define LCD_VERTICAL_MAX        240

/*
 * Translates a 24-bit RGB color to a display driver-specific color.
 *
 * \param c is the 24-bit RGB color.  The least-significant byte is the blue
 * channel, the next byte is the green channel, and the third byte is the red
 * channel.
 *
 * This macro translates a 24-bit RGB color into a value that can be written
 * into the display's frame buffer in order to reproduce that color, or the
 * closest possible approximation of that color.
 *
 * \return Returns the display-driver specific color.
 */
#define DPYCOLORTRANSLATE(c)    ((((c) & 0x00f80000) >> 8) |                  \
                                 (((c) & 0x0000fc00) >> 5) |                  \
                                 (((c) & 0x000000f8) >> 3))

/*
 *  ======== WriteData ========
 * Writes a data word to the SSD2119.
 */
static inline void WriteData(uint16_t ui16Data)
{
    LCDIDDDataWrite(LCD0_BASE, 0, ui16Data);
}

/*
 *  ========== WriteCommand ==========
 *  Writes a command to the SSD2119.
 */
static inline void WriteCommand(uint8_t ui8Data)
{
    /* Pass the write on to the controller. */
    LCDIDDCommandWrite(LCD0_BASE, 0, (uint16_t)ui8Data);
}

/*
 *  ======== Kentec_PixelDraw ========
 *  Draws a pixel on the screen.
 *
 *  This function sets the given pixel to a particular color.  The coordinates
 *  of the pixel are assumed to be within the extents of the display.
 */
static void Kentec_PixelDraw(const Graphics_Display *displayData,
        int16_t xCoordinate, int16_t yCoordinate, uint16_t color)
{
    /* Set the X address of the display cursor.*/
    WriteCommand(SSD2119_X_RAM_ADDR_REG);
    WriteData(MAPPED_X(xCoordinate, yCoordinate));

    /* Set the Y address of the display cursor. */
    WriteCommand(SSD2119_Y_RAM_ADDR_REG);
    WriteData(MAPPED_Y(xCoordinate, yCoordinate));

    /* Write the pixel value. */
    WriteCommand(SSD2119_RAM_DATA_REG);
    WriteData(color);
}

/*
 *  ======== Kentec_PixelDrawMultiple ========
 *  Draws a horizontal sequence of pixels on the screen.
 */
static void Kentec_PixelDrawMultiple(const Graphics_Display *displayData,
        int16_t xCoordinate, int16_t yCoordinate, int16_t x0,
        int16_t numPixels, int16_t bitsPerPixel, const uint8_t *pixelData,
        const uint32_t *pixelPalette)
{
    uint32_t imageData;

    /* Set the cursor increment to left to right, followed by top to bottom. */
    WriteCommand(SSD2119_ENTRY_MODE_REG);
    WriteData(MAKE_ENTRY_MODE(HORIZ_DIRECTION));

    /* Set the starting X address of the display cursor. */
    WriteCommand(SSD2119_X_RAM_ADDR_REG);
    WriteData(MAPPED_X(xCoordinate, yCoordinate));

    /* Set the Y address of the display cursor. */
    WriteCommand(SSD2119_Y_RAM_ADDR_REG);
    WriteData(MAPPED_Y(xCoordinate, yCoordinate));

    /* Write the data RAM write command. */
    WriteCommand(SSD2119_RAM_DATA_REG);

    /* Determine how to interpret the pixel data based on the number of bits
       per pixel. */
    switch(bitsPerPixel & ~GRLIB_DRIVER_FLAG_NEW_IMAGE) {
        /* The pixel data is in 1 bit per pixel format. */
        case 1:
            /* Loop while there are more pixels to draw. */
            while(numPixels) {
                /* Get the next byte of image data. */
                imageData = *pixelData++;

                /* Loop through the pixels in this byte of image data. */
                for(; (x0 < 8) && numPixels; x0++, numPixels--) {
                    /* Draw this pixel in the appropriate color. */
                    WriteData(((uint32_t *)pixelPalette)[(imageData >>
                        (7 - x0)) & 1]);
                }

                /* Start at the beginning of the next byte of image data. */
                x0 = 0;
            }

            /* The image data has been drawn. */
            break;

        /* The pixel data is in 4 bit per pixel format. */
        case 4:
            /* Loop while there are more pixels to draw.  "Duff's device" is
               used to jump into the middle of the loop if the first nibble of
               the pixel data should not be used.  Duff's device makes use of
               the fact that a case statement is legal anywhere within a
               sub-block of a switch statement.  See
               http://en.wikipedia.org/wiki/Duff's_device for detailed
               information about Duff's device.
             */
            switch(x0 & 1) {
                case 0:
                    while(numPixels) {
                        /* Get the upper nibble of the next byte of pixel data
                           and extract the corresponding entry from the
                           palette.
                        */
                        imageData = (*pixelData >> 4) * 3;
                        imageData = (*(uint32_t *)(pixelPalette + imageData) &
                                0x00ffffff);

                        /* Translate this palette entry and write it to the
                           screen. */
                        WriteData(DPYCOLORTRANSLATE(imageData));

                        /* Decrement the count of pixels to draw. */
                        numPixels--;

                        /* See if there is another pixel to draw. */
                        if(numPixels) {
                case 1:
                            /* Get the lower nibble of the next byte of pixel
                               data and extract the corresponding entry from
                               the palette.
                            */
                            imageData = (*pixelData++ & 15) * 3;
                            imageData = (*(uint32_t *)(pixelPalette + imageData) &
                                    0x00ffffff);

                            /* Translate this palette entry and write it to the
                               screen. */
                            WriteData(DPYCOLORTRANSLATE(imageData));

                            /* Decrement the count of pixels to draw. */
                            numPixels--;
                        }
                    }
            }

            /* The image data has been drawn. */
            break;

        /* The pixel data is in 8 bit per pixel format. */
        case 8:
            /* Loop while there are more pixels to draw. */
            while(numPixels--) {
                /* Get the next byte of pixel data and extract the
                   corresponding entry from the palette. */
                imageData = *pixelData++ * 3;
                imageData = *(uint32_t *)(pixelPalette + imageData) & 0x00ffffff;

                /* Translate this palette entry and write it to the screen. */
                WriteData(DPYCOLORTRANSLATE(imageData));
            }

            /* The image data has been drawn. */
            break;
    }
}

/*
 *  ======== Kentec_LineDrawH ========
 *  Draws a horizontal line.
 */
static void Kentec_LineDrawH(const Graphics_Display *displayData, int16_t x1,
        int16_t x2, int16_t yCoordinate, uint16_t color)
{
    /* Set the cursor increment to left to right, followed by top to bottom. */
    WriteCommand(SSD2119_ENTRY_MODE_REG);
    WriteData(MAKE_ENTRY_MODE(HORIZ_DIRECTION));

    /* Set the starting X address of the display cursor. */
    WriteCommand(SSD2119_X_RAM_ADDR_REG);
    WriteData(MAPPED_X(x1, yCoordinate));

    /* Set the Y address of the display cursor. */
    WriteCommand(SSD2119_Y_RAM_ADDR_REG);
    WriteData(MAPPED_Y(x1, yCoordinate));

    /* Write the data RAM write command. */
    WriteCommand(SSD2119_RAM_DATA_REG);

    /* Loop through the pixels of this horizontal line. */
    while(x1++ <= x2) {
        /* Write the pixel value. */
        WriteData(color);
    }
}

/*
 *  ======== Kentec_LineDrawV ========
 *  Draws a vertical line.
 */
static void Kentec_LineDrawV(const Graphics_Display *displayData,
        int16_t xCoordinate, int16_t y1, int16_t y2, uint16_t color)
{
    /* Set the cursor increment to top to bottom, followed by left to right. */
    WriteCommand(SSD2119_ENTRY_MODE_REG);
    WriteData(MAKE_ENTRY_MODE(VERT_DIRECTION));

    /* Set the X address of the display cursor. */
    WriteCommand(SSD2119_X_RAM_ADDR_REG);
    WriteData(MAPPED_X(xCoordinate, y1));

    /* Set the starting Y address of the display cursor. */
    WriteCommand(SSD2119_Y_RAM_ADDR_REG);
    WriteData(MAPPED_Y(xCoordinate, y1));

    /* Write the data RAM write command. */
    WriteCommand(SSD2119_RAM_DATA_REG);

    /* Loop through the pixels of this vertical line. */
    while(y1++ <= y2) {
        /* Write the pixel value. */
        WriteData(color);
    }
}

/*
 *  ======== Kentec_RectFill ========
 */
static void Kentec_RectFill(const Graphics_Display *displayData,
        const Graphics_Rectangle *rectangle, uint16_t color)
{
    int32_t count;

    /* Write the Y extents of the rectangle. */
    WriteCommand(SSD2119_ENTRY_MODE_REG);
    WriteData(MAKE_ENTRY_MODE(HORIZ_DIRECTION));

    /* Write the X extents of the rectangle. */
    WriteCommand(SSD2119_H_RAM_START_REG);
#if (defined PORTRAIT) || (defined LANDSCAPE)
    WriteData(MAPPED_X(rectangle->xMax, rectangle->yMax));
#else
    WriteData(MAPPED_X(rectangle->xMin, rectangle->yMin));
#endif

    WriteCommand(SSD2119_H_RAM_END_REG);
#if (defined PORTRAIT) || (defined LANDSCAPE)
    WriteData(MAPPED_X(rectangle->xMin, rectangle->yMin));
#else
    WriteData(MAPPED_X(rectangle->xMax, rectangle->yMax));
#endif

    /* Write the Y extents of the rectangle */
    WriteCommand(SSD2119_V_RAM_POS_REG);
#if (defined LANDSCAPE_FLIP) || (defined PORTRAIT)
    WriteData(MAPPED_Y(rectangle->xMin, rectangle->yMin) |
            (MAPPED_Y(rectangle->xMax, rectangle->yMax) << 8));
#else
    WriteData(MAPPED_Y(rectangle->xMax, rectangle->yMax) |
            (MAPPED_Y(rectangle->xMin, rectangle->yMin) << 8));
#endif

    /* Set the display cursor to the upper left of the rectangle (in
       application coordinate space). */
    WriteCommand(SSD2119_X_RAM_ADDR_REG);
    WriteData(MAPPED_X(rectangle->xMin, rectangle->yMin));
    WriteCommand(SSD2119_Y_RAM_ADDR_REG);
    WriteData(MAPPED_Y(rectangle->xMin, rectangle->yMin));

    /* Tell the controller to write data into its RAM. */
    WriteCommand(SSD2119_RAM_DATA_REG);

    /* Loop through the pixels of this filled rectangle. */
    for(count = ((rectangle->xMax - rectangle->xMin + 1) *
            (rectangle->yMax - rectangle->yMin + 1)); count >= 0; count--) {
        /* Write the pixel value. */
        WriteData(color);
    }

    /* Reset the X extents to the entire screen. */
    WriteCommand(SSD2119_H_RAM_START_REG);
    WriteData(0x0000);
    WriteCommand(SSD2119_H_RAM_END_REG);
    WriteData(0x013f);

    /* Reset the Y extent to the full screen */
    WriteCommand(SSD2119_V_RAM_POS_REG);
    WriteData(0xef00);
}

/*
 *  ======== Kentec_ColorTranslate ========
 *  This function translates a 24-bit RGB color into a value that can be
 * written into the display's frame buffer in order to reproduce that color,
 * or the closest possible approximation of that color.
 */
static uint32_t Kentec_ColorTranslate(const Graphics_Display *displayData,
        uint32_t color)
{
    /* Translate from a 24-bit RGB color to a 5-6-5 RGB color. */
    return(DPYCOLORTRANSLATE(color));
}

/*
 *  ======== Kentec_Flush ========
 *  Flushes any cached drawing operations.
 */
static void Kentec_Flush(const Graphics_Display *displayData)
{
    /* There is nothing to be done. */
}

static void Kentec_Clear(const Graphics_Display *displayData,
        uint16_t ulValue)
{
    uint16_t y0;

    for(y0 = 0; y0 < LCD_VERTICAL_MAX; y0++) {
        Kentec_LineDrawH(displayData->displayData, 0,
            LCD_HORIZONTAL_MAX - 1, y0, ulValue);
    }
}

/*
 *  ========== Kentec_Init ==========
 * This function initializes the LCD controller and the SSD2119 display
 * controller on the panel, preparing it to display data.
 */
void Kentec_Init(uint32_t sysClock)
{
    uint32_t clockMS, count;
    tLCDIDDTiming timings;

    /* Determine the number of system clock cycles in 1mS */
    clockMS = CYCLES_FROM_TIME_US(sysClock, 1000);
    /* Divide by 3 to get the number of SysCtlDelay loops in 1mS. */
    clockMS /= 3;

    /* Enable the LCD controller. */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_LCD0);

    /* Configure GPIOs used by Kentec LCD */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOR);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOS);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOT);

    GPIOPinConfigure(GPIO_PJ6_LCDAC);
    GPIOPinConfigure(GPIO_PR0_LCDCP);
    GPIOPinConfigure(GPIO_PR1_LCDFP);
    GPIOPinConfigure(GPIO_PR2_LCDLP);
    GPIOPinConfigure(GPIO_PR4_LCDDATA00);
    GPIOPinConfigure(GPIO_PR5_LCDDATA01);
    GPIOPinConfigure(GPIO_PF7_LCDDATA02);
    GPIOPinConfigure(GPIO_PR3_LCDDATA03);
    GPIOPinConfigure(GPIO_PR6_LCDDATA04);
    GPIOPinConfigure(GPIO_PR7_LCDDATA05);
    GPIOPinConfigure(GPIO_PS4_LCDDATA06);
    GPIOPinConfigure(GPIO_PS5_LCDDATA07);

    GPIOPinConfigure(GPIO_PS6_LCDDATA08);
    GPIOPinConfigure(GPIO_PS7_LCDDATA09);
    GPIOPinConfigure(GPIO_PT0_LCDDATA10);
    GPIOPinConfigure(GPIO_PT1_LCDDATA11);
    GPIOPinConfigure(GPIO_PN7_LCDDATA12);
    GPIOPinConfigure(GPIO_PN6_LCDDATA13);
    GPIOPinConfigure(GPIO_PJ2_LCDDATA14);
    GPIOPinConfigure(GPIO_PJ3_LCDDATA15);

    GPIOPinTypeGPIOOutput(GPIO_PORTS_BASE, GPIO_PIN_3);
    GPIOPinWrite(GPIO_PORTS_BASE, GPIO_PIN_3, GPIO_PIN_3);

    GPIOPinTypeLCD(GPIO_PORTF_BASE, GPIO_PIN_7);
    GPIOPinTypeLCD(GPIO_PORTJ_BASE, GPIO_PIN_6 | GPIO_PIN_3 | GPIO_PIN_2);
    GPIOPinTypeLCD(GPIO_PORTR_BASE, (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 |
        GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7));
    GPIOPinTypeLCD(GPIO_PORTS_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);
    GPIOPinTypeLCD(GPIO_PORTT_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    GPIOPinTypeLCD(GPIO_PORTN_BASE, GPIO_PIN_7 | GPIO_PIN_6);

    /* Assert the LCD reset signal. */
    GPIOPinWrite(GPIO_PORTS_BASE, GPIO_PIN_3, 0);
    /* Delay for 50ms. */
    SysCtlDelay(50 * clockMS);
    /* Deassert the LCD reset signal. */
    GPIOPinWrite(GPIO_PORTS_BASE, GPIO_PIN_3, GPIO_PIN_3);

    /* Delay for 50ms while the LCD comes out of reset. */
    SysCtlDelay(50 * clockMS);
    /* Configure the LCD controller for LIDD-mode operation. */
    LCDModeSet(LCD0_BASE, LCD_MODE_LIDD, sysClock, sysClock);

    /* Configure DMA-related parameters. */
    LCDDMAConfigSet(LCD0_BASE, LCD_DMA_BURST_4);

    /* Set control signal parameters and polarities. */
    LCDIDDConfigSet(LCD0_BASE, LIDD_CONFIG_ASYNC_MPU80);

    /* Set the LIDD interface timings for the Kentec display.  Note that the
       inter-transaction delay is set at at 50nS to match the write case.
       Software needs to ensure that it delays at least 450nS more between each
       read or the read timings will be violated.
    */
    timings.ui8WSSetup = CYCLES_FROM_TIME_NS(sysClock, 5);
    timings.ui8WSDuration = CYCLES_FROM_TIME_NS(sysClock, 40);
    timings.ui8WSHold = CYCLES_FROM_TIME_NS(sysClock, 5);
    timings.ui8RSSetup = CYCLES_FROM_TIME_NS(sysClock, 0);
    timings.ui8RSDuration = CYCLES_FROM_TIME_NS(sysClock, 500);
    timings.ui8RSHold = CYCLES_FROM_TIME_NS(sysClock, 100);
    timings.ui8DelayCycles = CYCLES_FROM_TIME_NS(sysClock, 50);
    LCDIDDTimingSet(LCD0_BASE, 0, &timings);

    /* Enter sleep mode (if not already there). */
    WriteCommand(SSD2119_SLEEP_MODE_1_REG);
    WriteData(0x0001);

    /* Set initial power parameters. */
    WriteCommand(SSD2119_PWR_CTRL_5_REG);
    WriteData(0x00b2);
    WriteCommand(SSD2119_VCOM_OTP_1_REG);
    WriteData(0x0006);

    /* Start the oscillator. */
    WriteCommand(SSD2119_OSC_START_REG);
    WriteData(0x0001);

    /* Set pixel format and basic display orientation (scanning direction). */
    WriteCommand(SSD2119_OUTPUT_CTRL_REG);
    WriteData(0x30ef);
    WriteCommand(SSD2119_LCD_DRIVE_AC_CTRL_REG);
    WriteData(0x0600);

    /* Exit sleep mode. */
    WriteCommand(SSD2119_SLEEP_MODE_1_REG);
    WriteData(0x0000);

    /* Delay 30mS */
    SysCtlDelay(30 * clockMS);
    /* Configure pixel color format and MCU interface parameters. */
    WriteCommand(SSD2119_ENTRY_MODE_REG);
    WriteData(ENTRY_MODE_DEFAULT);

    /* Set analog parameters. */
    WriteCommand(SSD2119_SLEEP_MODE_2_REG);
    WriteData(0x0999);
    WriteCommand(SSD2119_ANALOG_SET_REG);
    WriteData(0x3800);

    /* Enable the display. */
    WriteCommand(SSD2119_DISPLAY_CTRL_REG);
    WriteData(0x0033);

    /* Set VCIX2 voltage to 6.1V. */
    WriteCommand(SSD2119_PWR_CTRL_2_REG);
    WriteData(0x0005);

    /* Configure gamma correction. */
    WriteCommand(SSD2119_GAMMA_CTRL_1_REG);
    WriteData(0x0000);
    WriteCommand(SSD2119_GAMMA_CTRL_2_REG);
    WriteData(0x0303);
    WriteCommand(SSD2119_GAMMA_CTRL_3_REG);
    WriteData(0x0407);
    WriteCommand(SSD2119_GAMMA_CTRL_4_REG);
    WriteData(0x0301);
    WriteCommand(SSD2119_GAMMA_CTRL_5_REG);
    WriteData(0x0301);
    WriteCommand(SSD2119_GAMMA_CTRL_6_REG);
    WriteData(0x0403);
    WriteCommand(SSD2119_GAMMA_CTRL_7_REG);
    WriteData(0x0707);
    WriteCommand(SSD2119_GAMMA_CTRL_8_REG);
    WriteData(0x0400);
    WriteCommand(SSD2119_GAMMA_CTRL_9_REG);
    WriteData(0x0a00);
    WriteCommand(SSD2119_GAMMA_CTRL_10_REG);
    WriteData(0x1000);

    /* Configure Vlcd63 and VCOMl. */
    WriteCommand(SSD2119_PWR_CTRL_3_REG);
    WriteData(0x000a);
    WriteCommand(SSD2119_PWR_CTRL_4_REG);
    WriteData(0x2e00);

    /* Set the display size and ensure that the GRAM window is set to allow
       access to the full display buffer. */
    WriteCommand(SSD2119_V_RAM_POS_REG);
    WriteData((LCD_VERTICAL_MAX-1) << 8);
    WriteCommand(SSD2119_H_RAM_START_REG);
    WriteData(0x0000);
    WriteCommand(SSD2119_H_RAM_END_REG);
    WriteData(LCD_HORIZONTAL_MAX-1);
    WriteCommand(SSD2119_X_RAM_ADDR_REG);
    WriteData(0x0000);
    WriteCommand(SSD2119_Y_RAM_ADDR_REG);
    WriteData(0x0000);

    /* Clear the contents of the display buffer. */
    WriteCommand(SSD2119_RAM_DATA_REG);
    for(count = 0; count < (320 * 240); count++) {
        WriteData(0x0000);
    }
}
