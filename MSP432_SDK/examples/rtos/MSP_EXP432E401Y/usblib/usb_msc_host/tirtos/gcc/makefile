SIMPLELINK_MSP432E4_SDK_INSTALL_DIR ?= $(abspath ../../../../../../..)

include $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/imports.mak

KERNEL_BUILD := $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/kernel/tirtos/builds/MSP_EXP432E401Y/release

CC = "$(GCC_ARMCOMPILER)/bin/arm-none-eabi-gcc"
LNK = "$(GCC_ARMCOMPILER)/bin/arm-none-eabi-gcc"

OBJECTS = USBMSCH.obj fat_usbmsc.obj fatsfile.obj cmdline.obj ti_drivers_config.obj ti_usblib_config.obj tirtos_main_tirtos.obj

CONFIGPKG = $(KERNEL_BUILD)/gcc

NAME = usb_msc_host

CFLAGS = -I../.. \
    -I. \
    "-I$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/CMSIS/Include" \
    "-I$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source" \
    "-I$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/posix/gcc" \
    "-I$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/net/bsd" \
    -DDeviceFamily_MSP432E4 \
    -D__MSP432E401Y__ \
    -mcpu=cortex-m4 \
    -march=armv7e-m \
    -mthumb \
    -std=c99 \
    -mfloat-abi=hard \
    -mfpu=fpv4-sp-d16 \
    -ffunction-sections \
    -fdata-sections \
    -g \
    -gstrict-dwarf \
    -Wall \
    "-I$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/kernel/tirtos/packages/gnu/targets/arm/libs/install-native/arm-none-eabi/include/newlib-nano" \
    "-I$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/kernel/tirtos/packages/gnu/targets/arm/libs/install-native/arm-none-eabi/include" \
    "-I$(GCC_ARMCOMPILER)/arm-none-eabi/include"

LFLAGS = "-L$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/usblib/msp432e4/lib/gcc/m4f" \
    -l:msp432e4_usblib.a \
    "-L$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/fatfs/lib/gcc/m4f" \
    -l:fatfs.a \
    "-L$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/kernel/tirtos/packages/ti/dpl/lib" \
    -l:dpl_msp432e4.am4fg \
    "-L$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/kernel/tirtos/packages/gnu/targets/arm/rtsv7M/lib" \
    -l:boot.am4fg \
    -Wl,-T,../../tirtos/gcc/MSP_EXP432E401Y_TIRTOS.lds \
    "-Wl,-Map,$(NAME).map" \
    "-L$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source" \
    -l:ti/display/lib/display.am4fg \
    -l:ti/grlib/lib/gcc/m4f/grlib.a \
    -l:third_party/spiffs/lib/gcc/m4f/spiffs.a \
    -l:ti/drivers/lib/drivers_msp432e4.am4fg \
    -l:third_party/fatfs/lib/gcc/m4f/fatfs.a \
    -l:ti/net/lib/gcc/m4f/slnetsock_release.a \
    -l:ti/net/sntp/lib/gcc/m4f/sntp_release.a \
    -l:ti/net/mqtt/lib/gcc/m4f/mqtt_release.a \
    -l:ti/net/http/lib/gcc/m4f/httpclient_release.a \
    -l:ti/ndk/slnetif/lib/slnetifndk.am4fg \
    -l:third_party/mbedtls/ti/lib/gcc/m4f/mbedtls.a \
    -l:ti/ndk/hal/timer_bios/lib/hal_timer.am4fg \
    -l:ti/ndk/hal/eth_stub/lib/hal_eth_stub.am4fg \
    -l:ti/ndk/tools/hdlc/lib/hdlc.am4fg \
    -l:ti/ndk/tools/console/lib/console_min_ipv4.am4fg \
    -l:ti/ndk/netctrl/lib/netctrl_min_ipv4.am4fg \
    -l:ti/ndk/nettools/lib/nettool_ipv4.am4fg \
    -l:ti/ndk/hal/ser_stub/lib/hal_ser_stub.am4fg \
    -l:ti/ndk/tools/servers/lib/servers_min_ipv4.am4fg \
    -l:ti/ndk/hal/userled_stub/lib/hal_userled_stub.am4fg \
    -l:ti/ndk/stack/lib/stk.am4fg \
    -l:ti/ndk/os/lib/os.am4fg \
    -nostartfiles \
    "-L$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/kernel/tirtos/packages" \
    -l:ti/dpl/lib/dpl_msp432e4.am4fg \
    "-Wl,-T,$(KERNEL_BUILD)/gcc/linker.cmd" \
    -l:ti/devices/msp432e4/driverlib/lib/gcc/m4f/msp432e4_driverlib.a \
    -march=armv7e-m \
    -mthumb \
    -mfloat-abi=hard \
    -mfpu=fpv4-sp-d16 \
    -static \
    -Wl,--gc-sections \
    "-L$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/kernel/tirtos/packages/gnu/targets/arm/libs/install-native/arm-none-eabi/lib/thumb/v7e-m/hard" \
    -lgcc \
    -lc \
    -lm \
    -lnosys \
    --specs=nano.specs

all: $(NAME).out

$(CONFIGPKG)/linker.cmd $(CONFIGPKG)/compiler.opt:
	@ $(ECHOBLANKLINE)
	@ echo $(CONFIGPKG) is not built.
	@ echo You can build it by issuing $(MAKE) in $(CONFIGPKG).
	@ $(ECHOBLANKLINE)

.INTERMEDIATE: syscfg
ti_drivers_config.c ti_usblib_config.c ti_drivers_config.h ti_usblib_config.h: syscfg
	@ echo generation complete

syscfg: ../../usb_msc_host.syscfg
	@ echo Generating configuration files...
	@ $(SYSCONFIG_TOOL) --compiler gcc --product $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/.metadata/product.json --output $(@D) $<

USBMSCH.obj: ../../USBMSCH.c ti_drivers_config.h ti_usblib_config.h $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c @$(CONFIGPKG)/compiler.opt -o $@

fat_usbmsc.obj: ../../fat_usbmsc.c ti_drivers_config.h ti_usblib_config.h $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c @$(CONFIGPKG)/compiler.opt -o $@

fatsfile.obj: ../../fatsfile.c ti_drivers_config.h ti_usblib_config.h $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c @$(CONFIGPKG)/compiler.opt -o $@

cmdline.obj: ../../cmdline.c ti_drivers_config.h ti_usblib_config.h $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c @$(CONFIGPKG)/compiler.opt -o $@

tirtos_main_tirtos.obj: ../../tirtos/main_tirtos.c ti_drivers_config.h ti_usblib_config.h $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c @$(CONFIGPKG)/compiler.opt -o $@

ti_drivers_config.obj: ti_drivers_config.c ti_drivers_config.h ti_usblib_config.h $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c @$(CONFIGPKG)/compiler.opt -o $@

ti_usblib_config.obj: ti_usblib_config.c ti_drivers_config.h ti_usblib_config.h $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c @$(CONFIGPKG)/compiler.opt -o $@

$(NAME).out: $(OBJECTS) $(CONFIGPKG)/linker.cmd
	@ echo linking $@
	@ $(LNK)  $(OBJECTS)  $(LFLAGS) -o $(NAME).out

clean:
	@ echo Cleaning...
	@ $(RM) $(OBJECTS) > $(DEVNULL) 2>&1
	@ $(RM) $(NAME).out > $(DEVNULL) 2>&1
	@ $(RM) $(NAME).map > $(DEVNULL) 2>&1
	@ $(RM) ti_drivers_config.c ti_usblib_config.c ti_drivers_config.h ti_usblib_config.h > $(DEVNULL) 2>&1
