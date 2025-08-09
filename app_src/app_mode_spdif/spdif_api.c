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
#include "app_message.h"
#include "app_config.h"
#include "debug.h"
#include "delay.h"
#include "spdif.h"
#include "clk.h"
#include "audio_core_api.h"
#include "audio_core_service.h"
#include "mcu_circular_buf.h"
#include "audio_vol.h"

#ifdef CFG_FUNC_SPDIF_MIX_MODE

// ���֧��TWS��������BT_TWS_SUPPORT  SPDIFλ�������Ϊ0;
#define SPDIF_BIT_Width				    	0 // 0: 16bit   1: 24bit


bool MixSpdifLockFlag = FALSE;
uint32_t Mixsamplerate = 0;
#define MIX_PCM_REMAIN_SMAPLES				3 //spdif���ݶ�����ʱ,�������ݻᳬ��һ֡128�����ڻ������ݣ������и��Ӻ�

//spdif����������8�ֽ�
//recv, dma buf len,MAX_FRAME_SAMPLES * 2 * 2 * 2 * 2�ǻ�����OS�л����ʵ����Ҫ�ӱ���
#define	MIX_SPDIF_FIFO_LEN					(MAX_FRAME_SAMPLES * 2 * 2 * 2 * 2 * 2 * 2)

//���ڲ���ֵ��32bitת��Ϊ16bit������ʹ��ͬһ��buf������Ҫ��������
#define MIX_SPDIF_CARRY_LEN					(MAX_FRAME_SAMPLES * 2 * 2 * 2 * 2 + MIX_PCM_REMAIN_SMAPLES * 2 * 4)//֧��192000���� buf len for get data form dma fifo, deal + 6samples ���ڻ���ż����������ݣ�#9817


typedef struct _SpdifPlayContext
{
	uint32_t			*SpdifPwcFIFO;		
	uint32_t            *SpdifCarry;
#if (SPDIF_BIT_Width == 1)
	uint32_t            *SpdifCarry24;
#endif	
	uint32_t			*SpdifPcmFifo;
	MCU_CIRCULAR_CONTEXT SpdifPcmCircularBuf;

	//play
	uint32_t 			 SampleRate;

}MixSpdifPlayContext;
static  MixSpdifPlayContext*		MixSpdifPlayCt = NULL;


//sampleΪ��λ
uint16_t MixSpdif_Rx_DataLenGet(void)
{
#if (SPDIF_BIT_Width == 1)
	return MCUCircular_GetDataLen(&MixSpdifPlayCt->SpdifPcmCircularBuf)/8;
#else
	return MCUCircular_GetDataLen(&MixSpdifPlayCt->SpdifPcmCircularBuf)/4;
#endif
}

//sampleΪ��λ��buf��С��8 * MaxSize
uint16_t MixSpdif_Rx_DataGet(void *pcm_out, uint16_t MaxPoint)
{
#if (SPDIF_BIT_Width == 1)
	return MCUCircular_GetData(&MixSpdifPlayCt->SpdifPcmCircularBuf, pcm_out, MaxPoint * 8) / 8;
#else
	return MCUCircular_GetData(&MixSpdifPlayCt->SpdifPcmCircularBuf, pcm_out, MaxPoint * 4) / 4;
#endif
}

void MixSpdifDataCarry(void)
{
	int16_t pcm_space;
	uint16_t spdif_len;
	int16_t pcm_len;
	uint32_t *pcmBuf = MixSpdifPlayCt->SpdifCarry;
	uint16_t cnt;

	#if (SPDIF_BIT_Width == 1)
	uint16_t Spdif_bit_width = 32;
	#else
	uint16_t Spdif_bit_width = 16;
	#endif

	spdif_len = DMA_CircularDataLenGet(PERIPHERAL_ID_SPDIF_RX);
	pcm_space = MCUCircular_GetSpaceLen(&MixSpdifPlayCt->SpdifPcmCircularBuf) - Spdif_bit_width;

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

		//���ڴ�32bitת��Ϊ16bit��buf����ʹ��ͬһ��������Ҫ�������롣
		#if (SPDIF_BIT_Width == 1)
		pcm_len = SPDIF_SPDIFDataToPCMData((int32_t *)pcmBuf, MAX_FRAME_SAMPLES * (Spdif_bit_width/2), (int32_t *)MixSpdifPlayCt->SpdifCarry24, SPDIF_WORDLTH_24BIT);
		//printf("pcm_len = %d\n",pcm_len);
		if(pcm_len < 0)
		{
			return;
		}
		MCUCircular_PutData(&MixSpdifPlayCt->SpdifPcmCircularBuf, MixSpdifPlayCt->SpdifCarry24, pcm_len);
		#else
		pcm_len = SPDIF_SPDIFDataToPCMData((int32_t *)pcmBuf, MAX_FRAME_SAMPLES * (Spdif_bit_width/2), (int32_t *)pcmBuf, SPDIF_WORDLTH_16BIT);
		//printf("pcm_len = %d\n",pcm_len);
		if(pcm_len < 0)
		{
			return;
		}
		MCUCircular_PutData(&MixSpdifPlayCt->SpdifPcmCircularBuf, pcmBuf, pcm_len);
		#endif
	}
}

void AudioSpdif_DataInProcess()
{
	if(MixSpdifLockFlag && !SPDIF_FlagStatusGet(LOCK_FLAG_STATUS))
	{
		APP_DBG("SPDIF RX UNLOCK!\n");
		MixSpdifLockFlag = FALSE;
		AudioCoreSourceDisable(SPDIF_MIX_SOURCE_NUM);
	}
	if(!MixSpdifLockFlag && SPDIF_FlagStatusGet(LOCK_FLAG_STATUS))
	{
		APP_DBG("SPDIF RX LOCK!\n");
		MixSpdifLockFlag = TRUE;
		if(IsAudioPlayerMute() == FALSE)
		{
			HardWareMuteOrUnMute();
		}
		vTaskDelay(20);
		AudioCoreSourceEnable(SPDIF_MIX_SOURCE_NUM);
		AudioCoreSourceUnmute(SPDIF_MIX_SOURCE_NUM, TRUE, TRUE);
		if(IsAudioPlayerMute() == TRUE)
		{
			HardWareMuteOrUnMute();
		}
	}
	//���SPDIF RX�������Ƿ�ı�
	if(MixSpdifLockFlag == TRUE)
	{
		if(Mixsamplerate != SPDIF_SampleRateGet())
		{
			Mixsamplerate = SPDIF_SampleRateGet();

			MixSpdifPlayCt->SampleRate = Mixsamplerate;
			APP_DBG("Get SampleRate: %d\n", (int)MixSpdifPlayCt->SampleRate);
			AudioCoreSourceChange(SPDIF_MIX_SOURCE_NUM, 2, MixSpdifPlayCt->SampleRate);
		}
		MixSpdifDataCarry();
	}
}

bool MixSpdifPlayInit(void)
{
	AudioCoreIO AudioIOSet;
	
	if(MixSpdifPlayCt != NULL)
		return FALSE;
	
	DBG("%s\n",__func__);
	
	SPDIF_ModuleDisable();
	Clock_SpdifClkSelect(APLL_CLK_MODE);

	MixSpdifPlayCt = (MixSpdifPlayContext*)osPortMalloc(sizeof(MixSpdifPlayContext));
	if(MixSpdifPlayCt == NULL)
		return FALSE;

	memset(MixSpdifPlayCt, 0, sizeof(MixSpdifPlayContext));
	MixSpdifPlayCt->SampleRate = CFG_PARA_SAMPLE_RATE;

	MixSpdifPlayCt->SpdifPcmFifo = (uint32_t *)osPortMalloc(MIX_SPDIF_FIFO_LEN);//end bkd
	if(MixSpdifPlayCt->SpdifPcmFifo == NULL)
		return FALSE;

	memset(MixSpdifPlayCt->SpdifPcmFifo, 0, MIX_SPDIF_FIFO_LEN);
	MCUCircular_Config(&MixSpdifPlayCt->SpdifPcmCircularBuf, MixSpdifPlayCt->SpdifPcmFifo, MIX_SPDIF_FIFO_LEN);
	DBG("spdif frame size %d sample / frame\n",AudioCoreFrameSizeGet(DefaultNet));

	//  (DMA buffer)
	MixSpdifPlayCt->SpdifPwcFIFO = (uint32_t*)osPortMalloc(MIX_SPDIF_FIFO_LEN);
	if(MixSpdifPlayCt->SpdifPwcFIFO == NULL)
		return FALSE;

	memset(MixSpdifPlayCt->SpdifPwcFIFO, 0, MIX_SPDIF_FIFO_LEN);
	
#if (SPDIF_BIT_Width == 1)
	MixSpdifPlayCt->SpdifCarry24 = (uint32_t *)osPortMalloc(MIX_SPDIF_CARRY_LEN);
	if(MixSpdifPlayCt->SpdifCarry24 == NULL)
		return FALSE;

	memset(MixSpdifPlayCt->SpdifCarry24, 0, MIX_SPDIF_CARRY_LEN);
#endif	

	MixSpdifPlayCt->SpdifCarry = (uint32_t *)osPortMalloc(MIX_SPDIF_CARRY_LEN);
	if(MixSpdifPlayCt->SpdifCarry == NULL)
		return FALSE;

	memset(MixSpdifPlayCt->SpdifCarry, 0, MIX_SPDIF_CARRY_LEN);

	//Audio init
	memset(&AudioIOSet, 0, sizeof(AudioCoreIO));

	AudioIOSet.Adapt = SRC_SRA;//STD;
	AudioIOSet.Sync = FALSE;
	AudioIOSet.Channels = 2;
	AudioIOSet.Net = DefaultNet;

	AudioIOSet.DataIOFunc = MixSpdif_Rx_DataGet;
	AudioIOSet.LenGetFunc = MixSpdif_Rx_DataLenGet;
	
	AudioIOSet.Depth = AudioCoreFrameSizeGet(DefaultNet) * 2;
	AudioIOSet.LowLevelCent = 40;
	AudioIOSet.HighLevelCent = 60;
	AudioIOSet.SampleRate = MixSpdifPlayCt->SampleRate;

#ifdef	CFG_AUDIO_WIDTH_24BIT
	#ifdef BT_TWS_SUPPORT
		AudioIOSet.IOBitWidth = 0;//0,16bit,1:24bit
		AudioIOSet.IOBitWidthConvFlag = 0;//tws����Ҫ���ݽ���λ����չ������TWS_SOURCE_NUM�Ժ�ͳһת��24bit
	#else
		AudioIOSet.IOBitWidth = SPDIF_BIT_Width;//0,16bit,1:24bit
		AudioIOSet.IOBitWidthConvFlag = !SPDIF_BIT_Width;//��Ҫ���ݽ���λ����չ
	#endif
#endif

	if(!AudioCoreSourceInit(&AudioIOSet, SPDIF_MIX_SOURCE_NUM))
	{
		DBG("spdif source init error!\n");
		return FALSE;
	}
	
	AudioCoreSourceAdjust(SPDIF_MIX_SOURCE_NUM, TRUE);

	GPIO_PortAModeSet(SPDIF_MIX_INDEX, SPDIF_MIX_PORT_MODE);
	SPDIF_AnalogModuleEnable(SPDIF_MIX_PORT_ANA_INPUT, SPDIF_ANA_LEVEL_300mVpp);

	DMA_ChannelDisable(PERIPHERAL_ID_SPDIF_RX);
	//Reset_FunctionReset(DMAC_FUNC_SEPA);
	SPDIF_RXInit(1, 0, 0);
	//ʹ��
	DMA_CircularConfig(PERIPHERAL_ID_SPDIF_RX, MIX_SPDIF_FIFO_LEN / 2, (void*)MixSpdifPlayCt->SpdifPwcFIFO, MIX_SPDIF_FIFO_LEN);
	DMA_ChannelEnable(PERIPHERAL_ID_SPDIF_RX);
	SPDIF_ModuleEnable();

#ifdef CFG_FUNC_LINE_MIX_MODE
	AudioCoreSourceUnmute(LINE_SOURCE_NUM,TRUE,TRUE);
#endif
	MixSpdifLockFlag = FALSE;
	Mixsamplerate = 0;

	return TRUE;
}


bool MixSpdifPlayDeinit(void)
{
	DBG("%s\n",__func__);

	if(MixSpdifPlayCt == NULL)
		return FALSE;

	AudioCoreSourceDisable(SPDIF_MIX_SOURCE_NUM);//SPDIF_MIX_SOURCE_NUM
	AudioCoreSourceDeinit(SPDIF_MIX_SOURCE_NUM);

	SPDIF_ModuleDisable();
	DMA_InterruptFlagClear(PERIPHERAL_ID_SPDIF_RX, DMA_DONE_INT);
	DMA_InterruptFlagClear(PERIPHERAL_ID_SPDIF_RX, DMA_THRESHOLD_INT);
	DMA_InterruptFlagClear(PERIPHERAL_ID_SPDIF_RX, DMA_ERROR_INT);
	DMA_ChannelDisable(PERIPHERAL_ID_SPDIF_RX);

	SPDIF_AnalogModuleDisable();

	GPIO_PortAModeSet(SPDIF_MIX_INDEX, 0);

	if(MixSpdifPlayCt->SpdifCarry != NULL){
		osPortFree(MixSpdifPlayCt->SpdifCarry);
		MixSpdifPlayCt->SpdifCarry = NULL;
	}
	
#if (SPDIF_BIT_Width == 1)
	if(MixSpdifPlayCt->SpdifCarry24 != NULL){
		osPortFree(MixSpdifPlayCt->SpdifCarry24);
		MixSpdifPlayCt->SpdifCarry24 = NULL;
	}
#endif

	if(MixSpdifPlayCt->SpdifPcmFifo != NULL){
		osPortFree(MixSpdifPlayCt->SpdifPcmFifo);
		MixSpdifPlayCt->SpdifPcmFifo = NULL;
	}

	if(MixSpdifPlayCt->SpdifPwcFIFO != NULL){
		osPortFree(MixSpdifPlayCt->SpdifPwcFIFO);
		MixSpdifPlayCt->SpdifPwcFIFO = NULL;
	}
	
	osPortFree(MixSpdifPlayCt);
	MixSpdifPlayCt = NULL;
	
	return TRUE;
}
#endif 

