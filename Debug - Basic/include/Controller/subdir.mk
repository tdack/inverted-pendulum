################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../include/Controller/basic.cpp \
../include/Controller/lqr.cpp \
../include/Controller/velocity.cpp 

OBJS += \
./include/Controller/basic.o \
./include/Controller/lqr.o \
./include/Controller/velocity.o 

CPP_DEPS += \
./include/Controller/basic.d \
./include/Controller/lqr.d \
./include/Controller/velocity.d 


# Each subdirectory must supply rules for building sources it contributes
include/Controller/%.o: ../include/Controller/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	arm-linux-gnueabihf-g++ -std=c++0x -I"/home/troy/workspace/pendulum/include" -I/usr/arm-linux-gnueabihf/include/c++/4.7.2 -O0 -g3 -Wall -c -fmessage-length=0 -pthread -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


