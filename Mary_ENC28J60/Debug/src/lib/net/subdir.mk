################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/lib/net/arp.cpp \
../src/lib/net/ethernet.cpp \
../src/lib/net/ipv4.cpp \
../src/lib/net/packet.cpp 

OBJS += \
./src/lib/net/arp.o \
./src/lib/net/ethernet.o \
./src/lib/net/ipv4.o \
./src/lib/net/packet.o 

CPP_DEPS += \
./src/lib/net/arp.d \
./src/lib/net/ethernet.d \
./src/lib/net/ipv4.d \
./src/lib/net/packet.d 


# Each subdirectory must supply rules for building sources it contributes
src/lib/net/%.o: ../src/lib/net/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C++ Compiler'
	arm-none-eabi-c++ -DDEBUG -D__CODE_RED -DCORE_M0 -D__LPC11XX__ -D__NEWLIB__ -I"C:\Users\hogehogei\Desktop\Mary_ENC28J60\Mary_ENC28J60\inc" -O0 -fno-common -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -fno-rtti -fno-exceptions -std=c++11 -mcpu=cortex-m0 -mthumb -D__NEWLIB__ -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


