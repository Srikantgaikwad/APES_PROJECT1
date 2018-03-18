################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/light.c \
../src/main.c \
../src/temperature.c \
../src/ledlight.c 

OBJS += \
./src/light.o \
./src/main.o \
./src/temperature.o \
./src/ledlight.o 

C_DEPS += \
./src/light.d \
./src/main.d \
./src/temperature.d \
./src/ledlight.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	arm-linux-gnueabihf-gcc -I/usr/arm-linux-gnueabihf/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


