
#ifndef __APP_CONFIG_H__
#define __APP_CONFIG_H__

#include "type.h"

#ifndef Main_config

	#ifndef FUNC_OS_EN
		#define FUNC_OS_EN 			1
		#define CFG_APP_CONFIG
		#define HAVE_CONFIG_H
	#endif

	#define ENABLE						TRUE
	#define DISABLE						FALSE

	#define CFG_CHIP_BP1048B2
	#define CFG_SDK_VER_CHIPID (0xB1)
	#define CFG_SDK_MAJOR_VERSION (0)
	#define CFG_SDK_MINOR_VERSION (5)
	#define CFG_SDK_PATCH_VERSION (2)

	#define AUDIO_ONLY 0
	#define MIC_ONLY 1
	#define AUDIO_MIC 2

	#define ANA_INPUT_CH_NONE 0
	#define ANA_INPUT_CH_LINEIN1 1
	#define ANA_INPUT_CH_LINEIN2 2
	#define ANA_INPUT_CH_LINEIN3 3
	#define ANA_INPUT_CH_LINEIN4 4
	#define ANA_INPUT_CH_LINEIN5 5

	#define CFG_FUNC_SW_DEBUG_EN
	//#define CFG_FUNC_OPEN_SLOW_DEVICE_TASK

	/*
	* Lựa chọn nguồn xung nhịp; có thể sử dụng xung nhịp bên ngoài.
	* Lưu ý:
	* 		1. Nếu sử dụng I2S, I2S phải ở chế độ slave để hoạt động; DAC cần sử dụng xung nhịp bên ngoài.
	* 		2. Nếu đặt giá trị là 0, sử dụng xung nhịp nội bộ (mặc định của hệ thống, ổn định hơn).
	* 		3. Nếu đặt giá trị là 1, sử dụng MCLK của I2S0 làm xung nhịp hệ thống.
	* 		4. Nếu đặt giá trị là 2, sử dụng MCLK của I2S1 làm xung nhịp hệ thống.
	*/

	#define USE_MCLK_IN_MODE 0

	//#define CFG_AUDIO_WIDTH_24BIT

#endif // Thu gọn



//#define  GPIO_VR_3V3
//#define  LED_OUT


 
#define  TEST 
//#define  KEY_TEST

//#define  PUB_FIX

//#define  	LDOIN_3V3
#define 	GPIO_FLASH_BOOT 	GPIOA5






//********************************[SYS Thông Số]*********************************************

	#define VOLUME_DEFAULT 	16
	#define REMIND_VOLUME 	5

	#define ADC_KEY_EN 		ENABLE
	#define ADC_VR_EN 		ENABLE  //DISABLE



//********************************[LED]*********************************************
	#ifdef TEST
		#define PCM_EN 			GPIOA23

		#define LED_CPU 		GPIOA15
		#define LED_MODE 		GPIOA16

		//#define GPIO_PCM_SCK 	GPIOA0

		#define GPIO_MUTE 		GPIOA17
		#define GPIO_MUTE_PORT 	A

	#else

		#define LED_CPU 		GPIOA15
		#define LED_MODE 		GPIOA16

		#define GPIO_MUTE_PORT 	B
		#define GPIO_MUTE 		GPIOB1	

		#ifdef LED_OUT
			#define LED_AUX_OUT 	GPIOA31
			#define LED_BLE_OUT 	GPIOB0
			#define LED_PC_OUT 		GPIOB6
		#endif

		#ifdef GPIO_VR_3V3
			#define GPIO_VR 		GPIOB4
		#endif


	#endif




//===================================[ KEY EN ]====================================================
#define CFG_RES_ADC_KEY_SCAN

//===================================[ VR EN ]====================================================
#define CFG_ADC_LEVEL_KEY_EN

//===================================[ MODE EN ]====================================================
#define CFG_APP_BT_MODE_EN
#define CFG_APP_LINEIN_MODE_EN
#define CFG_APP_USB_AUDIO_MODE_EN
// #define CFG_APP_IDLE_MODE_EN
// #define CFG_APP_USB_PLAY_MODE_EN
// #define CFG_APP_CARD_PLAY_MODE_EN
// #define CFG_APP_RADIOIN_MODE_EN
// #define CFG_APP_I2SIN_MODE_EN
// #define CFG_APP_OPTICAL_MODE_EN
// #define CFG_APP_COAXIAL_MODE_EN
//  #define CFG_FUNC_SPDIF_MIX_MODE
//  #define CFG_APP_HDMIIN_MODE_EN

//===================================[ AUDIO OUT ]===================================================
#define CFG_RES_AUDIO_DAC0_EN
#define CFG_RES_AUDIO_DACX_EN
//#define CFG_RES_AUDIO_I2SOUT_EN

#define CFG_RES_AUDIO_I2S0OUT_EN
#define CFG_RES_AUDIO_I2S1OUT_EN



//===================================[ SAMPLE_RATE ]==================================================
// #define CFG_AUDIO_OUT_AUTO_SAMPLE_RATE_44100_48000
#define CFG_PARA_SAMPLE_RATE 			(44100)		// Tần số lấy mẫu mặc định là 44.1kHz
#define CFG_BTHF_PARA_SAMPLE_RATE 		(16000)	  	// Tần số lấy mẫu cho chế độ HFP, thường là 16kHz
#define CFG_PARA_SAMPLES_PER_FRAME 		(256)	  	// Kích thước mẫu mỗi khung (frame) trong hệ thống, mặc định là 256 mẫu
#define CFG_BTHF_PARA_SAMPLES_PER_FRAME (256) 		// Kích thước mẫu mỗi khung cho chế độ Bluetooth HFP
#define CFG_PARA_MIN_SAMPLES_PER_FRAME 	(256)  		// Giá trị tối thiểu của mẫu mỗi khung, đảm bảo độ trễ mic nhỏ. Lưu ý: Nếu đặt là 128, hiệu quả TWS có thể giảm.
#define CFG_PARA_MAX_SAMPLES_PER_FRAME 	(512)  		// Giá trị tối đa của mẫu mỗi khung, mặc định là 512 mẫu

//===================================[ VOLUME ]=======================================================
#if (BT_AVRCP_VOLUME_SYNC == ENABLE) ||  defined(CFG_APP_BT_MODE_EN)
	#define CFG_PARA_MAX_VOLUME_NUM 		(16)
	#define CFG_PARA_SYS_VOLUME_DEFAULT 	(12)
#else
	#define CFG_PARA_MAX_VOLUME_NUM 		(32)
	#define CFG_PARA_SYS_VOLUME_DEFAULT 	(25)
#endif

//===================================[ MIXER_SRC ]=======================================================
//    1. Khi bật chế độ này, hệ thống sẽ sử dụng mixer để kết hợp các tín hiệu âm thanh, mặc định là 44.1kHz.
//    2. Phiên bản mặc định yêu cầu bật mixer, nhưng có thể tắt nếu không cần thiết.
#define CFG_FUNC_MIXER_SRC_EN

//===================================[ FREQ_ADJUST ]=======================================================
// Chế độ điều chỉnh tần số này dùng để đồng bộ với hệ thống âm thanh. Khi bật, chỉ có thể sử dụng một nguồn tần số tại một thời điểm.
// #define	CFG_FUNC_FREQ_ADJUST
#ifdef CFG_FUNC_FREQ_ADJUST
	#define CFG_PARA_BT_FREQ_ADJUST
	#define CFG_PARA_HFP_FREQ_ADJUST
#endif

//===================================[ FLASH_PARAM ]=======================================================
// #define CFG_FUNC_FLASH_PARAM_ONLINE_TUNING_EN

//===================================[ LINEIN_MODE ]==================================================

#if defined(CFG_APP_LINEIN_MODE_EN)
	#define LINEIN_INPUT_CHANNEL ANA_INPUT_CH_LINEIN5
#endif

//===================================[ USB_AUDIO_MODE ]==================================================
#ifdef CFG_APP_USB_AUDIO_MODE_EN
	#define CFG_PARA_USB_MODE AUDIO_ONLY
	#define CFG_RES_AUDIO_USB_IN_EN
	#define CFG_PARA_AUDIO_USB_IN_SYNC
	#define CFG_PARA_AUDIO_USB_IN_SRC

	#if (CFG_PARA_USB_MODE != AUDIO_ONLY)
		#define CFG_RES_AUDIO_USB_OUT_EN
		#define CFG_PARA_AUDIO_USB_OUT_SYNC
		#define CFG_PARA_AUDIO_USB_OUT_SRC
		#define CFG_RES_AUDIO_USB_VOL_SET_EN
		// #define USB_READER_EN
	#endif
#endif

#define CFG_RES_MIC_SELECT (1) // 0=NO MIC, 1= MIC1, 2= MIC2, 3 = MCI1+MIC2

//===================================[ IDLE ]==================================================
#ifdef CFG_APP_IDLE_MODE_EN
#define CFG_IDLE_MODE_POWER_KEY	 // power key
#define CFG_IDLE_MODE_DEEP_SLEEP // deepsleep
#ifdef CFG_IDLE_MODE_POWER_KEY
#define BAKEUP_FIRST_ENTER_POWERDOWN				 // ��һ���ϵ���Ҫ����PowerKey
#define POWERKEY_MODE POWERKEY_MODE_SLIDE_SWITCH_LPD // POWERKEY_MODE_PUSH_BUTTON
#if (POWERKEY_MODE == POWERKEY_MODE_SLIDE_SWITCH_LPD) || (POWERKEY_MODE == POWERKEY_MODE_SLIDE_SWITCH_HPD)
#define POWERKEY_CNT 20
#else
#define POWERKEY_CNT 2000
#endif

#if (POWERKEY_MODE == POWERKEY_MODE_PUSH_BUTTON)
// powerkey���ö̰������ܡ�
#define USE_POWERKEY_PUSH_BUTTON_MSG_SP MSG_PLAY_PAUSE
#endif
#endif
#ifdef CFG_IDLE_MODE_DEEP_SLEEP
//		#define  CFG_FUNC_MAIN_DEEPSLEEP_EN		//ϵͳ������˯�ߣ��Ȼ��ѡ�

/*���ⰴ������,ע��CFG_PARA_WAKEUP_GPIO_IR�� ���Ѽ�IR_KEY_POWER����*/
#define CFG_PARA_WAKEUP_SOURCE_IR SYSWAKEUP_SOURCE9_IR // �̶�source9
/*ADCKey���� ���CFG_PARA_WAKEUP_GPIO_ADCKEY ���Ѽ�ADCKEYWAKEUP���ü����ƽ*/
#define CFG_PARA_WAKEUP_SOURCE_ADCKEY SYSWAKEUP_SOURCE1_GPIO // ��ʹ��ADC���ѣ������CFG_RES_ADC_KEY_SCAN ��CFG_RES_ADC_KEY_USE
#define CFG_PARA_WAKEUP_SOURCE_CEC SYSWAKEUP_SOURCE2_GPIO	 // HDMIר�ã�CFG_PARA_WAKEUP_GPIO_CEC
#define CFG_PARA_WAKEUP_SOURCE_POWERKEY SYSWAKEUP_SOURCE6_POWERKEY
#define CFG_PARA_WAKEUP_SOURCE_IOKEY1 SYSWAKEUP_SOURCE3_GPIO
#define CFG_PARA_WAKEUP_SOURCE_IOKEY2 SYSWAKEUP_SOURCE4_GPIO
#endif
#endif

//===================================[ I2S ]==================================================

#if defined(CFG_APP_I2SIN_MODE_EN) || defined(CFG_RES_AUDIO_I2SOUT_EN)
	#include "i2s.h"
	#define CFG_RES_I2S 		0
	#define CFG_RES_I2S_MODE 	0 

	#if (CFG_RES_I2S == 0)
		#define CFG_RES_I2S_MODULE I2S0_MODULE
	#elif (CFG_RES_I2S == 1)
		#define CFG_RES_I2S_MODULE I2S1_MODULE
	#endif

	#define CFG_PARA_I2S_SAMPLERATE 44100 
	#define CFG_FUNC_I2S_IN_SYNC_EN		 
	#define CFG_FUNC_I2S_OUT_SYNC_EN	
#endif


#if defined(CFG_RES_AUDIO_I2S0OUT_EN) || defined(CFG_RES_AUDIO_I2S1OUT_EN)
	#include "i2s.h"
	#define CFG_RES_I2S 		0
	#define CFG_RES_I2S_MODE 	0 
	#define CFG_PARA_I2S_SAMPLERATE 44100 
	#define CFG_FUNC_I2S_IN_SYNC_EN		 
	#define CFG_FUNC_I2S_OUT_SYNC_EN	
#endif


//===================================[ DECODER ]==================================================
// #define LOSSLESS_DECODER_HIGH_RESOLUTION

#define USE_MP3_DECODER
#define USE_SBC_DECODER
#define USE_AAC_DECODER
// #define USE_WMA_DECODER
// #define USE_WAV_DECODER
// #define USE_DTS_DECODER
// #define USE_FLAC_DECODER
// #define USE_AIF_DECODER
// #define USE_AMR_DECODER
// #define USE_APE_DECODER

//===================================[ EFFECT ]==================================================
#define CFG_FUNC_AUDIO_EFFECT_EN
#ifdef CFG_FUNC_AUDIO_EFFECT_EN

	// #define CFG_FUNC_ECHO_DENOISE
	// #define CFG_FUNC_MUSIC_EQ_MODE_EN
	#ifdef CFG_FUNC_MUSIC_EQ_MODE_EN
		//	#define MUSIC_EQ_DRC
		#define CFG_FUNC_EQMODE_FADIN_FADOUT_EN
	#endif

	#define CFG_FUNC_MUSIC_TREB_BASS_EN

	// #define CFG_FUNC_SILENCE_AUTO_POWER_OFF_EN
	#ifdef CFG_FUNC_SILENCE_AUTO_POWER_OFF_EN
		#define SILENCE_THRESHOLD 120
		#define SILENCE_POWER_OFF_DELAY_TIME 10 * 60 * 100
	#endif

	#define CFG_FUNC_AUDIO_EFFECT_ONLINE_TUNING_EN
	#ifdef CFG_FUNC_AUDIO_EFFECT_ONLINE_TUNING_EN

		#define CFG_EFFECT_MAJOR_VERSION (CFG_SDK_MAJOR_VERSION)
		#define CFG_EFFECT_MINOR_VERSION (CFG_SDK_MINOR_VERSION)
		#define CFG_EFFECT_USER_VERSION (CFG_SDK_PATCH_VERSION)

		#define CFG_COMMUNICATION_BY_USB // usb or uart
		#define CFG_COMMUNICATION_CRYPTO (0)
		#define CFG_COMMUNICATION_PASSWORD 0x11223344
	#endif

	#define CFG_EFFECT_PARAM_IN_FLASH_EN
	#ifdef CFG_EFFECT_PARAM_IN_FLASH_EN
		#define CFG_EFFECT_PARAM_IN_FLASH_MAX_NUM (10)
		#define CFG_EFFECT_PARAM_IN_FLASH_ACTIVCE_NUM (6)
		#ifdef CFG_FUNC_AUDIO_EFFECT_ONLINE_TUNING_EN
			#define CFG_EFFECT_PARAM_UPDATA_BY_ACPWORKBENCH
		#endif
	#endif

#endif

//===================================[ USB_PLAY ]==================================================
#if (defined(CFG_APP_USB_PLAY_MODE_EN) || defined(CFG_APP_CARD_PLAY_MODE_EN) || BT_AVRCP_SONG_TRACK_INFOR)
/**LRC��ʹ��� **/
// #define CFG_FUNC_LRC_EN			 	// LRC����ļ�����

/*------browser function------*/
// #define FUNC_BROWSER_PARALLEL_EN  		//browser Parallel
// #define FUNC_BROWSER_TREE_EN  			//browser tree
#if defined(FUNC_BROWSER_PARALLEL_EN) || defined(FUNC_BROWSER_TREE_EN)
#define FUNCTION_FILE_SYSTEM_REENTRY
#if defined(FUNC_BROWSER_TREE_EN) || defined(FUNC_BROWSER_PARALLEL_EN)
#define GUI_ROW_CNT_MAX 5 // �����ʾ������
#else
#define GUI_ROW_CNT_MAX 1 // �����ʾ������
#endif
#endif
/*------browser function------*/

/**�ַ���������ת�� **/
/**Ŀǰ֧��Unicode     ==> Simplified Chinese (DBCS)**/
/**�ַ�ת������fatfs�ṩ������Ҫ�����ļ�ϵͳ**/
/**���֧��ת���������ԣ���Ҫ�޸�fatfs���ñ�**/
/**ע�⣺ ֧���ֿ⹦����Ҫʹ�� flash����>=2M��оƬ**/
/**�����˹��ܣ���ο��ĵ���24bit sdk�����ֿ�ʹ��˵��.pdf���������õ��޸� **/
// #define CFG_FUNC_STRING_CONVERT_EN	// ֧���ַ�����ת��

/**ȡ��AA55�ж�**/
/*fatfs�ڴ���ϵͳMBR��DBR������β�д˱�Ǽ�⣬Ϊ��߷Ǳ������̼����ԣ��ɿ�������, Ϊ��Ч������Ч�̣�����Ĭ�Ϲر�*/
// #define	CANCEL_COMMON_SIGNATURE_JUDGMENT
// #define FUNC_UPDATE_CONTROL   //�����������̿���(ͨ������ȷ������)
#endif

/**USB Host**/
#if (defined(CFG_APP_USB_PLAY_MODE_EN))
	#define CFG_RES_UDISK_USE
	#define CFG_FUNC_UDISK_DETECT
#endif

/**USB Device**/
#if (defined(CFG_APP_USB_AUDIO_MODE_EN)) || (defined(CFG_COMMUNICATION_BY_USB))
	#define CFG_FUNC_USB_DEVICE_EN
#endif

//===================================[ bt_config ]==================================================
#include "bt_config.h"

#ifdef BT_TWS_SUPPORT
	#ifdef CFG_RES_AUDIO_DAC0_EN
		#define TWS_DAC0_OUT 1
	#endif
	#ifdef CFG_RES_AUDIO_DACX_EN
		#define TWS_DACX_OUT 2
	#endif

	#if (defined(CFG_RES_AUDIO_I2SOUT_EN) && (CFG_RES_I2S == 0)) || defined(CFG_RES_AUDIO_I2S0OUT_EN)
		#define TWS_IIS0_OUT 3
	#endif

	#if (defined(CFG_RES_AUDIO_I2SOUT_EN) && (CFG_RES_I2S == 1)) || defined(CFG_RES_AUDIO_I2S1OUT_EN)
		#define TWS_IIS1_OUT 4
	#endif

	#ifdef BT_TWS_SUPPORT
		#define TWS_SINK_DEV_FIFO_SAMPLES (CFG_PARA_MAX_SAMPLES_PER_FRAME * 2)
	#else
		#define TWS_SINK_DEV_FIFO_SAMPLES (CFG_PARA_SAMPLES_PER_FRAME * 2)
	#endif
	#define TWS_AUDIO_OUT_PATH 	TWS_DAC0_OUT // TWS_IIS0_OUT//TWS_IIS1_OUT

#else
	#define TWS_AUDIO_OUT_PATH 0xff
#endif

#define DAC_RESET_SET 0

//===================================[ LOW_POWER ]==================================================
// #define CFG_FUNC_IDLE_TASK_LOW_POWER
#ifdef CFG_FUNC_IDLE_TASK_LOW_POWER
	#define CFG_GOTO_SLEEP_USE
#endif

//===================================[ DEBUG ]==================================================
// #define CFG_FUNC_SHELL_EN
#define CFG_FUNC_DEBUG_EN
#ifdef CFG_FUNC_DEBUG_EN
	// 38400 57600 115200 256000 512000 1000000

	#define CFG_UART_TX_PORT 		(0) 		// A6
	#define CFG_UART_BANDRATE 		115200
#endif

//===================================[ REMIND ]==================================================
#define CFG_FUNC_REMIND_SOUND_EN
#ifdef CFG_FUNC_REMIND_SOUND_EN
#define CFG_PARAM_FIXED_REMIND_VOL 10
#endif

//===================================[ BREAKPOINT ]==================================================
//#define CFG_FUNC_BREAKPOINT_EN
#ifdef CFG_FUNC_BREAKPOINT_EN
#define BP_PART_SAVE_TO_NVM
#define BP_SAVE_TO_FLASH
#define FUNC_MATCH_PLAYER_BP
#endif

//===================================[ BEEP ]==================================================
// #define  CFG_FUNC_BEEP_EN
#define CFG_PARA_BEEP_DEFAULT_VOLUME 15

//===================================[ DBCLICK ]==================================================
//#define CFG_FUNC_DBCLICK_MSG_EN
#ifdef CFG_FUNC_DBCLICK_MSG_EN
	#define CFG_PARA_CLICK_MSG 			MSG_PLAY_PAUSE
	#define CFG_PARA_DBCLICK_MSG 		MSG_BT_HF_REDAIL_LAST_NUM
	#define CFG_PARA_DBCLICK_DLY_TIME 	35
#endif

//===================================[ ADC ]==================================================
/**ADC**/
#if defined(CFG_RES_ADC_KEY_SCAN) || defined(CFG_PARA_WAKEUP_SOURCE_ADCKEY)
	#define CFG_RES_ADC_KEY_USE
#endif
// #define CFG_RES_IR_KEY_SCAN
//  #define	CFG_RES_CODE_KEY_USE
//  #define CFG_RES_IO_KEY_SCAN
//  #define CFG_ADC_LEVEL_KEY_EN

#ifndef BT_TWS_SUPPORT
#ifdef CFG_CHIP_BP1064L2
#define CFG_FUNC_RTC_EN
#else
#ifndef CFG_FUNC_BACKUP_EN
// #define CFG_FUNC_RTC_EN
#endif
#endif

#ifdef CFG_FUNC_RTC_EN
#define CFG_RES_RTC_EN
#ifdef CFG_RES_RTC_EN
#define CFG_PARA_RTC_SRC_OSC32K
#ifdef CFG_PARA_RTC_SRC_OSC32K
#define CFG_FUNC_OSC32K_STARTUP_DETECT
#endif
#endif

#define CFG_FUNC_ALARM_EN
#define CFG_FUNC_LUNAR_EN
#ifdef CFG_FUNC_ALARM_EN
#define CFG_FUNC_SNOOZE_EN
#endif
#endif
#endif

//===================================[ DISPLAY_TASK ]==================================================
// #define CFG_FUNC_DISPLAY_TASK_EN
// #define CFG_FUNC_DISPLAY_EN
#ifdef CFG_FUNC_DISPLAY_EN
//  #define  DISP_DEV_SLED
#define DISP_DEV_7_LED
#ifdef DISP_DEV_7_LED
#define CFG_FUNC_LED_REFRESH
#endif

#if defined(DISP_DEV_SLED) && defined(DISP_DEV_7_LED) && defined(LED_IO_TOGGLE)
#error Conflict: display setting error
#endif
#endif

// #define    CFG_FUNC_DETECT_IPHONE
// #define  CFG_FUNC_DETECT_PHONE_EN
// #define  CFG_FUNC_DETECT_MIC_SEG_EN

#if defined(CFG_FUNC_SHELL_EN) && defined(CFG_USE_SW_UART)
#error Conflict: shell  X  SW UART No RX!
#endif

// #define  CFG_FUNC_BT_OTA_EN

#include "sys_gpio.h"
#endif /* APP_CONFIG_H_ */
