/**
 **************************************************************************************
 * @file    bt_hf_mode.c
 * @brief   ����ͨ��ģʽ
 *
 * @author  KK
 * @version V1.0.1
 *
 * $Created: 2019-3-28 18:00:00$
 *
 * @Copyright (C) 2016, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#include <string.h>
#include "type.h"
#include "app_config.h"
#include "app_message.h"
#include "mvintrinsics.h"
//driver
#include "chip_info.h"
#include "dac.h"
#include "gpio.h"
#include "dma.h"
#include "dac.h"
#include "audio_adc.h"
#include "debug.h"
//middleware
#include "main_task.h"
#include "audio_vol.h"
#include "rtos_api.h"
#include "adc_interface.h"
#include "dac_interface.h"
#include "bt_manager.h"
#include "resampler.h"
#include "mcu_circular_buf.h"
#include "audio_core_api.h"
#include "audio_decoder_api.h"
#include "sbcenc_api.h"
#include "bt_config.h"
#include "cvsd_plc.h"
#include "ctrlvars.h"
//application
#include "bt_hf_mode.h"
#include "decoder.h"
#include "audio_core_service.h"
#include "audio_core_api.h"
#include "bt_hf_api.h"
#include "bt_interface.h"
#include "mode_task.h"
#include "mode_task_api.h"
#include "audio_effect.h"
#include "audio_effect_flash_param.h"
#ifdef CFG_FUNC_REMIND_SOUND_EN
#include "remind_sound.h"
#endif

#include "ctrlvars.h"
#include "reset.h"
#include "clk.h"
#include "bt_stack_service.h"
#include "bt_app_hfp_deal.h"

/*******************************************************************************************************************************************************************
*           		SaveScoData	 			   		   InAdapter	  	    CoreProcess			   OutAdapter			SaveScoData
*			 	********************  *******  ******  *********  *******  *************  *******  **********  ******  **********************************
* Mic16K   									*->*FIFO*->*       *->*Frame*->*AEC *MixNet*->*Frame*->* Src    *->*FIFO*->*				     * ScoOutPCM:8K Send
* RefSco Delay Fifo Get	 					*                  *->*Frame*->*						 				  \ Encode * mSbcOutFIFO * ScoOutmSBC Send
* ScoInPCM    								*->*FIFO*->*Src    *->*Frame*->*           *->*Frame*->*        *->*FIFO*->*Dac
* ScoInMsbc *mSbcRecvfifo*mSbcInFIFO *Decode*->*FIFO* /
*							  													   	   *->*Frame*->*		*->*FIFO*->*RefSco Delay Fifo Set
*				********************  ******   *****  **********  *******  *************  *******  **********  ******  **********************************
*
*****************************************************************************************************************************************************************/

#if defined(CFG_APP_BT_MODE_EN) && (BT_HFP_SUPPORT == ENABLE)
/**����appconfigȱʡ����:DMA 8��ͨ������**/
/*1��cec��PERIPHERAL_ID_TIMER3*/
/*2��SD��¼����PERIPHERAL_ID_SDIO RX/TX*/
/*3�����ߴ��ڵ�����PERIPHERAL_ID_UART1 RX/TX,����ʹ��USB HID����ʡDMA��Դ*/
/*4����·������PERIPHERAL_ID_AUDIO_ADC0_RX*/
/*5��Mic������PERIPHERAL_ID_AUDIO_ADC1_RX��mode֮��ͨ������һ��*/
/*6��Dac0������PERIPHERAL_ID_AUDIO_DAC0_TX mode֮��ͨ������һ��*/
/*7��DacX�迪��PERIPHERAL_ID_AUDIO_DAC1_TX mode֮��ͨ������һ��*/
/*ע��DMA 8��ͨ�����ó�ͻ:*/
/*a��UART���ߵ�����DAC-X�г�ͻ,Ĭ�����ߵ���ʹ��USB HID*/
static const uint8_t DmaChannelMap[29] = {
	255,//PERIPHERAL_ID_SPIS_RX = 0,	//0
	255,//PERIPHERAL_ID_SPIS_TX,		//1
	
#ifdef CFG_APP_HDMIIN_MODE_EN
	5,//PERIPHERAL_ID_TIMER3,			//2
#else
	255,//PERIPHERAL_ID_TIMER3,			//2
#endif

#if defined (CFG_FUNC_I2S_MIX_MODE) && defined (CFG_RES_AUDIO_I2S1IN_EN)
	255,//PERIPHERAL_ID_SDIO_RX,		//3
	255,//PERIPHERAL_ID_SDIO_TX,		//4
#else
	4,//PERIPHERAL_ID_SDIO_RX,			//3
	4,//PERIPHERAL_ID_SDIO_TX,			//4
#endif

	255,//PERIPHERAL_ID_UART0_RX,		//5
	255,//PERIPHERAL_ID_TIMER1,			//6
	255,//PERIPHERAL_ID_TIMER2,			//7
#ifdef CFG_RES_AUDIO_SPDIFOUT_EN
	6,//PERIPHERAL_ID_SDPIF_RX,			//8 SPDIF_RX /TX same chanell
	6,//PERIPHERAL_ID_SDPIF_TX,		    //8 SPDIF_RX /TX same chanell
#else
	255,//PERIPHERAL_ID_SDPIF_RX,		//8 SPDIF_RX /TX same chanell
	255,//PERIPHERAL_ID_SDPIF_TX,		//8 SPDIF_RX /TX same chanell
#endif
	255,//PERIPHERAL_ID_SPIM_RX,		//9
	255,//PERIPHERAL_ID_SPIM_TX,		//10
	
#if (defined(CFG_DUMP_DEBUG_EN)&&(CFG_DUMP_UART_TX_PORT_GROUP == 0))
	7,//PERIPHERAL_ID_UART0_TX,			//11
#else
	255,//PERIPHERAL_ID_UART0_TX,		//11
#endif
	255,//PERIPHERAL_ID_UART1_RX,		//12
	
#if (defined(CFG_DUMP_DEBUG_EN)&&(CFG_DUMP_UART_TX_PORT_GROUP == 1))
	7,//PERIPHERAL_ID_UART1_TX,			//13
#else
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

////////////////////////////////////////////////////////////////////////////////////
//msbc encoder
#define MSBC_CHANNE_MODE	    1 		// mono
#define MSBC_SAMPLE_REATE	    16000	// 16kHz
#define MSBC_BLOCK_LENGTH	    15

#define DELAY_EXIT_BT_HF_TIME   300 //��ʱ�˳�HFģʽʱ������  ��λ/ms
#define CFG_BT_RING_TIME		2000//���ñ����������ż��ʱ��

/////////////////////////////////////////////////////////////////////////////////////
BtHfContext	*gBtHfCt = NULL;

//���롢�˳�ͨ��ģʽ����������ȫ�ֱ���
static uint32_t sBtHfModeEnterFlag = 0;
uint32_t BtHfModeExitFlag = 0;
uint32_t BtHfModeEnterFlag = 0;
static uint32_t sBtHfModeEixtList = 0; //������Ϣ���У���ģʽ������ɺ��ٵ����˳�ģʽ

extern uint32_t gSpecificDevice;
extern uint32_t gHfFuncState;

static uint32_t	flagVoiceDelayExitBtHf = 0;	//��������־�ر��˳�ʱ����ʱ1s�˳�bt hfģʽ
uint32_t	HfStateCount = 0;

//////////////////////////////////////////////////////////////////////////////////////
extern void ModeCommonDeinit(void);

void SetsBtHfModeEnterFlag(uint32_t flag)
{
	sBtHfModeEnterFlag = flag;
}

uint8_t GetBtHfModeEnterFlag(void)
{
	return BtHfModeEnterFlag;
}

void BtModeEnterDetect(void)
{
	if(GetBtHfModeEnterFlag())
	{
		APP_DBG("BtHfModeEnterFlag -->> BtHfModeEnter\n");
		BtHfModeEnter();
	}
}

/*****************************************************************************************
 * ����������뱨�� ��ʾ����������
 ****************************************************************************************/
#ifdef CFG_FUNC_REMIND_SOUND_EN
static void BtHfRingRemindNumberInit(void)
{
	if(gBtHfCt == NULL)
		return;
	if(bt_CallinRingType)
	{
		TimeOutSet(&gBtHfCt->CallRingTmr,CFG_BT_RING_TIME);
		gBtHfCt->RemindSoundStart = 0;
	}

	if(bt_CallinRingType == USE_NUMBER_REMIND_RING)
	{
		memset(gBtHfCt->PhoneNumber, 0, sizeof(gBtHfCt->PhoneNumber));
		gBtHfCt->PhoneNumberLen = 0;
	}
}

static void BtHfRingRemindNumberStop(void)
{
	if(gBtHfCt == NULL)
		return;
	if(bt_CallinRingType )
	{
		if(gBtHfCt->RemindSoundStart)
		{
			gBtHfCt->RemindSoundStart = 0;
			RemindSoundClearPlay();

			if(bt_CallinRingType == USE_NUMBER_REMIND_RING)
			{
				memset(gBtHfCt->PhoneNumber, 0, sizeof(gBtHfCt->PhoneNumber));
				gBtHfCt->PhoneNumberLen = 0;
			}	
		}
	}
}

static void BtHfRingRemindNumberRunning(void)
{
	static uint8_t i = 0;
	static uint8_t len = 0;

	if((!bt_CallinRingType) || (GetHfpState(BtCurIndex_Get()) != BT_HFP_STATE_INCOMING))
	{
		i = 0;
		len = 0;
		BtHfRingRemindNumberStop();
		return;
	}
	if(gBtHfCt->RemindSoundStart && IsTimeOut(&gBtHfCt->CallRingTmr))
	{
		if(bt_CallinRingType >= USE_LOCAL_AND_PHONE_RING)
		{
			if(!RemindSoundIsPlay())
			{
				if(gBtHfCt->WaitFlag)
				{
					gBtHfCt->WaitFlag = 0;
					TimeOutSet(&gBtHfCt->CallRingTmr,CFG_BT_RING_TIME);
					return;
				}		
				TimeOutSet(&gBtHfCt->CallRingTmr,0);
				RemindSoundServiceItemRequest(SOUND_REMIND_CALLRING, REMIND_PRIO_NORMAL);
				gBtHfCt->WaitFlag = 1;
			}
		}
		else
		{
			char *SoundItem = NULL;

			if(gBtHfCt->WaitFlag)
			{
				gBtHfCt->WaitFlag = 0;
				TimeOutSet(&gBtHfCt->CallRingTmr, 800);
				return;
			}
			gBtHfCt->WaitFlag = 1;
			len = (gBtHfCt->PhoneNumberLen >= sizeof(gBtHfCt->PhoneNumber)) ? sizeof(gBtHfCt->PhoneNumber) : gBtHfCt->PhoneNumberLen;
			switch(gBtHfCt->PhoneNumber[i])
			{
				case '0':
					SoundItem = SOUND_REMIND_0;
					break;
				case '1':
					SoundItem = SOUND_REMIND_1;
					break;
				case '2':
					SoundItem = SOUND_REMIND_2;
					break;
				case '3':
					SoundItem = SOUND_REMIND_3;
					break;
				case '4':
					SoundItem = SOUND_REMIND_4;
					break;
				case '5':
					SoundItem = SOUND_REMIND_5;
					break;
				case '6':
					SoundItem = SOUND_REMIND_6;
					break;
				case '7':
					SoundItem = SOUND_REMIND_7;
					break;
				case '8':
					SoundItem = SOUND_REMIND_8;
					break;
				case '9':
					SoundItem = SOUND_REMIND_9;
					break;
				default:
					SoundItem = NULL;
					break;
			}

			if (i < len)
			{
				i++;
				if(SoundItem && RemindSoundServiceItemRequest(SoundItem, REMIND_PRIO_ORDER) == FALSE)
				{
					DBG("Phone Number play error!\n");
				}
			}
			else if(i == len)
			{
				i++;
				RemindSoundServiceItemRequest(SOUND_REMIND_CALLRING, REMIND_PRIO_NORMAL);
			}
			else
			{
				i = 0;
			}
		}
	}	
}
#endif

#ifdef BT_HFP_CALL_DURATION_DISP
void BtHfActiveTimeCount(void)
{
	if(GetHfpState(BtCurIndex_Get()) >= BT_HFP_STATE_ACTIVE)
	{
		gBtHfCt->BtHfTimerMsCount++;
		if(gBtHfCt->BtHfTimerMsCount>=1000) //1s
		{
			gBtHfCt->BtHfTimerMsCount = 0;
			gBtHfCt->BtHfTimeUpdate = 1;
			gBtHfCt->BtHfActiveTime++;
		}
	}
}

void BtHfActiveTimeInit(void)
{
	gBtHfCt->BtHfActiveTime = 0;
	gBtHfCt->BtHfTimerMsCount = 0;
	gBtHfCt->BtHfTimeUpdate = 0;
}

void BtHfActiveTimeUpdate(void)
{
	if(gBtHfCt->BtHfTimeUpdate)
	{
		uint8_t hour = (uint8_t)(gBtHfCt->BtHfActiveTime / 3600);
		uint8_t minute = (uint8_t)((gBtHfCt->BtHfActiveTime % 3600) / 60);
		uint8_t second = (uint8_t)(gBtHfCt->BtHfActiveTime % 60);
		gBtHfCt->BtHfTimeUpdate = 0;
		APP_DBG("ͨ���У�%02d:%02d:%02d   GetHfpState(%d) = %d\n", hour, minute, second, BtCurIndex_Get(), GetHfpState(BtCurIndex_Get()));
	}
	else if(GetHfpState(BtCurIndex_Get()) < BT_HFP_STATE_ACTIVE)
	{
		if (HfStateCount++ > 1000)
		{
			HfStateCount = 0;
			APP_DBG("GetHfpState(%d) = %d\n", BtCurIndex_Get(), GetHfpState(BtCurIndex_Get()));
		}
	}
}
#endif

/*******************************************************************************
 * TIMER2��ʱ������ͨ��ʱ���ļ�ʱ
 * 1MS����1��
 ******************************************************************************/
void BtHf_Timer1msProcess(void)
{
#ifdef BT_HFP_CALL_DURATION_DISP
	if(gBtHfCt == NULL)
		return;
	BtHfActiveTimeCount();
#endif
}

/***********************************************************************************
 **********************************************************************************/
bool BtHf_CvsdInit(void)
{
	if(gBtHfCt->CvsdInitFlag)
		return FALSE;
	
	//cvsd plc config
	gBtHfCt->cvsdPlcState = osPortMalloc(sizeof(CVSD_PLC_State));
	if(gBtHfCt->cvsdPlcState == NULL)
	{
		APP_DBG("CVSD PLC error\n");
		return FALSE;
	}
	memset(gBtHfCt->cvsdPlcState, 0, sizeof(CVSD_PLC_State));
	cvsd_plc_init(gBtHfCt->cvsdPlcState);

	gBtHfCt->CvsdInitFlag = 1;
	return TRUE;
}

/***********************************************************************************
 **********************************************************************************/
bool BtHf_MsbcInit(void)
{
	if(gBtHfCt->MsbcInitFlag)
		return FALSE;
	
	//msbc decoder init
	if(BtHf_MsbcDecoderInit() != 0)
	{
		APP_DBG("msbc decoder error\n");
		return FALSE;
	}

	//encoder init
	BtHf_MsbcEncoderInit();

	gBtHfCt->MsbcInitFlag = 1;
	return TRUE;
}

/***********************************************************************************
 **********************************************************************************/
void BtHfCodecTypeUpdate(uint8_t codecType)
{
	if(gBtHfCt == NULL)
		return;

	if(gBtHfCt->codecType != codecType)
	{
		MessageContext		msgSend;
		APP_DBG("[HF]codec type %d -> %d\n", gBtHfCt->codecType, codecType);
		
		gBtHfCt->codecType = codecType;
		
		//reply
		msgSend.msgId		= MSG_BT_HF_MODE_CODEC_UPDATE;
		MessageSend(GetSysModeMsgHandle(), &msgSend);
	}
}

/***********************************************************************************
 **********************************************************************************/
static void BtHfModeRunningConfig(void)
{
	APP_DBG("calling start...\n");

	DecoderSourceNumSet(BT_HF_SOURCE_NUM,DECODER_MODE_CHANNEL);

	//call resume 
	if(GetHfpScoAudioCodecType(BtCurIndex_Get()))
	{
		if(gBtHfCt->codecType != HFP_AUDIO_DATA_mSBC)
		{
			gBtHfCt->codecType = HFP_AUDIO_DATA_mSBC;
			AudioCoreSinkChange(AUDIO_HF_SCO_SINK_NUM, 1, BTHF_MSBC_SAMPLE_RATE);
		}
	}
	else
	{
		if(gBtHfCt->codecType != HFP_AUDIO_DATA_PCM)
		{
			gBtHfCt->codecType = HFP_AUDIO_DATA_PCM;
			AudioCoreSinkChange(AUDIO_HF_SCO_SINK_NUM, 1, BTHF_CVSD_SAMPLE_RATE);
		}
	}

	//��ʼ��������
	BtHf_MsbcInit();
	
	//DecoderServiceCreate(GetSysModeMsgHandle(), DECODER_BUF_SIZE_MP3, DECODER_FIFO_SIZE_FOR_MP3);
	
	DecoderServiceInit(GetSysModeMsgHandle(),DECODER_MODE_CHANNEL,DECODER_BUF_SIZE_SBC, DECODER_FIFO_SIZE_FOR_SBC);
	
	BtHf_CvsdInit();
	
	//source1 config
	AudioCoreIO	AudioIOSet;
	memset(&AudioIOSet, 0, sizeof(AudioCoreIO));
#if defined(CFG_PARA_HFP_SYNC) && !defined(CFG_PARA_HFP_FREQ_ADJUST)
	AudioIOSet.Adapt = SRC_SRA;
#else
	AudioIOSet.Adapt = SRC_ONLY;
#endif
	AudioIOSet.Sync = FALSE;
	AudioIOSet.Channels = 1;
	AudioIOSet.Net = DefaultNet;
	AudioIOSet.DataIOFunc = BtHf_ScoDataGet;
	AudioIOSet.LenGetFunc = BtHf_ScoDataLenGet;
#if	defined(CFG_PARA_HFP_SYNC) && !defined(CFG_PARA_HFP_FREQ_ADJUST)
	AudioIOSet.HighLevelCent = 60;
	AudioIOSet.LowLevelCent = 40;
#endif
	if(gBtHfCt->codecType == HFP_AUDIO_DATA_PCM)
	{
		AudioIOSet.SampleRate = BTHF_CVSD_SAMPLE_RATE;
#if defined(CFG_PARA_HFP_SYNC) && !defined(CFG_PARA_HFP_FREQ_ADJUST)
		AudioIOSet.Depth = BT_SCO_PCM_FIFO_LEN / 2;
#endif
	}
	else
	{
		AudioIOSet.SampleRate = CFG_BTHF_PARA_SAMPLE_RATE;
#if defined(CFG_PARA_HFP_SYNC) && !defined(CFG_PARA_HFP_FREQ_ADJUST)
		AudioIOSet.Depth = DECODER_FIFO_SIZE_FOR_SBC / 2; //
#endif
	}
	if(!AudioCoreSourceInit(&AudioIOSet, BT_HF_SOURCE_NUM))
	{
		DBG("Handfree source error!\n");
	}
#if defined(CFG_PARA_HFP_SYNC) && !defined(CFG_PARA_HFP_FREQ_ADJUST)
	AudioCoreSourceAdjust(APP_SOURCE_NUM, TRUE); //ͨ·������ ΢��������
#endif
	
	gBtHfCt->callingState = BT_HF_CALLING_ACTIVE;
	
	BtAppiFunc_SaveScoData(BtHf_SaveScoData);
//	BtAppiFunc_BtScoSendProcess(BtHf_SendData);//unused
	AudioCoreSourceEnable(MIC_SOURCE_NUM);
	//AudioCoreSourceUnmute(MIC_SOURCE_NUM, TRUE, TRUE);
	AudioCoreSinkEnable(AUDIO_DAC0_SINK_NUM);
	AudioCoreSinkEnable(AUDIO_HF_SCO_SINK_NUM);

	gBtHfCt->btHfScoSendReady = 1;
	gBtHfCt->btHfScoSendStart = 0;

	if(GetBtHfpVoiceRecognition())
	{
		SetHfpState(BtCurIndex_Get(),BT_HFP_STATE_ACTIVE);
	}
	DecoderSourceNumSet(BT_HF_SOURCE_NUM,DECODER_MODE_CHANNEL);
	AudioCoreSourceEnable(BT_HF_SOURCE_NUM);
	//AudioCoreSourceUnmute(BT_HF_SOURCE_NUM, TRUE, TRUE);
}

void BtHfModeRunningResume(void)
{
	if(gBtHfCt == NULL)
		return;

	//if(gBtHfCt->state == TaskStatePaused)
	{
		BtHfTaskResume();
	}
}

/************************************************************************************
 * @func        BtHfInit
 * @brief       BtHfģʽ�������ã���Դ��ʼ��
 * @param       MessageHandle   
 * @Output      None
 * @return      int32_t
 * @Others      ����顢Dac��AudioCore���ã�����Դ��DecoderService
 * @Others      ��������Decoder��audiocore���к���ָ�룬audioCore��Dacͬ������audiocoreService����������
 * @Others      ��ʼ��ͨ��ģʽ�������ģ��,��ģʽ������,����SCO��·�������������ݸ�ʽ��Ӧ�Ĵ�������������
 * Record
 ***********************************************************************************/
bool BtHfInit(void)
{
	AudioCoreIO	AudioIOSet;
	APP_DBG("BT HF init\n");
	
#ifdef BT_TWS_SUPPORT
	tws_wait_transfer();
	tws_audio_bt_state(BT_TWS_STATE_NONE);
#endif

	//�����������ȼ��ָ�����һ��,��AudioCoreServiceͬ��
	vTaskPrioritySet(GetBtStackServiceTaskHandle(), (GetBtStackServiceTaskPrio()+1));

	//DMA channel
	DMA_ChannelAllocTableSet((uint8_t *)DmaChannelMap);

#ifdef CFG_FUNC_AUDIO_EFFECT_EN
	//��Ч����������ȷ��ϵͳ֡�������޸ģ�sam, mark
#endif

	AudioCoreFrameSizeSet(DefaultNet, CFG_BTHF_PARA_SAMPLES_PER_FRAME);
	AudioCoreMixSampleRateSet(DefaultNet, CFG_BTHF_PARA_SAMPLE_RATE);
	//Task & App Config
	gBtHfCt = (BtHfContext*)osPortMalloc(sizeof(BtHfContext));
	if(gBtHfCt == NULL)
	{
		return FALSE;
	}
	memset(gBtHfCt, 0, sizeof(BtHfContext));
		
	/* Create media audio services */
	gBtHfCt->SampleRate = CFG_BTHF_PARA_SAMPLE_RATE;//����ģʽ�²�����Ϊ16K

	if(!AudioIoCommonForHfp(CFG_BTHF_PARA_SAMPLE_RATE, BT_HFP_MIC_PGA_GAIN, 2))
	{
		DBG("Mic��Dac set error\n");
	}

	//sco input/output ��������
	gBtHfCt->ScoInDataBuf = osPortMalloc(SCO_IN_DATA_SIZE);
	if(gBtHfCt->ScoInDataBuf == NULL)
	{
		return FALSE;
	}
	memset(gBtHfCt->ScoInDataBuf, 0, SCO_IN_DATA_SIZE);
	
	gBtHfCt->PcmBufForMsbc = osPortMalloc(MSBC_PACKET_PCM_SIZE);
	if(gBtHfCt->PcmBufForMsbc == NULL)
	{
		return FALSE;
	}
	memset(gBtHfCt->PcmBufForMsbc, 0, MSBC_PACKET_PCM_SIZE);
	
	//call resume 
	if(GetHfpScoAudioCodecType(BtCurIndex_Get()))
		gBtHfCt->codecType = HFP_AUDIO_DATA_mSBC;
	else
		gBtHfCt->codecType = HFP_AUDIO_DATA_PCM;
	
	//if(gBtHfCt->codecType == HFP_AUDIO_DATA_PCM)
	{
		//sco ���ջ��� fifo
		gBtHfCt->ScoInPcmFIFO = (int8_t*)osPortMalloc(BT_SCO_PCM_FIFO_LEN);
		if(gBtHfCt->ScoInPcmFIFO == NULL)
		{
			return FALSE;
		}
		MCUCircular_Config(&gBtHfCt->ScoInPcmFIFOCircular, gBtHfCt->ScoInPcmFIFO, BT_SCO_PCM_FIFO_LEN);
		memset(gBtHfCt->ScoInPcmFIFO, 0, BT_SCO_PCM_FIFO_LEN);
	}

#ifdef CFG_FUNC_AUDIO_EFFECT_EN
	AudioCoreProcessConfig((void*)AudioEffectProcessBTHF);
#else
	AudioCoreProcessConfig((void*)AudioBypassProcess);
#endif

	//sink2 for sco out
	gBtHfCt->ScoOutPcmFifo = (int16_t*)osPortMalloc(BT_SCO_PCM_FIFO_LEN);
	if(gBtHfCt->ScoOutPcmFifo == NULL)
	{
		return FALSE;
	}
	memset(gBtHfCt->ScoOutPcmFifo, 0, BT_SCO_PCM_FIFO_LEN);
	MCUCircular_Config(&gBtHfCt->ScoOutPcmFIFOCircular, gBtHfCt->ScoOutPcmFifo, BT_SCO_PCM_FIFO_LEN);
	gBtHfCt->hfpSendDataCnt = 0;

//AudioCore sink2 config -- ����HFP SCO��������
	memset(&AudioIOSet, 0, sizeof(AudioCoreIO));
#ifdef	CFG_PARA_HFP_SYNC
#ifdef CFG_PARA_HFP_FREQ_ADJUST
	AudioIOSet.Adapt = SRC_ADJUST;
#else//����΢��
	AudioIOSet.Adapt = SRC_SRA;
#endif
#else //��΢��
	AudioIOSet.Adapt = SRC_ONLY;
#endif
	AudioIOSet.Sync = FALSE;//TRUE;
	AudioIOSet.Channels = 1;
	AudioIOSet.Net = DefaultNet;
	AudioIOSet.Depth = BT_SCO_PCM_FIFO_LEN / 2;
	AudioIOSet.DataIOFunc = BtHf_SinkScoDataSet;
	AudioIOSet.LenGetFunc = BtHf_SinkScoDataSpaceLenGet;

#ifdef	CFG_PARA_HFP_SYNC
	AudioIOSet.HighLevelCent = 60;
	AudioIOSet.LowLevelCent = 40;
#endif

	if(gBtHfCt->codecType == HFP_AUDIO_DATA_PCM)
	{
		AudioIOSet.SampleRate = BTHF_CVSD_SAMPLE_RATE;
	}
	else
	{
		AudioIOSet.SampleRate = CFG_BTHF_PARA_SAMPLE_RATE;
	}

#ifdef	CFG_AUDIO_WIDTH_24BIT
	AudioIOSet.IOBitWidth = 0;//0,16bit,1:24bit
	AudioIOSet.IOBitWidthConvFlag = 0;//SCO 16bit ,����Ҫ���⴦��
#endif
	if(!AudioCoreSinkInit(&AudioIOSet, AUDIO_HF_SCO_SINK_NUM))
	{
		DBG("ScoSink init error");
		return FALSE;
	}
#ifdef	CFG_PARA_HFP_SYNC
	AudioCoreSinkAdjust(AUDIO_HF_SCO_SINK_NUM, TRUE);//ͨ·δʹ��ʱ����������
#endif

	if((gBtHfCt->ScoOutPcmFifoMutex = xSemaphoreCreateMutex()) == NULL)
	{
		return FALSE;
	}

	//��Ҫ�������ݻ���
	gBtHfCt->MsbcSendFifo = (uint8_t*)osPortMalloc(BT_SCO_PCM_FIFO_LEN);
	if(gBtHfCt->MsbcSendFifo == NULL)
	{
		return FALSE;
	}
	MCUCircular_Config(&gBtHfCt->MsbcSendCircular, gBtHfCt->MsbcSendFifo, BT_SCO_PCM_FIFO_LEN);
	memset(gBtHfCt->MsbcSendFifo, 0, BT_SCO_PCM_FIFO_LEN);

	//AEC
	BtHf_AECInit();
	//AEC effect init
	BtHf_AECEffectInit();

	//SBC receivr fifo
	gBtHfCt->msbcRecvFifo = (uint8_t*)osPortMalloc(BT_SBC_RECV_FIFO_SIZE);
	if(gBtHfCt->msbcRecvFifo == NULL) 
	{
		return FALSE;
	}
	MCUCircular_Config(&gBtHfCt->msbcRecvFifoCircular, gBtHfCt->msbcRecvFifo, BT_SBC_RECV_FIFO_SIZE);
	
	APP_DBG("Bt Handfree Call mode\n");

#ifdef BT_HFP_CALL_DURATION_DISP
	BtHfActiveTimeInit();
#endif

	SetBtHfSyncVolume(mainAppCt.HfVolume);
	AudioHfVolSet(mainAppCt.HfVolume);
	
	BtHfModeRunningConfig();
#ifdef CFG_FUNC_REMIND_SOUND_EN
	BtHfRingRemindNumberInit();
#endif
		
	APP_DBG("BT HF Run\n");

	gBtHfCt->SystemEffectMode = mainAppCt.EffectMode;

#ifdef CFG_FUNC_AUDIO_EFFECT_EN
#ifdef CFG_EFFECT_PARAM_IN_FLASH_EN
	mainAppCt.EffectMode = EFFECT_MODE_FLASH_HFP_AEC;
#else
	mainAppCt.EffectMode = EFFECT_MODE_HFP_AEC;
#endif
	AudioEffectsLoadInit(1, mainAppCt.EffectMode);
#endif

	//bt hf ģʽ��ʼ�����
	sBtHfModeEnterFlag = 0;
	BtHfModeEnterFlag = 0;
	mainAppCt.gSysVol.MuteFlag = 1;//��ʱ�����ͨ������������ (hfpvolsetʱ���mute ��ʱdac����δ��ʼ��)
	if(IsAudioPlayerMute() == TRUE)
	{
		HardWareMuteOrUnMute();
	}

	//ע�� ͨ�������м���ֻ�ͨ��״̬����
	BtHfpRunloopRegister();
	return TRUE;
}

/*****************************************************************************************
 * 
 ****************************************************************************************/
void BtHfMsgToParent(uint16_t id)
{
	MessageContext		msgSend;

	// Send message to main app
	msgSend.msgId		= id;
	MessageSend(GetMainMessageHandle(), &msgSend);
}

/*****************************************************************************************
 * 
 ****************************************************************************************/
void BtHfTaskResume(void)
{
	if((!gBtHfCt)||(gBtHfCt->ModeKillFlag))
		return;
	
	APP_DBG("Bt Handfree Mode Resume\n");
	
#ifdef CFG_PARA_HFP_SYNC
	AudioCoreSinkAdjust(AUDIO_HF_SCO_SINK_NUM, TRUE);
#ifndef CFG_PARA_HFP_FREQ_ADJUST
	AudioCoreSourceAdjust(APP_SOURCE_NUM, TRUE);
#endif
#endif
}

/*****************************************************************************************
 * 
 ****************************************************************************************/
void BtHfRun(uint16_t msgId)
{
#ifdef CFG_FUNC_AUDIO_EFFECT_EN
	if(IsEffectChange == 1)
	{
		IsEffectChange = 0;
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

	if(sBtHfModeEixtList)
	{
		//BtHfModeExit();
		flagVoiceDelayExitBtHf = 200;
		sBtHfModeEixtList = 0;
	}
	switch(msgId)
	{
		case MSG_BT_HF_MODE_CODEC_UPDATE:
		{
			if(gBtHfCt->codecType == HFP_AUDIO_DATA_mSBC)
			{
				AudioCoreSourceChange(APP_SOURCE_NUM, 1, BTHF_MSBC_SAMPLE_RATE);
				#if defined(CFG_PARA_HFP_SYNC) && !defined(CFG_PARA_HFP_FREQ_ADJUST)
				AudioCoreSourceDepthChange(APP_SOURCE_NUM, DECODER_FIFO_SIZE_FOR_SBC / 2);
				#endif
				AudioCoreSinkChange(AUDIO_HF_SCO_SINK_NUM, 1, BTHF_MSBC_SAMPLE_RATE);
			}
			else
			{
				AudioCoreSourceChange(APP_SOURCE_NUM, 1, BTHF_CVSD_SAMPLE_RATE);
				#if defined(CFG_PARA_HFP_SYNC) && !defined(CFG_PARA_HFP_FREQ_ADJUST)
				AudioCoreSourceDepthChange(APP_SOURCE_NUM, BT_SCO_PCM_FIFO_LEN / 2);
				#endif
				AudioCoreSinkChange(AUDIO_HF_SCO_SINK_NUM, 1, BTHF_CVSD_SAMPLE_RATE);
			}
			DBG("CODEC_UPDATE");
			break;
		}
		
		#ifdef CFG_FUNC_REMIND_SOUND_EN			
		case MSG_BT_HF_MODE_REMIND_PLAY:
			if(!SoftFlagGet(SoftFlagNoRemind) 
				&& (GetHfpState(BtCurIndex_Get()) == BT_HFP_STATE_INCOMING)
				&& ((gBtHfCt->CvsdInitFlag) || (gBtHfCt->MsbcInitFlag)))	
			{
				switch(bt_CallinRingType)
				{
					case USE_NUMBER_REMIND_RING:
						if (gBtHfCt->RemindSoundStart == 0)
						{
							gBtHfCt->PhoneNumberLen = GetBtCallInPhoneNumber(BtCurIndex_Get(),gBtHfCt->PhoneNumber);
							if(gBtHfCt->PhoneNumberLen > 0)
								gBtHfCt->RemindSoundStart = 1;
						}
						break;

					case USE_LOCAL_AND_PHONE_RING:
						if(!GetScoConnectFlag())
						{
							gBtHfCt->RemindSoundStart = 1;
							TimeOutSet(&gBtHfCt->CallRingTmr,0);
						}					
						break;
					
					case USE_ONLY_LOCAL_RING:
						gBtHfCt->RemindSoundStart = 1;
						TimeOutSet(&gBtHfCt->CallRingTmr,0);					
						break;
					
					default:
						break;
				}
			}
			break;
		#endif		
		
		//HFP control
		case MSG_PLAY_PAUSE:
			switch(GetHfpState(BtCurIndex_Get()))
			{
				case BT_HFP_STATE_CONNECTED:
				case BT_HFP_STATE_INCOMING:
					APP_DBG("call answer\n");
					BtStackServiceMsgSend(MSG_BTSTACK_MSG_BT_ANSWER_CALL);
					break;

				case BT_HFP_STATE_OUTGOING:
				case BT_HFP_STATE_ACTIVE:
					if(GetBtHfpVoiceRecognition())
					{
						BtStackServiceMsgSend(MSG_BTSTACK_MSG_BT_CLOSE_VOICE_RECONGNITION);
					}
					else
					{
						//�Ҷ�ͨ��
						BtStackServiceMsgSend(MSG_BTSTACK_MSG_BT_HANGUP); 
					}
					break;
						
				case BT_HFP_STATE_3WAY_INCOMING_CALL:
					//����ǰͨ��, ������һ���绰
					BtHfp_HoldCur_Answer_Call(); 
					break;
				case BT_HFP_STATE_3WAY_OUTGOING_CALL:	
				case BT_HFP_STATE_3WAY_ATCTIVE_CALL:
					//�Ҷ�ͨ��
					BtStackServiceMsgSend(MSG_BTSTACK_MSG_BT_HANGUP); 
					break;
					
				default:
					break;
			}
			break;
				
		case MSG_BT_HF_CALL_REJECT:
			switch(GetHfpState(BtCurIndex_Get()))
			{
				case BT_HFP_STATE_INCOMING:
				case BT_HFP_STATE_OUTGOING:
					APP_DBG("call reject\n");
					BtStackServiceMsgSend(MSG_BTSTACK_MSG_BT_HANGUP);
					break;
					
				case BT_HFP_STATE_ACTIVE:
					if (btManager.voiceRecognition == 0)
					{
						APP_DBG("Hfp Audio Transfer.\n");
						BtStackServiceMsgSend(MSG_BTSTACK_MSG_BT_HF_AUDIO_TRANSFER);
					}
					break;
					
				case BT_HFP_STATE_3WAY_INCOMING_CALL:
					//�Ҷϵ��������磬��������ԭ��ͨ��
					BtHfp_Hangup_Another_Call();
					break;
					
				case BT_HFP_STATE_3WAY_OUTGOING_CALL:
					//�Ҷ�ͨ��
					BtStackServiceMsgSend(MSG_BTSTACK_MSG_BT_HANGUP); 
					break;
				case BT_HFP_STATE_3WAY_ATCTIVE_CALL:
					//����������֮���л�
					BtStackServiceMsgSend(MSG_BTSTACK_MSG_BT_HF_CALL_HOLDCUR_ANSWER_CALL); 
					break;
				default:
					break;
			}
			break;
		
		case MSG_BT_HF_TRANS_CHANGED:
			if (btManager.voiceRecognition == 0)
			{
				APP_DBG("Hfp Audio Transfer.\n");
				BtStackServiceMsgSend(MSG_BTSTACK_MSG_BT_HF_AUDIO_TRANSFER);
			}
			break;
			
		case MSG_BT_HF_VOICE_RECOGNITION:
			if (btManager.voiceRecognition)
			{
				BtStackServiceMsgSend(MSG_BTSTACK_MSG_BT_CLOSE_VOICE_RECONGNITION);
			}
			break;
			
		default:
			CommonMsgProccess(msgId);
			break;
	}
	
	BtHf_EncodeProcess();
	ModeDecodeProcess();
#ifdef BT_HFP_CALL_DURATION_DISP
	BtHfActiveTimeUpdate();
#endif

#ifdef CFG_FUNC_REMIND_SOUND_EN
	BtHfRingRemindNumberRunning();
#endif

	DelayExitBtHfMode();
}


/*****************************************************************************************
 * ɾ��HF����
 ****************************************************************************************/
bool BtHfDeinit(void)
{
	//clear voice recognition flag
	if(gBtHfCt == NULL)
	{
		return TRUE;
	}
	
	//ע�� ͨ�������м���ֻ�ͨ��״̬����
	BtHfpRunloopDeregister();
	
	mainAppCt.EffectMode = gBtHfCt->SystemEffectMode;
	
	if(IsAudioPlayerMute() == FALSE)
	{
		HardWareMuteOrUnMute();
	}	
	
	PauseAuidoCore();
	
#ifdef CFG_FUNC_REMIND_SOUND_EN
	BtHfRingRemindNumberStop();
#endif	
	APP_DBG("BT HF Deinit\n");
	
	SetBtHfpVoiceRecognition(0);
	BtAppiFunc_SaveScoData(NULL);
	AudioCoreSourceDeinit(BT_HF_SOURCE_NUM)	;
	AudioCoreSinkDeinit(AUDIO_HF_SCO_SINK_NUM);

	//BtHfDeinitialize();	
	AudioCoreProcessConfig((void*)AudioNoAppProcess);

	//AEC
	BtHf_AECDeinit();

	if(gBtHfCt->MsbcSendFifo)
	{
		osPortFree(gBtHfCt->MsbcSendFifo);
		gBtHfCt->MsbcSendFifo = NULL;
	}

	//msbc
	//if(gBtHfCt->codecType == HFP_AUDIO_DATA_mSBC)
	{
		DecoderServiceDeinit(DECODER_MODE_CHANNEL);
	}

	if(gBtHfCt->msbcRecvFifo)
	{
		osPortFree(gBtHfCt->msbcRecvFifo);
		gBtHfCt->msbcRecvFifo = NULL;
	}

	if(gBtHfCt->ScoOutPcmFifoMutex != NULL)
	{
		vSemaphoreDelete(gBtHfCt->ScoOutPcmFifoMutex);
		gBtHfCt->ScoOutPcmFifoMutex = NULL;
	}

	if(gBtHfCt->ScoOutPcmFifo)
	{
		osPortFree(gBtHfCt->ScoOutPcmFifo);
		gBtHfCt->ScoOutPcmFifo = NULL;
	}

	//msbc decoder/encoder deinit
	BtHf_MsbcDecoderDeinit();
	BtHf_MsbcEncoderDeinit();

	//deinit cvsd_plc
	if(gBtHfCt->cvsdPlcState)
	{
		osPortFree(gBtHfCt->cvsdPlcState);
		gBtHfCt->cvsdPlcState = NULL;
	}

	if(gBtHfCt->ScoInPcmFIFO)
	{
		osPortFree(gBtHfCt->ScoInPcmFIFO);
		gBtHfCt->ScoInPcmFIFO = NULL;
	}

	//release used buffer: input/output sco fifo
	if(gBtHfCt->PcmBufForMsbc)
	{
		osPortFree(gBtHfCt->PcmBufForMsbc);
		gBtHfCt->PcmBufForMsbc = NULL;
	}

	if(gBtHfCt->ScoInDataBuf)
	{
		osPortFree(gBtHfCt->ScoInDataBuf);
		gBtHfCt->ScoInDataBuf = NULL;
	}

	ModeCommonDeinit();//mic dac ���� �ȴ���һ��ģʽ����
	AudioCoreMixSampleRateSet(DefaultNet, CFG_PARA_SAMPLE_RATE);

#ifdef CFG_FUNC_REMIND_SOUND_EN
	BtHfRingRemindNumberInit();
#endif	

#ifdef CFG_FUNC_AUDIO_EFFECT_EN
	//ģʽ�˳�ʱ��Ҫ�ͷ���Ч
	AudioEffectsDeInit();
#endif

	//�ָ�����ģʽ������ֵ
	//AudioMusicVolSet(mainAppCt.MusicVolume);
	AudioMusicVol(mainAppCt.MusicVolume);

	SetScoConnectFlag(FALSE);

#if (BT_LINK_DEV_NUM == 2)
	if(!BtHfModeEnterFlag)//�˳������У��д�绰��Ҫ����ͨ��ģʽ �������־
	{
		//���ͨ����־
		FirstTalkingPhoneIndexSet(0xff);
		//SecondTalkingPhoneIndexSet(0xff);//Ŀǰ����secondindex��δʹ��
	}
#endif

	osPortFree(gBtHfCt);
	gBtHfCt = NULL;
	APP_DBG("!!gBtHfCt\n");
	
#ifdef BT_TWS_SUPPORT
	if(GetBtManager()->twsState == BT_TWS_STATE_CONNECTED)
	{
		tws_sync_reinit();
		#ifdef	TWS_CODE_BACKUP //������
			tws_link_state_set(1);//BT_TWS_STATE_NONE 
		#endif
	}
#endif

	//�����������ȼ��ָ���Ĭ�����ȼ�(4)
	vTaskPrioritySet(GetBtStackServiceTaskHandle(), GetBtStackServiceTaskPrio());

	//ͨ��ģʽ�˳���ɣ������־
	BtHfModeExitFlag = 0;
	sBtHfModeEixtList = 0;

	return TRUE;
}

/*****************************************************************************************
 ****************************************************************************************/
MessageHandle GetBtHfMessageHandle(void)
{
	if(gBtHfCt == NULL)
	{
		return NULL;
	}
	return GetSysModeMsgHandle();
}

/*****************************************************************************************
 * ����/�˳�HFģʽ
 ****************************************************************************************/
void BtHfModeEnter_Index(uint8_t index)
{
	if((btManager.btLinked_env[index].btLinkState == 0)||(SoftFlagGet(SoftFlagWaitBtRemindEnd)))
	{
		DBG("btManager.btLinked_env[%d].btLinkState = %d   SoftFlagGet(SoftFlagWaitBtRemindEnd) = %d\n", index, btManager.btLinked_env[index].btLinkState, SoftFlagGet(SoftFlagWaitBtRemindEnd));
		if(!SoftFlagGet(SoftFlagDelayEnterBtHf))
			SoftFlagRegister(SoftFlagDelayEnterBtHf);
		return;
	}
	
	if(BtHfModeExitFlag)
	{
		APP_DBG("BtHfModeEnter: BtHfModeExitFlag is return\n");
		BtHfModeEnterFlag = 1;
		return;
	}

	if((!IsBtHfMode())&&(!sBtHfModeEnterFlag))
	{
		extern bool GetBtCurPlayState(void);
		if((GetSystemMode() == ModeBtAudioPlay) && GetBtCurPlayState() && (!SoftFlagGet(SoftFlagBtCurPlayStateMask)))
		{
			SoftFlagRegister(SoftFlagBtCurPlayStateMask);
		}
		
		if(GetSysModeState(ModeBtHfPlay) == ModeStateSusend)
		{
			SetSysModeState(ModeBtHfPlay,ModeStateReady);
		}
		
		BtHfMsgToParent(MSG_DEVICE_SERVICE_BTHF_IN);
		//ͨ��ģʽ�����־������
		sBtHfModeEnterFlag = 1;
		sBtHfModeEixtList = 0;
	}
	else
	{
		DBG("GetSystemMode() = %d   sBtHfModeEnterFlag = %d\n", (int)GetSystemMode(), (int)sBtHfModeEnterFlag);
	}
}

void BtHfModeEnter(void)
{
#if (BT_LINK_DEV_NUM == 2)
	if((btManager.btLinked_env[0].btLinkState == 0 && btManager.btLinked_env[1].btLinkState == 0)||(SoftFlagGet(SoftFlagWaitBtRemindEnd)))
#else
	if((btManager.btLinked_env[0].btLinkState == 0)||(SoftFlagGet(SoftFlagWaitBtRemindEnd)))
#endif
	{
		DBG("btManager.btLinked_env[0].btLinkState = %d\n", btManager.btLinked_env[0].btLinkState);
		DBG("SoftFlagGet(SoftFlagWaitBtRemindEnd) = %d\n", SoftFlagGet(SoftFlagWaitBtRemindEnd));
		if(!SoftFlagGet(SoftFlagDelayEnterBtHf))
		{
			SoftFlagRegister(SoftFlagDelayEnterBtHf);
		}
		return;
	}

	if(BtHfModeExitFlag)
	{
		APP_DBG("BtHfModeEnter: BtHfModeExitFlag is return\n");
		BtHfModeEnterFlag = 1;
		return;
	}
	DBG("GetSystemMode() = %d   sBtHfModeEnterFlag = %d\n", (int)GetSystemMode(), (int)sBtHfModeEnterFlag);
	if((GetSystemMode() != ModeBtHfPlay)&&(!sBtHfModeEnterFlag))
	{
		if((GetSystemMode() == ModeBtAudioPlay))
		{
			extern bool GetBtCurPlayState(void);
			if(GetBtCurPlayState())
			{
				if(!SoftFlagGet(SoftFlagBtCurPlayStateMask))
					SoftFlagRegister(SoftFlagBtCurPlayStateMask);
			}
		}
		
		if(GetSysModeState(ModeBtHfPlay) == ModeStateSusend)
		{
			SetSysModeState(ModeBtHfPlay,ModeStateReady);
		}
		
		BtHfMsgToParent(MSG_DEVICE_SERVICE_BTHF_IN);
		//ͨ��ģʽ�����־������
		sBtHfModeEnterFlag = 1;
	}
}

/*****************************************************************************************
 * 
 ****************************************************************************************/
void BtHfModeExit(void)
{
	SoftFlagDeregister(SoftFlagDelayEnterBtHf);

	if(flagVoiceDelayExitBtHf &&(flagVoiceDelayExitBtHf<=DELAY_EXIT_BT_HF_TIME))
	{
		return;
	}
	flagVoiceDelayExitBtHf = 0;

	DBG("BtHfModeExitFlag = %d\n", (int)BtHfModeExitFlag);
	//�˳��������ڽ���ʱ�����ٴδ�������ֹ����
	if(BtHfModeExitFlag)
		return;
	
	if(IsBtHfMode())
	{
		//��ͨ��ģʽ��δ��ȫ��ʼ����ɣ����˳�����Ϣ���뵽�����Ķ��У��ڳ�ʼ����ɺ��˳�ͨ��ģʽ
		if((gBtHfCt==NULL)||(sBtHfModeEnterFlag))
		{
			sBtHfModeEixtList = 1;
			DBG("sBtHfModeEnterFlag1 = %d\n", (int)sBtHfModeEnterFlag);
			return;
		}
		
		//ResourceDeregister(AppResourceBtHf);
		BtHfMsgToParent(MSG_DEVICE_SERVICE_BTHF_OUT);

		//���˳�ͨ��ģʽǰ,�ȹر�sink2ͨ·,����AudioCore����תʱ,ʹ�õ�sink��ص��ڴ�,��Hfģʽkill�������ͻ,����Ұָ������
		AudioCoreSinkDisable(AUDIO_HF_SCO_SINK_NUM);

		gBtHfCt->ModeKillFlag=1;

		//ͨ��ģʽ�˳���־������
		BtHfModeExitFlag = 1;
	}
	else if(sBtHfModeEnterFlag)
	{
		//��ͨ��ģʽ���̻�δ���У����˳�����Ϣ���뵽�����Ķ��У��ڽ���ͨ��ģʽ��ʼ����ɺ��˳�ͨ��ģʽ
		sBtHfModeEixtList = 1;
		DBG("sBtHfModeEnterFlag2 = %d\n", (int)sBtHfModeEnterFlag);
		return;
	}
	else
	{
		DBG("BtHfModeExit is error\n");
	}
}

/*****************************************************************************************
 * 
 ****************************************************************************************/
void DelayExitBtHfModeSet(void)
{
//#if (BT_LINK_DEV_NUM == 2)
//	if(SecondTalkingPhoneIndexGet()==0xff)//��ֹ�������ֽ�����ת����B�ֻ�ͨ��ģʽ      //Ŀǰ����secondindex��δʹ��
	flagVoiceDelayExitBtHf = 1;
//#endif
}

bool GetDelayExitBtHfMode(void)
{
	if(flagVoiceDelayExitBtHf)
	{
		return TRUE;
	}
	return FALSE;
}

/*****************************************************************************************
 * 
 ****************************************************************************************/
void DelayExitBtHfModeCancel(void)
{
	flagVoiceDelayExitBtHf = 0;
	sBtHfModeEixtList = 0;
}

void DelayExitBtHfMode(void)
{
	//delay exit hf mode
	if(flagVoiceDelayExitBtHf)
	{
		flagVoiceDelayExitBtHf++;
		if(flagVoiceDelayExitBtHf>=DELAY_EXIT_BT_HF_TIME)
		{
			flagVoiceDelayExitBtHf=0;
			BtHfModeExit();
		}
	}
}

/*****************************************************************************************
 * 
 ****************************************************************************************/
void SpecialDeviceFunc(void)
{
	if(gSpecificDevice)
	{
		if((GetSystemMode() == ModeBtHfPlay)&&(GetHfpState(BtCurIndex_Get()) >= BT_HFP_STATE_CONNECTED))
		{
			//HfpAudioTransfer();
			BtStackServiceMsgSend(MSG_BTSTACK_MSG_BT_HF_AUDIO_TRANSFER);
		}
	}
}

#endif //#if defined(CFG_APP_BT_MODE_EN) && (BT_HFP_SUPPORT == ENABLE)

