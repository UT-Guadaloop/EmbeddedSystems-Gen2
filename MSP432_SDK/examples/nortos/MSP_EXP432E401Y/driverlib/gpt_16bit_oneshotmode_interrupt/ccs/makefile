SIMPLELINK_MSP432E4_SDK_INSTALL_DIR ?= $(abspath ../../../../../..)

include $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/imports.mak

CC = "$(CCS_ARMCOMPILER)/bin/armcl"
LNK = "$(CCS_ARMCOMPILER)/bin/armcl"

OBJECTS = gpt_16bit_oneshotmode_interrupt.obj system_msp432e401y.obj ccs_startup_msp432e401y_ccs.obj

NAME = gpt_16bit_oneshotmode_interrupt

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

LFLAGS = --stack_size=512 \
    "-i$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source" \
    -lti/display/lib/display.aem4f \
    -lti/grlib/lib/ccs/m4f/grlib.a \
    -lthird_party/spiffs/lib/ccs/m4f/spiffs.a \
    -lti/drivers/lib/drivers_msp432e4.aem4f \
    -lthird_party/fatfs/lib/ccs/m4f/fatfs.a \
    -lti/devices/msp432e4/driverlib/lib/ccs/m4f/msp432e4_driverlib.a \
    ../ccs/msp432e401y.cmd \
    "-m$(NAME).map" \
    --warn_sections \
    --display_error_number \
    --diag_wrap=off \
    --rom_model \
    "-i$(CCS_ARMCOMPILER)/lib" \
    -llibc.a

all: $(NAME).out

gpt_16bit_oneshotmode_interrupt.obj: ../gpt_16bit_oneshotmode_interrupt.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< --output_file=$@

system_msp432e401y.obj: ../system_msp432e401y.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< --output_file=$@

ccs_startup_msp432e401y_ccs.obj: ../ccs/startup_msp432e401y_ccs.c
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
