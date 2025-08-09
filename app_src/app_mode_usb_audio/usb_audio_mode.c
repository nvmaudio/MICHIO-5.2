/**
 **************************************************************************************
 * @file    usb_audio_mode.c
 * @brief
 *
 * @author  Owen
 * @version V1.0.0
 *
 * $Created: 2018-04-27 13:06:47$
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
#include "otg_device_hcd.h"
#include "otg_device_audio.h"
#include "otg_device_standard_request.h"
#include "mcu_circular_buf.h"
#include "decoder.h"
#include "remind_sound.h"
#include "main_task.h"
#include "timer.h"
#include "powercontroller.h"
#include "deepsleep.h"
#include "backup_interface.h"
#include "breakpoint.h"
#include "ctrlvars.h"
#include "usb_audio_mode.h"
#include "audio_vol.h"
#include "ctrlvars.h"
#include "device_detect.h" 
#include "otg_device_audio.h"
#include "otg_device_stor.h"
#include "mode_task_api.h"
#include "device_detect.h"
#include "otg_detect.h"
#include "audio_effect.h"
#include "audio_effect_flash_param.h"
#include "../../app_framework/audio_engine/audio_core_api.h"
#include "../../app_framework/audio_engine/audio_core_service.h"

#include "sys_param.h"
#include "michio.h"


#ifdef CFG_APP_USB_AUDIO_MODE_EN

#define USB_AUDIO_SRC_BUF_LEN				(150 * 2 * 2)
#define USB_DEVICE_PLAY_TASK_STACK_SIZE		(1024)
#define USB_DEVICE_PLAY_TASK_PRIO			3
#define USB_DEVICE_NUM_MESSAGE_QUEUE		10

extern UsbAudio 			UsbAudioSpeaker;
extern UsbAudio 			UsbAudioMic;

extern volatile uint32_t 	gDeviceUSBDeviceTimer;
static uint32_t 			FramCount = 0;

typedef struct _UsbDevicePlayContext
{
	MessageHandle		parentMsgHandle;
	AudioCoreContext 	*AudioCoreUsb;
}UsbDevicePlayContext;


static const uint8_t DmaChannelMap[29] =
{
	255,//PERIPHERAL_ID_SPIS_RX = 0,	//0
	255,//PERIPHERAL_ID_SPIS_TX,		//1
	255,//PERIPHERAL_ID_TIMER3,			//2
	4,//PERIPHERAL_ID_SDIO_RX,			//3
	4,//PERIPHERAL_ID_SDIO_TX,			//4
	255,//PERIPHERAL_ID_UART0_RX,		//5
	255,//PERIPHERAL_ID_TIMER1,			//6
	255,//PERIPHERAL_ID_TIMER2,			//7
	255,//PERIPHERAL_ID_SDPIF_RX,		//8 
	255,//PERIPHERAL_ID_SDPIF_TX,		//8
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
	255,//PERIPHERAL_ID_I2S0_RX,		//21

	#if	((defined(CFG_RES_AUDIO_I2SOUT_EN )&&(CFG_RES_I2S == 0)) || defined(CFG_RES_AUDIO_I2S0OUT_EN))
		7,//PERIPHERAL_ID_I2S0_TX,		//22
	#else	
		255,//PERIPHERAL_ID_I2S0_TX,	//22
	#endif	

	255,//PERIPHERAL_ID_I2S1_RX,		//23

	#if	((defined(CFG_RES_AUDIO_I2SOUT_EN )&&(CFG_RES_I2S == 1))|| defined(CFG_RES_AUDIO_I2S1OUT_EN))
		5,	//PERIPHERAL_ID_I2S1_TX,	//24
	#else
		255,//PERIPHERAL_ID_I2S1_TX,	//24
	#endif

	255,//PERIPHERAL_ID_PPWM,			//25
	255,//PERIPHERAL_ID_ADC,     		//26
	255,//PERIPHERAL_ID_SOFTWARE,		//27
};



static  UsbDevicePlayContext*		UsbDevicePlayCt;


uint16_t UsbAudioSpeakerDataGet(void *Buffer,uint16_t Len);
uint16_t UsbAudioSpeakerDataLenGet(void);
uint16_t UsbAudioMicDataSet(void *Buffer,uint16_t Len);
uint16_t UsbAudioMicSpaceLenGet(void);


void UsbDevicePlayResRelease(void)
{
	#ifdef CFG_RES_AUDIO_USB_IN_EN	
		if(UsbAudioSpeaker.PCMBuffer != NULL)
		{
			APP_DBG("UsbAudioSpeaker.PCMBuffer free\n");
			osPortFree(UsbAudioSpeaker.PCMBuffer);
			UsbAudioSpeaker.PCMBuffer = NULL;
		}
	#endif
	#ifdef CFG_RES_AUDIO_USB_OUT_EN	
		if(UsbAudioMic.PCMBuffer != NULL)
		{
			APP_DBG("UsbAudioMic.PCMBuffer free\n");
			osPortFree(UsbAudioMic.PCMBuffer);
			UsbAudioMic.PCMBuffer = NULL;
		}
	#endif
}

bool UsbDevicePlayResMalloc(uint16_t SampleLen)
{
	UsbDevicePlayCt = (UsbDevicePlayContext*)osPortMalloc(sizeof(UsbDevicePlayContext));
	memset(UsbDevicePlayCt, 0, sizeof(UsbDevicePlayContext));

	APP_DBG("UsbDevicePlayResMalloc %u\n", SampleLen);

	#ifdef CFG_RES_AUDIO_USB_IN_EN
		//Speaker FIFO
		UsbAudioSpeaker.PCMBuffer = osPortMallocFromEnd(SampleLen * 16);
		if(UsbAudioSpeaker.PCMBuffer == NULL)
		{
			APP_DBG("UsbAudioSpeaker.PCMBuffer memory error\n");
			return FALSE;
		}
		memset(UsbAudioSpeaker.PCMBuffer, 0, SampleLen * 16);
		MCUCircular_Config(&UsbAudioSpeaker.CircularBuf, UsbAudioSpeaker.PCMBuffer, SampleLen * 16);
	#endif//end of CFG_RES_USB_IN_EN

	#ifdef CFG_RES_AUDIO_USB_OUT_EN
		//MIC FIFO
		UsbAudioMic.PCMBuffer = osPortMallocFromEnd(SampleLen * 16);
		if(UsbAudioMic.PCMBuffer == NULL)
		{
			APP_DBG("UsbAudioMic.PCMBuffer memory error\n");
			return FALSE;
		}
		memset(UsbAudioMic.PCMBuffer, 0, SampleLen * 16);
		MCUCircular_Config(&UsbAudioMic.CircularBuf, UsbAudioMic.PCMBuffer, SampleLen * 16);
	#endif///end of CFG_REGS_AUDIO_USB_OUT_EN

	return TRUE;
}

void UsbDevicePlayResInit(void)
{
	UsbDevicePlayCt->AudioCoreUsb = (AudioCoreContext*)&AudioCore;

	AudioCoreIO	AudioIOSet;
	#ifdef CFG_RES_AUDIO_USB_IN_EN
		memset(&AudioIOSet, 0, sizeof(AudioCoreIO));
		#if !defined(CFG_PARA_AUDIO_USB_IN_SYNC)
			#if !defined(CFG_PARA_AUDIO_USB_IN_SRC)
				AudioIOSet.Adapt = STD;
			#else
				AudioIOSet.Adapt = SRC_ONLY;
			#endif
		#else 
			#if !defined(CFG_PARA_AUDIO_USB_IN_SRC)
				AudioIOSet.Adapt = SRA_ONLY;//CLK_ADJUST_ONLY;
			#else
				AudioIOSet.Adapt = SRC_SRA;//SRC_ADJUST;
			#endif
		#endif
		AudioIOSet.Sync = TRUE;
		AudioIOSet.Channels = 2;
		AudioIOSet.Net = DefaultNet;
		AudioIOSet.DataIOFunc = UsbAudioSpeakerDataGet;
		AudioIOSet.LenGetFunc = UsbAudioSpeakerDataLenGet;
		AudioIOSet.Depth = AudioCoreFrameSizeGet(DefaultNet) * 2;
		AudioIOSet.LowLevelCent = 40;
		AudioIOSet.HighLevelCent = 60;
		if(UsbAudioSpeaker.AudioSampleRate) 
		{
			AudioIOSet.SampleRate = UsbAudioSpeaker.AudioSampleRate;
		}
		else
		{
			AudioIOSet.SampleRate = CFG_PARA_SAMPLE_RATE;
		}
		#ifdef CFG_AUDIO_OUT_AUTO_SAMPLE_RATE_44100_48000
			AudioOutSampleRateSet(AudioIOSet.SampleRate);
		#endif
		#ifdef	CFG_AUDIO_WIDTH_24BIT
			AudioIOSet.IOBitWidth = 0;//0,16bit,1:24bit
			#ifdef BT_TWS_SUPPORT
				AudioIOSet.IOBitWidthConvFlag = 0;
			#else
				AudioIOSet.IOBitWidthConvFlag = 1;
			#endif
		#endif
		if(!AudioCoreSourceInit(&AudioIOSet, USB_AUDIO_SOURCE_NUM))
		{
			DBG("Usbin source error!\n");
		}
		AudioCoreSourceAdjust(USB_AUDIO_SOURCE_NUM, TRUE);
	#endif //CFG_RES_AUDIO_USB_IN_EN
	#ifdef CFG_RES_AUDIO_USB_OUT_EN
		memset(&AudioIOSet, 0, sizeof(AudioCoreIO));
		#if !defined(CFG_PARA_AUDIO_USB_OUT_SYNC)
			#if !defined(CFG_PARA_AUDIO_USB_OUT_SRC)
				AudioIOSet.Adapt = STD;
			#else
				AudioIOSet.Adapt = SRC_ONLY;
			#endif
		#else 
			#if !defined(CFG_PARA_AUDIO_USB_OUT_SRC)
				AudioIOSet.Adapt = SRA_ONLY;//CLK_ADJUST_ONLY;
			#else
				AudioIOSet.Adapt = SRC_SRA;//SRC_ADJUST;
			#endif
		#endif
		AudioIOSet.Sync = TRUE;//FALSE;
		AudioIOSet.Channels = 2;
		AudioIOSet.Net = DefaultNet;
		AudioIOSet.DataIOFunc = UsbAudioMicDataSet;
		AudioIOSet.LenGetFunc = UsbAudioMicSpaceLenGet;
		AudioIOSet.Depth = AudioCoreFrameSizeGet(DefaultNet) * 2;
		AudioIOSet.LowLevelCent = 40;
		AudioIOSet.HighLevelCent = 60;

		if(UsbAudioMic.AudioSampleRate) 
		{
			AudioIOSet.SampleRate = UsbAudioMic.AudioSampleRate;
		}
		else
		{
			AudioIOSet.SampleRate = CFG_PARA_SAMPLE_RATE;
		}
		if(!AudioCoreSinkInit(&AudioIOSet, USB_AUDIO_SINK_NUM))
		{
			DBG("Usbout sink error!\n");
		}
		AudioCoreSinkAdjust(USB_AUDIO_SINK_NUM, TRUE);
	#endif //CFG_RES_AUDIO_USB_OUT_EN

	//Core Process
	#ifdef CFG_FUNC_AUDIO_EFFECT_EN
		UsbDevicePlayCt->AudioCoreUsb->AudioEffectProcess = (AudioCoreProcessFunc)AudioMusicProcess;
	#else
		UsbDevicePlayCt->AudioCoreUsb->AudioEffectProcess = (AudioCoreProcessFunc)AudioBypassProcess;
	#endif

	
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
}



bool UsbDevicePlayInit(void)
{

	APP_MUTE_CMD(TRUE);

	APP_DBG("UsbDevice Play Init\n");
	DMA_ChannelAllocTableSet((uint8_t *)DmaChannelMap);

	if(!ModeCommonInit())
	{
		return FALSE;
	}

	if(UsbAudioSpeaker.InitOk != 1)
	{
		memset(&UsbAudioSpeaker,0,sizeof(UsbAudioSpeaker));
		memset(&UsbAudioMic,0,sizeof(UsbAudioMic));

		UsbAudioSpeaker.Channels    = 2;
		UsbAudioSpeaker.LeftVol    = AUDIO_MAX_VOLUME;
		UsbAudioSpeaker.RightVol   = AUDIO_MAX_VOLUME;

		UsbAudioMic.Channels       = 2;
		UsbAudioMic.LeftVol        = AUDIO_MAX_VOLUME;
		UsbAudioMic.RightVol       = AUDIO_MAX_VOLUME;
	}

	if(!UsbDevicePlayResMalloc(AudioCoreFrameSizeGet(DefaultNet)))
	{
		APP_DBG("UsbDevicePlayResMalloc Res Error!\n");
		return FALSE;
	}
	UsbDevicePlayResInit();

	#ifdef CFG_COMMUNICATION_BY_USB
		SetUSBDeviceInitState(TRUE);
	#endif

	OTG_DeviceModeSel(CFG_PARA_USB_MODE, USB_VID, USBPID(CFG_PARA_USB_MODE));

	#ifdef CFG_FUNC_AUDIO_EFFECT_EN
		mainAppCt.EffectMode = 	data_dem.mode;
		AudioEffectsLoadInit(1, mainAppCt.EffectMode);
		AudioAPPDigitalGianProcess(mainAppCt.SysCurrentMode);
		AudioEffectsLoadInit(0, mainAppCt.EffectMode);
	#endif


	gDeviceUSBDeviceTimer = DEVICE_USB_DEVICE_DETECT_TIMER+3000;
	OTG_DeviceInit();
	NVIC_EnableIRQ(Usb_IRQn);
	
	#ifdef CFG_FUNC_REMIND_SOUND_EN
		#include "sys_param.h"
		if(sys_parameter.Item_thong_bao[2]){
			if(RemindSoundServiceItemRequest(SOUND_REMIND_PCMODE, REMIND_PRIO_NORMAL) == FALSE)
			{
				if(IsAudioPlayerMute() == TRUE)
				{
					HardWareMuteOrUnMute();
				}
			}
		} 
	#endif
	{
		if(IsAudioPlayerMute() == TRUE)
		{
			HardWareMuteOrUnMute();
		}
	}

	return TRUE;
}

void PCAudioPP(void);
void PCAudioNext(void);
void PCAudioPrev(void);
void PCAudioStop(void);
void PCAudioVolUp(void);
void PCAudioVolDn(void);

void UsbDevicePlayRun(uint16_t msgId)
{
	#ifdef CFG_FUNC_AUDIO_EFFECT_EN
		if(IsEffectChange == 1)
		{
			IsEffectChange = 0;
			AudioAPPDigitalGianProcess(mainAppCt.SysCurrentMode);
			AudioEffectsLoadInit(0, mainAppCt.EffectMode);
		}
		if(IsEffectChangeReload == 1)
		{
			IsEffectChangeReload = 0;
			AudioEffectsLoadInit(1, mainAppCt.EffectMode);
			AudioAPPDigitalGianProcess(mainAppCt.SysCurrentMode);
			AudioEffectsLoadInit(0, mainAppCt.EffectMode);
		}
		#ifdef CFG_EFFECT_PARAM_UPDATA_BY_ACPWORKBENCH
			if(EffectParamFlashUpdataFlag == 1)
			{
				EffectParamFlashUpdataFlag = 0;
				EffectParamFlashUpdata();
			}
		#endif
	#endif

	switch(msgId)
	{
		case MSG_PLAY_PAUSE:
			APP_DBG("Play Pause\n");
			PCAudioPP();
			break;

		case MSG_PRE:
			APP_DBG("PRE Song\n");
			PCAudioPrev();
			break;

		case MSG_NEXT:
			APP_DBG("next Song\n");
			PCAudioNext();
			break;

		case MSG_MUSIC_VOLUP:
			APP_DBG("VOLUP\n");
			PCAudioVolUp();
			break;

		case MSG_MUSIC_VOLDOWN:
			APP_DBG("VOLDOWN\n");
			PCAudioVolDn();
			break;

		default:
			CommonMsgProccess(msgId);
			break;
	}

	OTG_DeviceRequestProcess();

}

bool UsbDevicePlayDeinit(void)
{
	APP_MUTE_CMD(TRUE);

	APP_DBG("UsbDevice Play Deinit\n");
	if(UsbDevicePlayCt == NULL)
	{
		return TRUE;
	}
	
	if(IsAudioPlayerMute() == FALSE)
	{
		HardWareMuteOrUnMute();
	}	
	
	PauseAuidoCore();	

	AudioCoreProcessConfig((void*)AudioNoAppProcess);
	AudioCoreSourceDisable(USB_AUDIO_SOURCE_NUM);

	#ifdef CFG_RES_AUDIO_USB_OUT_EN
		AudioCoreSinkDisable(USB_AUDIO_SINK_NUM);
	#endif

	if(!OTG_PortDeviceIsLink())
	{
		OTG_DeviceDisConnect();
		#ifdef CFG_COMMUNICATION_BY_USB
			SetUSBDeviceInitState(FALSE);
		#endif
	}
	
	//NVIC_DisableIRQ(Usb_IRQn);
	UsbDevicePlayResRelease();
	AudioCoreSourceDeinit(USB_AUDIO_SOURCE_NUM);

	#ifdef CFG_RES_AUDIO_USB_OUT_EN
		AudioCoreSinkDeinit(USB_AUDIO_SINK_NUM);
	#endif
	#ifdef CFG_FUNC_AUDIO_EFFECT_EN
		AudioEffectsDeInit();
	#endif

	ModeCommonDeinit();
	osPortFree(UsbDevicePlayCt);
	UsbDevicePlayCt = NULL;

	return TRUE;
}


uint16_t UsbAudioSpeakerDataGet(void *Buffer, uint16_t Len)
{
	uint16_t Length = 0;

	if(!UsbAudioSpeaker.PCMBuffer)
	{
		return 0;
	}

	Length = Len * 4;
	Length = MCUCircular_GetData(&UsbAudioSpeaker.CircularBuf, Buffer, Length);
	return Length / 4;
}


uint16_t UsbAudioSpeakerDataLenGet(void)
{
	uint16_t Len;

	if(!UsbAudioSpeaker.PCMBuffer)
	{
		return 0;
	}
	Len = MCUCircular_GetDataLen(&UsbAudioSpeaker.CircularBuf);
	Len = Len / 4;
	return Len;
}


uint16_t UsbAudioMicDataSet(void *Buffer, uint16_t Len)
{
	#ifdef CFG_RES_AUDIO_USB_OUT_EN
		if(!UsbAudioMic.PCMBuffer)
		{
			return 0;
		}

		MCUCircular_PutData(&UsbAudioMic.CircularBuf, Buffer, Len * 4);
	#endif
	return Len;
}

uint16_t UsbAudioMicSpaceLenGet(void)
{
	uint16_t Len;
	
	if(!UsbAudioMic.PCMBuffer)
	{
		return 0;
	}	
	Len = MCUCircular_GetSpaceLen(&UsbAudioMic.CircularBuf);
	Len = Len / 4;
	return Len;
}

void UsbAudioTimer1msProcess(void)
{
	if(GetSystemMode() != ModeUsbDevicePlay)
	{
		return;
	}
	FramCount++;
	if(FramCount % 2)//2ms
	{
		return;
	}
	#ifdef CFG_RES_AUDIO_USB_IN_EN
		if(UsbAudioSpeaker.AltSet)
		{
			if(UsbAudioSpeaker.FramCount)
			{
				if(UsbAudioSpeaker.FramCount != UsbAudioSpeaker.TempFramCount)
				{
					UsbAudioSpeaker.TempFramCount = UsbAudioSpeaker.FramCount;
					if(AudioCore.AudioSource[USB_AUDIO_SOURCE_NUM].Enable == FALSE)
					{
						AudioCoreSourceEnable(USB_AUDIO_SOURCE_NUM);
					}
				}
				else
				{
					AudioCoreSourceDisable(USB_AUDIO_SOURCE_NUM);
				}
			}
		}
		else
		{
			UsbAudioSpeaker.FramCount = 0;
			UsbAudioSpeaker.TempFramCount = 0;
			AudioCoreSourceDisable(USB_AUDIO_SOURCE_NUM);
		}
	#endif

	#ifdef CFG_RES_AUDIO_USB_IN_EN
		if(UsbAudioMic.AltSet)
		{
			if(UsbAudioMic.FramCount)
			{
				if(UsbAudioMic.FramCount != UsbAudioMic.TempFramCount)
				{
					UsbAudioMic.TempFramCount = UsbAudioMic.FramCount;
					if(AudioCore.AudioSink[USB_AUDIO_SINK_NUM].Enable == FALSE)
					{
						AudioCoreSinkEnable(USB_AUDIO_SINK_NUM);
					}
				}
			}
		}
		else
		{
			UsbAudioMic.FramCount = 0;
			UsbAudioMic.TempFramCount = 0;
			AudioCoreSinkDisable(USB_AUDIO_SINK_NUM);
		}
	#endif
}

#endif //CFG_APP_USB_AUDIO_MODE_EN

