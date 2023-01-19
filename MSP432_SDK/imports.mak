#
# Set location of various cgtools
#
# These variables can be set here or on the command line.
#
# The CCS_ARMCOMPILER, GCC_ARMCOMPILER, and IAR_ARMCOMPILER variables,
# in addition to pointing to their respective locations, also serve
# as "switches" for disabling a build using those cgtools. To disable a
# build using a specific cgtool, either set the cgtool's variable to
# empty or delete/comment-out its definition:
#     IAR_ARMCOMPILER ?=
# or
#     #IAR_ARMCOMPILER ?= c:/Program Files (x86)/IAR Systems/Embedded Workbench 8.0/arm
#
# If a cgtool's *_ARMCOMPILER variable is set (non-empty), various sub-makes
# in the installation will attempt to build with that cgtool.  This means
# that if multiple *_ARMCOMPILER cgtool variables are set, the sub-makes
# will build using each non-empty *_ARMCOMPILER cgtool.
#

XDC_INSTALL_DIR        ?= c:/ti/xdctools_3_61_01_25_core
SYSCONFIG_TOOL         ?= c:/ti/ccs1010/ccs/utils/sysconfig_1.5.0/sysconfig_cli.bat

FREERTOS_INSTALL_DIR   ?= C:\Users\dario\OneDrive\Documents\FreeRTOS\FreeRTOSv202212.00\FreeRTOSv202212.00

CCS_ARMCOMPILER        ?= c:/ti/ccs1010/ccs/tools/compiler/ti-cgt-arm_20.2.1.LTS
GCC_ARMCOMPILER        ?= c:/ti/ccs1010/ccs/tools/compiler/gcc-arm-none-eabi-9-2019-q4-major-win32
IAR_ARMCOMPILER        ?= c:/Program Files (x86)/IAR Systems/Embedded Workbench 8.4/arm

ifeq ("$(SHELL)","sh.exe")
# for Windows/DOS shell
    RM     = del
    RMDIR  = -rmdir /S /Q
    DEVNULL = NUL
    ECHOBLANKLINE = @cmd /c echo.
else
# for Linux-like shells
    RM     = rm -f
    RMDIR  = rm -rf
    DEVNULL = /dev/null
    ECHOBLANKLINE = echo
endif
