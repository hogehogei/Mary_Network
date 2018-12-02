################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/lib/memory/Exlib_BoundaryTag.cpp \
../src/lib/memory/Exlib_TLSFAllocator.cpp \
../src/lib/memory/allocator.cpp 

OBJS += \
./src/lib/memory/Exlib_BoundaryTag.o \
./src/lib/memory/Exlib_TLSFAllocator.o \
./src/lib/memory/allocator.o 

CPP_DEPS += \
./src/lib/memory/Exlib_BoundaryTag.d \
./src/lib/memory/Exlib_TLSFAllocator.d \
./src/lib/memory/allocator.d 


# Each subdirectory must supply rules for building sources it contributes
src/lib/memory/%.o: ../src/lib/memory/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C++ Compiler'
	arm-none-eabi-c++ -DDEBUG -D__CODE_RED -DCORE_M0 -D__LPC11XX__ -D__NEWLIB__ -I"C:\Users\hogehogei\Desktop\Mary_ENC28J60\Mary_ENC28J60\inc" -O0 -fno-common -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -fno-rtti -fno-exceptions -std=c++11 -mcpu=cortex-m0 -mthumb -D__NEWLIB__ -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


