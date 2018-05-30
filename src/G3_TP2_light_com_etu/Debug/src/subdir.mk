################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/command_decoder.c \
../src/cr_startup_lpc17.c \
../src/debug.c \
../src/leds.c \
../src/light_com.c \
../src/light_decoder.c \
../src/load.c 

OBJS += \
./src/command_decoder.o \
./src/cr_startup_lpc17.o \
./src/debug.o \
./src/leds.o \
./src/light_com.o \
./src/light_decoder.o \
./src/load.o 

C_DEPS += \
./src/command_decoder.d \
./src/cr_startup_lpc17.d \
./src/debug.d \
./src/leds.d \
./src/light_com.d \
./src/light_decoder.d \
./src/load.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -std=c99 -D__REDLIB__ -D__USE_CMSIS=CMSISv2p00_LPC17xx -DDEBUG -D__CODE_RED -DPACK_STRUCT_END=__attribute\(\(packed\)\) -DGCC_ARMCM3 -I../src -I"/home/david.gonzalez3/Work/PTR/TP_light_communication/src/CMSISv2p00_LPC17xx/inc" -I../FreeRTOS_include -I../FreeRTOS_portable -O0 -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -mcpu=cortex-m3 -mthumb -D__REDLIB__ -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


