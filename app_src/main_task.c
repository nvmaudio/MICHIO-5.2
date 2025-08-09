/**
 **************************************************************************************
 * @file    main_task.c
 * @brief   Program Entry 
 *
 * @author  Sam
 * @version V1.0.0
 *
 * $Created: 2016-6-29 13:06:47$
 *
 * @Copyright (C) 2016, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#include <string.h>
#include "type.h"
#include "app_config.h"
#include "timeout.h"
#include "rtos_api.h"
#include "app_message.h"
#include "debug.h"
#include "dma.h"
#include "clk.h"
#include "main_task.h"
#include "timer.h"
#include "irqn.h"
#include "watchdog.h"
#include "dac.h"
#include "ctrlvars.h"
#include "delay.h"
#include "efuse.h"
#include "uarts_interface.h"
//functions
#include "powercontroller.h"
#include "deepsleep.h"
#include "flash_boot.h"
#include "breakpoint.h"
#include "remind_sound.h"
#include "rtc_alarm.h"
#include "rtc.h"
#include "audio_vol.h"
#include "device_detect.h"
#include "otg_detect.h"
#include "sadc_interface.h"
#include "adc.h"
#include "adc_key.h"
#include "ir_key.h"
#include "otg_device_hcd.h"
#include "otg_device_standard_request.h"
//services
#include "shell.h"
#include "audio_core_service.h"
#include "audio_core_api.h"
#include "bt_stack_service.h"
#include "bb_api.h"
#include "bt_config.h"
#include "bt_app_ddb_info.h"
#ifdef BT_TWS_SUPPORT
	#include "bt_tws_api.h"
	#include "bt_app_tws.h"
#endif
#include "bt_hf_mode.h"
#include "idle_mode.h"
#include "bt_app_tws_connect.h"
#include "tws_mode.h"
#include "bt_app_connect.h"
#include "backup_interface.h"
#include <components/soft_watchdog/soft_watch_dog.h>
#include "display_task.h"



#include "michio.h"


uint32_t SoftwareFlag;
MainAppContext	mainAppCt;


extern volatile uint32_t gInsertEventDelayActTimer;
extern void BtTwsDisconnectApi(void);
#ifdef BT_TWS_SUPPORT
	extern void tws_master_mute_send(bool MuteFlag);
#endif
#if FLASH_BOOT_EN
	void start_up_grate(uint32_t UpdateResource);
#endif
extern void PowerOnModeGenerate(void *BpSysInfo);


#define MAIN_APP_TASK_STACK_SIZE			1024		//512//1024
#ifdef	CFG_FUNC_OPEN_SLOW_DEVICE_TASK
	#define MAIN_APP_TASK_PRIO				4
	#define MAIN_APP_MSG_TIMEOUT			10	
#else
	#define MAIN_APP_TASK_PRIO				3
	#define MAIN_APP_MSG_TIMEOUT			1	
#endif
#define MAIN_APP_TASK_SLEEP_PRIO		6
#define MAIN_NUM_MESSAGE_QUEUE			20
#define SHELL_TASK_STACK_SIZE			512				//1024
#define SHELL_TASK_PRIO					2



static const uint8_t DmaChannelMap[29] = {
	255,//PERIPHERAL_ID_SPIS_RX = 0,	//0
	255,//PERIPHERAL_ID_SPIS_TX,		//1
#ifdef CFG_APP_HDMIIN_MODE_EN
	5,//PERIPHERAL_ID_TIMER3,			//2
#else
	255,//PERIPHERAL_ID_TIMER3,			//2
#endif

	255,//PERIPHERAL_ID_SDIO_RX,			//3
	255,//PERIPHERAL_ID_SDIO_TX,			//4

	255,//PERIPHERAL_ID_UART0_RX,		//5
	255,//PERIPHERAL_ID_TIMER1,			//6
	255,//PERIPHERAL_ID_TIMER2,			//7
#if defined CFG_RES_AUDIO_SPDIFOUT_EN || defined CFG_FUNC_SPDIF_MIX_MODE
	6,//PERIPHERAL_ID_SDPIF_RX,			//8 SPDIF_RX /TX same chanell
	6,//PERIPHERAL_ID_SDPIF_TX,			//8 SPDIF_RX /TX same chanell
#else
	255,//PERIPHERAL_ID_SDPIF_RX,		//8 SPDIF_RX /TX same chanell
	255,//PERIPHERAL_ID_SDPIF_TX,		//8 SPDIF_RX /TX same chanell
#endif
	255,//PERIPHERAL_ID_SPIM_RX,		//9
	255,//PERIPHERAL_ID_SPIM_TX,		//10
	255,//PERIPHERAL_ID_UART0_TX,		//11
	255,//PERIPHERAL_ID_UART1_RX,		//12
	255,//PERIPHERAL_ID_UART1_TX,		//13
	255,//PERIPHERAL_ID_TIMER4,			//14
	255,//PERIPHERAL_ID_TIMER5,			//15
	255,//PERIPHERAL_ID_TIMER6,			//16
	0,//PERIPHERAL_ID_AUDIO_ADC0_RX,	//17
	1,//PERIPHERAL_ID_AUDIO_ADC1_RX,	//18
	2,//PERIPHERAL_ID_AUDIO_DAC0_TX,	//19
	3,//PERIPHERAL_ID_AUDIO_DAC1_TX,	//20

#if defined (CFG_FUNC_I2S_MIX_MODE) && defined (CFG_RES_AUDIO_I2S0IN_EN)	
	6,//PERIPHERAL_ID_I2S0_RX,			//21
#else
	255,//PERIPHERAL_ID_I2S0_RX,		//21
#endif

#if	((defined(CFG_RES_AUDIO_I2SOUT_EN )&&(CFG_RES_I2S == 0)) || defined(CFG_RES_AUDIO_I2S0OUT_EN))
	7,//PERIPHERAL_ID_I2S0_TX,			//22
#else	
	255,//PERIPHERAL_ID_I2S0_TX,		//22
#endif	

#if defined (CFG_FUNC_I2S_MIX_MODE) && defined (CFG_RES_AUDIO_I2S1IN_EN)	
	4,//PERIPHERAL_ID_I2S1_RX,			//23
#else
	255,//PERIPHERAL_ID_I2S1_RX,		//23
#endif

#if	((defined(CFG_RES_AUDIO_I2SOUT_EN )&&(CFG_RES_I2S == 1))|| defined(CFG_RES_AUDIO_I2S1OUT_EN))
	5,	//PERIPHERAL_ID_I2S1_TX,		//24
#else
	255,//PERIPHERAL_ID_I2S1_TX,		//24
#endif

	255,//PERIPHERAL_ID_PPWM,			//25
	255,//PERIPHERAL_ID_ADC,     		//26
	255,//PERIPHERAL_ID_SOFTWARE,		//27
};

static void MainAppInit(void)
{
	memset(&mainAppCt, 0, sizeof(MainAppContext));

	mainAppCt.msgHandle = MessageRegister(MAIN_NUM_MESSAGE_QUEUE);
	mainAppCt.state = TaskStateCreating;
	mainAppCt.SysCurrentMode = ModeIdle;
	mainAppCt.SysPrevMode = ModeIdle;
}

static void SysVarInit(void)
{
	int16_t i;

	#ifdef CFG_FUNC_BREAKPOINT_EN
		BP_SYS_INFO *pBpSysInfo = NULL;

		pBpSysInfo = (BP_SYS_INFO *)BP_GetInfo(BP_SYS_INFO_TYPE);

		
	#ifdef BT_TWS_SUPPORT
		if(pBpSysInfo->CurModuleId != ModeBtAudioPlay)
		{
			tws_delay = BT_TWS_DELAY_DEINIT;
		}
		else
		{
			tws_delay = TWS_DELAY_FRAMES;
		}
	#endif
		mainAppCt.MusicVolume = pBpSysInfo->MusicVolume;
		if((mainAppCt.MusicVolume > CFG_PARA_MAX_VOLUME_NUM) || (mainAppCt.MusicVolume <= 0))
		{
			mainAppCt.MusicVolume = CFG_PARA_MAX_VOLUME_NUM;
		}
		APP_DBG("MusicVolume:%d,%d\n", mainAppCt.MusicVolume, pBpSysInfo->MusicVolume);	

	#ifdef CFG_EFFECT_PARAM_IN_FLASH_EN
		mainAppCt.EffectMode = EFFECT_MODE_FLASH_Music;
	#else
		mainAppCt.EffectMode = EFFECT_MODE_NORMAL;
	#endif	
		APP_DBG("EffectMode:%d,%d\n", mainAppCt.EffectMode, pBpSysInfo->EffectMode);
		
		mainAppCt.MicVolume = pBpSysInfo->MicVolume;
		if((mainAppCt.MicVolume > CFG_PARA_MAX_VOLUME_NUM) || (mainAppCt.MicVolume <= 0))
		{
			mainAppCt.MicVolume = CFG_PARA_MAX_VOLUME_NUM;
		}
		mainAppCt.MicVolumeBak = mainAppCt.MicVolume;
		APP_DBG("MicVolume:%d,%d\n", mainAppCt.MicVolume, pBpSysInfo->MicVolume);

		#ifdef CFG_APP_BT_MODE_EN
		mainAppCt.HfVolume = pBpSysInfo->HfVolume;
		if((mainAppCt.HfVolume > CFG_PARA_MAX_VOLUME_NUM) || (mainAppCt.HfVolume <= 0))
		{
			mainAppCt.HfVolume = CFG_PARA_MAX_VOLUME_NUM;
		}
		APP_DBG("HfVolume:%d,%d\n", mainAppCt.HfVolume, pBpSysInfo->HfVolume);
		#endif
		
	#ifdef CFG_FUNC_MUSIC_EQ_MODE_EN
		mainAppCt.EqMode = pBpSysInfo->EqMode;
		if(mainAppCt.EqMode > EQ_MODE_VOCAL_BOOST)
		{
			mainAppCt.EqMode = EQ_MODE_FLAT;
		}
		EqModeSet(mainAppCt.EqMode);
		#ifdef CFG_FUNC_EQMODE_FADIN_FADOUT_EN    
		mainAppCt.EqModeBak = mainAppCt.EqMode;
		mainAppCt.EqModeFadeIn = 0;
		mainAppCt.eqSwitchFlag = 0;
		#endif
		APP_DBG("EqMode:%d,%d\n", mainAppCt.EqMode, pBpSysInfo->EqMode);
	#endif

		mainAppCt.MicEffectDelayStep = pBpSysInfo->MicEffectDelayStep;
		if((mainAppCt.MicEffectDelayStep > MAX_MIC_EFFECT_DELAY_STEP) || (mainAppCt.MicEffectDelayStep <= 0))
		{
			mainAppCt.MicEffectDelayStep = MAX_MIC_EFFECT_DELAY_STEP;
		}
		mainAppCt.MicEffectDelayStepBak = mainAppCt.MicEffectDelayStep;
		APP_DBG("MicEffectDelayStep:%d,%d\n", mainAppCt.MicEffectDelayStep, pBpSysInfo->MicEffectDelayStep);
		
	#ifdef CFG_FUNC_MUSIC_TREB_BASS_EN	
		mainAppCt.MusicBassStep = pBpSysInfo->MusicBassStep;
		if((mainAppCt.MusicBassStep > MAX_MUSIC_DIG_STEP) || (mainAppCt.MusicBassStep <= 0))
		{
			mainAppCt.MusicBassStep = 7;
		}
		mainAppCt.MusicTrebStep = pBpSysInfo->MusicTrebStep;
		if((mainAppCt.MusicTrebStep > MAX_MUSIC_DIG_STEP) || (mainAppCt.MusicTrebStep <= 0))
		{
			mainAppCt.MusicTrebStep = 7;
		}
		APP_DBG("MusicTrebStep:%d,%d\n", mainAppCt.MusicTrebStep, pBpSysInfo->MusicTrebStep);
		APP_DBG("MusicBassStep:%d,%d\n", mainAppCt.MusicBassStep, pBpSysInfo->MusicBassStep);
	#endif

	#else

		
		mainAppCt.MusicVolume 		= sys_parameter.Volume_tong;
		mainAppCt.MusicVolume_VR 	= 0;

		mainAppCt.EffectMode 		= data_dem.mode;

		mainAppCt.MicVolume 		= sys_parameter.Volume_tong;

		#ifdef CFG_FUNC_MUSIC_TREB_BASS_EN	
			mainAppCt.MusicBassStep 	= 16;
			mainAppCt.MusicMidStep		= 16;
			mainAppCt.MusicTrebStep 	= 16;
		#endif	

		#include "adc_levels.h"


		if(sys_parameter.Nho_volume)
		{
			AudioMusicVol(data_dem.volume);
		}

		if(sys_parameter.Dongbo_volume)
		{
			AudioMusicVol(0);
		}
		
		if(sys_parameter.Vr_en)
		{
			load_vr();
			mainAppCt.MusicVolume = mainAppCt.MusicVolume_VR/2;
		}

	#endif


	#ifdef CFG_FUNC_BREAKPOINT_EN
		#ifdef CFG_APP_BT_MODE_EN
			if(SoftFlagGet(SoftFlagUpgradeOK))
			{
				pBpSysInfo->CurModuleId = ModeBtAudioPlay;
				DBG("u or sd upgrade ok ,set mode to ModeBtAudioPlay \n");
			}
		#endif
		PowerOnModeGenerate((void *)pBpSysInfo);
	#else
		PowerOnModeGenerate(NULL);
	#endif

    for(i = 0; i < AUDIO_CORE_SINK_MAX_NUM; i++)
	{
		mainAppCt.gSysVol.AudioSinkVol[i] = CFG_PARA_MAX_VOLUME_NUM;
	}

	

	for(i = 0; i < AUDIO_CORE_SOURCE_MAX_NUM; i++)
	{
		if(i == MIC_SOURCE_NUM)
		{
			mainAppCt.gSysVol.AudioSourceVol32[MIC_SOURCE_NUM] = 32 ;
			mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM] = mainAppCt.MicVolume;
		}
		else if(i == APP_SOURCE_NUM)
		{
			mainAppCt.gSysVol.AudioSourceVol32[APP_SOURCE_NUM] = mainAppCt.MusicVolume_VR;
			mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM] = mainAppCt.MusicVolume;
		}
		else if(i == REMIND_SOURCE_NUM)
		{
			mainAppCt.gSysVol.AudioSourceVol32[REMIND_SOURCE_NUM] = sys_parameter.Volume_thong_bao * 2;	
			mainAppCt.gSysVol.AudioSourceVol[REMIND_SOURCE_NUM] = sys_parameter.Volume_thong_bao;	
		}
		else
		{
			mainAppCt.gSysVol.AudioSourceVol32[i] = 32;
			mainAppCt.gSysVol.AudioSourceVol[i] = CFG_PARA_MAX_VOLUME_NUM;		
		}
	}

	mainAppCt.gSysVol.MuteFlag = TRUE;	
	
	#ifdef CFG_FUNC_SILENCE_AUTO_POWER_OFF_EN
		mainAppCt.Silence_Power_Off_Time = 0;
	#endif
}



static void SystemInit(void)
{
	int16_t i;
	DelayMsFunc = (DelayMsFunction)vTaskDelay;
	DMA_ChannelAllocTableSet((uint8_t*)DmaChannelMap);
	SarADC_Init();

	#if defined(CFG_FUNC_UDISK_DETECT)
		#if defined(CFG_FUNC_USB_DEVICE_EN)
			OTG_PortSetDetectMode(1,1);
		#else
			OTG_PortSetDetectMode(1,0);
		#endif
	#else
		#if defined(CFG_FUNC_USB_DEVICE_EN)
			OTG_PortSetDetectMode(0,1);
		#else
			OTG_PortSetDetectMode(0,0);
		#endif
	#endif

 	Timer_Config(TIMER2,1000,0);
 	Timer_Start(TIMER2);
 	NVIC_EnableIRQ(Timer2_IRQn);

	#ifdef CFG_FUNC_BREAKPOINT_EN
		BP_LoadInfo();
	#endif

	SysVarInit();
	
	#ifdef CFG_FUNC_BT_OTA_EN
		SoftFlagDeregister((~(SoftFlagUpgradeOK|SoftFlagBtOtaUpgradeOK))&SoftFlagMask);
	#else
		SoftFlagDeregister((~SoftFlagUpgradeOK)&SoftFlagMask);
	#endif

	CtrlVarsInit();

	#ifdef BT_TWS_SUPPORT
		TWS_Params_Init();
	#endif	


	mainAppCt.AudioCore =  (AudioCoreContext*)&AudioCore;
	memset(mainAppCt.AudioCore, 0, sizeof(AudioCoreContext));
	for(i = 0; i < MaxNet; i++)
	{
		AudioCoreFrameSizeSet(i, CFG_PARA_SAMPLES_PER_FRAME);
		AudioCoreMixSampleRateSet(i, CFG_PARA_SAMPLE_RATE);
	}
	mainAppCt.SampleRate = CFG_PARA_SAMPLE_RATE;
	
	SystemVolSet();
	
	for( i = 0; i < AUDIO_CORE_SOURCE_MAX_NUM; i++)
	{
	   mainAppCt.AudioCore->AudioSource[i].PreGain = 4095;
	}

	AudioCoreServiceCreate(mainAppCt.msgHandle);

	mainAppCt.AudioCoreSync = FALSE;

	#ifdef CFG_FUNC_REMIND_SOUND_EN	
		RemindSoundInit();
	#endif


	#ifdef CFG_APP_BT_MODE_EN
		if(sys_parameter.Bt_BackgroundType != BT_BACKGROUND_DISABLE)
		{
			BtStackServiceStart();
		}
	#endif

	DeviceServiceInit();
	#ifdef TWS_DAC0_OUT
		mainAppCt.DACFIFO_LEN = TWS_SINK_DEV_FIFO_SAMPLES * sizeof(PCM_DATA_TYPE) * 2;
		mainAppCt.DACFIFO = (uint32_t*)osPortMalloc(mainAppCt.DACFIFO_LEN);//DAC0 fifo
	#endif
	#ifdef TWS_DACX_OUT
		mainAppCt.DACXFIFO_LEN = TWS_SINK_DEV_FIFO_SAMPLES * sizeof(PCM_DATA_TYPE);
		mainAppCt.DACXFIFO = (uint32_t*)osPortMalloc(mainAppCt.DACXFIFO_LEN);//DACX fifo
	#endif
	#if defined(CFG_RES_AUDIO_I2SOUT_EN) && (defined(TWS_IIS0_OUT) || defined(TWS_IIS1_OUT))
		mainAppCt.I2SFIFO_LEN = TWS_SINK_DEV_FIFO_SAMPLES * sizeof(PCM_DATA_TYPE) * 2;
		mainAppCt.I2SFIFO = (uint32_t*)osPortMalloc(mainAppCt.I2SFIFO_LEN);
	#endif

	#ifdef CFG_RES_AUDIO_I2S0IN_EN
		mainAppCt.I2S0_RX_FIFO_LEN = AudioCoreFrameSizeGet(DefaultNet) * sizeof(PCM_DATA_TYPE) * 2 * 2; //I2S0 rx fifo
		mainAppCt.I2S0_RX_FIFO = (uint32_t*)osPortMallocFromEnd(mainAppCt.I2S0_RX_FIFO_LEN);//I2S0 rx fifo
	#endif
	#ifdef CFG_RES_AUDIO_I2S1IN_EN
		mainAppCt.I2S1_RX_FIFO_LEN = AudioCoreFrameSizeGet(DefaultNet) * sizeof(PCM_DATA_TYPE) * 2 * 2; //I2S1 rx fifo
		mainAppCt.I2S1_RX_FIFO = (uint32_t*)osPortMallocFromEnd(mainAppCt.I2S1_RX_FIFO_LEN);//I2S1 rx fifo
	#endif

	#ifdef CFG_RES_AUDIO_I2S0OUT_EN
		#ifdef TWS_IIS0_OUT
		mainAppCt.I2S0_TX_FIFO_LEN = TWS_SINK_DEV_FIFO_SAMPLES * sizeof(PCM_DATA_TYPE) * 2;				//I2S0 tx fifo
		#else
		mainAppCt.I2S0_TX_FIFO_LEN = AudioCoreFrameSizeGet(DefaultNet) * sizeof(PCM_DATA_TYPE) * 2 * 2;	//I2S0 tx fifo
		#endif
		mainAppCt.I2S0_TX_FIFO = (uint32_t*)osPortMalloc(mainAppCt.I2S0_TX_FIFO_LEN);
	#endif

	#ifdef CFG_RES_AUDIO_I2S1OUT_EN
		#ifdef TWS_IIS1_OUT
		mainAppCt.I2S1_TX_FIFO_LEN = TWS_SINK_DEV_FIFO_SAMPLES * sizeof(PCM_DATA_TYPE) * 2;				//I2S1 tx fifo
		#else
		mainAppCt.I2S1_TX_FIFO_LEN = AudioCoreFrameSizeGet(DefaultNet) * sizeof(PCM_DATA_TYPE) * 2 * 2;	//I2S0 tx fifo
		#endif
		mainAppCt.I2S1_TX_FIFO = (uint32_t*)osPortMalloc(mainAppCt.I2S1_TX_FIFO_LEN);
	#endif

	#ifdef CFG_FUNC_DISPLAY_TASK_EN
		DisplayServiceCreate();
	#endif

	#ifdef  CFG_FUNC_MAIN_DEEPSLEEP_EN
		SoftFlagRegister(SoftFlagIdleModeEnterSleep);
	#endif

	APP_DBG("MainApp:run\n");
}


//�����²�service created��Ϣ����Ϻ�start��Щservcie
static void MainAppServiceCreating(uint16_t msgId)
{
	if(msgId == MSG_AUDIO_CORE_SERVICE_CREATED)
	{
		APP_DBG("MainApp:AudioCore service created\n");
		mainAppCt.AudioCoreSync = TRUE;
	}
	
	if(mainAppCt.AudioCoreSync)
	{
		AudioCoreServiceStart();
		mainAppCt.AudioCoreSync = FALSE;
		mainAppCt.state = TaskStateReady;
	}
}

//�����²�service started����Ϻ�׼��ģʽ�л���
static void MainAppServiceStarting(uint16_t msgId)
{
	if(msgId == MSG_AUDIO_CORE_SERVICE_STARTED)
	{
		APP_DBG("MainApp:AudioCore service started\n");
		mainAppCt.AudioCoreSync = TRUE;
	}

	if(mainAppCt.AudioCoreSync)
	{
		mainAppCt.AudioCoreSync = FALSE;
		mainAppCt.state = TaskStateRunning;
		SysModeTaskCreat();
#ifdef	CFG_FUNC_OPEN_SLOW_DEVICE_TASK
		{
		extern	void CreatSlowDeviceTask(void);
		CreatSlowDeviceTask();
		}
#endif
	}
}

static void PublicDetect(void)
{
	#if defined(BT_SNIFF_ENABLE) && defined(CFG_APP_BT_MODE_EN)
			tws_sniff_check_adda_process();//���sniff��adda�Ƿ�ָ�����ѯ
	#endif

	#ifdef CFG_FUNC_SILENCE_AUTO_POWER_OFF_EN
			switch(GetSystemMode())
			{
				// Idle,Slave,HFP,USB Audio��ʡ��ػ�
				case ModeIdle:
				case ModeTwsSlavePlay:
				case ModeBtHfPlay:
				case ModeUsbDevicePlay:
				mainAppCt.Silence_Power_Off_Time = 0;
				break;

				// BT �����������ػ�
				case ModeBtAudioPlay:
				if(btManager.btLinkState)
					mainAppCt.Silence_Power_Off_Time = 0;
				break;

				default:
				break;
			}

			mainAppCt.Silence_Power_Off_Time++;
			if(mainAppCt.Silence_Power_Off_Time >= SILENCE_POWER_OFF_DELAY_TIME)
			{
				mainAppCt.Silence_Power_Off_Time = 0;
				APP_DBG("Silence Power Off!!\n");

				MessageContext		msgSend;
	#if	defined(CFG_IDLE_MODE_POWER_KEY) && (POWERKEY_MODE == POWERKEY_MODE_PUSH_BUTTON)
				msgSend.msgId = MSG_POWERDOWN;
				APP_DBG("msgSend.msgId = MSG_DEVICE_SERVICE_POWERDOWN\n");
	#elif defined(CFG_SOFT_POWER_KEY_EN)
				msgSend.msgId = MSG_SOFT_POWER;
				APP_DBG("msgSend.msgId = MSG_DEVICE_SERVICE_SOFT_POWEROFF\n");
	#elif defined(CFG_IDLE_MODE_DEEP_SLEEP)
				msgSend.msgId = MSG_DEEPSLEEP;
				APP_DBG("msgSend.msgId = MSG_DEVICE_SERVICE_DEEPSLEEP\n");
	#else
				msgSend.msgId = MSG_POWER;
				APP_DBG("msgSend.msgId = MSG_POWER\n");
	#endif
				MessageSend(GetMainMessageHandle(), &msgSend);
			}
	#endif

	#if FLASH_BOOT_EN
			#ifndef FUNC_UPDATE_CONTROL
			if(SoftFlagGet(SoftFlagMvaInCard) && GetSystemMode() == ModeCardAudioPlay && (!SoftFlagGet(SoftFlagUpgradeOK)))
			{
				APP_DBG("MainApp:updata file exist in Card\n");
				#ifdef FUNC_OS_EN
					if(SDIOMutex != NULL)
					{
						osMutexLock(SDIOMutex);
					}
				#endif
				start_up_grate(SysResourceCard);
				#ifdef FUNC_OS_EN
					if(SDIOMutex != NULL)
					{
						osMutexUnlock(SDIOMutex);
					}
				#endif
			}
			else if(SoftFlagGet(SoftFlagMvaInUDisk) && GetSystemMode() == ModeUDiskAudioPlay&& (!SoftFlagGet(SoftFlagUpgradeOK)))
			{
				APP_DBG("MainApp:updata file exist in Udisk\n");
				#ifdef FUNC_OS_EN
					if(UDiskMutex != NULL)
					{
						//osMutexLock(UDiskMutex);
						while(osMutexLock_1000ms(UDiskMutex) != 1)
						{
							WDG_Feed();
						}
					}
				#endif
				start_up_grate(SysResourceUDisk);
				#ifdef FUNC_OS_EN
					if(UDiskMutex != NULL)
					{
						osMutexUnlock(UDiskMutex);
					}
				#endif
			}
			#endif
	#endif
}



static void PublicMsgPross(MessageContext msg)
{
	switch(msg.msgId)
	{
		case MSG_AUDIO_CORE_SERVICE_CREATED:	
			MainAppServiceCreating(msg.msgId);
			break;
		
		case MSG_AUDIO_CORE_SERVICE_STARTED:
			MainAppServiceStarting(msg.msgId);
			break;
#if FLASH_BOOT_EN
		case MSG_DEVICE_SERVICE_CARD_OUT:
			SoftFlagDeregister(SoftFlagUpgradeOK);
			SoftFlagDeregister(SoftFlagMvaInCard);
			break;
		
		case MSG_DEVICE_SERVICE_U_DISK_OUT:
			SoftFlagDeregister(SoftFlagUpgradeOK);
			SoftFlagDeregister(SoftFlagMvaInUDisk);
			break;
		
		case MSG_UPDATE:
			//if(SoftFlagGet(SoftFlagUpgradeOK))break;
			#ifdef FUNC_UPDATE_CONTROL
				APP_DBG("MainApp:UPDATE MSG\n");
				//�豸�������ţ�Ԥ����mva���Ǽǣ����������γ�ʱȡ���Ǽǡ�
				if(SoftFlagGet(SoftFlagMvaInCard) && GetSystemMode() == ModeCardAudioPlay)
				{
					APP_DBG("MainApp:updata file exist in Card\n");
					start_up_grate(SysResourceCard);
				}
				else if(SoftFlagGet(SoftFlagMvaInUDisk) && GetSystemMode() == ModeUDiskAudioPlay)
				{
					APP_DBG("MainApp:updata file exist in Udisk\n");
					start_up_grate(SysResourceUDisk);
				}
			#endif
			break;
#endif		

#ifdef CFG_APP_IDLE_MODE_EN		
		case MSG_POWER:
		case MSG_POWERDOWN:
		case MSG_DEEPSLEEP:
		case MSG_BTSTACK_DEEPSLEEP:
#ifdef CFG_APP_HDMIIN_MODE_EN
			if(GetSystemMode() == ModeHdmiAudioPlay)
			{
				if(SoftFlagGet(SoftFlagDeepSleepMsgIsFromTV) == 0)//�ǵ��Ӷ˷����Ĺػ�
				{
					HDMI_CEC_ActivePowerOff(200);
				}
				SoftFlagDeregister(SoftFlagDeepSleepMsgIsFromTV);
			}
#endif
			if(GetSystemMode() != ModeIdle)
			{			
				if (msg.msgId == MSG_POWER){
					DBG("Main task MSG_POWER\n");
				}else if (msg.msgId == MSG_POWERDOWN){
					DBG("Main task MSG_POWERDOWN\n");
				}else if (msg.msgId == MSG_DEEPSLEEP){
					DBG("Main task MSG_DEEPSLEEP\n");
				}else if (msg.msgId == MSG_BTSTACK_DEEPSLEEP){
					DBG("Main task MSG_BTSTACK_DEEPSLEEP\n");
				}
				SendEnterIdleModeMsg();
			#if (defined(CFG_APP_BT_MODE_EN) && (BT_HFP_SUPPORT == ENABLE))
				if(GetSystemMode() == ModeBtHfPlay)
				{
					BtHfModeExit();				
				}
				extern void SetsBtHfModeEnterFlag(uint32_t flag);
				SetsBtHfModeEnterFlag(0);
			#endif		
			
#ifndef CFG_TWS_SOUNDBAR_APP
			#ifdef BT_TWS_SUPPORT
				BtStackServiceMsgSend(MSG_BT_STACK_TWS_PAIRING_STOP);
				BtReconnectTwsStop();

				if(btManager.twsState == BT_TWS_STATE_CONNECTED)
				{
					#ifdef TWS_POWEROFF_MODE_SYNC
					if (btManager.twsRole == BT_TWS_MASTER)
					{
						void tws_send_key_msg(uint16_t msg);
						tws_send_key_msg(msg.msgId);
						btManager.twsState = BT_TWS_STATE_NONE;		
					}
					#else
					tws_link_disconnect();
					#endif
				}
			#endif
#endif

			#ifdef	CFG_IDLE_MODE_POWER_KEY
				if(msg.msgId == MSG_POWERDOWN)
				{
					SoftFlagRegister(SoftFlagIdleModeEnterPowerDown);
				}
			#endif				
			
			#ifdef CFG_IDLE_MODE_DEEP_SLEEP
				if(msg.msgId == MSG_DEEPSLEEP)
				{					
					SoftFlagRegister(SoftFlagIdleModeEnterSleep);
				}
#ifdef BT_SNIFF_ENABLE
				if(msg.msgId == MSG_BTSTACK_DEEPSLEEP)
				{
					SoftFlagRegister(SoftFlagIdleModeEnterSleep);
					SoftFlagRegister(SoftFlagIdleModeEnterSniff);
				}
#endif

			#endif	
			}
			break;
#endif	

#ifdef CFG_SOFT_POWER_KEY_EN
		case MSG_SOFT_POWER:
			SoftKeyPowerOff();
			break;
#endif

#ifdef CFG_APP_BT_MODE_EN
		case MSG_BT_ENTER_DUT_MODE:
			BtEnterDutModeFunc();
			break;

		case MSG_BT_CLEAR_BT_LIST:
			memset(btManager.btLinkDeviceInfo,0,sizeof(btManager.btLinkDeviceInfo));
			BtDdb_EraseBtLinkInforMsg();
			break;		


#ifdef BT_SNIFF_ENABLE
	#ifndef BT_TWS_SUPPORT
			case MSG_BT_SNIFF:
				BtStartEnterSniffMode();
				break;
	#endif
#endif
//		case MSG_BTSTACK_DEEPSLEEP:
//			APP_DBG("MSG_BTSTACK_DEEPSLEEP\n");
//#ifdef BT_TWS_SUPPORT
//			{
//				MessageContext		msgSend;
//
//				msgSend.msgId		= MSG_BT_SNIFF;
//				MessageSend(GetMainMessageHandle(), &msgSend);
//			}
//#endif
//			break;

		case MSG_BTSTACK_BB_ERROR:
			APP_DBG("bb and bt stack reset\n");
			RF_PowerDownBySw();
			WDG_Feed();
			
			//reset bb and bt stack
			rwip_reset();
			BtStackServiceKill();
			WDG_Feed();
			vTaskDelay(50);
			RF_PowerDownByHw();
			WDG_Feed();

			//reset bb and bt stack
			BtStackServiceStart();
			BtStackServiceMsgSend(MSG_BTSTACK_BB_ERROR_RESTART);
			if(GetSystemMode() == ModeTwsSlavePlay)
			{
				MessageContext		msgSend;
				APP_DBG("Exit Tws Slave Mode\n");
				msgSend.msgId = MSG_DEVICE_SERVICE_TWS_SLAVE_DISCONNECT;
				MessageSend(GetMainMessageHandle(), &msgSend);
			}
			break;
#ifdef CFG_FUNC_BT_OTA_EN
		case MSG_BT_START_OTA:
			APP_DBG("\nMSG_BT_START_OTA\n");
			RF_PowerDownBySw();
			WDG_Feed();
			rwip_reset();
			BtStackServiceKill();
			WDG_Feed();
			vTaskDelay(50);
			RF_PowerDownByHw();
			WDG_Feed();
			start_up_grate(SysResourceBtOTA);
			break;
#endif
#if (BT_HFP_SUPPORT == ENABLE)
			case MSG_DEVICE_SERVICE_ENTER_BTHF_MODE:
			if(GetHfpState(BtCurIndex_Get()) >= BT_HFP_STATE_CONNECTED)
			{
				BtHfModeEnter();
			}
			break;
#endif
#ifdef BT_AUTO_ENTER_PLAY_MODE
		case MSG_BT_A2DP_STREAMING:
			if((GetSystemMode() != ModeBtAudioPlay)&&(GetSystemMode() != ModeBtHfPlay))
			{
				MessageContext		msgSend;
				
				APP_DBG("Enter Bt Audio Play Mode...\n");
				//ResourceRegister(AppResourceBtPlay);
				
				// Send message to main app
				msgSend.msgId		= MSG_DEVICE_SERVICE_BTPLAY_IN;
				MessageSend(GetMainMessageHandle(), &msgSend);
			}
			break;
#endif
#endif
	}

	#ifdef BT_TWS_SUPPORT
		tws_msg_process(msg.msgId);
	#endif
}

static void MainAppTaskEntrance(void * param)
{
	MessageContext		msg;
	SystemInit();
	while(1)
	{
		MessageRecv(mainAppCt.msgHandle, &msg, MAIN_APP_MSG_TIMEOUT);
		Michio();
		PublicDetect();
		PublicMsgPross(msg);
		WDG_Feed();
		
		if(mainAppCt.state == TaskStateRunning)
		{
			DeviceServicePocess(msg.msgId);
			if(msg.msgId != MSG_NONE)
			{
				SysModeGenerate(msg.msgId);
				MessageSend(GetSysModeMsgHandle(), &msg);	
			}
			SysModeChangeTimeoutProcess();
		}

		#if (defined(CFG_APP_BT_MODE_EN) && (BT_AVRCP_SONG_TRACK_INFOR == ENABLE))
				BtMediaInfoDisp();
		#endif

	}
}



int32_t MainAppTaskStart(void)
{
	MainAppInit();
	xTaskCreate(MainAppTaskEntrance, "MainApp", MAIN_APP_TASK_STACK_SIZE, NULL, MAIN_APP_TASK_PRIO, &mainAppCt.taskHandle);
	return 0;
}

MessageHandle GetMainMessageHandle(void)
{
	return mainAppCt.msgHandle;
}


uint32_t GetSystemMode(void)
{
	return mainAppCt.SysCurrentMode;
}

void SoftFlagRegister(uint32_t SoftEvent)
{
	SoftwareFlag |= SoftEvent;
	return ;
}

void SoftFlagDeregister(uint32_t SoftEvent)
{
	SoftwareFlag &= ~SoftEvent;
	return ;
}

bool SoftFlagGet(uint32_t SoftEvent)
{
	return SoftwareFlag & SoftEvent ? TRUE : FALSE;
}

void SamplesFrameUpdataMsg(void)
{
	MessageContext		msgSend;
	APP_DBG("SamplesFrameUpdataMsg\n");

	msgSend.msgId		= MSG_AUDIO_CORE_FRAME_SIZE_CHANGE;
    MessageSend(mainAppCt.msgHandle, &msgSend);
}

void EffectUpdataMsg(void)
{
	MessageContext		msgSend;
	APP_DBG("EffectUpdataMsg\n");

	msgSend.msgId		= MSG_AUDIO_CORE_EFFECT_CHANGE;
	MessageSend(mainAppCt.msgHandle, &msgSend);
}

uint32_t IsBtAudioMode(void)
{
	return (GetSysModeState(ModeBtAudioPlay) == ModeStateInit || GetSysModeState(ModeBtAudioPlay) == ModeStateRunning);
}

uint32_t IsBtHfMode(void)
{
	return (GetSysModeState(ModeBtHfPlay) == ModeStateInit || GetSysModeState(ModeBtHfPlay) == ModeStateRunning);
}

uint32_t IsBtTwsSlaveMode(void)
{
	return (GetSysModeState(ModeTwsSlavePlay) == ModeStateInit || GetSysModeState(ModeTwsSlavePlay) == ModeStateRunning);
}

uint32_t IsIdleModeReady(void)
{
	if(GetModeDefineState(ModeIdle)
#ifdef BT_SNIFF_ENABLE
		&& (!Bt_sniff_sniff_start_state_get())//sniff ״̬���ܽ�ֹidleģʽ���������ɼ���
#endif
		)
	{
		if(GetSysModeState(ModeIdle) == ModeStateInit || GetSysModeState(ModeIdle) == ModeStateRunning )
			return 1;
	}
	return 0;
}


void PowerOffMessage(void)
{
	MessageContext		msgSend;

#if	defined(CFG_IDLE_MODE_POWER_KEY) && (POWERKEY_MODE == POWERKEY_MODE_PUSH_BUTTON)
	msgSend.msgId = MSG_POWERDOWN;
	APP_DBG("msgSend.msgId = MSG_DEVICE_SERVICE_POWERDOWN\n");
#elif defined(CFG_SOFT_POWER_KEY_EN)
	msgSend.msgId = MSG_SOFT_POWER;
	APP_DBG("msgSend.msgId = MSG_DEVICE_SERVICE_SOFT_POWEROFF\n");
#elif	defined(CFG_IDLE_MODE_DEEP_SLEEP) 
	msgSend.msgId = MSG_DEEPSLEEP;
	APP_DBG("msgSend.msgId = MSG_DEVICE_SERVICE_DEEPSLEEP\n");
#else
	msgSend.msgId = MSG_POWER;
	APP_DBG("msgSend.msgId = MSG_POWER\n");
#endif

	MessageSend(GetMainMessageHandle(), &msgSend);
}



void BatteryLowMessage(void)
{
	MessageContext		msgSend;

	APP_DBG("msgSend.msgId = MSG_DEVICE_SERVICE_BATTERY_LOW\n");
	msgSend.msgId = MSG_DEVICE_SERVICE_BATTERY_LOW;
	MessageSend(GetMainMessageHandle(), &msgSend);
}

void TwsSlaveModeSwitchDeal(SysModeNumber pre, SysModeNumber Cur)
{
	#ifdef TWS_SLAVE_MODE_SWITCH_EN
		//null
		if(sys_parameter.Bt_BackgroundType == BT_BACKGROUND_FAST_POWER_ON_OFF)
		{
			if (pre == ModeTwsSlavePlay)
			{
				if((Cur != ModeBtAudioPlay)&&(Cur != ModeTwsSlavePlay))
				{
					BtFastPowerOff();
					BtStackServiceWaitClear();
				}
				else
				{
					BtStackServiceWaitResume();
				}
			}
		}
		else if(sys_parameter.Bt_BackgroundType == BT_BACKGROUND_DISABLE)
		{
			if(Cur != ModeBtAudioPlay)
			{
				BtPowerOff();
			}
		}
	#endif	
}

