################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../include/SSD1306/glcdfont.c 

CPP_SRCS += \
../include/SSD1306/OLED.cpp \
../include/SSD1306/gfx.cpp \
../include/SSD1306/rgb_driver.cpp \
../include/SSD1306/ssd1306.cpp 

OBJS += \
./include/SSD1306/OLED.o \
./include/SSD1306/gfx.o \
./include/SSD1306/glcdfont.o \
./include/SSD1306/rgb_driver.o \
./include/SSD1306/ssd1306.o 

C_DEPS += \
./include/SSD1306/glcdfont.d 

CPP_DEPS += \
./include/SSD1306/OLED.d \
./include/SSD1306/gfx.d \
./include/SSD1306/rgb_driver.d \
./include/SSD1306/ssd1306.d 


# Each subdirectory must supply rules for building sources it contributes
include/SSD1306/%.o: ../include/SSD1306/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	arm-linux-gnueabihf-g++ -std=c++0x -I"/home/troy/workspace/pendulum/include" -I/usr/arm-linux-gnueabihf/include/c++/4.7.2 -O0 -g3 -Wall -c -fmessage-length=0 -pthread -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

include/SSD1306/%.o: ../include/SSD1306/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-linux-gnueabihf-gcc -std=c11 -I"/home/troy/workspace/pendulum/include" -I/usr/arm-linux-gnueabihf/include -O0 -g3 -Wall -c -fmessage-length=0 -pthread -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


