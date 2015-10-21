################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/overlays.cpp \
../src/pendulum.cpp \
../src/threadedEQEP.cpp 

OBJS += \
./src/overlays.o \
./src/pendulum.o \
./src/threadedEQEP.o 

CPP_DEPS += \
./src/overlays.d \
./src/pendulum.d \
./src/threadedEQEP.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	arm-linux-gnueabihf-g++ -std=c++0x -I"/home/troy/workspace/pendulum/include" -I/usr/arm-linux-gnueabihf/include/c++/4.7.2 -O0 -g3 -Wall -c -fmessage-length=0 -pthread -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


