SIMPLELINK_MSP432E4_SDK_INSTALL_DIR ?= $(abspath ../../../../../../..)

include $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/imports.mak

KERNEL_BUILD := $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/kernel/freertos/builds/MSP_EXP432E401Y/release

CC = "$(IAR_ARMCOMPILER)/bin/iccarm"
LNK = "$(IAR_ARMCOMPILER)/bin/ilinkarm"

OBJECTS = USBKBH.obj usbkeyboardhost.obj ti_drivers_config.obj ti_usblib_config.obj freertos_main_freertos.obj

NAME = usb_keyboard_host

CFLAGS = -I../.. \
    -I. \
    "-I$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/CMSIS/Include" \
    "-I$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source" \
    "-I$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/net/bsd" \
    -DDeviceFamily_MSP432E4 \
    -D__MSP432E401Y__ \
    "-I$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/posix/iar" \
    "-I$(FREERTOS_INSTALL_DIR)/FreeRTOS/Source/include" \
    "-I$(FREERTOS_INSTALL_DIR)/FreeRTOS/Source/portable/IAR/ARM_CM4F" \
    "-I$(KERNEL_BUILD)" \
    --cpu=Cortex-M4F \
    --debug \
    --silent \
    -e \
    --aeabi \
    --thumb \
    --diag_suppress=Pa050

LFLAGS = "$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/usblib/msp432e4/lib/iar/m4f/msp432e4_usblib.a" \
    "$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/display/lib/display.arm4f" \
    "$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/grlib/lib/iar/m4f/grlib.a" \
    "$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/spiffs/lib/iar/m4f/spiffs.a" \
    "$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/drivers/lib/drivers_msp432e4.arm4f" \
    "$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/fatfs/lib/iar/m4f/fatfs.a" \
    "$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/net/lib/iar/m4f/slnetsock_release.a" \
    "$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/net/sntp/lib/iar/m4f/sntp_release.a" \
    "$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/net/mqtt/lib/iar/m4f/mqtt_release.a" \
    "$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/net/http/lib/iar/m4f/httpclient_release.a" \
    "$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/ndk/slnetif/lib/slnetifndk.arm4f" \
    "$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/mbedtls/ti/lib/iar/m4f/mbedtls.a" \
    "$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/ndk/hal/timer_bios/lib/hal_timer.arm4f" \
    "$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/ndk/hal/eth_stub/lib/hal_eth_stub.arm4f" \
    "$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/ndk/tools/hdlc/lib/hdlc.arm4f" \
    "$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/ndk/tools/console/lib/console_min_ipv4.arm4f" \
    "$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/ndk/netctrl/lib/netctrl_min_ipv4.arm4f" \
    "$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/ndk/nettools/lib/nettool_ipv4.arm4f" \
    "$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/ndk/hal/ser_stub/lib/hal_ser_stub.arm4f" \
    "$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/ndk/tools/servers/lib/servers_min_ipv4.arm4f" \
    "$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/ndk/hal/userled_stub/lib/hal_userled_stub.arm4f" \
    "$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/ndk/stack/lib/stk.arm4f" \
    "$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/ndk/os/lib/os_freertos.arm4f" \
    --threaded_lib \
    "$(KERNEL_BUILD)/iar/freertos.lib" \
    "$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/devices/msp432e4/driverlib/lib/iar/m4f/msp432e4_driverlib.a" \
    --config ../../freertos/iar/MSP_EXP432E401Y_FREERTOS.icf \
    --map "$(NAME).map" \
    --silent \
    --cpu=Cortex-M4F \
    --semihosting=iar_breakpoint

all: $(NAME).out

$(KERNEL_BUILD)/iar/freertos.lib:
	@ $(ECHOBLANKLINE)
	@ echo $@ is not built.
	@ echo You can build it by issuing $(MAKE) in $(dir $@).
	@ $(ECHOBLANKLINE)

.INTERMEDIATE: syscfg
ti_drivers_config.c ti_usblib_config.c ti_drivers_config.h ti_usblib_config.h: syscfg
	@ echo generation complete

syscfg: ../../usb_keyboard_host.syscfg
	@ echo Generating configuration files...
	@ $(SYSCONFIG_TOOL) --compiler iar --product $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/.metadata/product.json --output $(@D) $<

USBKBH.obj: ../../USBKBH.c ti_drivers_config.h ti_usblib_config.h
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -o $@

usbkeyboardhost.obj: ../../usbkeyboardhost.c ti_drivers_config.h ti_usblib_config.h
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -o $@

freertos_main_freertos.obj: ../../freertos/main_freertos.c ti_drivers_config.h ti_usblib_config.h
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -o $@

ti_drivers_config.obj: ti_drivers_config.c ti_drivers_config.h ti_usblib_config.h
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -o $@

ti_usblib_config.obj: ti_usblib_config.c ti_drivers_config.h ti_usblib_config.h
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -o $@

$(NAME).out: $(OBJECTS) $(KERNEL_BUILD)/iar/freertos.lib
	@ echo linking $@
	@ $(LNK)  $(OBJECTS)  $(LFLAGS) -o $(NAME).out

clean:
	@ echo Cleaning...
	@ $(RM) $(OBJECTS) > $(DEVNULL) 2>&1
	@ $(RM) $(NAME).out > $(DEVNULL) 2>&1
	@ $(RM) $(NAME).map > $(DEVNULL) 2>&1
	@ $(RM) ti_drivers_config.c ti_usblib_config.c ti_drivers_config.h ti_usblib_config.h > $(DEVNULL) 2>&1
