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
all: TP2_light_Tx_etu.axf

# Tool invocations
TP2_light_Tx_etu.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: MCU Linker'
	arm-none-eabi-gcc -nostdlib -L"C:\Users\vince\Documents\LPCXpresso_5.2.6_2137\ptr\TP2_light_Tx_etu\Debug" -L"C:\Users\vince\Documents\LPCXpresso_5.2.6_2137\ptr\CMSISv2p00_LPC17xx\Debug" -Xlinker -Map="TP2_light_Tx_etu.map" -Xlinker --gc-sections -mcpu=cortex-m3 -mthumb -T "Gx_TP2_emetteur_lumineux_Debug.ld" -o "TP2_light_Tx_etu.axf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) TP2_light_Tx_etu.axf
	-@echo ' '

post-build:
	-@echo 'Performing post-build steps'
	-arm-none-eabi-size "TP2_light_Tx_etu.axf"; # arm-none-eabi-objcopy -O binary "TP2_light_Tx_etu.axf" "TP2_light_Tx_etu.bin" ; checksum -p LPC1769 -d "TP2_light_Tx_etu.bin";
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY: post-build

-include ../makefile.targets
