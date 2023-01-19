SIMPLELINK_MSP432E4_SDK_INSTALL_DIR ?= $(abspath ../../../../../..)

include $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/imports.mak

CC = "$(CCS_ARMCOMPILER)/bin/armcl"
LNK = "$(CCS_ARMCOMPILER)/bin/armcl"

OBJECTS = system_msp432e401y.obj bl_autobaud.obj bl_can.obj bl_check.obj bl_crc32.obj bl_decrypt.obj bl_emac.obj bl_flash.obj bl_i2c.obj bl_main.obj bl_packet.obj bl_ssi.obj bl_uart.obj bl_usb.obj bl_usbfuncs.obj ccs_bl_startup_ccs.obj

NAME = boot_serial_uart_flash

CFLAGS = -I.. \
    "-I$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source" \
    "-I$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/CMSIS/Include" \
    "-I$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/net/bsd" \
    --advice:power=none \
    -DDeviceFamily_MSP432E4 \
    -D__MSP432E401Y__ \
    --silicon_version=7M4 \
    --code_state=16 \
    --little_endian \
    --display_error_number \
    --diag_warning=255 \
    --diag_wrap=off \
    --gen_func_subsections=on \
    --float_support=FPv4SPD16 \
    --symdebug:dwarf \
    "-I$(CCS_ARMCOMPILER)/include"

LFLAGS = --ram_model \
    --stack_size=512 \
    "-i$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source" \
    -lti/display/lib/display.aem4f \
    -lti/grlib/lib/ccs/m4f/grlib.a \
    -lthird_party/spiffs/lib/ccs/m4f/spiffs.a \
    -lti/drivers/lib/drivers_msp432e4.aem4f \
    -lthird_party/fatfs/lib/ccs/m4f/fatfs.a \
    -lti/devices/msp432e4/driverlib/lib/ccs/m4f/msp432e4_driverlib.a \
    ../ccs/bl_link_ccs.cmd \
    "-m$(NAME).map" \
    --warn_sections \
    --display_error_number \
    --diag_wrap=off \
     \
    "-i$(CCS_ARMCOMPILER)/lib" \
    -llibc.a

all: $(NAME).out

system_msp432e401y.obj: ../system_msp432e401y.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< --output_file=$@

bl_autobaud.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/devices/msp432e4/boot_loader/bl_autobaud.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< --output_file=$@

bl_can.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/devices/msp432e4/boot_loader/bl_can.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< --output_file=$@

bl_check.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/devices/msp432e4/boot_loader/bl_check.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< --output_file=$@

bl_crc32.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/devices/msp432e4/boot_loader/bl_crc32.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< --output_file=$@

bl_decrypt.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/devices/msp432e4/boot_loader/bl_decrypt.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< --output_file=$@

bl_emac.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/devices/msp432e4/boot_loader/bl_emac.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< --output_file=$@

bl_flash.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/devices/msp432e4/boot_loader/bl_flash.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< --output_file=$@

bl_i2c.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/devices/msp432e4/boot_loader/bl_i2c.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< --output_file=$@

bl_main.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/devices/msp432e4/boot_loader/bl_main.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< --output_file=$@

bl_packet.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/devices/msp432e4/boot_loader/bl_packet.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< --output_file=$@

bl_ssi.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/devices/msp432e4/boot_loader/bl_ssi.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< --output_file=$@

bl_uart.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/devices/msp432e4/boot_loader/bl_uart.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< --output_file=$@

bl_usb.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/devices/msp432e4/boot_loader/bl_usb.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< --output_file=$@

bl_usbfuncs.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/devices/msp432e4/boot_loader/bl_usbfuncs.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< --output_file=$@

ccs_bl_startup_ccs.obj: ../ccs/bl_startup_ccs.s
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< --output_file=$@

$(NAME).out: $(OBJECTS)
	@ echo linking $@
	@ $(LNK) -z $(OBJECTS)  $(LFLAGS) -o $(NAME).out

clean:
	@ echo Cleaning...
	@ $(RM) $(OBJECTS) > $(DEVNULL) 2>&1
	@ $(RM) $(NAME).out > $(DEVNULL) 2>&1
	@ $(RM) $(NAME).map > $(DEVNULL) 2>&1
