################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
targetConfigs/%.obj: ../targetConfigs/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs1250/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/ti/ccs1250/ccs/ccs_base/arm/include" --include_path="C:/ti/ccs1250/ccs/ccs_base/arm/include/CMSIS" --include_path="C:/Users/gibbonec/ECE230/ece230finalGibbonsE" --include_path="C:/ti/ccs1250/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/include" --advice:power=all --define=__MSP432P4111__ --define=ccs -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="targetConfigs/$(basename $(<F)).d_raw" --obj_directory="targetConfigs" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


