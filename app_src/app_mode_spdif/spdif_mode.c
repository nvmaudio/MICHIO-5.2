/**
 **************************************************************************************
 * @file    Spdif_mode.c
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
#include "spdif.h"
#include "dac_interface.h"
#include "clk.h"
#include "audio_core_api.h"
#include "audio_core_service.h"
#include "audio_effect.h"
#include "powercontroller.h"
#include "deepsleep.h"
#include "backup_interface.h"
#include "resampler_polyphase.h"
#include "mcu_circular_buf.h"
#include "breakpoint.h"
#include "otg_device_hcd.h"
#include "otg_device_standard_request.h"
#include "main_task.h"
#include "remind_sound_item.h"
#include "mode_task_api.h"
#include "remind_sound.h"
#include "spdif_mode.h"
#include "audio_vol.h"
#include "ctrlvars.h"
#include "reset.h"
#include "audio_effect_flash_param.h"

#if defined(CFG_APP_OPTICAL_MODE_EN) || defined(CFG_APP_COAXIAL_MODE_EN)

// 如果支持TWS，即打开了BT_TWS_SUPPORT  SPDIF位宽必须设为0;
#define SPDIF_BIT_Width				    0  // 0: 16bit   1: 24bit


bool SpdifLockFlag = FALSE;
uint32_t samplerate = 0;

#define SPDIF_SOURCE_NUM				APP_SOURCE_NUM
#define PCM_REMAIN_SMAPLES				3//spdif数据丢声道时,返回数据会超过一帧128，用于缓存数据，试听感更加好

//spdif单个采样点8字节
//recv, dma buf len,MAX_FRAME_SAMPLES * 2 * 2 * 2 * 2是基础，OS切换间隔实测需要加倍。
#define	SPDIF_FIFO_LEN					(MAX_FRAME_SAMPLES * 2 * 2 * 2 * 2 * 2 * 2)

//由于采样值从32bit转换为16bit，可以使用同一个buf，否则要独立申请
#define SPDIF_CARRY_LEN					(MAX_FRAME_SAMPLES * 2 * 2 * 2 * 2 + PCM_REMAIN_SMAPLES * 2 * 4)//支持192000输入 buf len for get data form dma fifo, deal + 6samples 用于缓存偶尔多余的数据，#9817


/**根据appconfig缺省配置:DMA 8个通道配置**/
/*1、cec需PERIPHERAL_ID_TIMER3*/
/*2、SD卡录音需PERIPHERAL_ID_SDIO RX/TX*/
/*3、在线串口调音需PERIPHERAL_ID_UART1 RX/TX,建议使用USB HID，节省DMA资源*/
/*4、线路输入需PERIPHERAL_ID_AUDIO_ADC0_RX*/
/*5、Mic开启需PERIPHERAL_ID_AUDIO_ADC1_RX，mode之间通道必须一致*/
/*6、Dac0开启需PERIPHERAL_ID_AUDIO_DAC0_TX mode之间通道必须一致*/
/*7、DacX需开启PERIPHERAL_ID_AUDIO_DAC1_TX mode之间通道必须一致*/
/*注意DMA 8个通道配置冲突:*/
/*a、UART在线调音和DAC-X有冲突,默认在线调音使用USB HID*/
/*b、UART在线调音与HDMI/SPDIF模式冲突*/
/*注意DMA 8个通道配置冲突:*/
/*a、UART在线调音和DAC-X有冲突,默认在线调音使用USB HID*/
/*b、UART在线调音与HDMI/SPDIF模式冲突*/
static const uint8_t DmaChannelMap[29] =
{
	255,//PERIPHERAL_ID_SPIS_RX = 0,	//0
	255,//PERIPHERAL_ID_SPIS_TX,		//1

#ifdef CFG_APP_HDMIIN_MODE_EN
	5,//PERIPHERAL_ID_TIMER3,			//2
#else
	255,//PERIPHERAL_ID_TIMER3,			//2
#endif

#if defined (CFG_FUNC_I2S_MIX_MODE) && defined (CFG_RES_AUDIO_I2S1IN_EN) && !defined (CFG_FUNC_RECORDER_EN)
	255,//PERIPHERAL_ID_SDIO_RX,		//3
	255,//PERIPHERAL_ID_SDIO_TX,		//4
#elif defined (CFG_FUNC_RECORDER_EN) && defined (CFG_RES_AUDIO_I2S1IN_EN)
	5,//PERIPHERAL_ID_SDIO_RX,			//3
	5,//PERIPHERAL_ID_SDIO_TX,			//4
#else
	4,//PERIPHERAL_ID_SDIO_RX,			//3
	4,//PERIPHERAL_ID_SDIO_TX,			//4
#endif

	255,//PERIPHERAL_ID_UART0_RX,		//5
	255,//PERIPHERAL_ID_TIMER1,			//6
	255,//PERIPHERAL_ID_TIMER2,			//7
	6,//PERIPHERAL_ID_SDPIF_RX,			//8 SPDIF_RX /TX same chanell
	6,//PERIPHERAL_ID_SDPIF_TX,			//8 SPDIF_RX /TX same chanell
	255,//PERIPHERAL_ID_SPIM_RX,		//9
	255,//PERIPHERAL_ID_SPIM_TX,		//10
	255,//PERIPHERAL_ID_UART0_TX,		//11
	
#ifdef CFG_COMMUNICATION_BY_UART	
	7,//PERIPHERAL_ID_UART1_RX,			//12
	6,//PERIPHERAL_ID_UART1_TX,			//13
#else
	255,//PERIPHERAL_ID_UART1_RX,		//12
	255,//PERIPHERAL_ID_UART1_TX,		//13
#endif

	255,//PERIPHERAL_ID_TIMER4,			//14
	255,//PERIPHERAL_ID_TIMER5,			//15
	255,//PERIPHERAL_ID_TIMER6,			//16
	0,//PERIPHERAL_ID_AUDIO_ADC0_RX,	//17
	1,//PERIPHERAL_ID_AUDIO_ADC1_RX,	//18
	2,//PERIPHERAL_ID_AUDIO_DAC0_TX,	//19
	3,//PERIPHERAL_ID_AUDIO_DAC1_TX,	//20

#if defined (CFG_FUNC_I2S_MIX_MODE) && defined (CFG_RES_AUDIO_I2S0IN_EN)	
	6,//PERIPHERAL_ID_I2S0_RX,		//21
#else
	255,//PERIPHERAL_ID_I2S0_RX,		//21
#endif

#if	((defined(CFG_RES_AUDIO_I2SOUT_EN )&&(CFG_RES_I2S == 0)) || defined(CFG_RES_AUDIO_I2S0OUT_EN))
	7,//PERIPHERAL_ID_I2S0_TX,		//22
#else	
	255,//PERIPHERAL_ID_I2S0_TX,		//22
#endif	

#if defined (CFG_FUNC_I2S_MIX_MODE) && defined (CFG_RES_AUDIO_I2S1IN_EN)	
	4,//PERIPHERAL_ID_I2S1_RX,		//23
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

typedef struct _SpdifPlayContext
{
	TaskState			state;
	uint32_t			*SpdifPwcFIFO;		//
	uint16_t 			*Source1Buf_Spdif;//
	uint32_t            *SpdifCarry;
#if (SPDIF_BIT_Width == 1)
	uint32_t            *SpdifCarry24;
#endif	
	uint32_t			*SpdifPcmFifo;
	MCU_CIRCULAR_CONTEXT SpdifPcmCircularBuf;

	AudioCoreContext 	*AudioCoreSpdif;
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	uint16_t*			Source2Decoder;
	TaskState			DecoderSync;
	bool				IsSoundRemindDone;
#endif

#ifdef CFG_FUNC_RECORDER_EN
	TaskState			RecorderSync;
#endif
	//play
	uint32_t 			SampleRate;

	uint32_t 			SpdifDmaWritePtr;
	uint32_t 			SpdifDmaReadPtr;
	uint32_t 			SpdifPreSample;
	uint8_t  			SpdifSampleRateCheckFlg;
	uint32_t 			SpdifSampleRateFromSW;

}SpdifPlayContext;
static  SpdifPlayContext*		SpdifPlayCt = NULL;


static void SpdifPlayRunning(uint16_t msgId)
{
	switch(msgId)
	{
		default:
			CommonMsgProccess(msgId);
			break;
	}
}

//sample为单位
uint16_t Spdif_Rx_DataLenGet(void)
{
#if (SPDIF_BIT_Width == 1)
	return MCUCircular_GetDataLen(&SpdifPlayCt->SpdifPcmCircularBuf)/8;
#else
	return MCUCircular_GetDataLen(&SpdifPlayCt->SpdifPcmCircularBuf)/4;
#endif
}

//sample为单位，buf大小：8 * MaxSize
uint16_t Spdif_Rx_DataGet(void *pcm_out, uint16_t MaxPoint)
{
#if (SPDIF_BIT_Width == 1)
	return MCUCircular_GetData(&SpdifPlayCt->SpdifPcmCircularBuf, pcm_out, MaxPoint * 8) / 8;
#else
	return MCUCircular_GetData(&SpdifPlayCt->SpdifPcmCircularBuf, pcm_out, MaxPoint * 4) / 4;
#endif
}

void SpdifDataCarry(void)
{
	int16_t pcm_space;
	uint16_t spdif_len;
	int16_t pcm_len;
	uint32_t *pcmBuf = SpdifPlayCt->SpdifCarry;
	uint16_t cnt;

	#if (SPDIF_BIT_Width == 1)
	uint16_t Spdif_bit_width = 32;
	#else
	uint16_t Spdif_bit_width = 16;
	#endif

	spdif_len = DMA_CircularDataLenGet(PERIPHERAL_ID_SPDIF_RX);
	pcm_space = MCUCircular_GetSpaceLen(&SpdifPlayCt->SpdifPcmCircularBuf) - Spdif_bit_width;

	if(pcm_space < Spdif_bit_width)
	{
		DBG("err\n");
		return;
	}

	if((spdif_len >> 1) > pcm_space)
	{
		spdif_len = pcm_space * 2;
	}

	spdif_len = spdif_len & 0xFFF8;
	if(!spdif_len)
	{
		return ;
	}

	cnt = (spdif_len / (Spdif_bit_width/2)) / (MAX_FRAME_SAMPLES);
	while(cnt--)
	{
		pcm_len = DMA_CircularDataGet(PERIPHERAL_ID_SPDIF_RX, pcmBuf, MAX_FRAME_SAMPLES * (Spdif_bit_width/2));

		//由于从32bit转换为16bit，buf可以使用同一个，否则要独立申请。
		#if (SPDIF_BIT_Width == 1)
		pcm_len = SPDIF_SPDIFDataToPCMData((int32_t *)pcmBuf, MAX_FRAME_SAMPLES * (Spdif_bit_width/2), (int32_t *)SpdifPlayCt->SpdifCarry24, SPDIF_WORDLTH_24BIT);
		//printf("pcm_len = %d\n",pcm_len);
		if(pcm_len < 0)
		{
			return;
		}
		MCUCircular_PutData(&SpdifPlayCt->SpdifPcmCircularBuf, SpdifPlayCt->SpdifCarry24, pcm_len);
		#else
		pcm_len = SPDIF_SPDIFDataToPCMData((int32_t *)pcmBuf, MAX_FRAME_SAMPLES * (Spdif_bit_width/2), (int32_t *)pcmBuf, SPDIF_WORDLTH_16BIT);
		//printf("pcm_len = %d\n",pcm_len);
		if(pcm_len < 0)
		{
			return;
		}
		MCUCircular_PutData(&SpdifPlayCt->SpdifPcmCircularBuf, pcmBuf, pcm_len);
		#endif
	}
}

bool SpdifPlayInit(void)
{
	AudioCoreIO AudioIOSet;
	bool ret;
	
	if(SpdifPlayCt != NULL){
		return FALSE;
	}	
	
#ifdef CFG_FUNC_REMIND_SOUND_EN
	if(GetSystemMode() == ModeOpticalAudioPlay){
		DBG("Optical Play Init\n");
	}
	else{
		DBG("Coaxial Play Init\n");
	}
#endif

	SPDIF_ModuleDisable();
	Clock_SpdifClkSelect(APLL_CLK_MODE);
	DMA_ChannelAllocTableSet((uint8_t*)DmaChannelMap);//optical
	
	SpdifPlayCt = (SpdifPlayContext*)osPortMalloc(sizeof(SpdifPlayContext));
	if(SpdifPlayCt == NULL)
	{
		return FALSE;
	}
	memset(SpdifPlayCt, 0, sizeof(SpdifPlayContext));

	SpdifPlayCt->SampleRate = CFG_PARA_SAMPLE_RATE;

	SpdifPlayCt->SpdifPcmFifo = (uint32_t *)osPortMalloc(SPDIF_FIFO_LEN);//end bkd
	if(SpdifPlayCt->SpdifPcmFifo == NULL)
	{
		return FALSE;
	}
	memset(SpdifPlayCt->SpdifPcmFifo, 0, SPDIF_FIFO_LEN);
	MCUCircular_Config(&SpdifPlayCt->SpdifPcmCircularBuf, SpdifPlayCt->SpdifPcmFifo, SPDIF_FIFO_LEN);
	DBG("spdif frame size %d sample / frame\n",AudioCoreFrameSizeGet(DefaultNet));

	//  (DMA buffer)
	SpdifPlayCt->SpdifPwcFIFO = (uint32_t*)osPortMalloc(SPDIF_FIFO_LEN);
	if(SpdifPlayCt->SpdifPwcFIFO == NULL)
	{
		return FALSE;
	}
	memset(SpdifPlayCt->SpdifPwcFIFO, 0, SPDIF_FIFO_LEN);
	
#if (SPDIF_BIT_Width == 1)
	SpdifPlayCt->SpdifCarry24 = (uint32_t *)osPortMalloc(SPDIF_CARRY_LEN);
	if(SpdifPlayCt->SpdifCarry24 == NULL)
	{
		return FALSE;
	}
	memset(SpdifPlayCt->SpdifCarry24, 0, SPDIF_CARRY_LEN);
#endif	

	SpdifPlayCt->SpdifCarry = (uint32_t *)osPortMalloc(SPDIF_CARRY_LEN);
	if(SpdifPlayCt->SpdifCarry == NULL)
	{
		return FALSE;
	}
	memset(SpdifPlayCt->SpdifCarry, 0, SPDIF_CARRY_LEN);

	SpdifPlayCt->SpdifDmaWritePtr 		 = 0;
	SpdifPlayCt->SpdifDmaReadPtr  		 = 0;
	SpdifPlayCt->SpdifSampleRateCheckFlg = 0;
	SpdifPlayCt->SpdifSampleRateFromSW 	 = 0;
	SpdifPlayCt->SpdifPreSample			 = 0;

	if(!ModeCommonInit())
	{
		return FALSE;
	}

	SpdifPlayCt->AudioCoreSpdif = (AudioCoreContext*)&AudioCore;

#ifdef CFG_FUNC_AUDIO_EFFECT_EN
#ifdef CFG_FUNC_MIC_KARAOKE_EN
	SpdifPlayCt->AudioCoreSpdif->AudioEffectProcess = (AudioCoreProcessFunc)AudioEffectProcess;
#else
	SpdifPlayCt->AudioCoreSpdif->AudioEffectProcess = (AudioCoreProcessFunc)AudioMusicProcess;
#endif
#else
	SpdifPlayCt->AudioCoreSpdif->AudioEffectProcess = (AudioCoreProcessFunc)AudioBypassProcess;
#endif

	//Audio init
	memset(&AudioIOSet, 0, sizeof(AudioCoreIO));

	AudioIOSet.Adapt = SRC_SRA;//STD;
	AudioIOSet.Sync = FALSE;
	AudioIOSet.Channels = 2;
	AudioIOSet.Net = DefaultNet;

	AudioIOSet.DataIOFunc = Spdif_Rx_DataGet;
	AudioIOSet.LenGetFunc = Spdif_Rx_DataLenGet;
	
	AudioIOSet.Depth = AudioCoreFrameSizeGet(DefaultNet) * 2;
	AudioIOSet.LowLevelCent = 40;
	AudioIOSet.HighLevelCent = 60;

	AudioIOSet.SampleRate = SpdifPlayCt->SampleRate;

#ifdef	CFG_AUDIO_WIDTH_24BIT
	#ifdef BT_TWS_SUPPORT
		AudioIOSet.IOBitWidth = 0;//0,16bit,1:24bit
		AudioIOSet.IOBitWidthConvFlag = 0;//tws不需要数据进行位宽扩展，会在TWS_SOURCE_NUM以后统一转成24bit
	#else
		AudioIOSet.IOBitWidth = SPDIF_BIT_Width;//0,16bit,1:24bit
    	#if (SPDIF_BIT_Width == 1)
		AudioIOSet.IOBitWidthConvFlag = 0;//需要数据进行位宽扩展
		#else
		AudioIOSet.IOBitWidthConvFlag = 1;//需要数据进行位宽扩展
		#endif
	#endif
#endif

	if(!AudioCoreSourceInit(&AudioIOSet, SPDIF_SOURCE_NUM))
	{
		DBG("spdif source init error!\n");
		return FALSE;
	}
	
	AudioCoreSourceAdjust(SPDIF_SOURCE_NUM, TRUE);

#ifdef CFG_APP_OPTICAL_MODE_EN
	if(GetSystemMode() == ModeOpticalAudioPlay)
	{
		//spdif config
#ifndef PORT_B_INPUT_DIGATAL	//bkd add	
		GPIO_PortAModeSet(SPDIF_OPTICAL_INDEX, SPDIF_OPTICAL_PORT_MODE);
#else
		GPIO_PortBModeSet(SPDIF_OPTICAL_INDEX, SPDIF_OPTICAL_PORT_MODE);
#endif
#ifdef CFG_APP_COAXIAL_MODE_EN
		GPIO_PortAModeSet(SPDIF_COAXIAL_INDEX, 0);
#endif
		//SPDIF_AnalogModuleDisable();//bkd mark
		SPDIF_AnalogModuleEnable(SPDIF_OPTICAL_PORT_ANA_INPUT, SPDIF_ANA_LEVEL_300mVpp);
	}
#endif

#ifdef CFG_APP_COAXIAL_MODE_EN
	if(GetSystemMode() == ModeCoaxialAudioPlay)
	{	
		GPIO_PortAModeSet(SPDIF_COAXIAL_INDEX, SPDIF_COAXIAL_PORT_MODE);
#ifdef CFG_APP_OPTICAL_MODE_EN
		GPIO_PortAModeSet(SPDIF_OPTICAL_INDEX, 0);
#endif
		SPDIF_AnalogModuleEnable(SPDIF_COAXIAL_PORT_ANA_INPUT, SPDIF_ANA_LEVEL_300mVpp);
	}
#endif


	DMA_ChannelDisable(PERIPHERAL_ID_SPDIF_RX);
	//Reset_FunctionReset(DMAC_FUNC_SEPA);
	SPDIF_RXInit(1, 0, 0);
	//使用
	DMA_CircularConfig(PERIPHERAL_ID_SPDIF_RX, SPDIF_FIFO_LEN / 2, (void*)SpdifPlayCt->SpdifPwcFIFO, SPDIF_FIFO_LEN);
	DMA_ChannelEnable(PERIPHERAL_ID_SPDIF_RX);
	SPDIF_ModuleEnable();

	
#ifdef CFG_FUNC_BREAKPOINT_EN
	BackupInfoUpdata(BACKUP_SYS_INFO);
#endif


#ifdef CFG_FUNC_AUDIO_EFFECT_EN
#ifdef CFG_EFFECT_PARAM_IN_FLASH_EN
	mainAppCt.EffectMode = EFFECT_MODE_FLASH_Music;
#else
	mainAppCt.EffectMode = EFFECT_MODE_NORMAL;
#endif
	AudioEffectsLoadInit(1, mainAppCt.EffectMode);
	AudioAPPDigitalGianProcess(mainAppCt.SysCurrentMode);
	AudioEffectsLoadInit(0, mainAppCt.EffectMode);
#endif

#ifdef CFG_FUNC_REMIND_SOUND_EN
	if(GetSystemMode() == ModeOpticalAudioPlay)
	{
		DBG("Optical Play run\n");
		ret = RemindSoundServiceItemRequest(SOUND_REMIND_GXIANMOD, REMIND_PRIO_NORMAL);
	}
	else
	{
		DBG("Coaxial Play run\n");
		ret = RemindSoundServiceItemRequest(SOUND_REMIND_TZHOUMOD, REMIND_PRIO_NORMAL);
	}
	if(ret == FALSE)
	{
		if(IsAudioPlayerMute() == TRUE)
		{
			HardWareMuteOrUnMute();
		}
	}
#else
	 if(IsAudioPlayerMute() == TRUE)
	 {
		 HardWareMuteOrUnMute();
	 }
#endif

	SpdifLockFlag = FALSE;
	samplerate = 0;

	return TRUE;
}


void SpdifPlayRun(uint16_t msgId)
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
		default:
		{
			SpdifPlayRunning(msgId);
#ifdef CFG_APP_COAXIAL_MODE_EN
			if(GetSystemMode() == ModeCoaxialAudioPlay)
			{
				if(SPDIF_FlagStatusGet(SYNC_FLAG_STATUS) || (!SPDIF_FlagStatusGet(LOCK_FLAG_STATUS)))
				{
					SPDIF_RXInit(1, 0, 0);
					SPDIF_ModuleEnable();
				}
			}
#endif

			if(SpdifLockFlag && !SPDIF_FlagStatusGet(LOCK_FLAG_STATUS))
			{
				APP_DBG("SPDIF RX UNLOCK!\n");
				SpdifLockFlag = FALSE;
				AudioCoreSourceDisable(SPDIF_SOURCE_NUM);
			}
			if(!SpdifLockFlag && SPDIF_FlagStatusGet(LOCK_FLAG_STATUS))
			{
				APP_DBG("SPDIF RX LOCK!\n");
				SpdifLockFlag = TRUE;
				if(IsAudioPlayerMute() == FALSE)
				{
					HardWareMuteOrUnMute();
				}
				vTaskDelay(20);
				AudioCoreSourceEnable(SPDIF_SOURCE_NUM);
				AudioCoreSourceUnmute(SPDIF_SOURCE_NUM, TRUE, TRUE);
				if(IsAudioPlayerMute() == TRUE)
				{
					HardWareMuteOrUnMute();
				}
			}

			//监控SPDIF RX采样率是否改变
			if(SpdifLockFlag == TRUE)
			{
				if(samplerate != SPDIF_SampleRateGet())
				{
					samplerate = SPDIF_SampleRateGet();

					SpdifPlayCt->SampleRate = samplerate;
					APP_DBG("Get SampleRate: %d\n", (int)SpdifPlayCt->SampleRate);
#ifdef CFG_AUDIO_OUT_AUTO_SAMPLE_RATE_44100_48000
					AudioOutSampleRateSet(SpdifPlayCt->SampleRate);
#endif
					AudioCoreSourceChange(SPDIF_SOURCE_NUM, 2, SpdifPlayCt->SampleRate);
				}

				SpdifDataCarry();
			}
			//任务优先级设置为4,通过发送该命令，可以提高AudioCore service有效利用率
			{
				MessageContext		msgSend;
				msgSend.msgId		= MSG_NONE;
				MessageSend(GetAudioCoreServiceMsgHandle(), &msgSend);
			}
		}
		break;
	}
}

bool SpdifPlayDeinit(void)
{
	APP_DBG("Spdif Play Deinit\n");
	if(SpdifPlayCt == NULL)
	{
		return FALSE;
	}

	if(IsAudioPlayerMute() == FALSE)
	{
		HardWareMuteOrUnMute();
	}
	
	PauseAuidoCore();
	AudioCoreProcessConfig((void*)AudioNoAppProcess);
	AudioCoreSourceDisable(SPDIF_SOURCE_NUM);//SPDIF_SOURCE_NUM
	AudioCoreSourceDeinit(SPDIF_SOURCE_NUM);

#ifdef CFG_FUNC_AUDIO_EFFECT_EN
	AudioEffectsDeInit();
#endif
	
	SPDIF_ModuleDisable();
	DMA_InterruptFlagClear(PERIPHERAL_ID_SPDIF_RX, DMA_DONE_INT);
	DMA_InterruptFlagClear(PERIPHERAL_ID_SPDIF_RX, DMA_THRESHOLD_INT);
	DMA_InterruptFlagClear(PERIPHERAL_ID_SPDIF_RX, DMA_ERROR_INT);
	DMA_ChannelDisable(PERIPHERAL_ID_SPDIF_RX);

	SPDIF_AnalogModuleDisable();

#ifdef CFG_APP_COAXIAL_MODE_EN
	GPIO_PortAModeSet(SPDIF_COAXIAL_INDEX, 0);
#endif
#ifdef CFG_APP_OPTICAL_MODE_EN
	GPIO_PortAModeSet(SPDIF_OPTICAL_INDEX, 0);
#endif

	if(SpdifPlayCt->SpdifCarry != NULL)
	{
		osPortFree(SpdifPlayCt->SpdifCarry);
		SpdifPlayCt->SpdifCarry = NULL;
	}
	
#if (SPDIF_BIT_Width == 1)
	if(SpdifPlayCt->SpdifCarry24 != NULL)
	{
		osPortFree(SpdifPlayCt->SpdifCarry24);
		SpdifPlayCt->SpdifCarry24 = NULL;
	}
#endif

	if(SpdifPlayCt->SpdifPcmFifo != NULL)
	{
		osPortFree(SpdifPlayCt->SpdifPcmFifo);
		SpdifPlayCt->SpdifPcmFifo = NULL;
	}

	if(SpdifPlayCt->SpdifPwcFIFO != NULL)
	{
		osPortFree(SpdifPlayCt->SpdifPwcFIFO);
		SpdifPlayCt->SpdifPwcFIFO = NULL;
	}
	SpdifPlayCt->AudioCoreSpdif = NULL;

	ModeCommonDeinit();//通路全部释放
	
	osPortFree(SpdifPlayCt);
	SpdifPlayCt = NULL;
	
	return TRUE;
}
#endif 

