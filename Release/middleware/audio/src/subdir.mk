################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../middleware/audio/src/audio_decoder_api.c \
../middleware/audio/src/bits.c \
../middleware/audio/src/libmp2dec.c \
../middleware/audio/src/mvstdio.c 

OBJS += \
./middleware/audio/src/audio_decoder_api.o \
./middleware/audio/src/bits.o \
./middleware/audio/src/libmp2dec.o \
./middleware/audio/src/mvstdio.o 

C_DEPS += \
./middleware/audio/src/audio_decoder_api.d \
./middleware/audio/src/bits.d \
./middleware/audio/src/libmp2dec.d \
./middleware/audio/src/mvstdio.d 


# Each subdirectory must supply rules for building sources it contributes
middleware/audio/src/%.o: ../middleware/audio/src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Andes C Compiler'
	$(CROSS_COMPILE)gcc -DFUNC_OS_EN=1 -DCFG_APP_CONFIG -DHAVE_CONFIG_H -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/middleware/cec/inc" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_src/app_mode_hdmi" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_src/components/audio" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_src/components/soft_watchdog" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_src/app_mode_spdif" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_framework/slow_device_engine" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_src/app_mode__common" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_src/app_mode_bt" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_src/app_mode_i2s" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_src/app_mode_idle" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_src/app_mode_linein" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_src/app_mode_media" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_src/app_mode_radio" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_src/app_mode_usb_audio" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_src/components/bluetooth" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_src/components/ble" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_src/components/flash_manage" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_src/components/rtc" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_src/components/upgrade" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_src/hmi/detect" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_src/hmi/display" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_src/hmi/fm" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_src/hmi/key" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_src/system_config" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/middleware/flashboot" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_framework/mode_engine" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_framework/bluetooth_engine" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_framework/audio_engine" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/driver/driver_api/inc" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/driver/driver_api/inc/otg" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/driver/driver/inc" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_src" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/app_src/power" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/middleware/rtc/inc" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/middleware/fatfs/inc" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/middleware/bluetooth/inc" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/middleware/audio/inc" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/middleware/mv_utils/inc" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/middleware/lrc/inc" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/middleware/rtos/rtos_api" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/middleware/rtos/freertos/inc" -I"/cygdrive/C/Users/Michio/Desktop/MICHIO 5.2/middleware/flashfs/inc" -Os1 -mcmodel=medium -g3 -Wall -mcpu=d1088-spu -c -fmessage-length=0 -ldsp -mext-dsp -std=gnu99 -ffunction-sections -fdata-sections -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d) $(@:%.o=%.o)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


