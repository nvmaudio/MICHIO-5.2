################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../app_src/components/audio/audio_effect.c \
../app_src/components/audio/audio_effect_class.c \
../app_src/components/audio/audio_effect_flash_param.c \
../app_src/components/audio/audio_effect_param.c \
../app_src/components/audio/audio_effect_process.c \
../app_src/components/audio/audio_effect_user.c \
../app_src/components/audio/audio_vol.c \
../app_src/components/audio/comm_param.c \
../app_src/components/audio/ctrlvars.c \
../app_src/components/audio/eq_drc_params.c \
../app_src/components/audio/eq_params.c 

OBJS += \
./app_src/components/audio/audio_effect.o \
./app_src/components/audio/audio_effect_class.o \
./app_src/components/audio/audio_effect_flash_param.o \
./app_src/components/audio/audio_effect_param.o \
./app_src/components/audio/audio_effect_process.o \
./app_src/components/audio/audio_effect_user.o \
./app_src/components/audio/audio_vol.o \
./app_src/components/audio/comm_param.o \
./app_src/components/audio/ctrlvars.o \
./app_src/components/audio/eq_drc_params.o \
./app_src/components/audio/eq_params.o 

C_DEPS += \
./app_src/components/audio/audio_effect.d \
./app_src/components/audio/audio_effect_class.d \
./app_src/components/audio/audio_effect_flash_param.d \
./app_src/components/audio/audio_effect_param.d \
./app_src/components/audio/audio_effect_process.d \
./app_src/components/audio/audio_effect_user.d \
./app_src/components/audio/audio_vol.d \
./app_src/components/audio/comm_param.d \
./app_src/components/audio/ctrlvars.d \
./app_src/components/audio/eq_drc_params.d \
./app_src/components/audio/eq_params.d 


# Each subdirectory must supply rules for building sources it contributes
app_src/components/audio/%.o: ../app_src/components/audio/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Andes C Compiler'
	$(CROSS_COMPILE)gcc -DFUNC_OS_EN=1 -DCFG_APP_CONFIG -DHAVE_CONFIG_H -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/middleware/cec/inc" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_src/app_mode_hdmi" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_src/components/audio" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_src/components/soft_watchdog" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_src/app_mode_spdif" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_framework/slow_device_engine" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_src/app_mode__common" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_src/app_mode_bt" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_src/app_mode_i2s" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_src/app_mode_idle" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_src/app_mode_linein" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_src/app_mode_media" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_src/app_mode_radio" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_src/app_mode_usb_audio" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_src/components/bluetooth" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_src/components/ble" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_src/components/flash_manage" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_src/components/rtc" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_src/components/upgrade" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_src/hmi/detect" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_src/hmi/display" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_src/hmi/fm" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_src/hmi/key" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_src/system_config" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/middleware/flashboot" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_framework/mode_engine" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_framework/bluetooth_engine" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_framework/audio_engine" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/driver/driver_api/inc" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/driver/driver_api/inc/otg" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/driver/driver/inc" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_src" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_src/power" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/middleware/rtc/inc" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/middleware/fatfs/inc" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/middleware/bluetooth/inc" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/middleware/audio/inc" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/middleware/mv_utils/inc" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/middleware/lrc/inc" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/middleware/rtos/rtos_api" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/middleware/rtos/freertos/inc" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/middleware/flashfs/inc" -Os1 -mcmodel=medium -g3 -Wall -mcpu=d1088-spu -c -fmessage-length=0 -ldsp -mext-dsp -std=gnu99 -ffunction-sections -fdata-sections -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d) $(@:%.o=%.o)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


