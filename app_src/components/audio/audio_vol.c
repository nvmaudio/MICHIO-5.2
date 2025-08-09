/**
 **************************************************************************************
 * @file    audio_vol.c
 * @brief   audio syetem vol set here
 *
 * @author  Sam
 * @version V1.0.0
 *
 * $Created: 2016-1-7 15:42:47$
 *
 * @Copyright (C) 2016, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#include "type.h"
#include "app_config.h"
#include "app_message.h"
#include "dac.h"
#include "audio_adc.h"
#include "adc_interface.h"
#include "dac_interface.h"
#include "debug.h"
#include "audio_vol.h"
#include "audio_core_api.h"
#include "main_task.h"
#include "timeout.h"
#include "bt_manager.h"
#include "bt_play_mode.h"
#include "remind_sound.h"
#include "bt_app_avrcp_deal.h"
#if (BT_HFP_SUPPORT == ENABLE)
#include "bt_hf_mode.h"
#include "bt_hf_api.h"
#endif
#include "ctrlvars.h"
#ifdef CFG_FUNC_RTC_EN
#include "rtc_ctrl.h"
#endif
#include "breakpoint.h"
#include "eq_params.h"
#include "bt_app_tws.h"
#ifdef BT_TWS_SUPPORT
#include "bt_tws_api.h"
#endif

#ifdef CFG_FUNC_REMIND_SOUND_EN
#include "remind_sound.h"
#endif

#include "i2s.h"
#include "delay.h"
#include "hdmi_in_api.h"


#include "hdmi_in_api.h"



#include "spi_flash.h"
#include "michio.h"


bool gIsVolSetEnable = FALSE;

int32_t SetChannel = 0xff;
static int32_t Cnt = -1;

uint8_t ChannelValid[AUDIO_CORE_SOURCE_MAX_NUM + AUDIO_CORE_SINK_MAX_NUM] = {0};
static volatile bool WhetherRecMusic = 1;
extern HDMIInfo *gHdmiCt;

TIMER MenuTimer;
TIMER Volume_max;

#ifdef BT_TWS_SUPPORT
#include "bt_tws_api.h"
extern void tws_master_mute_send(bool MuteFlag);
#endif




const uint16_t gSysVolArr[CFG_PARA_MAX_VOLUME_NUM + 1] =
{
	0 /*-72db*/,
	6 /*-56db*/, 41 /*-40db*/, 145 /*-29db*/, 258 /*-24db*/, 410 /*-20db*/, 576 /*-17db*/, 728 /*-15db*/, 917 /*-13db*/,
	1154 /*-11db*/, 1453 /*-9db*/, 1830 /*-7db*/, 2303 /*-5db*/, 2900 /*-3db*/, 3254 /*-2db*/, 3651 /*-1db*/, 4096 /*0db*/
};



const uint16_t gSysVolArr32[33] =
{
	0 /*-72db*/,
	3 /*-56db*/, 6 /*-56db*/, 15 /*-49db*/, 26 /*-44db*/, 41 /*-40db*/, 65 /*-36db*/, 103 /*-32db*/, 145 /*-29db*/,
	205 /*-26db*/, 258 /*-24db*/, 325 /*-22db*/, 410 /*-20db*/, 460 /*-19db*/, 516 /*-18db*/, 576 /*-17db*/, 649 /*-16db*/,
	728 /*-15db*/, 817 /*-14db*/, 917 /*-13db*/, 1029 /*-12db*/, 1154 /*-11db*/, 1295 /*-10db*/, 1453 /*-9db*/, 1631 /*-8db*/,
	1830 /*-7db*/, 2053 /*-6db*/, 2303 /*-5db*/, 2584 /*-4db*/, 2900 /*-3db*/, 3254 /*-2db*/, 3651 /*-1db*/, 4096 /*0db*/
};



uint8_t gBtAbsVolTable[17] = {
	0x00, 0x07, 0x0f, 0x17, 0x1f, 0x27, 0x2f, 0x37, 0x3f, 0x47,
	0x4f, 0x57, 0x5f, 0x67, 0x6f, 0x77, 0x7f};

uint8_t gBtAbsVolSetTable[17] = {
	0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38, 0x40, 0x48,
	0x50, 0x58, 0x60, 0x68, 0x70, 0x78, 0x7f};




uint8_t BtAbsVolume2VolLevel(uint8_t absValue)
{
	uint8_t i;
	for (i = 0; i <= 16; i++)
	{
		if (absValue == gBtAbsVolTable[i])
			return i;

		if (absValue < gBtAbsVolTable[i])
			return (i - 1);
	}
	if (i > 16)
		i = 16;
	return i;
}

uint8_t BtLocalVolLevel2AbsVolme(uint8_t localValue)
{
	return gBtAbsVolSetTable[localValue];
}

void HardWareMuteOrUnMute(void)
{
	mainAppCt.gSysVol.MuteFlag = !mainAppCt.gSysVol.MuteFlag;

	TimeOutSet(&MenuTimer, 5000);
	if (IsAudioPlayerMute() == TRUE)
	{
		#ifdef BT_TWS_SUPPORT
			tws_master_mute_send(TRUE);
		#endif
		APP_DBG("Mute Digital\n");
	}
	else
	{
		APP_DBG("Demute Digital\n");

		SystemVolSet();


		#ifdef BT_TWS_SUPPORT
			if (GetSystemMode() != ModeTwsSlavePlay)
			{
				tws_vol_send(mainAppCt.MusicVolume, get_APP_MUTE());
			}
		#endif
	}
}


void Digital_Mute(bool Mute)
{
	mainAppCt.gSysVol.MuteFlag = Mute;
	if(Mute)
	{
		#ifdef BT_TWS_SUPPORT
			tws_master_mute_send(TRUE);
		#endif

		APP_DBG("Mute Digital\n");
	}else{
		APP_DBG("Demute Digital\n");
		SystemVolSet();

		#ifdef BT_TWS_SUPPORT		
			if(GetSystemMode() != ModeTwsSlavePlay)
			{
				tws_vol_send(mainAppCt.MusicVolume, get_APP_MUTE());
			}
		#endif
	}
}

bool IsAudioPlayerMute(void)
{
	return mainAppCt.gSysVol.MuteFlag;
}

void AudioPlayerMenu(void)
{
	uint32_t ValidNum = 0;
	;
	uint32_t i;

	TimeOutSet(&MenuTimer, 5000);
	gIsVolSetEnable = TRUE;

	for (i = 0; i < AUDIO_CORE_SOURCE_MAX_NUM; i++)
	{
		if (mainAppCt.AudioCore->AudioSource[i].Enable)
		{
			ChannelValid[ValidNum] = i;
			ValidNum++;
		}
	}
	for (i = 0; i < AUDIO_CORE_SINK_MAX_NUM; i++)
	{
		if (mainAppCt.AudioCore->AudioSink[i].Enable)
		{
			ChannelValid[ValidNum] = AUDIO_CORE_SOURCE_MAX_NUM + i; // VOL_SET_CHANNEL_BASE_SINK
			ValidNum++;
		}
	}

	APP_DBG("Cnt=%d\n", (int)Cnt);
	SetChannel = ChannelValid[Cnt > ValidNum ? ValidNum : Cnt];

	Cnt++;
	if (Cnt > ValidNum)
	{
		Cnt = 0;
	}

	// APP_DBG
	{
		switch (SetChannel)
		{
		case MIC_SOURCE_NUM:
			APP_DBG("MIC_SOURCE_NUM vol set\n");
			break;
		case APP_SOURCE_NUM:
			APP_DBG("APP_SOURCE_NUM vol set\n");
			break;
#ifdef CFG_FUNC_REMIND_SOUND_EN
		case REMIND_SOURCE_NUM:
			APP_DBG("REMIND_SOURCE_NUM vol set\n");
#endif
			break;
			//		  case PLAYBACK_SOURCE_NUM:
			//			APP_DBG("PLAYBACK_SOURCE_NUM vol set\n");
			//		break;

		case AUDIO_DAC0_SINK_NUM + AUDIO_CORE_SOURCE_MAX_NUM:
			APP_DBG("Sink0 vol set(dac0)\n");
			break;

#if defined(CFG_RES_AUDIO_DACX_EN)
		case AUDIO_DACX_SINK_NUM + AUDIO_CORE_SOURCE_MAX_NUM:
			APP_DBG("Sink2 vol set(DACX)\n");
			break;
#endif
		default:
			break;
		}
	}
}

void AudioPlayerMenuCheck(void)
{
	if (gIsVolSetEnable && IsTimeOut(&MenuTimer))
	{
		gIsVolSetEnable = FALSE;
		SetChannel = 0xff;
		if (Cnt > 0)
		{
			Cnt--;
		}
		else
		{
			Cnt = AUDIO_CORE_SOURCE_MAX_NUM + AUDIO_CORE_SINK_MAX_NUM;
		}
		APP_DBG("Menu Timer OUT\n");
	}
}

uint8_t AudioMusicVolGet(void)
{
	return mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM];
}

void update_volume(void)
{
	uint32_t addr = get_sys_parameter_addr();
	SpiFlashErase(SECTOR_ERASE, addr /4096 , 1);
	SpiFlashWrite(addr, (uint8_t*)&sys_parameter, 4096, 1);
}


void AudioMusicVolUp(void)
{

	if(sys_parameter.Vr_en)
	{
		return;
	}


	if(IsAudioPlayerMute() == TRUE)
	{
		HardWareMuteOrUnMute();
	}
	
	if (mainAppCt.MusicVolume < CFG_PARA_MAX_VOLUME_NUM)
	{
		mainAppCt.MusicVolume++;

		#ifdef BT_TWS_SUPPORT
			tws_vol_send(mainAppCt.MusicVolume, get_APP_MUTE());
		#endif

		#ifdef CFG_FUNC_BREAKPOINT_EN
			BackupInfoUpdata(BACKUP_SYS_INFO);
		#endif

		if(sys_parameter.Nho_volume){
			data_dem.volume = mainAppCt.MusicVolume;
			sys_parameter_Flash();
		}
	}

	if (mainAppCt.MusicVolume > CFG_PARA_MAX_VOLUME_NUM)
	{
		if (IsTimeOut(&Volume_max))
		{
			if(sys_parameter.Item_thong_bao[6]){
				RemindSoundServiceItemRequest(SOUND_REMIND_VOLMAX, REMIND_ATTR_NEED_MIX);					
			}
			TimeOutSet(&Volume_max, 5000);
		}
		return;
	}

	mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM] = mainAppCt.MusicVolume;

	AudioCoreSourceVolSet(APP_SOURCE_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]]);

	APP_DBG("VOL =  %d\n",mainAppCt.MusicVolume);

	#ifdef CFG_APP_BT_MODE_EN
		#if (BT_AVRCP_VOLUME_SYNC == ENABLE)
			if(sys_parameter.Dongbo_volume){
				if (GetSystemMode() == ModeBtAudioPlay)
				{
					MessageContext msgSend;
					SetBtSyncVolume(mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]);
					msgSend.msgId = MSG_BT_PLAY_VOLUME_SET;
					MessageSend(GetSysModeMsgHandle(), &msgSend);
				}
			}
		#endif


	#endif
}


void AudioMusicVolDown(void)
{
	if(sys_parameter.Vr_en)
	{
		return;
	}

	if(IsAudioPlayerMute() == TRUE)
	{
		HardWareMuteOrUnMute();
	}
	

	if (mainAppCt.MusicVolume > 0)
	{
		mainAppCt.MusicVolume--;
		
		#ifdef BT_TWS_SUPPORT
			tws_vol_send(mainAppCt.MusicVolume, get_APP_MUTE());
		#endif
		#ifdef CFG_FUNC_BREAKPOINT_EN
			BackupInfoUpdata(BACKUP_SYS_INFO);
		#endif

		if(sys_parameter.Nho_volume){
			data_dem.volume = mainAppCt.MusicVolume;
			sys_parameter_Flash();
		}
	}

	mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM] = mainAppCt.MusicVolume;

	AudioCoreSourceVolSet(APP_SOURCE_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]]);
	
	APP_DBG("VOL =  %d\n",mainAppCt.MusicVolume);
	
	#ifdef CFG_APP_BT_MODE_EN
		#if (BT_AVRCP_VOLUME_SYNC == ENABLE)
		if(sys_parameter.Dongbo_volume){
			if (GetSystemMode() == ModeBtAudioPlay)
			{
				MessageContext msgSend;
				SetBtSyncVolume(mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]);
				msgSend.msgId = MSG_BT_PLAY_VOLUME_SET;
				MessageSend(GetSysModeMsgHandle(), &msgSend);
			}
		}
		#endif
	#endif

}



void AudioMusicVol(uint8_t musicVol)
{
	if(IsAudioPlayerMute() == TRUE)
	{
		HardWareMuteOrUnMute();
	}
	

	if (musicVol > CFG_PARA_MAX_VOLUME_NUM)
	{
		mainAppCt.MusicVolume = CFG_PARA_MAX_VOLUME_NUM;
	}
	else
	{
		mainAppCt.MusicVolume = musicVol;
	}

	mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM] = mainAppCt.MusicVolume;
	APP_DBG("APP_SOURCE_NUM vol = %d\n", mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]);
	AudioCoreSourceVolSet(APP_SOURCE_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]]);
}


void AudioMusicVol32(uint8_t musicVol)
{
	if (musicVol > 32)
	{
		mainAppCt.MusicVolume_VR 	= 32;
		mainAppCt.MusicVolume 		= 16;
	}
	else
	{
		mainAppCt.MusicVolume_VR = musicVol;
		mainAppCt.MusicVolume = (mainAppCt.MusicVolume_VR + 1) / 2;
	}

	APP_DBG("AudioMusicVol32 vol = %d   %d\n", mainAppCt.MusicVolume_VR , mainAppCt.MusicVolume);

	#ifdef BT_TWS_SUPPORT
		if (GetSystemMode() != ModeTwsSlavePlay)
		{
			tws_vol_send(mainAppCt.MusicVolume, get_APP_MUTE());
		}
	#endif

	mainAppCt.gSysVol.AudioSourceVol32[APP_SOURCE_NUM] = mainAppCt.MusicVolume_VR;

	//APP_DBG("Vol = %d\n", gSysVolArr32[mainAppCt.gSysVol.AudioSourceVol32[APP_SOURCE_NUM]]);;

	AudioCoreSourceVolSet(APP_SOURCE_NUM, gSysVolArr32[mainAppCt.gSysVol.AudioSourceVol32[APP_SOURCE_NUM]], gSysVolArr32[mainAppCt.gSysVol.AudioSourceVol32[APP_SOURCE_NUM]]);
}



void AudioMusicVolSet(uint8_t musicVol)
{ 
	if(IsAudioPlayerMute() == TRUE)
	{
		HardWareMuteOrUnMute();
	}

	AudioMusicVol(musicVol);
}




#ifdef CFG_APP_BT_MODE_EN
void AudioHfVolSet(uint8_t HfVol)
{
	if (IsAudioPlayerMute() == TRUE)
	{
		HardWareMuteOrUnMute();
	}

	if (HfVol > CFG_PARA_MAX_VOLUME_NUM)
		mainAppCt.HfVolume = CFG_PARA_MAX_VOLUME_NUM;
	else
		mainAppCt.HfVolume = HfVol;
	mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM] = mainAppCt.HfVolume;

	APP_DBG("source1 vol = %d\n", mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]);
	AudioCoreSourceVolSet(APP_SOURCE_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]]);
}
#endif



#if CFG_RES_MIC_SELECT
void AudioMicVolUp(void)
{
	if (IsAudioPlayerMute() == TRUE)
	{
		HardWareMuteOrUnMute();
	}

	if (mainAppCt.MicVolume < CFG_PARA_MAX_VOLUME_NUM)
	{
		mainAppCt.MicVolume++;
#ifdef CFG_FUNC_BREAKPOINT_EN
		BackupInfoUpdata(BACKUP_SYS_INFO);
#endif
	}
	mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM] = mainAppCt.MicVolume;
	APP_DBG("MIC_SOURCE_NUM vol = %d\n", mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM]);
	AudioCoreSourceVolSet(MIC_SOURCE_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM]]);
}

void AudioMicVolDown(void)
{
	if (IsAudioPlayerMute() == TRUE)
	{
		HardWareMuteOrUnMute();
	}

	if (mainAppCt.MicVolume > 0)
	{
		mainAppCt.MicVolume--;
#ifdef CFG_FUNC_BREAKPOINT_EN
		BackupInfoUpdata(BACKUP_SYS_INFO);
#endif
	}
	mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM] = mainAppCt.MicVolume;
	APP_DBG("MIC_SOURCE_NUM vol = %d\n", mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM]);
	AudioCoreSourceVolSet(MIC_SOURCE_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM]]);
}
#endif

void SystemVolUp(void)
{
	gIsVolSetEnable = TRUE;
	if (gIsVolSetEnable == TRUE)
	{
		APP_DBG("vol up \n");
		TimeOutSet(&MenuTimer, 5000);
		switch (SetChannel)
		{

		case AUDIO_DAC0_SINK_NUM + AUDIO_CORE_SOURCE_MAX_NUM: // sink0
			if (mainAppCt.gSysVol.AudioSinkVol[AUDIO_DAC0_SINK_NUM] < CFG_PARA_MAX_VOLUME_NUM)
			{
				mainAppCt.gSysVol.AudioSinkVol[AUDIO_DAC0_SINK_NUM]++;
			}

			APP_DBG("dac 0sink0 vol = %d\n", mainAppCt.gSysVol.AudioSinkVol[AUDIO_DAC0_SINK_NUM]);
			AudioCoreSinkVolSet(AUDIO_DAC0_SINK_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSinkVol[AUDIO_DAC0_SINK_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSinkVol[AUDIO_DAC0_SINK_NUM]]);
			break;
#ifdef CFG_FUNC_RECORDER_EN
		case AUDIO_RECORDER_SINK_NUM + AUDIO_CORE_SOURCE_MAX_NUM: // sink1
			if (mainAppCt.gSysVol.AudioSinkVol[AUDIO_RECORDER_SINK_NUM] < CFG_PARA_MAX_VOLUME_NUM)
			{
				mainAppCt.gSysVol.AudioSinkVol[AUDIO_RECORDER_SINK_NUM]++;
			}

			APP_DBG("rec sink1 vol = %d\n", mainAppCt.gSysVol.AudioSinkVol[AUDIO_RECORDER_SINK_NUM]);
			AudioCoreSinkVolSet(AUDIO_RECORDER_SINK_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSinkVol[AUDIO_RECORDER_SINK_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSinkVol[AUDIO_RECORDER_SINK_NUM]]);
			break;
#endif
#if defined(CFG_RES_AUDIO_DACX_EN)
		case AUDIO_DACX_SINK_NUM + AUDIO_CORE_SOURCE_MAX_NUM:
			if (mainAppCt.gSysVol.AudioSinkVol[AUDIO_DACX_SINK_NUM] < CFG_PARA_MAX_VOLUME_NUM)
			{
				mainAppCt.gSysVol.AudioSinkVol[AUDIO_DACX_SINK_NUM]++;
			}

			APP_DBG("dacx sink2 vol = %d\n", mainAppCt.gSysVol.AudioSinkVol[AUDIO_DACX_SINK_NUM]);
			AudioCoreSinkVolSet(AUDIO_DACX_SINK_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSinkVol[AUDIO_DACX_SINK_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSinkVol[AUDIO_DACX_SINK_NUM]]);
			break;
#endif
		case MIC_SOURCE_NUM: // source0
			if (mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM] < CFG_PARA_MAX_VOLUME_NUM)
			{
				mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM]++;
			}

			APP_DBG("MIC_SOURCE_NUM vol = %d\n", mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM]);
			AudioCoreSourceVolSet(MIC_SOURCE_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM]]);
			break;
		case APP_SOURCE_NUM: // source1
			if (mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM] < CFG_PARA_MAX_VOLUME_NUM)
			{
				mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]++;
			}

			APP_DBG("APP_SOURCE_NUM vol = %d\n", mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]);

			// #ifdef BT_TWS_SUPPORT
			//				AudioCoreSourceVolSet(TWS_SOURCE_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]]);
			// #endif
			AudioCoreSourceVolSet(APP_SOURCE_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]]);

#ifdef CFG_APP_BT_MODE_EN
#if (BT_AVRCP_VOLUME_SYNC == ENABLE)
			if(sys_parameter.Dongbo_volume){
				if (GetSystemMode() == ModeBtAudioPlay)
				{
					MessageContext msgSend;
					SetBtSyncVolume(mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]);
					msgSend.msgId = MSG_BT_PLAY_VOLUME_SET;
					MessageSend(GetSysModeMsgHandle(), &msgSend);
				}
			}
#endif

#if (BT_HFP_SUPPORT == ENABLE)
			if (GetSystemMode() == ModeBtHfPlay)
			{
				SetBtHfSyncVolume(mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]);
			}
#endif
#endif
			break;
#ifdef CFG_FUNC_REMIND_SOUND_EN
		case REMIND_SOURCE_NUM: // source2
			if (mainAppCt.gSysVol.AudioSourceVol[REMIND_SOURCE_NUM] < CFG_PARA_MAX_VOLUME_NUM)
			{
				mainAppCt.gSysVol.AudioSourceVol[REMIND_SOURCE_NUM]++;
			}

			APP_DBG("REMIND_SOURCE_NUM vol = %d\n", mainAppCt.gSysVol.AudioSourceVol[REMIND_SOURCE_NUM]);
			AudioCoreSourceVolSet(REMIND_SOURCE_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[REMIND_SOURCE_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[REMIND_SOURCE_NUM]]);
			break;
#endif
#ifdef CFG_FUNC_RECORDER_EN
		case PLAYBACK_SOURCE_NUM: // source3
			if (mainAppCt.gSysVol.AudioSourceVol[PLAYBACK_SOURCE_NUM] < CFG_PARA_MAX_VOLUME_NUM)
			{
				mainAppCt.gSysVol.AudioSourceVol[PLAYBACK_SOURCE_NUM]++;
			}

			APP_DBG("PLAYBACK_SOURCE_NUM vol = %d\n", mainAppCt.gSysVol.AudioSourceVol[PLAYBACK_SOURCE_NUM]);
			AudioCoreSourceVolSet(PLAYBACK_SOURCE_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[PLAYBACK_SOURCE_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[PLAYBACK_SOURCE_NUM]]);
			break;
#endif
			/*
						case VOL_SET_CHANNEL_SOURCE4://source3
							if(mainAppCt.gSysVol.AudioSourceVol[4] < CFG_PARA_MAX_VOLUME_NUM)
							{
								mainAppCt.gSysVol.AudioSourceVol[4]++;
							}

							APP_DBG("source4 vol = %d\n", mainAppCt.gSysVol.AudioSourceVol[4]);
							AudioCoreSourceVolSet(4, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[4]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[4]]);
							break;
			*/
		default:
			break;
		}
	}
	else
	{
		APP_DBG("vol set disable\n");
	}
}

void SystemVolDown(void)
{
	// if(mainAppCt.gSysVol.MuteFlag)
	if (IsAudioPlayerMute() == TRUE)
	{
		HardWareMuteOrUnMute();
	}
	gIsVolSetEnable = TRUE;

	if (gIsVolSetEnable == TRUE)
	{
		APP_DBG("vol down \n");
		TimeOutSet(&MenuTimer, 5000);
		switch (SetChannel)
		{

		case AUDIO_DAC0_SINK_NUM + AUDIO_CORE_SOURCE_MAX_NUM: // sink0
			if (mainAppCt.gSysVol.AudioSinkVol[AUDIO_DAC0_SINK_NUM] > 0)
			{
				mainAppCt.gSysVol.AudioSinkVol[AUDIO_DAC0_SINK_NUM]--;
			}

			APP_DBG("dac0 sink vol = %d\n", mainAppCt.gSysVol.AudioSinkVol[AUDIO_DAC0_SINK_NUM]);
			AudioCoreSinkVolSet(AUDIO_DAC0_SINK_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSinkVol[AUDIO_DAC0_SINK_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSinkVol[AUDIO_DAC0_SINK_NUM]]);
			break;
#ifdef CFG_FUNC_RECORDER_EN
		case AUDIO_RECORDER_SINK_NUM + AUDIO_CORE_SOURCE_MAX_NUM: // sink1
			if (mainAppCt.gSysVol.AudioSinkVol[AUDIO_RECORDER_SINK_NUM] > 0)
			{
				mainAppCt.gSysVol.AudioSinkVol[AUDIO_RECORDER_SINK_NUM]--;
			}

			APP_DBG("rec sink vol = %d\n", mainAppCt.gSysVol.AudioSinkVol[AUDIO_RECORDER_SINK_NUM]);
			AudioCoreSinkVolSet(AUDIO_RECORDER_SINK_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSinkVol[AUDIO_RECORDER_SINK_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSinkVol[AUDIO_RECORDER_SINK_NUM]]);
			break;
#endif
#if defined(CFG_RES_AUDIO_DACX_EN)
		case AUDIO_DACX_SINK_NUM + AUDIO_CORE_SOURCE_MAX_NUM:
			if (mainAppCt.gSysVol.AudioSinkVol[AUDIO_DACX_SINK_NUM] > 0)
			{
				mainAppCt.gSysVol.AudioSinkVol[AUDIO_DACX_SINK_NUM]--;
			}

			APP_DBG("dacx sink vol = %d\n", mainAppCt.gSysVol.AudioSinkVol[AUDIO_DACX_SINK_NUM]);
			AudioCoreSinkVolSet(AUDIO_DACX_SINK_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSinkVol[AUDIO_DACX_SINK_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSinkVol[AUDIO_DACX_SINK_NUM]]);
			break;
#endif

		case MIC_SOURCE_NUM: // source0
			if (mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM] > 0)
			{
				mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM]--;
			}

			APP_DBG("MIC_SOURCE_NUM vol = %d\n", mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM]);
			AudioCoreSourceVolSet(MIC_SOURCE_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM]]);
			break;
		case APP_SOURCE_NUM: // source1
			if (mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM] > 0)
			{
				mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]--;
			}

			APP_DBG("APP_SOURCE_NUM vol = %d\n", mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]);
			AudioCoreSourceVolSet(APP_SOURCE_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]]);
// #ifdef BT_TWS_SUPPORT
//				AudioCoreSourceVolSet(TWS_SOURCE_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]]);
// #endif
#ifdef CFG_APP_BT_MODE_EN
#if (BT_AVRCP_VOLUME_SYNC == ENABLE)
			if(sys_parameter.Dongbo_volume){
				if (GetSystemMode() == ModeBtAudioPlay)
				{
					MessageContext msgSend;
					SetBtSyncVolume(mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]);
					msgSend.msgId = MSG_BT_PLAY_VOLUME_SET;
					MessageSend(GetSysModeMsgHandle(), &msgSend);
				}
			}
#endif

#if (BT_HFP_SUPPORT == ENABLE)
			if (GetSystemMode() == ModeBtHfPlay)
			{
				SetBtHfSyncVolume(mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]);
			}
#endif
#endif
			break;
#ifdef CFG_FUNC_REMIND_SOUND_EN
		case REMIND_SOURCE_NUM: // source2
			if (mainAppCt.gSysVol.AudioSourceVol[REMIND_SOURCE_NUM] > 0)
			{
				mainAppCt.gSysVol.AudioSourceVol[REMIND_SOURCE_NUM]--;
			}

			APP_DBG("REMIND_SOURCE_NUM vol = %d\n", mainAppCt.gSysVol.AudioSourceVol[REMIND_SOURCE_NUM]);
			AudioCoreSourceVolSet(REMIND_SOURCE_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[REMIND_SOURCE_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[REMIND_SOURCE_NUM]]);
			break;
#endif
#ifdef CFG_FUNC_RECORDER_EN
		case PLAYBACK_SOURCE_NUM: // source3
			if (mainAppCt.gSysVol.AudioSourceVol[PLAYBACK_SOURCE_NUM] > 0)
			{
				mainAppCt.gSysVol.AudioSourceVol[PLAYBACK_SOURCE_NUM]--;
			}

			APP_DBG("PLAYBACK_SOURCE_NUM vol = %d\n", mainAppCt.gSysVol.AudioSourceVol[PLAYBACK_SOURCE_NUM]);
			AudioCoreSourceVolSet(PLAYBACK_SOURCE_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[PLAYBACK_SOURCE_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[PLAYBACK_SOURCE_NUM]]);
			break;
#endif

		default:
			break;
		}
	}
	else
	{
		APP_DBG("vol set disable\n");
	}
}

void SystemVolSet(void)
{
	uint32_t i;

	for (i = 0; i < AUDIO_CORE_SOURCE_MAX_NUM; i++)
	{
#if defined CFG_FUNC_LINE_MIX_MODE && defined CFG_FUNC_SPDIF_MIX_MODE
		if (i == LINE_SOURCE_NUM || i == SPDIF_MIX_SOURCE_NUM)
		{
			AudioCoreSourceVolSet(i, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]]);
		}
		else
		{
			AudioCoreSourceVolSet(i, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[i]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[i]]);
		}
#elif defined CFG_FUNC_LINE_MIX_MODE
		if (i == LINE_SOURCE_NUM)
		{
			AudioCoreSourceVolSet(i, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]]);
		}
		else
		{
			AudioCoreSourceVolSet(i, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[i]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[i]]);
		}
#elif defined CFG_FUNC_SPDIF_MIX_MODE
		if (i == SPDIF_MIX_SOURCE_NUM)
		{
			AudioCoreSourceVolSet(i, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]]);
		}
		else
		{
			AudioCoreSourceVolSet(i, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[i]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[i]]);
		}
#else
		if(sys_parameter.Vr_en)
		{
			AudioCoreSourceVolSet(i, gSysVolArr32[mainAppCt.gSysVol.AudioSourceVol32[i]], gSysVolArr32[mainAppCt.gSysVol.AudioSourceVol32[i]]);
		}else
		{
			AudioCoreSourceVolSet(i, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[i]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[i]]);
		}

#endif
	}
	for (i = 0; i < AUDIO_CORE_SINK_MAX_NUM; i++)
	{
#ifdef CFG_FUNC_RECORDER_EN
		if (i == AUDIO_RECORDER_SINK_NUM)
		{
			AudioCoreSinkVolSet(AUDIO_RECORDER_SINK_NUM, CFG_PARA_REC_GAIN, CFG_PARA_REC_GAIN);
		}
		else
#endif
		{
			AudioCoreSinkVolSet(i, gSysVolArr[mainAppCt.gSysVol.AudioSinkVol[i]], gSysVolArr[mainAppCt.gSysVol.AudioSinkVol[i]]);
		}
	}
}

void SystemVolSetChannel(int8_t SetChannel, uint8_t volume)
{
	switch (SetChannel)
	{

	case AUDIO_DAC0_SINK_NUM + AUDIO_CORE_SOURCE_MAX_NUM:
		// if(mainAppCt.gSysVol.MuteFlag)
		if (IsAudioPlayerMute() == TRUE)
		{
			HardWareMuteOrUnMute();
		}
		mainAppCt.gSysVol.AudioSinkVol[AUDIO_DAC0_SINK_NUM] = volume;
		APP_DBG("dac0 sink0 vol = %d\n", mainAppCt.gSysVol.AudioSinkVol[AUDIO_DAC0_SINK_NUM]);
		AudioCoreSinkVolSet(AUDIO_DAC0_SINK_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSinkVol[AUDIO_DAC0_SINK_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSinkVol[AUDIO_DAC0_SINK_NUM]]);
		break;
#ifdef CFG_FUNC_RECORDER_EN
	case AUDIO_RECORDER_SINK_NUM + AUDIO_CORE_SOURCE_MAX_NUM:
		// if(mainAppCt.gSysVol.MuteFlag)
		if (IsAudioPlayerMute() == TRUE)
		{
			HardWareMuteOrUnMute();
		}
		mainAppCt.gSysVol.AudioSinkVol[AUDIO_RECORDER_SINK_NUM] = volume;
		APP_DBG("rec sink1 vol = %d\n", mainAppCt.gSysVol.AudioSinkVol[AUDIO_RECORDER_SINK_NUM]);
		AudioCoreSinkVolSet(AUDIO_RECORDER_SINK_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSinkVol[AUDIO_RECORDER_SINK_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSinkVol[AUDIO_RECORDER_SINK_NUM]]);
		break;
#endif
#if defined(CFG_RES_AUDIO_DACX_EN)
	case AUDIO_DACX_SINK_NUM + AUDIO_CORE_SOURCE_MAX_NUM:
		// if(mainAppCt.gSysVol.MuteFlag)
		if (IsAudioPlayerMute() == TRUE)
		{
			HardWareMuteOrUnMute();
		}
		mainAppCt.gSysVol.AudioSinkVol[AUDIO_DACX_SINK_NUM] = volume;
		APP_DBG("dacx sink2 vol = %d\n", mainAppCt.gSysVol.AudioSinkVol[AUDIO_DACX_SINK_NUM]);
		AudioCoreSinkVolSet(AUDIO_DACX_SINK_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSinkVol[AUDIO_DACX_SINK_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSinkVol[AUDIO_DACX_SINK_NUM]]);
		break;
#endif

	case MIC_SOURCE_NUM:
		// if(mainAppCt.gSysVol.MuteFlag)
		if (IsAudioPlayerMute() == TRUE)
		{
			HardWareMuteOrUnMute();
		}
		mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM] = volume;
		APP_DBG("MIC_SOURCE_NUM vol = %d\n", mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM]);
		AudioCoreSourceVolSet(MIC_SOURCE_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM]]);
		break;

	case APP_SOURCE_NUM:
		// if(mainAppCt.gSysVol.MuteFlag)
		if (IsAudioPlayerMute() == TRUE)
		{
			HardWareMuteOrUnMute();
		}
		mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM] = volume;
		APP_DBG("APP_SOURCE_NUM vol = %d\n", mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]);
		AudioCoreSourceVolSet(APP_SOURCE_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]]);
		// #ifdef BT_TWS_SUPPORT
		//			AudioCoreSourceVolSet(TWS_SOURCE_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]]);
		// #endif
		break;
#ifdef CFG_FUNC_REMIND_SOUND_EN
	case REMIND_SOURCE_NUM:
		// if(mainAppCt.gSysVol.MuteFlag)
		if (IsAudioPlayerMute() == TRUE)
		{
			HardWareMuteOrUnMute();
		}
		mainAppCt.gSysVol.AudioSourceVol[REMIND_SOURCE_NUM] = volume;
		APP_DBG("REMIND_SOURCE_NUM vol = %d\n", mainAppCt.gSysVol.AudioSourceVol[REMIND_SOURCE_NUM]);
		AudioCoreSourceVolSet(REMIND_SOURCE_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[REMIND_SOURCE_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[REMIND_SOURCE_NUM]]);
		break;
#endif
#ifdef CFG_FUNC_RECORDER_EN
	case PLAYBACK_SOURCE_NUM:
		// if(mainAppCt.gSysVol.MuteFlag)
		if (IsAudioPlayerMute() == TRUE)
		{
			HardWareMuteOrUnMute();
		}
		mainAppCt.gSysVol.AudioSourceVol[PLAYBACK_SOURCE_NUM] = volume;
		APP_DBG("PLAYBACK_SOURCE_NUM vol = %d\n", mainAppCt.gSysVol.AudioSourceVol[PLAYBACK_SOURCE_NUM]);
		AudioCoreSourceVolSet(PLAYBACK_SOURCE_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[PLAYBACK_SOURCE_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[PLAYBACK_SOURCE_NUM]]);
		break;
#endif

	default:
		break;
	}
}

#ifdef CFG_ADC_LEVEL_KEY_EN
void AdcLevelMsgProcess(uint16_t Msg) // Sliding resistance
{
	uint16_t AdcLevelCh, AdcValue;

	if ((Msg > MSG_ADC_LEVEL_MSG_START) && (Msg < MSG_ADC_LEVEL_MSG_END))
	{
		AdcLevelCh = Msg & 0xff00;
		AdcLevelCh -= MSG_ADC_LEVEL_MSG_START;
		AdcLevelCh >>= 8;

		AdcValue = Msg & 0x00ff; 						//(0~~16) = (MSG_ADC_LEVEL_MSG_0~~MSG_ADC_LEVEL_MSG_16)

		//APP_DBG("AdcLevelCh = %d  AdcValue = %d \n", AdcLevelCh,AdcValue);
		
		switch (AdcLevelCh)
		{

		case 1: // ADC LEVEL Channel 1			Volume

			#ifdef TEST
				AudioMusicVol32(AdcValue);
				//AudioMusicVol32( 32 - AdcValue);
			#else
				#ifdef CFG_RES_AUDIO_I2SOUT_EN
					AudioMusicVol32(AdcValue);
				#else
					AudioMusicVol32( 32 - AdcValue);
				#endif
			#endif

			
			break;

		case 2: // ADC LEVEL Channel 2			MID
	
			#ifdef TEST
				mainAppCt.MusicMidStep = AdcValue;
			#else

				#ifdef CFG_RES_AUDIO_I2SOUT_EN
					mainAppCt.MusicMidStep = AdcValue;
				#else
					mainAppCt.MusicMidStep = 32 - AdcValue;
				#endif
				
			#endif
			
			MusicBassTrebAjust(mainAppCt.MusicBassStep, mainAppCt.MusicMidStep, mainAppCt.MusicTrebStep);
			break;

		case 3: // ADC LEVEL Channel 3			Bass


			#ifdef TEST
				mainAppCt.MusicBassStep = AdcValue;
			#else
				#ifdef CFG_RES_AUDIO_I2SOUT_EN
					mainAppCt.MusicBassStep = AdcValue;
				#else
					mainAppCt.MusicBassStep = 32 - AdcValue;
				#endif			
			#endif
		
			MusicBassTrebAjust(mainAppCt.MusicBassStep, mainAppCt.MusicMidStep, mainAppCt.MusicTrebStep);
			break;

		case 4: // ADC LEVEL Channel 4			Treb

			#ifdef TEST
				mainAppCt.MusicTrebStep = AdcValue;
			#else
				#ifdef CFG_RES_AUDIO_I2SOUT_EN
					mainAppCt.MusicTrebStep = AdcValue;
				#else
					mainAppCt.MusicTrebStep = 32 - AdcValue;
				#endif
			#endif

			MusicBassTrebAjust(mainAppCt.MusicBassStep, mainAppCt.MusicMidStep, mainAppCt.MusicTrebStep);
			break;
		case 5: // ADC LEVEL Channel 5

			break;
		case 6: // ADC LEVEL Channel 6

			break;
		case 7: // ADC LEVEL Channel 7

			break;
		case 8: // ADC LEVEL Channel 8

			break;
		case 9: // ADC LEVEL Channel 9

			break;
		case 10: // ADC LEVEL Channel 10

			break;
		case 11: // ADC LEVEL Channel 11

			break;
		}
	}
}
#endif

// WhetherRecMusic: TRUE:rec music 	FALSE:not rec music
// if_para_use:0:not use para 1:not rec music 2:rec music
void SetRecMusic(uint8_t if_para_use)
{
	if (if_para_use)
	{
		WhetherRecMusic = if_para_use - 1;
	}
	else
	{
		if (WhetherRecMusic)
		{
			APP_DBG("not rec music\n");
			WhetherRecMusic = FALSE;
		}
		else
		{
			APP_DBG("rec music\n");
			WhetherRecMusic = TRUE;
		}
	}
}

#ifdef CFG_APP_HDMIIN_MODE_EN
void HDMISourceMute(void)
{
	mainAppCt.hdmiSourceMuteFlg = 1;
	AudioCoreSourceMute(APP_SOURCE_NUM, TRUE, TRUE);
	// APP_DBG("hdmi mute\n");
}

void HDMISourceUnmute(void)
{
	mainAppCt.hdmiSourceMuteFlg = 0;
	AudioCoreSourceUnmute(APP_SOURCE_NUM, TRUE, TRUE);
	// APP_DBG("hdmi unmute\n");
}

bool IsHDMISourceMute(void)
{
	return mainAppCt.hdmiSourceMuteFlg;
}
#endif
