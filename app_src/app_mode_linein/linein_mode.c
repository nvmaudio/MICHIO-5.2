/**
 **************************************************************************************
 * @file    linein_mode.c
 * @brief
 *
 * @author  Sam
 * @version V1.0.0
 *
 * $Created: 2017-12-26 13:06:47$
 *
 * @Copyright (C) 2016, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#include <string.h>
#include "type.h"
#include "irqn.h"
#include "gpio.h"
#include "dma.h"
#include "rtos_api.h"
#include "app_message.h"
#include "app_config.h"
#include "debug.h"
#include "delay.h"
#include "audio_adc.h"
#include "dac.h"
#include "adc_interface.h"
#include "dac_interface.h"
#include "audio_core_api.h"
#include "audio_core_service.h"
#include "decoder.h"
#include "remind_sound.h"
#include "main_task.h"
#include "audio_effect.h"
#include "powercontroller.h"
#include "deepsleep.h"
#include "backup_interface.h"
#include "breakpoint.h"
#include "otg_device_hcd.h"
#include "otg_device_standard_request.h"
#include "audio_vol.h"
#include "ctrlvars.h"
#include "main_task.h"
#include "reset.h"
#include "mode_task_api.h"
#include "audio_effect_flash_param.h"

#include "sys_param.h"
#include "michio.h"

#ifdef CFG_APP_LINEIN_MODE_EN

#define LINEIN_PLAY_TASK_STACK_SIZE 512 // 1024
#define LINEIN_PLAY_TASK_PRIO 3
#define LINEIN_NUM_MESSAGE_QUEUE 10

#define LINEIN_SOURCE_NUM APP_SOURCE_NUM

typedef struct _LineInPlayContext
{
	uint32_t 			*ADCFIFO;
	AudioCoreContext 	*AudioCoreLineIn;
	uint32_t			SampleRate;
} LineInPlayContext;

static const uint8_t sDmaChannelMap[29] = {
	255, // PERIPHERAL_ID_SPIS_RX = 0,	//0
	255, // PERIPHERAL_ID_SPIS_TX,		//1
	255, // PERIPHERAL_ID_TIMER3,		//2
	4, // PERIPHERAL_ID_SDIO_RX,		//3
	4, // PERIPHERAL_ID_SDIO_TX,		//4
	255, // PERIPHERAL_ID_UART0_RX,		//5
	255, // PERIPHERAL_ID_TIMER1,		//6
	255, // PERIPHERAL_ID_TIMER2,		//7
	255, // PERIPHERAL_ID_SDPIF_RX,		//8 
	255, // PERIPHERAL_ID_SDPIF_TX,		//8
	255, // PERIPHERAL_ID_SPIM_RX,		//9
	255, // PERIPHERAL_ID_SPIM_TX,		//10
	255, // PERIPHERAL_ID_UART0_TX,		//11
	255, // PERIPHERAL_ID_UART1_RX,		//12
	255, // PERIPHERAL_ID_UART1_TX,		//13
	255, // PERIPHERAL_ID_TIMER4,		//14
	255, // PERIPHERAL_ID_TIMER5,		//15
	255, // PERIPHERAL_ID_TIMER6,		//16
	0,	 // PERIPHERAL_ID_AUDIO_ADC0_RX,	//17
	1,	 // PERIPHERAL_ID_AUDIO_ADC1_RX,	//18
	2,	 // PERIPHERAL_ID_AUDIO_DAC0_TX,	//19
	3,	 // PERIPHERAL_ID_AUDIO_DAC1_TX,	//20
	255, // PERIPHERAL_ID_I2S0_RX,			//21


	#if ((defined(CFG_RES_AUDIO_I2SOUT_EN) && (CFG_RES_I2S == 0)) || defined(CFG_RES_AUDIO_I2S0OUT_EN))
		7, // PERIPHERAL_ID_I2S0_TX,			//22
	#else
		255, // PERIPHERAL_ID_I2S0_TX,		//22
	#endif

	255, // PERIPHERAL_ID_I2S1_RX,		//23


	#if ((defined(CFG_RES_AUDIO_I2SOUT_EN) && (CFG_RES_I2S == 1)) || defined(CFG_RES_AUDIO_I2S1OUT_EN))
		5, // PERIPHERAL_ID_I2S1_TX,		//24
	#else
		255, // PERIPHERAL_ID_I2S1_TX,		//24
	#endif

	255, // PERIPHERAL_ID_PPWM,			//25
	255, // PERIPHERAL_ID_ADC,     		//26
	255, // PERIPHERAL_ID_SOFTWARE,		//27
};

static LineInPlayContext *sLineInPlayCt;

void LineInPlayResFree(void)
{
	AudioCoreProcessConfig((void *)AudioNoAppProcess);
	AudioCoreSourceDisable(LINEIN_SOURCE_NUM);
	AudioAnaChannelSet(ANA_INPUT_CH_NONE);

	#ifdef CFG_FUNC_AUDIO_EFFECT_EN
		AudioEffectsDeInit();
	#endif

	#if (LINEIN_INPUT_CHANNEL == ANA_INPUT_CH_LINEIN3)
		AudioADC_Disable(ADC1_MODULE);
		AudioADC_DeInit(ADC1_MODULE);
	#else
		AudioADC_Disable(ADC0_MODULE);
		AudioADC_DeInit(ADC0_MODULE);
	#endif

	if (sLineInPlayCt->ADCFIFO != NULL)
	{
		APP_DBG("ADCFIFO\n");
		osPortFree(sLineInPlayCt->ADCFIFO);
		sLineInPlayCt->ADCFIFO = NULL;
	}
	AudioCoreSourceDeinit(LINEIN_SOURCE_NUM);

	APP_DBG("Line:Kill Ct\n");
}



bool LineInPlayResMalloc(uint16_t SampleLen)
{
	AudioCoreIO AudioIOSet;
	memset(&AudioIOSet, 0, sizeof(AudioCoreIO));

	AudioIOSet.Adapt = STD;
	AudioIOSet.Sync = TRUE;
	AudioIOSet.Channels = 2;
	AudioIOSet.Net = DefaultNet;

	#if (LINEIN_INPUT_CHANNEL == ANA_INPUT_CH_LINEIN3)
		AudioIOSet.DataIOFunc = AudioADC1DataGet;
		AudioIOSet.LenGetFunc = AudioADC1DataLenGet;
	#else
		AudioIOSet.DataIOFunc = AudioADC0DataGet;
		AudioIOSet.LenGetFunc = AudioADC0DataLenGet;
	#endif

	#ifdef CFG_AUDIO_WIDTH_24BIT
		AudioIOSet.IOBitWidth = 0; 
	#ifdef BT_TWS_SUPPORT
		AudioIOSet.IOBitWidthConvFlag = 0; 
	#else
		AudioIOSet.IOBitWidthConvFlag = 1; 
	#endif
	#endif


	if (!AudioCoreSourceInit(&AudioIOSet, LINEIN_SOURCE_NUM))
	{
		DBG("Line source error!\n");
		return FALSE;
	}

	sLineInPlayCt = (LineInPlayContext *)osPortMalloc(sizeof(LineInPlayContext));
	if (sLineInPlayCt == NULL)
	{
		return FALSE;
	}
	memset(sLineInPlayCt, 0, sizeof(LineInPlayContext));

	sLineInPlayCt->ADCFIFO = (uint32_t *)osPortMallocFromEnd(SampleLen * 2 * 2 * 2);
	if (sLineInPlayCt->ADCFIFO == NULL)
	{
		return FALSE;
	}
	memset(sLineInPlayCt->ADCFIFO, 0, SampleLen * 2 * 2 * 2);

	return TRUE;
}

void LineInPlayResInit(void)
{
	sLineInPlayCt->SampleRate = CFG_PARA_SAMPLE_RATE;
	sLineInPlayCt->AudioCoreLineIn = (AudioCoreContext *)&AudioCore;
	sLineInPlayCt->AudioCoreLineIn->AudioSource[LINEIN_SOURCE_NUM].Enable = 1;

	#ifdef CFG_FUNC_AUDIO_EFFECT_EN
		sLineInPlayCt->AudioCoreLineIn->AudioEffectProcess = (AudioCoreProcessFunc)AudioMusicProcess;
	#else
		sLineInPlayCt->AudioCoreLineIn->AudioEffectProcess = (AudioCoreProcessFunc)AudioBypassProcess;
	#endif

	AudioAnaChannelSet(LINEIN_INPUT_CHANNEL);

	#if (LINEIN_INPUT_CHANNEL == ANA_INPUT_CH_LINEIN3)
		AudioADC_DigitalInit(ADC1_MODULE, sLineInPlayCt->SampleRate, (void *)sLineInPlayCt->ADCFIFO, AudioCoreFrameSizeGet(DefaultNet) * 2 * 2 * 2);
	#else
		AudioADC_DigitalInit(ADC0_MODULE, sLineInPlayCt->SampleRate, (void *)sLineInPlayCt->ADCFIFO, AudioCoreFrameSizeGet(DefaultNet) * 2 * 2 * 2);
	#endif
}

void LineInPlayRun(uint16_t msgId)
{
	#ifdef CFG_FUNC_AUDIO_EFFECT_EN
		if (IsEffectChange == 1)
		{
			IsEffectChange = 0;
			AudioAPPDigitalGianProcess(mainAppCt.SysCurrentMode);
			AudioEffectsLoadInit(0, mainAppCt.EffectMode);
		}
		if (IsEffectChangeReload == 1)
		{
			IsEffectChangeReload = 0;
			AudioEffectsLoadInit(1, mainAppCt.EffectMode);
			AudioAPPDigitalGianProcess(mainAppCt.SysCurrentMode);
			AudioEffectsLoadInit(0, mainAppCt.EffectMode);
		}
		#ifdef CFG_EFFECT_PARAM_UPDATA_BY_ACPWORKBENCH
			if (EffectParamFlashUpdataFlag == 1)
			{
				EffectParamFlashUpdataFlag = 0;
				EffectParamFlashUpdata();
			}
		#endif
	#endif

	switch (msgId)
	{
	case MSG_PLAY_PAUSE:
		HardWareMuteOrUnMute();
		break;

	default:
		CommonMsgProccess(msgId);
		break;
	}
}

bool LineInPlayInit(void)
{
	APP_MUTE_CMD(FALSE);

	APP_DBG("LineIn Play Init\n");
	DMA_ChannelAllocTableSet((uint8_t *)sDmaChannelMap); // lineIn

	if (!ModeCommonInit())
	{
		return FALSE;
	}
	if (!LineInPlayResMalloc(AudioCoreFrameSizeGet(DefaultNet)))
	{
		APP_DBG("LineInPlay Res Error!\n");
		return FALSE;
	}
	LineInPlayResInit();

	#ifdef CFG_FUNC_AUDIO_EFFECT_EN

		mainAppCt.EffectMode = 	data_dem.mode;
		AudioEffectsLoadInit(1, mainAppCt.EffectMode);
		AudioAPPDigitalGianProcess(mainAppCt.SysCurrentMode);
		AudioEffectsLoadInit(0, mainAppCt.EffectMode);
	#endif

	AudioCoreSourceUnmute(APP_SOURCE_NUM, TRUE, TRUE);

	#ifdef CFG_FUNC_REMIND_SOUND_EN
		if (sys_parameter.Item_thong_bao[1])
		{
			if (RemindSoundServiceItemRequest(SOUND_REMIND_AUXMODE, REMIND_PRIO_NORMAL) == FALSE)
			{
				if (IsAudioPlayerMute() == TRUE)
				{
					HardWareMuteOrUnMute();
				}
			}
		}
	#endif
	{
		if (IsAudioPlayerMute() == TRUE)
		{
			HardWareMuteOrUnMute();
		}
	}


	if(sys_parameter.Nho_volume)
	{
		AudioMusicVol(mainAppCt.MusicVolume);
	}

	if(sys_parameter.Dongbo_volume)
	{
		mainAppCt.MusicVolume = sys_parameter.Volume_tong;
		AudioMusicVol(mainAppCt.MusicVolume);
	}
	
	if(sys_parameter.Vr_en)
	{
		AudioMusicVol32(mainAppCt.MusicVolume_VR);
	}

	APP_MUTE_CMD(FALSE);

	return TRUE;
}

bool LineInPlayDeinit(void)
{

	APP_MUTE_CMD(TRUE);

	APP_DBG("LineIn Play Deinit\n");
	if (sLineInPlayCt == NULL)
	{
		return TRUE;
	}

	if (IsAudioPlayerMute() == FALSE)
	{
		HardWareMuteOrUnMute();
	}

	PauseAuidoCore();
	LineInPlayResFree();
	ModeCommonDeinit();

	osPortFree(sLineInPlayCt);
	sLineInPlayCt = NULL;

	return TRUE;
}
#endif // #ifdef CFG_APP_LINEIN_MODE_EN
