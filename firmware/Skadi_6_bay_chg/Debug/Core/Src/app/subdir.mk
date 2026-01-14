################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/app/app.c \
../Core/Src/app/board_io.c 

OBJS += \
./Core/Src/app/app.o \
./Core/Src/app/board_io.o 

C_DEPS += \
./Core/Src/app/app.d \
./Core/Src/app/board_io.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/app/%.o Core/Src/app/%.su Core/Src/app/%.cyclo: ../Core/Src/app/%.c Core/Src/app/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I../Core/Inc/app -I../Core/Inc/services -I../Core/Inc/drivers -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-app

clean-Core-2f-Src-2f-app:
	-$(RM) ./Core/Src/app/app.cyclo ./Core/Src/app/app.d ./Core/Src/app/app.o ./Core/Src/app/app.su ./Core/Src/app/board_io.cyclo ./Core/Src/app/board_io.d ./Core/Src/app/board_io.o ./Core/Src/app/board_io.su

.PHONY: clean-Core-2f-Src-2f-app

