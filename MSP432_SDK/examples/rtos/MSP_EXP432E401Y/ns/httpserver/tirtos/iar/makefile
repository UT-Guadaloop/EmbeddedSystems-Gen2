SIMPLELINK_MSP432E4_SDK_INSTALL_DIR ?= $(abspath ../../../../../../..)

include $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/imports.mak

KERNEL_BUILD := $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/kernel/tirtos/builds/MSP_EXP432E401Y/release

CC = "$(IAR_ARMCOMPILER)/bin/iccarm"
LNK = "$(IAR_ARMCOMPILER)/bin/ilinkarm"

OBJECTS = httpSrvBasic.obj startsntp.obj urlmemzip.obj memzip.obj memzip_files.obj urlsimple.obj urlsimpleput.obj httpSrvBasicHooks.obj tirtos_main_tirtos.obj ti_ndk_config.obj ti_net_config.obj ti_drivers_config.obj

CONFIGPKG = $(KERNEL_BUILD)/iar

NAME = httpserver

CFLAGS = -I../.. \
    -I. \
    "-I$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/net/bsd" \
    "-I$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source" \
    "-I$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/CMSIS/Include" \
    "-I$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/posix/iar" \
    --debug \
    --silent \
    -e \
    --aeabi \
    --thumb \
    --diag_suppress=Pa050

LFLAGS = "$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/net/lib/iar/m4f/slnetsock_release.a" \
    "$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/net/sntp/lib/iar/m4f/sntp_release.a" \
    "$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/net/mqtt/lib/iar/m4f/mqtt_release.a" \
    "$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/net/http/lib/iar/m4f/httpclient_release.a" \
    "$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/net/http/lib/iar/m4f/httpserver_release.a" \
    "$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/ndk/slnetif/lib/slnetifndk_msp432e4.arm4f" \
    "$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/mbedtls/ti/lib/iar/m4f/mbedtls_msp432e4.a" \
    "$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/ndk/hal/timer_bios/lib/hal_timer.arm4f" \
    "$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/ndk/hal/eth_stub/lib/hal_eth_stub.arm4f" \
    "$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/ndk/tools/hdlc/lib/hdlc.arm4f" \
    "$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/ndk/tools/console/lib/console_min_ipv4.arm4f" \
    "$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/ndk/netctrl/lib/netctrl_min_ipv4.arm4f" \
    "$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/ndk/nettools/lib/nettool_ipv4.arm4f" \
    "$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/ndk/hal/ser_stub/lib/hal_ser_stub.arm4f" \
    "$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/ndk/hal/userled_stub/lib/hal_userled_stub.arm4f" \
    "$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/ndk/stack/lib/stk.arm4f" \
    "$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/ndk/os/lib/os.arm4f" \
    "$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/display/lib/display.arm4f" \
    "$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/grlib/lib/iar/m4f/grlib.a" \
    "$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/spiffs/lib/iar/m4f/spiffs.a" \
    "$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/drivers/lib/drivers_msp432e4.arm4f" \
    "$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/fatfs/lib/iar/m4f/fatfs.a" \
    "$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/kernel/tirtos/packages/ti/dpl/lib/dpl_msp432e4.arm4f" \
    -f "$(KERNEL_BUILD)/iar/linker.cmd" \
    "$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/devices/msp432e4/driverlib/lib/iar/m4f/msp432e4_driverlib.a" \
    --config ../../tirtos/iar/MSP_EXP432E401Y_TIRTOS.icf \
    --map "$(NAME).map" \
    --silent \
    --cpu=Cortex-M4F \
    --semihosting=iar_breakpoint

all: $(NAME).out

$(CONFIGPKG)/linker.cmd $(CONFIGPKG)/compiler.opt:
	@ $(ECHOBLANKLINE)
	@ echo $(CONFIGPKG) is not built.
	@ echo You can build it by issuing $(MAKE) in $(CONFIGPKG).
	@ $(ECHOBLANKLINE)

.INTERMEDIATE: syscfg
ti_ndk_config.c ti_net_config.c ti_drivers_config.c ti_drivers_config.h: syscfg
	@ echo generation complete

syscfg: ../../tirtos/httpserver.syscfg
	@ echo Generating configuration files...
	@ $(SYSCONFIG_TOOL) --compiler iar --product $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/.metadata/product.json --output $(@D) $<

httpSrvBasic.obj: ../../httpSrvBasic.c ti_drivers_config.h $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -f $(CONFIGPKG)/compiler.opt -o $@

startsntp.obj: ../../startsntp.c ti_drivers_config.h $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -f $(CONFIGPKG)/compiler.opt -o $@

urlmemzip.obj: ../../urlmemzip.c ti_drivers_config.h $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -f $(CONFIGPKG)/compiler.opt -o $@

memzip.obj: ../../memzip.c ti_drivers_config.h $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -f $(CONFIGPKG)/compiler.opt -o $@

memzip_files.obj: ../../memzip_files.c ti_drivers_config.h $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -f $(CONFIGPKG)/compiler.opt -o $@

urlsimple.obj: ../../urlsimple.c ti_drivers_config.h $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -f $(CONFIGPKG)/compiler.opt -o $@

urlsimpleput.obj: ../../urlsimpleput.c ti_drivers_config.h $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -f $(CONFIGPKG)/compiler.opt -o $@

httpSrvBasicHooks.obj: ../../httpSrvBasicHooks.c ti_drivers_config.h $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -f $(CONFIGPKG)/compiler.opt -o $@

tirtos_main_tirtos.obj: ../../tirtos/main_tirtos.c ti_drivers_config.h $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -f $(CONFIGPKG)/compiler.opt -o $@

ti_ndk_config.obj: ti_ndk_config.c ti_drivers_config.h $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -f $(CONFIGPKG)/compiler.opt -o $@

ti_net_config.obj: ti_net_config.c ti_drivers_config.h $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -f $(CONFIGPKG)/compiler.opt -o $@

ti_drivers_config.obj: ti_drivers_config.c ti_drivers_config.h $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -f $(CONFIGPKG)/compiler.opt -o $@

$(NAME).out: $(OBJECTS) $(CONFIGPKG)/linker.cmd
	@ echo linking $@
	@ $(LNK)  $(OBJECTS)  $(LFLAGS) -o $(NAME).out

clean:
	@ echo Cleaning...
	@ $(RM) $(OBJECTS) > $(DEVNULL) 2>&1
	@ $(RM) $(NAME).out > $(DEVNULL) 2>&1
	@ $(RM) $(NAME).map > $(DEVNULL) 2>&1
	@ $(RM) ti_ndk_config.c ti_net_config.c ti_drivers_config.c ti_drivers_config.h > $(DEVNULL) 2>&1
