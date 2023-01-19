SIMPLELINK_MSP432E4_SDK_INSTALL_DIR ?= $(abspath ../../../../../..)

include $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/imports.mak

ASM = "$(IAR_ARMCOMPILER)/bin/iasmarm"
CC = "$(IAR_ARMCOMPILER)/bin/iccarm"
LNK = "$(IAR_ARMCOMPILER)/bin/ilinkarm"

OBJECTS = system_msp432e401y.obj bl_autobaud.obj bl_can.obj bl_check.obj bl_crc32.obj bl_decrypt.obj bl_emac.obj bl_flash.obj bl_i2c.obj bl_main.obj bl_packet.obj bl_ssi.obj bl_uart.obj bl_usb.obj bl_usbfuncs.obj iar_bl_startup_ewarm.obj

NAME = boot_serial_i2c_flash

AFLAGS = -I.. \
    "-I$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source" \
    "-I$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/CMSIS/Include" \
    -DDeviceFamily_MSP432E4 \
    -D__MSP432E401Y__ \
    -S \
    --cpu Cortex-M4F \
    --endian little \
    --thumb \
    "-I$(IAR_ARMCOMPILER)/inc/c"

CFLAGS = -I.. \
    "-I$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source" \
    "-I$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/CMSIS/Include" \
    "-I$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/net/bsd" \
    -DDeviceFamily_MSP432E4 \
    -D__MSP432E401Y__ \
    --cpu=Cortex-M4F \
    --debug \
    --silent \
    -e \
    --aeabi \
    --thumb \
    --diag_suppress=Pa050

LFLAGS = --entry \
    ResetISR \
    "$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/display/lib/display.arm4f" \
    "$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/grlib/lib/iar/m4f/grlib.a" \
    "$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/spiffs/lib/iar/m4f/spiffs.a" \
    "$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/drivers/lib/drivers_msp432e4.arm4f" \
    "$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/fatfs/lib/iar/m4f/fatfs.a" \
    "$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/devices/msp432e4/driverlib/lib/iar/m4f/msp432e4_driverlib.a" \
    --config ../iar/bl_link.icf \
    --map "$(NAME).map" \
    --silent \
    --cpu=Cortex-M4F \
    --semihosting=iar_breakpoint

all: $(NAME).out

system_msp432e401y.obj: ../system_msp432e401y.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -o $@

bl_autobaud.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/devices/msp432e4/boot_loader/bl_autobaud.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -o $@

bl_can.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/devices/msp432e4/boot_loader/bl_can.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -o $@

bl_check.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/devices/msp432e4/boot_loader/bl_check.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -o $@

bl_crc32.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/devices/msp432e4/boot_loader/bl_crc32.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -o $@

bl_decrypt.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/devices/msp432e4/boot_loader/bl_decrypt.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -o $@

bl_emac.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/devices/msp432e4/boot_loader/bl_emac.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -o $@

bl_flash.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/devices/msp432e4/boot_loader/bl_flash.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -o $@

bl_i2c.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/devices/msp432e4/boot_loader/bl_i2c.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -o $@

bl_main.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/devices/msp432e4/boot_loader/bl_main.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -o $@

bl_packet.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/devices/msp432e4/boot_loader/bl_packet.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -o $@

bl_ssi.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/devices/msp432e4/boot_loader/bl_ssi.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -o $@

bl_uart.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/devices/msp432e4/boot_loader/bl_uart.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -o $@

bl_usb.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/devices/msp432e4/boot_loader/bl_usb.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -o $@

bl_usbfuncs.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/devices/msp432e4/boot_loader/bl_usbfuncs.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -o $@

iar_bl_startup_ewarm.obj: ../iar/bl_startup_ewarm.S
	@ echo Building $@
	@ $(ASM) $(AFLAGS) $< -o $@

$(NAME).out: $(OBJECTS)
	@ echo linking $@
	@ $(LNK)  $(OBJECTS)  $(LFLAGS) -o $(NAME).out

clean:
	@ echo Cleaning...
	@ $(RM) $(OBJECTS) > $(DEVNULL) 2>&1
	@ $(RM) $(NAME).out > $(DEVNULL) 2>&1
	@ $(RM) $(NAME).map > $(DEVNULL) 2>&1
