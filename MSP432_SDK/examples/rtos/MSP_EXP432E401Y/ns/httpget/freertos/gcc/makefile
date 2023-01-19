SIMPLELINK_MSP432E4_SDK_INSTALL_DIR ?= $(abspath ../../../../../../..)

include $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/imports.mak

KERNEL_BUILD := $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/kernel/freertos/builds/MSP_EXP432E401Y/release

CC = "$(GCC_ARMCOMPILER)/bin/arm-none-eabi-gcc"
LNK = "$(GCC_ARMCOMPILER)/bin/arm-none-eabi-gcc"

OBJECTS = httpget.obj httpgethooks.obj freertos_main_freertos.obj ti_ndk_config.obj ti_net_config.obj ti_drivers_config.obj

NAME = httpget

CFLAGS = -I../.. \
    -I. \
    "-I$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/net/bsd" \
    "-I$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source" \
    "-I$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/CMSIS/Include" \
    "-I$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/posix/gcc" \
    "-I$(FREERTOS_INSTALL_DIR)/FreeRTOS/Source/include" \
    "-I$(FREERTOS_INSTALL_DIR)/FreeRTOS/Source/portable/GCC/ARM_CM4F" \
    "-I$(KERNEL_BUILD)" \
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
    "-I$(GCC_ARMCOMPILER)/arm-none-eabi/include/newlib-nano" \
    "-I$(GCC_ARMCOMPILER)/arm-none-eabi/include"

LFLAGS = "-L$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source" \
    -l:ti/net/lib/gcc/m4f/slnetsock_release.a \
    -l:ti/net/sntp/lib/gcc/m4f/sntp_release.a \
    -l:ti/net/mqtt/lib/gcc/m4f/mqtt_release.a \
    -l:ti/net/http/lib/gcc/m4f/httpclient_release.a \
    -l:ti/net/http/lib/gcc/m4f/httpserver_release.a \
    -l:ti/ndk/slnetif/lib/slnetifndk_msp432e4.am4fg \
    -l:third_party/mbedtls/ti/lib/gcc/m4f/mbedtls_msp432e4.a \
    -l:ti/ndk/hal/timer_bios/lib/hal_timer.am4fg \
    -l:ti/ndk/hal/eth_stub/lib/hal_eth_stub.am4fg \
    -l:ti/ndk/tools/hdlc/lib/hdlc.am4fg \
    -l:ti/ndk/tools/console/lib/console_min_ipv4.am4fg \
    -l:ti/ndk/netctrl/lib/netctrl_min_ipv4.am4fg \
    -l:ti/ndk/nettools/lib/nettool_ipv4.am4fg \
    -l:ti/ndk/hal/ser_stub/lib/hal_ser_stub.am4fg \
    -l:ti/ndk/hal/userled_stub/lib/hal_userled_stub.am4fg \
    -l:ti/ndk/stack/lib/stk.am4fg \
    -l:ti/ndk/os/lib/os_freertos.am4fg \
    -Wl,-T,../../freertos/gcc/MSP_EXP432E401Y_FREERTOS.lds \
    "-Wl,-Map,$(NAME).map" \
    -nostartfiles \
    -l:ti/display/lib/display.am4fg \
    -l:ti/grlib/lib/gcc/m4f/grlib.a \
    -l:third_party/spiffs/lib/gcc/m4f/spiffs.a \
    -l:ti/drivers/lib/drivers_msp432e4.am4fg \
    -l:third_party/fatfs/lib/gcc/m4f/fatfs.a \
    "-L$(KERNEL_BUILD)/gcc" \
    -l:freertos.lib \
    -l:ti/devices/msp432e4/driverlib/lib/gcc/m4f/msp432e4_driverlib.a \
    -march=armv7e-m \
    -mthumb \
    -mfloat-abi=hard \
    -mfpu=fpv4-sp-d16 \
    -static \
    -Wl,--gc-sections \
    "-L$(GCC_ARMCOMPILER)/arm-none-eabi/lib/thumb/v7e-m/hard" \
    -lgcc \
    -lc \
    -lm \
    -lnosys \
    --specs=nano.specs

all: $(NAME).out

$(KERNEL_BUILD)/gcc/freertos.lib:
	@ $(ECHOBLANKLINE)
	@ echo $@ is not built.
	@ echo You can build it by issuing $(MAKE) in $(dir $@).
	@ $(ECHOBLANKLINE)

.INTERMEDIATE: syscfg
ti_ndk_config.c ti_net_config.c ti_drivers_config.c ti_drivers_config.h: syscfg
	@ echo generation complete

syscfg: ../../freertos/httpget.syscfg
	@ echo Generating configuration files...
	@ $(SYSCONFIG_TOOL) --compiler gcc --product $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/.metadata/product.json --output $(@D) $<

httpget.obj: ../../httpget.c ti_drivers_config.h
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

httpgethooks.obj: ../../httpgethooks.c ti_drivers_config.h
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

freertos_main_freertos.obj: ../../freertos/main_freertos.c ti_drivers_config.h
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

ti_ndk_config.obj: ti_ndk_config.c ti_drivers_config.h
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

ti_net_config.obj: ti_net_config.c ti_drivers_config.h
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

ti_drivers_config.obj: ti_drivers_config.c ti_drivers_config.h
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

$(NAME).out: $(OBJECTS) $(KERNEL_BUILD)/gcc/freertos.lib
	@ echo linking $@
	@ $(LNK)  $(OBJECTS)  $(LFLAGS) -o $(NAME).out

clean:
	@ echo Cleaning...
	@ $(RM) $(OBJECTS) > $(DEVNULL) 2>&1
	@ $(RM) $(NAME).out > $(DEVNULL) 2>&1
	@ $(RM) $(NAME).map > $(DEVNULL) 2>&1
	@ $(RM) ti_ndk_config.c ti_net_config.c ti_drivers_config.c ti_drivers_config.h > $(DEVNULL) 2>&1
