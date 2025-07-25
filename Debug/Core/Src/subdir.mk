################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/freertos.c \
../Core/Src/gpio_driver.c \
../Core/Src/gps_app.c \
../Core/Src/heap_api.c \
../Core/Src/i2c_driver.c \
../Core/Src/main.c \
../Core/Src/modem_api.c \
../Core/Src/ring_buffer.c \
../Core/Src/stm32f4xx_hal_msp.c \
../Core/Src/stm32f4xx_hal_timebase_tim.c \
../Core/Src/stm32f4xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32f4xx.c \
../Core/Src/uart_api.c \
../Core/Src/uart_driver.c 

OBJS += \
./Core/Src/freertos.o \
./Core/Src/gpio_driver.o \
./Core/Src/gps_app.o \
./Core/Src/heap_api.o \
./Core/Src/i2c_driver.o \
./Core/Src/main.o \
./Core/Src/modem_api.o \
./Core/Src/ring_buffer.o \
./Core/Src/stm32f4xx_hal_msp.o \
./Core/Src/stm32f4xx_hal_timebase_tim.o \
./Core/Src/stm32f4xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32f4xx.o \
./Core/Src/uart_api.o \
./Core/Src/uart_driver.o 

C_DEPS += \
./Core/Src/freertos.d \
./Core/Src/gpio_driver.d \
./Core/Src/gps_app.d \
./Core/Src/heap_api.d \
./Core/Src/i2c_driver.d \
./Core/Src/main.d \
./Core/Src/modem_api.d \
./Core/Src/ring_buffer.d \
./Core/Src/stm32f4xx_hal_msp.d \
./Core/Src/stm32f4xx_hal_timebase_tim.d \
./Core/Src/stm32f4xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32f4xx.d \
./Core/Src/uart_api.d \
./Core/Src/uart_driver.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F401xE -DUSE_FULL_LL_DRIVER -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/freertos.cyclo ./Core/Src/freertos.d ./Core/Src/freertos.o ./Core/Src/freertos.su ./Core/Src/gpio_driver.cyclo ./Core/Src/gpio_driver.d ./Core/Src/gpio_driver.o ./Core/Src/gpio_driver.su ./Core/Src/gps_app.cyclo ./Core/Src/gps_app.d ./Core/Src/gps_app.o ./Core/Src/gps_app.su ./Core/Src/heap_api.cyclo ./Core/Src/heap_api.d ./Core/Src/heap_api.o ./Core/Src/heap_api.su ./Core/Src/i2c_driver.cyclo ./Core/Src/i2c_driver.d ./Core/Src/i2c_driver.o ./Core/Src/i2c_driver.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/modem_api.cyclo ./Core/Src/modem_api.d ./Core/Src/modem_api.o ./Core/Src/modem_api.su ./Core/Src/ring_buffer.cyclo ./Core/Src/ring_buffer.d ./Core/Src/ring_buffer.o ./Core/Src/ring_buffer.su ./Core/Src/stm32f4xx_hal_msp.cyclo ./Core/Src/stm32f4xx_hal_msp.d ./Core/Src/stm32f4xx_hal_msp.o ./Core/Src/stm32f4xx_hal_msp.su ./Core/Src/stm32f4xx_hal_timebase_tim.cyclo ./Core/Src/stm32f4xx_hal_timebase_tim.d ./Core/Src/stm32f4xx_hal_timebase_tim.o ./Core/Src/stm32f4xx_hal_timebase_tim.su ./Core/Src/stm32f4xx_it.cyclo ./Core/Src/stm32f4xx_it.d ./Core/Src/stm32f4xx_it.o ./Core/Src/stm32f4xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32f4xx.cyclo ./Core/Src/system_stm32f4xx.d ./Core/Src/system_stm32f4xx.o ./Core/Src/system_stm32f4xx.su ./Core/Src/uart_api.cyclo ./Core/Src/uart_api.d ./Core/Src/uart_api.o ./Core/Src/uart_api.su ./Core/Src/uart_driver.cyclo ./Core/Src/uart_driver.d ./Core/Src/uart_driver.o ./Core/Src/uart_driver.su

.PHONY: clean-Core-2f-Src

