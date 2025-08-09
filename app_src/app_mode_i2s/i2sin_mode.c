/**
 **************************************************************************************
 * @file    i2sin_mode.c
 * @brief   
 *
 * @author  Sam
 * @version V1.0.0
 *
 * $Created: 2019-1-4 17:29:47$
 *
 * @Copyright (C) 2019, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
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
#include "i2s.h"
#include "i2s_interface.h"
#include "ctrlvars.h"
#include "mcu_circular_buf.h"
#include "mode_task_api.h"
#include "audio_effect_flash_param.h"

#ifdef CFG_APP_I2SIN_MODE_EN

#define I2SIN_SOURCE_NUM				APP_SOURCE_NUM

typedef struct _I2SInPlayContext
{
	xTaskHandle 		taskHandle;
	MessageHandle		msgHandle;

	uint32_t			*I2SFIFO1;			//I2S的DMA循环fifo
	AudioCoreContext 	*AudioCoreI2SIn;

	//play
	uint32_t 			SampleRate; //带提示音时，如果不重采样，要避免采样率配置冲突

}I2SInPlayContext;

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
static const uint8_t DmaChannelMap[29] = {
	255,//PERIPHERAL_ID_SPIS_RX = 0,	//0
	255,//PERIPHERAL_ID_SPIS_TX,		//1
#ifdef CFG_APP_HDMIIN_MODE_EN
	5,//PERIPHERAL_ID_TIMER3,			//2
#else
	255,//PERIPHERAL_ID_TIMER3,			//2
#endif

#if (defined (CFG_FUNC_I2S_MIX_MODE) && defined (CFG_RES_AUDIO_I2S1IN_EN)) || (CFG_RES_I2S == 1)
	255,//PERIPHERAL_ID_SDIO_RX,		//3
	255,//PERIPHERAL_ID_SDIO_TX,		//4
#else
	4,//PERIPHERAL_ID_SDIO_RX,			//3
	4,//PERIPHERAL_ID_SDIO_TX,			//4
#endif

	255,//PERIPHERAL_ID_UART0_RX,		//5
	255,//PERIPHERAL_ID_TIMER1,			//6
	255,//PERIPHERAL_ID_TIMER2,			//7

#if defined CFG_RES_AUDIO_SPDIFOUT_EN || defined CFG_FUNC_SPDIF_MIX_MODE
	6,//PERIPHERAL_ID_SDPIF_RX,			//8 SPDIF_RX /TX same chanell
	6,//PERIPHERAL_ID_SDPIF_TX,		    //8 SPDIF_RX /TX same chanell
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

#if (defined (CFG_FUNC_I2S_MIX_MODE) && defined (CFG_RES_AUDIO_I2S0IN_EN)) || (CFG_RES_I2S == 0)
	6,//PERIPHERAL_ID_I2S0_RX,			//21
#else
	255,//PERIPHERAL_ID_I2S0_RX,		//21
#endif

#if	((defined(CFG_RES_AUDIO_I2SOUT_EN )&&(CFG_RES_I2S == 0)) || defined(CFG_RES_AUDIO_I2S0OUT_EN))
	7,//PERIPHERAL_ID_I2S0_TX,			//22
#else
	255,//PERIPHERAL_ID_I2S0_TX,		//22
#endif

#if (defined (CFG_FUNC_I2S_MIX_MODE) && defined (CFG_RES_AUDIO_I2S1IN_EN)) || (CFG_RES_I2S == 1)
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

static  I2SInPlayContext*		sI2SInPlayCt;
uint8_t I2SInDecoderSourceNum(void);

void I2SInPlayResFree(void)
{
	//注意：AudioCore父任务调整到mainApp下，此处只关闭AudioCore通道，不关闭任务
	AudioCoreProcessConfig((void*)AudioNoAppProcess);
	AudioCoreSourceDisable(I2SIN_SOURCE_NUM);
	AudioCoreSourceDeinit(I2SIN_SOURCE_NUM);

	I2S_ModuleRxDisable(CFG_RES_I2S_MODULE);
	I2S_ModuleDisable(CFG_RES_I2S_MODULE);
#ifndef CFG_RES_AUDIO_I2SOUT_EN
#if (CFG_RES_I2S == 0)
	DMA_ChannelDisable(PERIPHERAL_ID_I2S0_RX);
#else
	DMA_ChannelDisable(PERIPHERAL_ID_I2S1_RX);
#endif
#endif

	//PortFree
	sI2SInPlayCt->AudioCoreI2SIn = NULL;

	if(sI2SInPlayCt->I2SFIFO1 != NULL)
	{
		osPortFree(sI2SInPlayCt->I2SFIFO1);
		sI2SInPlayCt->I2SFIFO1 = NULL;
	}

	APP_DBG("I2s:Kill Ct\n");

#ifdef CFG_FUNC_AUDIO_EFFECT_EN
	AudioEffectsDeInit();
#endif
}

bool I2SInPlayResMalloc(uint16_t SampleLen)
{
	sI2SInPlayCt = (I2SInPlayContext*)osPortMalloc(sizeof(I2SInPlayContext));
	if(sI2SInPlayCt == NULL)
	{
		return FALSE;
	}
	memset(sI2SInPlayCt, 0, sizeof(I2SInPlayContext));

	//I2SIn  digital (DMA)
	sI2SInPlayCt->I2SFIFO1 = (uint32_t*)osPortMallocFromEnd(SampleLen * sizeof(PCM_DATA_TYPE) * 2 * 2);
	if(sI2SInPlayCt->I2SFIFO1 == NULL)
	{
		return FALSE;
	}
	memset(sI2SInPlayCt->I2SFIFO1, 0, SampleLen * sizeof(PCM_DATA_TYPE) * 2 * 2);

	return TRUE;
}

void I2SInDmaFifoRest(void)
{
	if(sI2SInPlayCt == NULL)
		return;

	uint32_t I2S_RX_FIFO_LEN;
	I2S_RX_FIFO_LEN = AudioCoreFrameSizeGet(DefaultNet) * sizeof(PCM_DATA_TYPE) * 2 * 2;

	memset(sI2SInPlayCt->I2SFIFO1, 0, I2S_RX_FIFO_LEN);

	#if CFG_RES_I2S == 0
	*(volatile unsigned long *)0x40029000 &= ~0x80;//disable iis0
	#else
	*(volatile unsigned long *)0x4002A000 &= ~0x80;//disable iis1
	#endif
	RST_I2SModule(I2S0_MODULE + CFG_RES_I2S);
	DMA_InterruptFlagClear((PERIPHERAL_ID_I2S0_RX + CFG_RES_I2S * 2), DMA_DONE_INT);
	DMA_InterruptFlagClear((PERIPHERAL_ID_I2S0_RX + CFG_RES_I2S * 2), DMA_THRESHOLD_INT);
	DMA_InterruptFlagClear((PERIPHERAL_ID_I2S0_RX + CFG_RES_I2S * 2), DMA_ERROR_INT);
	DMA_ChannelDisable(PERIPHERAL_ID_I2S0_RX + CFG_RES_I2S * 2);
	DMA_CircularConfig((PERIPHERAL_ID_I2S0_RX + CFG_RES_I2S * 2), I2S_RX_FIFO_LEN/2, sI2SInPlayCt->I2SFIFO1, I2S_RX_FIFO_LEN);
	GIE_DISABLE();
	DMA_CircularWritePtrSet((PERIPHERAL_ID_I2S0_RX + CFG_RES_I2S * 2), I2S_RX_FIFO_LEN - sizeof(PCM_DATA_TYPE) * 2);
	GIE_ENABLE();
	DMA_ChannelEnable(PERIPHERAL_ID_I2S0_RX + CFG_RES_I2S * 2);
}

bool I2SInPlayResInit(void)
{
	I2SParamCt i2s_set;

	sI2SInPlayCt->SampleRate = CFG_PARA_I2S_SAMPLERATE;//根据实际外设选择
	//Core Source1 para
	sI2SInPlayCt->AudioCoreI2SIn = (AudioCoreContext*)&AudioCore;

	i2s_set.IsMasterMode=CFG_RES_I2S_MODE;// 0:master 1:slave
	i2s_set.SampleRate=sI2SInPlayCt->SampleRate;
	i2s_set.I2sFormat=I2S_FORMAT_I2S;
#ifdef	CFG_AUDIO_WIDTH_24BIT
	i2s_set.I2sBits = I2S_LENGTH_24BITS;
#else
	i2s_set.I2sBits = I2S_LENGTH_16BITS;
#endif
	i2s_set.I2sTxRxEnable = 2;
#if (CFG_RES_I2S == 0)
	i2s_set.RxPeripheralID=PERIPHERAL_ID_I2S0_RX;
#else
	i2s_set.RxPeripheralID=PERIPHERAL_ID_I2S1_RX;
#endif
	i2s_set.RxBuf=sI2SInPlayCt->I2SFIFO1;
	i2s_set.RxLen=AudioCoreFrameSizeGet(DefaultNet) * sizeof(PCM_DATA_TYPE) * 2 * 2 ;//I2SIN_FIFO_LEN;

#ifdef CFG_RES_AUDIO_I2SOUT_EN
#if (CFG_RES_I2S == 0)
	i2s_set.TxPeripheralID = PERIPHERAL_ID_I2S0_TX;
#else
	i2s_set.TxPeripheralID = PERIPHERAL_ID_I2S1_TX;
#endif
	i2s_set.TxBuf = (void*)mainAppCt.I2SFIFO;

	i2s_set.TxLen = mainAppCt.I2SFIFO_LEN;

	i2s_set.I2sTxRxEnable = 3;
#endif

	// I2S GPIO配置
	I2S_GPIO_Port_ModeSet(CFG_RES_I2S_MODULE, CFG_RES_I2S_MODE);

	I2S_ModuleDisable(CFG_RES_I2S_MODULE);
	I2S_AlignModeSet(CFG_RES_I2S_MODULE, I2S_LOW_BITS_ACTIVE);
	AudioI2S_Init(CFG_RES_I2S_MODULE,&i2s_set);

	//note Soure0.和sink0已经在main app中配置，不要随意配置
	//Core Soure1.Para
	AudioCoreIO	AudioIOSet;
	memset(&AudioIOSet, 0, sizeof(AudioCoreIO));
#if CFG_RES_I2S_MODE == 0 || !defined(CFG_FUNC_I2S_IN_SYNC_EN) || (USE_MCLK_IN_MODE != 0)//master 或者关微调
	#if CFG_PARA_I2S_SAMPLERATE == CFG_PARA_SAMPLE_RATE
	AudioIOSet.Adapt = STD;
	#else
	AudioIOSet.Adapt = SRC_ONLY;
	#endif
#else //slave
	#if CFG_PARA_I2S_SAMPLERATE == CFG_PARA_SAMPLE_RATE
	AudioIOSet.Adapt = SRA_ONLY;//CLK_ADJUST_ONLY;//
	#else
	AudioIOSet.Adapt = SRC_SRA;//SRC_ADJUST;//
	#endif
#endif
	AudioIOSet.Sync = TRUE; //I2S slave 时候如果master没有接，有可能会导致DAC也不出声音。
	AudioIOSet.Channels = 2;
	AudioIOSet.Net = DefaultNet;
	AudioIOSet.Depth = AudioCoreFrameSizeGet(DefaultNet) * 2;//sI2SInPlayCt->I2SFIFO1 采样点深度
//	DBG("Depth:%d", AudioIOSet.Depth);
	AudioIOSet.HighLevelCent = 60;
	AudioIOSet.LowLevelCent = 40;
	AudioIOSet.SampleRate = CFG_PARA_I2S_SAMPLERATE;//根据实际外设选择

#if (CFG_RES_I2S == 0)
	AudioIOSet.DataIOFunc = AudioI2S0_DataGet ;
	AudioIOSet.LenGetFunc = AudioI2S0_DataLenGet;
#else
	AudioIOSet.DataIOFunc = AudioI2S1_DataGet ;
	AudioIOSet.LenGetFunc = AudioI2S1_DataLenGet;
#endif
#ifdef	CFG_AUDIO_WIDTH_24BIT
	#ifdef BT_TWS_SUPPORT
	AudioIOSet.IOBitWidth = 0;//0,16bit,1:24bit
	#else
	AudioIOSet.IOBitWidth = 1;//0,16bit,1:24bit
	#endif
	AudioIOSet.IOBitWidthConvFlag = 0;//tws不需要数据进行位宽扩展，会在TWS_SOURCE_NUM以后统一转成24bit
#endif
	if(!AudioCoreSourceInit(&AudioIOSet, I2SIN_SOURCE_NUM))
	{
		DBG("I2Splay source error!\n");
		return FALSE;
	}

	//Core Process	
#ifdef CFG_FUNC_AUDIO_EFFECT_EN
	sI2SInPlayCt->AudioCoreI2SIn->AudioEffectProcess = (AudioCoreProcessFunc)AudioMusicProcess;
#else
	sI2SInPlayCt->AudioCoreI2SIn->AudioEffectProcess = (AudioCoreProcessFunc)AudioBypassProcess;
#endif

	return TRUE;
}

/**
 * @func        I2SInPlay_Init
 * @brief       I2SIn模式参数配置，资源初始化
 * @param       MessageHandle 
 * @Output      None
 * @return      bool
 * @Others      任务块、I2S、Dac、AudioCore配置
 * @Others      数据流从I2S到audiocore配有函数指针，audioCore到Dac同理，由audiocoreService任务按需驱动
 * Record
 */
bool  I2SInPlayInit(void)
{
	bool ret;

	APP_DBG("I2SIn init\n");
	DMA_ChannelAllocTableSet((uint8_t *)DmaChannelMap);//
#ifdef CFG_FUNC_AUDIO_EFFECT_EN
	//音效参数遍历，确定系统帧长，待修改，sam, mark
#endif

	if(!ModeCommonInit())
	{
		return FALSE;
	}
	if(!I2SInPlayResMalloc(AudioCoreFrameSizeGet(DefaultNet)))
	{
		APP_DBG("I2SInPlay Res Error!\n");
		return FALSE;
	}

	ret = I2SInPlayResInit();
	AudioCoreSourceEnable(APP_SOURCE_NUM);
	AudioCoreSourceAdjust(APP_SOURCE_NUM, TRUE);
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
	if(RemindSoundServiceItemRequest(SOUND_REMIND_I2SMODE, REMIND_ATTR_NEED_MUTE_APP_SOURCE) == FALSE)
	#endif
	{
		if(IsAudioPlayerMute() == TRUE)
		{
			HardWareMuteOrUnMute();
		}
	}

	return ret;
}
/**
 * @func        I2sInPlayEntrance
 * @brief       模式执行主体
 * @param       void * param  
 * @Output      None
 * @return      None
 * @Others      模式建立和结束过程
 * Record
 */
void I2SInPlayRun(uint16_t msgId)
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
	switch(msgId)//警告：在此段代码，禁止新增提示音插播位置。
	{
		default:
			CommonMsgProccess(msgId);
			break;
	}
}

bool I2SInPlayDeinit(void)
{
	if(sI2SInPlayCt == NULL)
	{
		return TRUE;
	}
	APP_DBG("I2SIn Play deinit\n");
	if(IsAudioPlayerMute() == FALSE)
	{
		HardWareMuteOrUnMute();
	}	

	AudioCoreSourceDisable(I2SIN_SOURCE_NUM);
	PauseAuidoCore();
	
	I2SInPlayResFree();
	ModeCommonDeinit();//通路全部释放
	
#ifdef CFG_RES_AUDIO_I2SOUT_EN
	AudioI2sOutParamsSet();
#endif

	osPortFree(sI2SInPlayCt);
	sI2SInPlayCt = NULL;

	return TRUE;
}

#endif//#ifdef CFG_APP_I2SIN_MODE_EN
