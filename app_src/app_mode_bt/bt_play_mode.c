/**
 **************************************************************************************
 * @file    bt_play_mode.c
 * @brief   
 *
 * @author  KK
 * @version V1.0.0
 *
 * $Created: 2017-12-28 18:00:00$
 *
 * @Copyright (C) 2016, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#include <string.h>
#include "clk.h"
#include "type.h"
#include "app_config.h"
#include "app_message.h"
#include "chip_info.h"
#include "gpio.h"
#include "dma.h"
#include "dac.h"
#include "audio_adc.h"
#include "main_task.h"
#include "audio_vol.h"
#include "rtos_api.h"
#include "adc_interface.h"
#include "dac_interface.h"
#include "bt_avrcp_api.h"
#include "bt_manager.h"
#include "bt_play_api.h"
#include "bt_play_mode.h"
#include "audio_core_api.h"
#include "decoder.h"
#include "audio_core_service.h"
#include "remind_sound.h"
#include "powercontroller.h"
#include "deepsleep.h"
#include "backup_interface.h"
#include "breakpoint.h"
#include "debug.h"
#include "otg_device_standard_request.h"
#include "irqn.h"
#include "otg_device_hcd.h"
#include "bt_stack_service.h"
#include "mcu_circular_buf.h"
#include "bt_app_ddb_info.h"
#include "ctrlvars.h"
#include "mode_task_api.h"
#include "bt_app_connect.h"
#include "audio_effect.h"
#include "bt_app_tws_connect.h"
#include "bt_app_tws.h"
#include "audio_effect_flash_param.h"
#include "Bt_interface.h"

#include "michio.h"


#ifdef CFG_APP_BT_MODE_EN

#define BT_PLAY_DECODER_SOURCE_NUM		1 

bool GetBtCurPlayState(void);

typedef struct _btPlayContext
{
	BT_PLAYER_STATE		curPlayState;
	uint32_t			fastControl;
	uint32_t			btCurPlayStateMaskCnt;
}BtPlayContext;


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
		7,//PERIPHERAL_ID_I2S0_TX,			//22
	#else	
		255,//PERIPHERAL_ID_I2S0_TX,		//22
	#endif	

	255,//PERIPHERAL_ID_I2S1_RX,		//23


	#if	((defined(CFG_RES_AUDIO_I2SOUT_EN )&&(CFG_RES_I2S == 1))|| defined(CFG_RES_AUDIO_I2S1OUT_EN))
		5,	//PERIPHERAL_ID_I2S1_TX,		//24
	#else
		255,//PERIPHERAL_ID_I2S1_TX,		//24
	#endif

	255,//PERIPHERAL_ID_PPWM,			//25
	255,//PERIPHERAL_ID_ADC,     		//26
	255,//PERIPHERAL_ID_SOFTWARE,		//27
};

static BtPlayContext	*BtPlayCt;
BT_A2DP_PLAYER			*a2dp_player = NULL;

extern uint32_t gBtPlaySbcDecoderInitFlag;

#ifdef BT_TWS_SUPPORT
	extern uint32_t gBtTwsSniffLinkLoss;
#endif

extern uint16_t A2DPDataLenGet(void);
extern uint16_t A2DPDataGet(void* Buf, uint16_t Samples);
static void BtPlayRunning(uint16_t msgId);


#ifdef BT_USER_VISIBILITY_STATE
	BT_USER_STATE GetBtUserState(void)
	{
		return btManager.btuserstate;
	}
	void SetBtUserState(BT_USER_STATE bt_state)
	{
		btManager.btuserstate = bt_state;
	}

	void SetBtUserVsibilityLock(bool visibility)
	{
		btManager.btuservisibility = visibility;
	}

	bool GetBtUserVsibilityLock(void)
	{
		return btManager.btuservisibility;
	}
#endif


static bool BtPlayInitRes(void)
{
	a2dp_player = (BT_A2DP_PLAYER*)osPortMalloc(sizeof(BT_A2DP_PLAYER));
	if(a2dp_player == NULL)
	{
		return FALSE;
	}
	gBtPlaySbcDecoderInitFlag = 0;
	a2dp_sbc_decoer_init();

	BtPlayCt = (BtPlayContext*)osPortMalloc(sizeof(BtPlayContext));
	if(BtPlayCt == NULL)
	{
		return FALSE;
	}
	memset(BtPlayCt, 0, sizeof(BtPlayContext));
	
	
	DecoderSourceNumSet(BT_PLAY_DECODER_SOURCE_NUM,DECODER_MODE_CHANNEL);
	AudioCoreIO	AudioIOSet;
	memset(&AudioIOSet, 0, sizeof(AudioCoreIO));

	#ifdef CFG_PARA_BT_SYNC
		#ifdef CFG_PARA_BT_FREQ_ADJUST
			AudioIOSet.Adapt = SRC_ADJUST;
		#else
			AudioIOSet.Adapt = SRC_SRA;
		#endif
	#else 
		AudioIOSet.Adapt = SRC_ONLY;
	#endif

	AudioIOSet.Sync = FALSE;
	AudioIOSet.Channels = 2;
	AudioIOSet.Net = DefaultNet;
	AudioIOSet.DataIOFunc = A2DPDataGet;
	AudioIOSet.LenGetFunc = A2DPDataLenGet;
	AudioIOSet.SampleRate = CFG_PARA_SAMPLE_RATE;
	#ifdef BT_AUDIO_AAC_ENABLE
		AudioIOSet.Depth = DECODER_FIFO_SIZE_FOR_MP3/2;
	#else
		AudioIOSet.Depth = sizeof(a2dp_player->sbc_fifo) / 119 * 128 + sizeof(a2dp_player->last_pcm_buf) / 4;
	#endif
	AudioIOSet.LowLevelCent = 40;
	AudioIOSet.HighLevelCent = 90;

	#ifdef	CFG_AUDIO_WIDTH_24BIT
		AudioIOSet.IOBitWidth = 0;
	#ifdef BT_TWS_SUPPORT
		AudioIOSet.IOBitWidthConvFlag = 0;
	#else
		AudioIOSet.IOBitWidthConvFlag = 1;
	#endif
	#endif

	if(!AudioCoreSourceInit(&AudioIOSet, BT_PLAY_DECODER_SOURCE_NUM))
	{
		DBG("btplay source error!\n");
		return FALSE;
	}

	#if (BT_AVRCP_SONG_TRACK_INFOR == ENABLE)
		extern void BtMediaInfoCallback(void *params);
		BtAppiFunc_GetMediaInfo(BtMediaInfoCallback);
	#else
		BtAppiFunc_GetMediaInfo(NULL);
	#endif	

	#if (BT_AVRCP_VOLUME_SYNC == ENABLE)
		if(GetBtManager()->avrcpSyncEnable && sys_parameter.Dongbo_volume)
		{
			AudioMusicVolSet(GetBtManager()->avrcpSyncVol);
		}
	#endif

	#ifdef CFG_FUNC_AUDIO_EFFECT_EN
		AudioCoreProcessConfig((void*)AudioMusicProcess);
	#else
		AudioCoreProcessConfig((void*)AudioBypassProcess);
	#endif

	return TRUE;
}


void BtPlayRun(uint16_t msgId)
{

	if(SoftFlagGet(SoftFlagBtCurPlayStateMask)&&(BtPlayCt->btCurPlayStateMaskCnt)&&(IsAvrcpConnected(BtCurIndex_Get())))
	{
		BtPlayCt->btCurPlayStateMaskCnt++;
		if(GetBtCurPlayState())
		{
			BtPlayCt->btCurPlayStateMaskCnt = 0;
			SoftFlagDeregister(SoftFlagBtCurPlayStateMask);
		}
		else if(BtPlayCt->btCurPlayStateMaskCnt>=15)
		{
			APP_DBG("BT memory play 1\n");
			BtStackServiceMsgSend(MSG_BTSTACK_MSG_BT_PLAY);
			BtPlayCt->btCurPlayStateMaskCnt = 0;
			SoftFlagDeregister(SoftFlagBtCurPlayStateMask);
		}
	}

	BtPlayRunning(msgId);
	ChangePlayer();

	#ifdef BT_TWS_SUPPORT
		if(btManager.TwsDacNeedTimeout)
		{
			if(IsTimeOut(&TwsDacOnTimer))
			{
				if(!btManager.TwsDacStatus)
				{
					tws_device_open_isr();
					APP_DBG("\n--- timeout dac on\n");
				}
				btManager.TwsDacNeedTimeout = FALSE;
			}
		}
	#endif
}



#if (BT_AUTO_PLAY_MUSIC == ENABLE) 
static struct 
{
	uint32_t delay_cnt;
	uint8_t  state;
	uint8_t  play_state;
}auto_play = {0,0,0};
extern uint32_t gSysTick;

void BtAutoPlayMusic(void)
{
	//������������̨�������ת��BTģʽҲ�������Զ�����
	auto_play.state 		= 1;
	auto_play.play_state 	= 0;
}

void BtAutoPlaySetAvrcpPlayStatus(uint8_t play_state)
{
	auto_play.play_state = play_state;
}

void BtAutoPlayMusicProcess(void)
{
	if(GetA2dpState(BtCurIndex_Get()) < BT_A2DP_STATE_CONNECTED && GetAvrcpState(BtCurIndex_Get()) < BT_AVRCP_STATE_CONNECTED)
		auto_play.state = 0;
	else
	{
		switch(auto_play.state)
		{
			default:
				auto_play.state = 0;
			case 0:
				break;
			case 1:
				auto_play.delay_cnt = gSysTick;
				auto_play.state = 2;
				break;
			case 2:
				// 1����ʱ�Ժ��жϵ�ǰ����״̬
				if((gSysTick > (auto_play.delay_cnt + 1000)) && (!RemindSoundIsPlay()))
				{
					auto_play.state 	= 0;
					APP_DBG("BtCurPlayState %d %d\n", GetBtPlayState(),auto_play.play_state);
					if(GetBtPlayState() != BT_PLAYER_STATE_PLAYING || auto_play.play_state == AVRCP_ADV_MEDIA_PAUSED)
						BtStackServiceMsgSend(MSG_BTSTACK_MSG_BT_PLAY);
				}				
				break;
		}
	}
}
#endif

#ifdef BT_USER_VISIBILITY_STATE
void BtPlayVisibilityStatusRun()
{
	static BT_USER_STATE CurStatus = BT_USER_STATE_NONE;

	if((GetBtUserState() == BT_USER_STATE_PREPAIR) 
		#ifdef CFG_FUNC_REMIND_SOUND_EN
			&& (!RemindSoundIsPlay())
		#endif
		) 
	{
		SetBtUserState(BT_USER_STATE_PAIRING);
		#ifdef CFG_FUNC_REMIND_SOUND_EN
			if(sys_parameter.Item_thong_bao[7]){
				RemindSoundServiceItemRequest(SOUND_REMIND_BTPAIR, REMIND_PRIO_NORMAL);
			}
		#endif
	}

	if(CurStatus != GetBtUserState())
	{
		switch (GetBtUserState())
		{
			case BT_USER_STATE_NONE:
				break;
			
			case BT_USER_STATE_RECON_BEGIAN:
				APP_DBG("---user set recon status begian\n");
				BtSetAccessMode_NoDisc_Con();
				SetBtUserVsibilityLock(TRUE);
				break;
			case BT_USER_STATE_RECON_END:
				APP_DBG("---user set recon status end\n");
				SetBtUserVsibilityLock(FALSE);
				BtSetAccessMode_Disc_Con();
				SetBtUserState(BT_USER_STATE_PREPAIR);
				break;
			case BT_USER_STATE_CONNECTED:
			case BT_USER_STATE_DISCONNECTED:
				SetBtUserVsibilityLock(FALSE);
				break;
			default:
				break;
		}
		CurStatus = GetBtUserState();
	}
}
#endif



extern uint32_t ChangePlayerStateGet(void);
static void BtPlayRunning(uint16_t msgId)
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

	#if (BT_AUTO_PLAY_MUSIC == ENABLE)
		BtAutoPlayMusicProcess();
	#endif

	#ifdef BT_USER_VISIBILITY_STATE
		BtPlayVisibilityStatusRun();
	#endif

	switch(msgId)
	{
		#ifdef BT_AUDIO_AAC_ENABLE
			case MSG_DECODER_SERVICE_DISK_ERROR:
				a2dp_sbc_decoer_init();
				APP_DBG("BT:MSG_DECODER_SERVICE_DISK_ERROR!!!\n");
				break;
		#endif

		//AVRCP CONTROL
		case MSG_PLAY_PAUSE:
			if(!IsAvrcpConnected(BtCurIndex_Get()) || ChangePlayerStateGet())
				return;
			APP_DBG("MSG_PLAY_PAUSE = %d \n",GetBtPlayState());
			if(((GetBtPlayState() == BT_PLAYER_STATE_PLAYING)
				||(GetBtPlayState() == BT_PLAYER_STATE_FWD_SEEK)
				||(GetBtPlayState() == BT_PLAYER_STATE_REV_SEEK))
				)
			{
				//pause
				BtStackServiceMsgSend(MSG_BTSTACK_MSG_BT_PAUSE);
				SetBtPlayState(BT_PLAYER_STATE_PAUSED);
				#ifdef CFG_PARA_BT_SYNC
					AudioCoreSourceAdjust(APP_SOURCE_NUM, FALSE);
				#endif
			}
			else if((GetBtPlayState() == BT_PLAYER_STATE_PAUSED) 
				|| (GetBtPlayState() == BT_PLAYER_STATE_STOP)
				|| (GetA2dpState(BtCurIndex_Get()) == BT_A2DP_STATE_CONNECTED))
			{
				//play
				BtStackServiceMsgSend(MSG_BTSTACK_MSG_BT_PLAY);
				SetBtPlayState(BT_PLAYER_STATE_PLAYING);
			}
			break;
		
		case MSG_NEXT:
			if(!IsAvrcpConnected(BtCurIndex_Get()))
				break;
			APP_DBG("MSG_NEXT\n");

			BtStackServiceMsgSend(MSG_BTSTACK_MSG_BT_NEXT);
			break;
		
		case MSG_PRE:
			if(!IsAvrcpConnected(BtCurIndex_Get()))
				break;
			APP_DBG("MSG_PRE\n");
			
			BtStackServiceMsgSend(MSG_BTSTACK_MSG_BT_PREV);
			break;
		
		case MSG_FF_START:
			if(BtPlayCt->fastControl != 0x01) 
			{
				BtPlayCt->fastControl = 0x01;
				BtStackServiceMsgSend(MSG_BTSTACK_MSG_BT_FF_START);
				APP_DBG("BT:MSG_FF_START\n");
			}
			break;
		
		case MSG_FB_START:
			if(BtPlayCt->fastControl != 0x02)
			{
				BtPlayCt->fastControl = 0x02;
				BtStackServiceMsgSend(MSG_BTSTACK_MSG_BT_FB_START);
				APP_DBG("BT:MSG_FB_START\n");
			}
			break;
		
		case MSG_FF_FB_END:
			if(BtPlayCt->fastControl == 0x01)
				BtStackServiceMsgSend(MSG_BTSTACK_MSG_BT_FF_END);
			else if(BtPlayCt->fastControl == 0x02)
				BtStackServiceMsgSend(MSG_BTSTACK_MSG_BT_FB_END);
			BtPlayCt->fastControl = 0;
			APP_DBG("BT:MSG_FF_FB_END\n");
			break;

		case MSG_BT_PLAY_VOLUME_SET:
			#if (BT_AVRCP_VOLUME_SYNC == ENABLE)
			if(sys_parameter.Dongbo_volume){
				if(GetAvrcpState(BtCurIndex_Get()) != BT_AVRCP_STATE_CONNECTED)
					break;
				APP_DBG("MSG_BT_PLAY_VOLUME_SET\n");
				BtStackServiceMsgSend(MSG_BTSTACK_MSG_BT_VOLUME_SET);
			}
			#endif
			break;

		case MSG_BT_CONNECT_MODE:
			if(GetBtDeviceConnState() != BtAccessModeGeneralAccessible)
			{
				BTSetAccessMode(BtAccessModeGeneralAccessible);
			}
			else if(GetBtDeviceConnState() == BtAccessModeGeneralAccessible)
			{
				BTSetAccessMode(BtAccessModeConnectableOnly);
			}
			break;


		case MSG_BT_PAIRING_CLEAR_DATA:

			BtDdb_EraseBtLinkInforMsg();
			BtDdb_ClearTwsDeviceAddrList();

		case MSG_BT_PAIRING:
			if((GetA2dpState(BtCurIndex_Get()) >= BT_A2DP_STATE_CONNECTED)
				|| (GetAvrcpState(BtCurIndex_Get()) >= BT_AVRCP_STATE_CONNECTED)
				)
			{
				BtStackServiceMsgSend(MSG_BTSTACK_MSG_BT_DISCONNECT_DEV_CTRL);
			}
			else
			{
				#ifdef BT_TWS_SUPPORT
					if(BtTwsPairingState())
					{
						BtStackServiceMsgSend(MSG_BTSTACK_MSG_BT_TWS_PAIRING_CANCEL);
					}
					else
				#endif
				{
					BtStackServiceMsgSend(MSG_BTSTACK_MSG_BT_CONNECT_DEV_CTRL);
				}
			}
			break;

		case MSG_BT_RST:
			if(GetBtManager()->btRstState == BT_RST_STATE_NONE)
				GetBtManager()->btRstState = BT_RST_STATE_START;
			break;
		#ifdef CFG_FUNC_REMIND_SOUND_EN
			case MSG_REMIND_PLAY_END:
				if(GetBtPlayState()  == BT_PLAYER_STATE_STOP && (!RemindSoundIsPlay()))
				{
					if(IsAudioPlayerMute() == FALSE)
					{
						HardWareMuteOrUnMute();
					}			
					AudioCoreSourceDisable(APP_SOURCE_NUM);
				}
				break;
		#endif	
		default:
			CommonMsgProccess(msgId);
			break;
	}
}


bool BtPlayInit(void)
{

	APP_MUTE_CMD(TRUE);

	bool		ret = TRUE;

	#ifdef BT_TWS_SUPPORT
	//	tws_delay = BT_TWS_DELAY_DEFAULT;
	#endif

	DMA_ChannelAllocTableSet((uint8_t *)DmaChannelMap);

	if(!ModeCommonInit())
	{
		return FALSE;
	}

	if(sys_parameter.Bt_BackgroundType == BT_BACKGROUND_DISABLE)
	{
		BtPowerOn();
	}
	else if (sys_parameter.Bt_BackgroundType == BT_BACKGROUND_POWER_ON)
	{
		if((mainAppCt.SysPrevMode != ModeBtHfPlay)&&(mainAppCt.SysPrevMode != ModeTwsSlavePlay))
		{
			BtFastPowerOn();
		}
	}
	else
	{
		BtFastPowerOn();
	}
	
	ret = BtPlayInitRes();
	APP_DBG("Bt Play mode\n");

	#ifdef CFG_FUNC_AUDIO_EFFECT_EN
		mainAppCt.EffectMode = 	data_dem.mode;
		AudioEffectsLoadInit(1, mainAppCt.EffectMode);
		AudioAPPDigitalGianProcess(mainAppCt.SysCurrentMode);
		AudioEffectsLoadInit(0, mainAppCt.EffectMode);
	#endif

	if(GetA2dpState(BtCurIndex_Get()) == BT_A2DP_STATE_STREAMING)
		SetBtPlayState(BT_PLAYER_STATE_PLAYING);
	else
		SetBtPlayState(BT_PLAYER_STATE_STOP);

	btManager.btTwsPairingStartDelayCnt = 0;
	btManager.hfp_CallFalg = 0;

	if(SoftFlagGet(SoftFlagBtCurPlayStateMask))
	{
		APP_DBG("BT memory play 0\n");
		BtPlayCt->btCurPlayStateMaskCnt = 1;
	}
	
	#ifdef CFG_FUNC_REMIND_SOUND_EN
		if(mainAppCt.SysPrevMode != ModeBtHfPlay && sys_parameter.Item_thong_bao[0])
		{
			if(RemindSoundServiceItemRequest(SOUND_REMIND_BTMODE, REMIND_PRIO_NORMAL) == FALSE)
			{
				if(IsAudioPlayerMute() == TRUE)
				{
					HardWareMuteOrUnMute();
				}
			}
		}
		else
	#endif

	btManager.HfpCurIndex = 0xff;

	#ifdef BT_AUDIO_AAC_ENABLE
		DecoderServiceInit(GetSysModeMsgHandle(),DECODER_MODE_CHANNEL, DECODER_BUF_SIZE, DECODER_FIFO_SIZE_FOR_MP3);
	#endif


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
		AudioMusicVol32(mainAppCt.MusicVolume_VR);
	}

	return ret;
}





bool BtPlayDeinit(void)
{

	APP_MUTE_CMD(TRUE);
	
	if(BtPlayCt == NULL)
	{
		return TRUE;
	}
	
	APP_DBG("Bt Play mode Deinit\n");

	if(IsAudioPlayerMute() == FALSE)
	{
		HardWareMuteOrUnMute();
	}	

	PauseAuidoCore();
	
	AudioCoreProcessConfig((void*)AudioNoAppProcess);
	AudioCoreSourceUnmute(APP_SOURCE_NUM,TRUE,TRUE);
	AudioCoreSourceDisable(BT_PLAY_DECODER_SOURCE_NUM);

	AudioCoreSourceDeinit(BT_PLAY_DECODER_SOURCE_NUM);
	ModeCommonDeinit();

	#ifndef CFG_FUNC_MIXER_SRC_EN
		#ifdef CFG_RES_AUDIO_DACX_EN
			AudioDAC_SampleRateChange(ALL, CFG_PARA_SAMPLE_RATE);
		#endif
		#ifdef CFG_RES_AUDIO_DAC0_EN
			AudioDAC_SampleRateChange(DAC0, CFG_PARA_SAMPLE_RATE);
		#endif
	#endif
		
	osMutexLock(SbcDecoderMutex);
	osPortFree(a2dp_player);
	a2dp_player = NULL;
	osMutexUnlock(SbcDecoderMutex);
	
	APP_DBG("!!BtPlayCt\n");

	if(GetSysModeState(ModeBtHfPlay) != ModeStateInit && GetSysModeState(ModeTwsSlavePlay) != ModeStateInit)
	{
		if( sys_parameter.Bt_BackgroundType != BT_BACKGROUND_POWER_ON)
		{
			GetBtManager()->btReconnectDelayCount = 0;
			
			uint8_t i;
			for(i=0; i<BT_LINK_DEV_NUM;i++)
				BTDisconnect(i);
			i = 0;
			while(btManager.linkedNumber != 0)
			{
				//APP_DBG("...\n");
				vTaskDelay(10);
				if(i++ >= 200)
					break;
			}
		}

		if(sys_parameter.Bt_BackgroundType == BT_BACKGROUND_DISABLE)
		{
			BtPowerOff();
		}
		else if (sys_parameter.Bt_BackgroundType == BT_BACKGROUND_FAST_POWER_ON_OFF)
		{
			BtFastPowerOff();
		}
		else
		{
			#if (defined(BT_TWS_SUPPORT) && ((CFG_TWS_ONLY_IN_BT_MODE == ENABLE)))
			BtReconnectTwsStop();
			BtTwsDeviceDisconnectExt();
			#endif
		}
	}
	else if( sys_parameter.Bt_BackgroundType != BT_BACKGROUND_POWER_ON)
	{
		BtStackServiceWaitResume();
	}

	#ifdef CFG_FUNC_AUDIO_EFFECT_EN
		AudioEffectsDeInit();
	#endif

	osPortFree(BtPlayCt);
	BtPlayCt = NULL;

	#ifdef BT_AUDIO_AAC_ENABLE
		DecoderServiceDeinit(DECODER_MODE_CHANNEL);
		void BtDecoderDeinit(void);
		BtDecoderDeinit();
	#endif

	osTaskDelay(10);

	#ifdef BT_TWS_SUPPORT
	//	tws_delay = BT_TWS_DELAY_DEINIT;
	//	tws_delay_set(tws_delay);
	#endif

	return TRUE;
}


void SetBtPlayState(uint8_t state)
{
	if(!BtPlayCt)
		return;

	if(BtPlayCt->curPlayState != state)
	{
		BtPlayCt->curPlayState = state;
		//APP_DBG("BtPlayState[%d]", BtPlayCt->curPlayState);
	}
}

BT_PLAYER_STATE GetBtPlayState(void)
{
	if(!BtPlayCt)
		return 0;
	else
		return BtPlayCt->curPlayState;
}

bool GetBtCurPlayState(void)
{
	if(!BtPlayCt)
		return 0;
	else
		return (BtPlayCt->curPlayState == BT_PLAYER_STATE_PLAYING);
}

#endif//#ifdef CFG_APP_BT_MODE_EN
