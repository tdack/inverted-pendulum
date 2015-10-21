################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include include/bbb-eqep/subdir.mk
-include include/SSD1306/subdir.mk
-include include/Pololu/subdir.mk
-include include/Controller/subdir.mk
-include include/BlackLib/BlackUART/subdir.mk
-include include/BlackLib/BlackTime/subdir.mk
-include include/BlackLib/BlackThread/subdir.mk
-include include/BlackLib/BlackSPI/subdir.mk
-include include/BlackLib/BlackPWM/subdir.mk
-include include/BlackLib/BlackMutex/subdir.mk
-include include/BlackLib/BlackI2C/subdir.mk
-include include/BlackLib/BlackGPIO/subdir.mk
-include include/BlackLib/BlackDirectory/subdir.mk
-include include/BlackLib/BlackADC/subdir.mk
-include include/BlackLib/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: pendulum

# Tool invocations
pendulum: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Cross G++ Linker'
	arm-linux-gnueabihf-g++ -L/usr/arm-linux-gnueabihf/lib -pthread -o "pendulum" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(CC_DEPS)$(C++_DEPS)$(EXECUTABLES)$(OBJS)$(C_UPPER_DEPS)$(CXX_DEPS)$(C_DEPS)$(CPP_DEPS) pendulum
	-@echo ' '

post-build:
	-@echo 'Copy executable to BBB'
	-scp pendulum root@beaglebone.local:/var/lib/cloud9/code/inverted-pendulum/
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY: post-build

-include ../makefile.targets
