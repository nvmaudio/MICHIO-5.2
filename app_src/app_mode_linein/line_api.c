/*
 * line_api.c
 *
 *  Created on: Apr 13, 2020
 *      Author: szsj-1
 */
#include <string.h>
#include "type.h"
#include "irqn.h"
#include "gpio.h"
#include "dma.h"
#include "app_message.h"
#include "app_config.h"
#include "debug.h"
#include "delay.h"
#include "audio_adc.h"
#include "adc_interface.h"
#include "audio_core_api.h"
#include "audio_core_service.h"
#include "ctrlvars.h"
#include "mode_task_api.h"
#include "line_api.h"

#ifdef CFG_FUNC_LINE_MIX_MODE

typedef struct _MixLineInPlayContext
{
	uint32_t			*ADCFIFO;			//ADC的DMA循环fifo
	AudioCoreContext 	*AudioCoreLineIn;
	//play
	uint32_t 			SampleRate; 		//带提示音时，如果不重采样，要避免采样率配置冲突
}MixLineInPlayContext;

static  MixLineInPlayContext*		mixLineInPlayCt;


void AudioLine_ConfigInit(uint16_t SampleLen)
{
	DBG("%s\n",__func__);

	AudioCoreIO	AudioIOSet;
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

#ifdef	CFG_AUDIO_WIDTH_24BIT
	AudioIOSet.IOBitWidth = 0;//0,16bit,1:24bit
	#ifdef BT_TWS_SUPPORT
	AudioIOSet.IOBitWidthConvFlag = 0;//tws不需要数据进行位宽扩展，会在TWS_SOURCE_NUM以后统一转成24bit
	#else
	AudioIOSet.IOBitWidthConvFlag = 1;//需要数据进行位宽扩展
	#endif
#endif
	//AudioIOSet.
	if(!AudioCoreSourceInit(&AudioIOSet, LINE_SOURCE_NUM))
	{
		DBG("Line source error!\n");
		return;
	}

	mixLineInPlayCt = (MixLineInPlayContext*)osPortMalloc(sizeof(mixLineInPlayCt));
	if(mixLineInPlayCt == NULL)
		return;
	
	memset(mixLineInPlayCt, 0, sizeof(mixLineInPlayCt));

	//LineIn5  digital (DMA)
	mixLineInPlayCt->ADCFIFO = (uint32_t*)osPortMallocFromEnd(SampleLen * 2 * 2 * 2);
	if(mixLineInPlayCt->ADCFIFO == NULL)
		return;
	
	memset(mixLineInPlayCt->ADCFIFO, 0, SampleLen * 2 * 2 * 2);

	AudioCoreSourceEnable(LINE_SOURCE_NUM);

	MixLineInPlayResInit();
	
	DBG("%s end!\n",__func__);
}


void MixLineInPlayResInit(void)
{
	mixLineInPlayCt->SampleRate = CFG_PARA_SAMPLE_RATE;

	AudioAnaChannelSet(LINEIN_INPUT_CHANNEL);

#if (LINEIN_INPUT_CHANNEL == ANA_INPUT_CH_LINEIN3)
	AudioADC_DigitalInit(ADC1_MODULE, mixLineInPlayCt->SampleRate, (void*)mixLineInPlayCt->ADCFIFO, AudioCoreFrameSizeGet(DefaultNet) * 2 * 2 * 2);
#else
	AudioADC_DigitalInit(ADC0_MODULE, mixLineInPlayCt->SampleRate, (void*)mixLineInPlayCt->ADCFIFO, AudioCoreFrameSizeGet(DefaultNet) * 2 * 2 * 2);
#endif
}


void MixLineInPlayDeInit(void)
{
	DBG("%s\n",__func__);

	AudioCoreSourceDisable(LINE_SOURCE_NUM);

	AudioAnaChannelSet(ANA_INPUT_CH_NONE);

#if (LINEIN_INPUT_CHANNEL == ANA_INPUT_CH_LINEIN3)
	AudioADC_Disable(ADC1_MODULE);
	AudioADC_DeInit(ADC1_MODULE);
#else
	AudioADC_Disable(ADC0_MODULE);
	AudioADC_DeInit(ADC0_MODULE);
#endif

	if(mixLineInPlayCt->ADCFIFO != NULL)
	{
		APP_DBG("ADCFIFO\n");
		osPortFree(mixLineInPlayCt->ADCFIFO);
		mixLineInPlayCt->ADCFIFO = NULL;
	}
	AudioCoreSourceDeinit(LINE_SOURCE_NUM);
}
#endif///end of #ifdef CFG_FUNC_LINE_MIX_MODE



