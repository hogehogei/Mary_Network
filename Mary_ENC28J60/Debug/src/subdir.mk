################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/ENC28J60.cpp \
../src/Mary_ENC28J60.cpp \
../src/cr_cpp_config.cpp \
../src/cr_startup_lpc11xx.cpp \
../src/global.cpp \
../src/interrupt.cpp \
../src/led.cpp \
../src/network.cpp \
../src/spi.cpp \
../src/systick.cpp \
../src/timer32.cpp \
../src/uart.cpp 

C_SRCS += \
../src/crp.c 

OBJS += \
./src/ENC28J60.o \
./src/Mary_ENC28J60.o \
./src/cr_cpp_config.o \
./src/cr_startup_lpc11xx.o \
./src/crp.o \
./src/global.o \
./src/interrupt.o \
./src/led.o \
./src/network.o \
./src/spi.o \
./src/systick.o \
./src/timer32.o \
./src/uart.o 

CPP_DEPS += \
./src/ENC28J60.d \
./src/Mary_ENC28J60.d \
./src/cr_cpp_config.d \
./src/cr_startup_lpc11xx.d \
./src/global.d \
./src/interrupt.d \
./src/led.d \
./src/network.d \
./src/spi.d \
./src/systick.d \
./src/timer32.d \
./src/uart.d 

C_DEPS += \
./src/crp.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C++ Compiler'
	arm-none-eabi-c++ -DDEBUG -D__CODE_RED -DCORE_M0 -D__LPC11XX__ -D__NEWLIB__ -I"C:\Users\hogehogei\Desktop\Mary_ENC28J60\Mary_ENC28J60\inc" -O0 -fno-common -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -fno-rtti -fno-exceptions -std=c++11 -mcpu=cortex-m0 -mthumb -D__NEWLIB__ -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DDEBUG -D__CODE_RED -DCORE_M0 -D__LPC11XX__ -D__NEWLIB__ -I"C:\Users\hogehogei\Desktop\Mary_ENC28J60\Mary_ENC28J60\inc" -O0 -fno-common -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -mcpu=cortex-m0 -mthumb -D__NEWLIB__ -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


