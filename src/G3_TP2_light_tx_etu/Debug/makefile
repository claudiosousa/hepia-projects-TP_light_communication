################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: G3_TP2_light_tx_etu.axf

# Tool invocations
G3_TP2_light_tx_etu.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: MCU Linker'
	arm-none-eabi-gcc -nostdlib -L"/home/claudio/Documents/git/hepia/semestre_6/PTR/TPs/TP_light_communication/src/G3_TP2_light_tx_etu/Debug" -L"/home/claudio/Documents/git/hepia/semestre_6/PTR/TPs/TP_light_communication/src/CMSISv2p00_LPC17xx/Debug" -Xlinker -Map="G3_TP2_light_tx_etu.map" -Xlinker --gc-sections -mcpu=cortex-m3 -mthumb -T "G3_TP2_light_tx_etu_Debug.ld" -o "G3_TP2_light_tx_etu.axf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) G3_TP2_light_tx_etu.axf
	-@echo ' '

post-build:
	-@echo 'Performing post-build steps'
	-arm-none-eabi-size "G3_TP2_light_tx_etu.axf"; # arm-none-eabi-objcopy -O binary "G3_TP2_light_tx_etu.axf" "G3_TP2_light_tx_etu.bin" ; checksum -p LPC1769 -d "G3_TP2_light_tx_etu.bin";
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY: post-build

-include ../makefile.targets
