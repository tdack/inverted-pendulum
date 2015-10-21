################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../include/bbb-eqep/bbb-eqep.cpp 

OBJS += \
./include/bbb-eqep/bbb-eqep.o 

CPP_DEPS += \
./include/bbb-eqep/bbb-eqep.d 


# Each subdirectory must supply rules for building sources it contributes
include/bbb-eqep/%.o: ../include/bbb-eqep/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	arm-linux-gnueabihf-g++ -std=c++0x -I"/home/troy/workspace/pendulum/include" -I/usr/arm-linux-gnueabihf/include/c++/4.7.2 -O0 -g3 -Wall -c -fmessage-length=0 -pthread -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


