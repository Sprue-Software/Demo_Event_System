################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../gecko_sdk_4.0.2/platform/micrium_os/common/source/ring_buf/ring_buf.c 

OBJS += \
./gecko_sdk_4.0.2/platform/micrium_os/common/source/ring_buf/ring_buf.o 

C_DEPS += \
./gecko_sdk_4.0.2/platform/micrium_os/common/source/ring_buf/ring_buf.d 


# Each subdirectory must supply rules for building sources it contributes
gecko_sdk_4.0.2/platform/micrium_os/common/source/ring_buf/ring_buf.o: ../gecko_sdk_4.0.2/platform/micrium_os/common/source/ring_buf/ring_buf.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m33 -mthumb -std=c99 '-DDEBUG_EFM=1' '-DEFM32PG23B310F512IM48=1' '-DSL_BOARD_NAME="BRD2504A"' '-DSL_BOARD_REV="A03"' '-DSL_COMPONENT_CATALOG_PRESENT=1' -I"C:\Users\ndiwathe\SimplicityStudio\v5_workspace\Techem_2\app\inc" -I"C:\Users\ndiwathe\SimplicityStudio\v5_workspace\Techem_2\gecko_sdk_4.0.2\platform\service\power_manager\inc" -I"C:\Users\ndiwathe\SimplicityStudio\v5_workspace\Techem_2\gecko_sdk_4.0.2\platform\service\hfxo_manager\inc" -I"C:\Users\ndiwathe\SimplicityStudio\v5_workspace\Techem_2" -I"C:\Users\ndiwathe\SimplicityStudio\v5_workspace\Techem_2\gecko_sdk_4.0.2\platform\Device\SiliconLabs\EFM32PG23\Include" -I"C:\Users\ndiwathe\SimplicityStudio\v5_workspace\Techem_2\gecko_sdk_4.0.2\platform\common\inc" -I"C:\Users\ndiwathe\SimplicityStudio\v5_workspace\Techem_2\gecko_sdk_4.0.2\hardware\board\inc" -I"C:\Users\ndiwathe\SimplicityStudio\v5_workspace\Techem_2\gecko_sdk_4.0.2\platform\CMSIS\Include" -I"C:\Users\ndiwathe\SimplicityStudio\v5_workspace\Techem_2\gecko_sdk_4.0.2\platform\CMSIS\RTOS2\Include" -I"C:\Users\ndiwathe\SimplicityStudio\v5_workspace\Techem_2\gecko_sdk_4.0.2\platform\service\device_init\inc" -I"C:\Users\ndiwathe\SimplicityStudio\v5_workspace\Techem_2\gecko_sdk_4.0.2\platform\emdrv\dmadrv\inc" -I"C:\Users\ndiwathe\SimplicityStudio\v5_workspace\Techem_2\gecko_sdk_4.0.2\platform\emdrv\common\inc" -I"C:\Users\ndiwathe\SimplicityStudio\v5_workspace\Techem_2\gecko_sdk_4.0.2\platform\emlib\inc" -I"C:\Users\ndiwathe\SimplicityStudio\v5_workspace\Techem_2\gecko_sdk_4.0.2\platform\emdrv\gpiointerrupt\inc" -I"C:\Users\ndiwathe\SimplicityStudio\v5_workspace\Techem_2\gecko_sdk_4.0.2\platform\driver\leddrv\inc" -I"C:\Users\ndiwathe\SimplicityStudio\v5_workspace\Techem_2\gecko_sdk_4.0.2\platform\micrium_os\common\source" -I"C:\Users\ndiwathe\SimplicityStudio\v5_workspace\Techem_2\gecko_sdk_4.0.2\platform\micrium_os\common\include" -I"C:\Users\ndiwathe\SimplicityStudio\v5_workspace\Techem_2\gecko_sdk_4.0.2\platform\micrium_os\cpu\include" -I"C:\Users\ndiwathe\SimplicityStudio\v5_workspace\Techem_2\gecko_sdk_4.0.2\platform\micrium_os\ports\source" -I"C:\Users\ndiwathe\SimplicityStudio\v5_workspace\Techem_2\gecko_sdk_4.0.2\platform\micrium_os" -I"C:\Users\ndiwathe\SimplicityStudio\v5_workspace\Techem_2\gecko_sdk_4.0.2\platform\micrium_os\kernel\source" -I"C:\Users\ndiwathe\SimplicityStudio\v5_workspace\Techem_2\gecko_sdk_4.0.2\platform\micrium_os\kernel\include" -I"C:\Users\ndiwathe\SimplicityStudio\v5_workspace\Techem_2\gecko_sdk_4.0.2\platform\peripheral\inc" -I"C:\Users\ndiwathe\SimplicityStudio\v5_workspace\Techem_2\gecko_sdk_4.0.2\platform\common\toolchain\inc" -I"C:\Users\ndiwathe\SimplicityStudio\v5_workspace\Techem_2\gecko_sdk_4.0.2\platform\service\system\inc" -I"C:\Users\ndiwathe\SimplicityStudio\v5_workspace\Techem_2\gecko_sdk_4.0.2\platform\service\sleeptimer\inc" -I"C:\Users\ndiwathe\SimplicityStudio\v5_workspace\Techem_2\gecko_sdk_4.0.2\platform\emdrv\uartdrv\inc" -I"C:\Users\ndiwathe\SimplicityStudio\v5_workspace\Techem_2\autogen" -I"C:\Users\ndiwathe\SimplicityStudio\v5_workspace\Techem_2\config" -Os -Wall -Wextra -fno-builtin -ffunction-sections -fdata-sections -imacrossl_gcc_preinclude.h -mfpu=fpv5-sp-d16 -mfloat-abi=hard -c -fmessage-length=0 -MMD -MP -MF"gecko_sdk_4.0.2/platform/micrium_os/common/source/ring_buf/ring_buf.d" -MT"gecko_sdk_4.0.2/platform/micrium_os/common/source/ring_buf/ring_buf.o" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


