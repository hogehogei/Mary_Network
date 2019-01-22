################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/drv/ethif.cpp \
../src/drv/gpio.cpp \
../src/drv/spi.cpp 

OBJS += \
./src/drv/ethif.o \
./src/drv/gpio.o \
./src/drv/spi.o 

CPP_DEPS += \
./src/drv/ethif.d \
./src/drv/gpio.d \
./src/drv/spi.d 


# Each subdirectory must supply rules for building sources it contributes
src/drv/%.o: ../src/drv/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C++ Compiler'
	arm-none-eabi-c++ -DDEBUG -D__CODE_RED -DCORE_M0 -D__LPC11XX__ -D__NEWLIB__ -I"C:\Users\hogehogei\Desktop\Mary_ENC28J60\Mary_ENC28J60\inc" -Og -fno-common -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -fno-rtti -fno-exceptions -std=c++11 -mcpu=cortex-m0 -mthumb -D__NEWLIB__ -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


