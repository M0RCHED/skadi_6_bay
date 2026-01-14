################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/services/services_cmd.c \
../Core/Src/services/services_identity.c \
../Core/Src/services/services_led.c \
../Core/Src/services/services_telemetry.c 

OBJS += \
./Core/Src/services/services_cmd.o \
./Core/Src/services/services_identity.o \
./Core/Src/services/services_led.o \
./Core/Src/services/services_telemetry.o 

C_DEPS += \
./Core/Src/services/services_cmd.d \
./Core/Src/services/services_identity.d \
./Core/Src/services/services_led.d \
./Core/Src/services/services_telemetry.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/services/%.o Core/Src/services/%.su Core/Src/services/%.cyclo: ../Core/Src/services/%.c Core/Src/services/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I../Core/Inc/app -I../Core/Inc/services -I../Core/Inc/drivers -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-services

clean-Core-2f-Src-2f-services:
	-$(RM) ./Core/Src/services/services_cmd.cyclo ./Core/Src/services/services_cmd.d ./Core/Src/services/services_cmd.o ./Core/Src/services/services_cmd.su ./Core/Src/services/services_identity.cyclo ./Core/Src/services/services_identity.d ./Core/Src/services/services_identity.o ./Core/Src/services/services_identity.su ./Core/Src/services/services_led.cyclo ./Core/Src/services/services_led.d ./Core/Src/services/services_led.o ./Core/Src/services/services_led.su ./Core/Src/services/services_telemetry.cyclo ./Core/Src/services/services_telemetry.d ./Core/Src/services/services_telemetry.o ./Core/Src/services/services_telemetry.su

.PHONY: clean-Core-2f-Src-2f-services

