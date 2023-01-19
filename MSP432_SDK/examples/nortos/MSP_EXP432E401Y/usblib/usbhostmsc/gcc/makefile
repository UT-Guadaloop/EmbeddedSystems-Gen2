SIMPLELINK_MSP432E4_SDK_INSTALL_DIR ?= $(abspath ../../../../../..)

include $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/imports.mak

CC = "$(GCC_ARMCOMPILER)/bin/arm-none-eabi-gcc"
LNK = "$(GCC_ARMCOMPILER)/bin/arm-none-eabi-gcc"

OBJECTS = fat_usbmsc.obj fatsfile.obj usb_host_msc.obj cmdline.obj pinout.obj uartstdio.obj system_msp432e401y.obj gcc_startup_msp432e401y_gcc.obj

NAME = usbhostmsc

CFLAGS = -I.. \
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
    "-I$(GCC_ARMCOMPILER)/arm-none-eabi/include/newlib-nano" \
    "-I$(GCC_ARMCOMPILER)/arm-none-eabi/include"

LFLAGS = -Wl,--start-group \
    "-L$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/usblib/msp432e4/lib/gcc/m4f" \
    -l:msp432e4_usblib.a \
    "-L$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/drivers/lib" \
    -l:drivers_msp432e4.am4fg \
    "-L$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/fatfs/lib/gcc/m4f" \
    -l:fatfs.a \
    "-L$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/kernel/nortos/lib" \
    -l:nortos_msp432e4.am4fg \
    -Wl,--end-group \
    -Wl,-T,../gcc/msp432e401y.lds \
    "-Wl,-Map,$(NAME).map" \
    "-L$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source" \
    -l:ti/display/lib/display.am4fg \
    -l:ti/grlib/lib/gcc/m4f/grlib.a \
    -l:third_party/spiffs/lib/gcc/m4f/spiffs.a \
    -l:ti/drivers/lib/drivers_msp432e4.am4fg \
    -l:third_party/fatfs/lib/gcc/m4f/fatfs.a \
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

fat_usbmsc.obj: ../fat_usbmsc.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

fatsfile.obj: ../fatsfile.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

usb_host_msc.obj: ../usb_host_msc.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

cmdline.obj: ../cmdline.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

pinout.obj: ../pinout.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

uartstdio.obj: ../uartstdio.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

system_msp432e401y.obj: ../system_msp432e401y.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

gcc_startup_msp432e401y_gcc.obj: ../gcc/startup_msp432e401y_gcc.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

$(NAME).out: $(OBJECTS)
	@ echo linking $@
	@ $(LNK)  $(OBJECTS)  $(LFLAGS) -o $(NAME).out

clean:
	@ echo Cleaning...
	@ $(RM) $(OBJECTS) > $(DEVNULL) 2>&1
	@ $(RM) $(NAME).out > $(DEVNULL) 2>&1
	@ $(RM) $(NAME).map > $(DEVNULL) 2>&1
