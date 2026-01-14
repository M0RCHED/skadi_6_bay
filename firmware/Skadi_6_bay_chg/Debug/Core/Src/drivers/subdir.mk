################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/drivers/drivers_max17263.c \
../Core/Src/drivers/drivers_pca9634.c \
../Core/Src/drivers/drivers_tca9548a.c 

OBJS += \
./Core/Src/drivers/drivers_max17263.o \
./Core/Src/drivers/drivers_pca9634.o \
./Core/Src/drivers/drivers_tca9548a.o 

C_DEPS += \
./Core/Src/drivers/drivers_max17263.d \
./Core/Src/drivers/drivers_pca9634.d \
./Core/Src/drivers/drivers_tca9548a.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/drivers/%.o Core/Src/drivers/%.su Core/Src/drivers/%.cyclo: ../Core/Src/drivers/%.c Core/Src/drivers/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I../Core/Inc/app -I../Core/Inc/services -I../Core/Inc/drivers -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-drivers

clean-Core-2f-Src-2f-drivers:
	-$(RM) ./Core/Src/drivers/drivers_max17263.cyclo ./Core/Src/drivers/drivers_max17263.d ./Core/Src/drivers/drivers_max17263.o ./Core/Src/drivers/drivers_max17263.su ./Core/Src/drivers/drivers_pca9634.cyclo ./Core/Src/drivers/drivers_pca9634.d ./Core/Src/drivers/drivers_pca9634.o ./Core/Src/drivers/drivers_pca9634.su ./Core/Src/drivers/drivers_tca9548a.cyclo ./Core/Src/drivers/drivers_tca9548a.d ./Core/Src/drivers/drivers_tca9548a.o ./Core/Src/drivers/drivers_tca9548a.su

.PHONY: clean-Core-2f-Src-2f-drivers

