SIMPLELINK_MSP432E4_SDK_INSTALL_DIR ?= $(abspath ../../../../../../..)

include $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/imports.mak

KERNEL_BUILD := $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/kernel/tirtos/builds/MSP_EXP432E401Y/release

CC = "$(CCS_ARMCOMPILER)/bin/armcl"
LNK = "$(CCS_ARMCOMPILER)/bin/armcl"

OBJECTS = USBKBD.obj usbkeyboarddevice.obj ti_drivers_config.obj ti_usblib_config.obj tirtos_main_tirtos.obj

CONFIGPKG = $(KERNEL_BUILD)/ccs

NAME = usb_keyboard_device

CFLAGS = -I../.. \
    -I. \
    "-I$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/CMSIS/Include" \
    "-I$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source" \
    "-I$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/net/bsd" \
    --advice:power=none \
    -DDeviceFamily_MSP432E4 \
    -D__MSP432E401Y__ \
    "-I$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/posix/ccs" \
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

LFLAGS = "-l$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/usblib/msp432e4/lib/ccs/m4f/msp432e4_usblib.a" \
    "-i$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source" \
    -lti/display/lib/display.aem4f \
    -lti/grlib/lib/ccs/m4f/grlib.a \
    -lthird_party/spiffs/lib/ccs/m4f/spiffs.a \
    -lti/drivers/lib/drivers_msp432e4.aem4f \
    -lthird_party/fatfs/lib/ccs/m4f/fatfs.a \
    -lti/net/lib/ccs/m4f/slnetsock_release.a \
    -lti/net/sntp/lib/ccs/m4f/sntp_release.a \
    -lti/net/mqtt/lib/ccs/m4f/mqtt_release.a \
    -lti/net/http/lib/ccs/m4f/httpclient_release.a \
    -lti/ndk/slnetif/lib/slnetifndk.aem4f \
    -lthird_party/mbedtls/ti/lib/ccs/m4f/mbedtls.a \
    -lti/ndk/hal/timer_bios/lib/hal_timer.aem4f \
    -lti/ndk/hal/eth_stub/lib/hal_eth_stub.aem4f \
    -lti/ndk/tools/hdlc/lib/hdlc.aem4f \
    -lti/ndk/tools/console/lib/console_min_ipv4.aem4f \
    -lti/ndk/netctrl/lib/netctrl_min_ipv4.aem4f \
    -lti/ndk/nettools/lib/nettool_ipv4.aem4f \
    -lti/ndk/hal/ser_stub/lib/hal_ser_stub.aem4f \
    -lti/ndk/tools/servers/lib/servers_min_ipv4.aem4f \
    -lti/ndk/hal/userled_stub/lib/hal_userled_stub.aem4f \
    -lti/ndk/stack/lib/stk.aem4f \
    -lti/ndk/os/lib/os.aem4f \
    "-i$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/kernel/tirtos/packages" \
    -lti/dpl/lib/dpl_msp432e4.aem4f \
    "-l$(KERNEL_BUILD)/ccs/linker.cmd" \
    -lti/devices/msp432e4/driverlib/lib/ccs/m4f/msp432e4_driverlib.a \
    ../../tirtos/ccs/MSP_EXP432E401Y_TIRTOS.cmd \
    "-m$(NAME).map" \
    --warn_sections \
    --display_error_number \
    --diag_wrap=off \
    --rom_model \
    "-i$(CCS_ARMCOMPILER)/lib" \
    -llibc.a

all: $(NAME).out

$(CONFIGPKG)/linker.cmd $(CONFIGPKG)/compiler.opt:
	@ $(ECHOBLANKLINE)
	@ echo $(CONFIGPKG) is not built.
	@ echo You can build it by issuing $(MAKE) in $(CONFIGPKG).
	@ $(ECHOBLANKLINE)

.INTERMEDIATE: syscfg
ti_drivers_config.c ti_usblib_config.c ti_drivers_config.h ti_usblib_config.h: syscfg
	@ echo generation complete

syscfg: ../../usb_keyboard_device.syscfg
	@ echo Generating configuration files...
	@ $(SYSCONFIG_TOOL) --compiler ccs --product $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/.metadata/product.json --output $(@D) $<

USBKBD.obj: ../../USBKBD.c ti_drivers_config.h ti_usblib_config.h $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< --cmd_file=$(CONFIGPKG)/compiler.opt --output_file=$@

usbkeyboarddevice.obj: ../../usbkeyboarddevice.c ti_drivers_config.h ti_usblib_config.h $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< --cmd_file=$(CONFIGPKG)/compiler.opt --output_file=$@

tirtos_main_tirtos.obj: ../../tirtos/main_tirtos.c ti_drivers_config.h ti_usblib_config.h $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< --cmd_file=$(CONFIGPKG)/compiler.opt --output_file=$@

ti_drivers_config.obj: ti_drivers_config.c ti_drivers_config.h ti_usblib_config.h $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< --cmd_file=$(CONFIGPKG)/compiler.opt --output_file=$@

ti_usblib_config.obj: ti_usblib_config.c ti_drivers_config.h ti_usblib_config.h $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< --cmd_file=$(CONFIGPKG)/compiler.opt --output_file=$@

$(NAME).out: $(OBJECTS) $(CONFIGPKG)/linker.cmd
	@ echo linking $@
	@ $(LNK) -z $(OBJECTS)  $(LFLAGS) -o $(NAME).out

clean:
	@ echo Cleaning...
	@ $(RM) $(OBJECTS) > $(DEVNULL) 2>&1
	@ $(RM) $(NAME).out > $(DEVNULL) 2>&1
	@ $(RM) $(NAME).map > $(DEVNULL) 2>&1
	@ $(RM) ti_drivers_config.c ti_usblib_config.c ti_drivers_config.h ti_usblib_config.h > $(DEVNULL) 2>&1
