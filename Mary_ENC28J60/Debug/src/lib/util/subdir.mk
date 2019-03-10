################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/lib/util/ARM_Util.cpp 

OBJS += \
./src/lib/util/ARM_Util.o 

CPP_DEPS += \
./src/lib/util/ARM_Util.d 


# Each subdirectory must supply rules for building sources it contributes
src/lib/util/%.o: ../src/lib/util/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C++ Compiler'
	arm-none-eabi-c++ -DDEBUG -D__CODE_RED -DCORE_M0 -D__LPC11XX__ -D__NEWLIB__ -I"C:\Users\hogehogei\Desktop\Mary_ENC28J60\Mary_ENC28J60\inc" -Og -fno-common -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -fno-rtti -fno-exceptions -std=c++11 -mcpu=cortex-m0 -mthumb -D__NEWLIB__ -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


