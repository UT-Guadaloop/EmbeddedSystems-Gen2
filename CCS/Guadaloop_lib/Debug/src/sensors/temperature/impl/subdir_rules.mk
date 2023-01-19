################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
src/sensors/temperature/impl/%.obj: ../src/sensors/temperature/impl/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs1040/ccs/tools/compiler/ti-cgt-arm_20.2.5.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/ti/ccs1040/ccs/ccs_base/arm/include" --include_path="C:/ti/ccs1040/ccs/ccs_base/arm/include/CMSIS" --include_path="C:/Guadaloop_Gen2/CCS/Drivers" --include_path="C:/ti/ccs1040/ccs/tools/compiler/ti-cgt-arm_20.2.5.LTS/include" --advice:power=all --define=ccs --define=__MSP432E401Y__ -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="src/sensors/temperature/impl/$(basename $(<F)).d_raw" --obj_directory="src/sensors/temperature/impl" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


