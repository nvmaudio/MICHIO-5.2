/**
 **************************************************************************************
 * @file    bluetooth_a2dp_deal.c
 * @brief   
 *
 * @author  KK
 * @version V1.0.0
 *
 * $Created: 2021-4-18 18:00:00$
 *
 * @Copyright (C) Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#include "debug.h"
#include "app_config.h"
#include "bt_config.h"
#include "bt_interface.h"
#include "bt_app_a2dp_deal.h"
#include "bt_manager.h"
#include "main_task.h"
#include "bt_app_common.h"
#include "bt_app_connect.h"
#include "bt_app_tws.h"
#include "bt_play_api.h"
#include "bt_stack_service.h"
#include "remind_sound.h"
#include "audio_vol.h"
#include "mode_task.h"
#include "bt_play_mode.h"
#include "bt_app_ddb_info.h"

#include "michio.h"


#ifndef BT_TWS_SUPPORT
extern uint32_t gSpecificDevice;
#endif

#ifdef BT_HFP_MODE_DISABLE
extern uint32_t gHfpCallNoneWaiting;
#endif

#if (BT_LINK_DEV_NUM == 2)
uint8_t static sPlayingIndex = 0xff;
#endif

/*****************************************************************************************
* A2DP���ӳɹ�
****************************************************************************************/
void BtA2dpConnectedDev(BT_A2DP_CALLBACK_PARAMS * param)
{
	uint8_t ConnectIndex = 0;
#if(BT_LINK_DEV_NUM == 2)
	if((btManager.btLinked_env[0].a2dpState == BT_A2DP_STATE_STREAMING)
		|| (btManager.btLinked_env[1].a2dpState == BT_A2DP_STATE_STREAMING))
	{
		; //���ֻ��ڲ����� ���ܳ�ʼ����������ֹ���ֶ���
	}
	else
#endif
	{
		a2dp_sbc_decoer_init();
	}

#ifdef BT_TWS_SUPPORT
	if((btManager.twsState == BT_TWS_STATE_CONNECTED)&&(btManager.twsRole == BT_TWS_SLAVE))
	{
		BTHciDisconnectCmd(param->params.bd_addr);
		return;
	}
#endif
	APP_DBG("param->index:%d,A2dp Connected : bt address = %02x:%02x:%02x:%02x:%02x:%02x\n",param->index,
			(param->params.bd_addr)[0],
			(param->params.bd_addr)[1],
			(param->params.bd_addr)[2],
			(param->params.bd_addr)[3],
			(param->params.bd_addr)[4],
			(param->params.bd_addr)[5]);

	{
#if (BT_LINK_DEV_NUM == 2)
		for(ConnectIndex = 0; ConnectIndex < BT_LINK_DEV_NUM; ConnectIndex++)
		{
			if(memcmp(param->params.bd_addr,btManager.btLinked_env[ConnectIndex].remoteAddr,BT_ADDR_SIZE) == 0)
			{
				break;
			}
		}
		if(ConnectIndex >= BT_LINK_DEV_NUM)
		{
			for(ConnectIndex = 0; ConnectIndex < BT_LINK_DEV_NUM; ConnectIndex++)
			{
				if(!(btManager.btLinked_env[ConnectIndex].btLinkedProfile & BT_CONNECTED_A2DP_FLAG)
						&& !(btManager.btLinked_env[ConnectIndex].btLinkedProfile & BT_CONNECTED_AVRCP_FLAG)
				#if(BT_HFP_SUPPORT == ENABLE)
						&& !(btManager.btLinked_env[ConnectIndex].btLinkedProfile & BT_CONNECTED_HFP_FLAG)
				#endif
					)
				{
					break;
				}
			}
		}

		if(ConnectIndex >=2)
		{
			APP_DBG("a2dp link full\n");
			BTHciDisconnectCmd(param->params.bd_addr);//���ӳ����쳣ֱ�ӶϿ�����
			if(btManager.btLinked_env[0].btLinkState)
			{
				btManager.btLinked_env[1].avrcpState = BT_AVRCP_STATE_NONE;
				btManager.btLinked_env[1].a2dpState = BT_A2DP_STATE_NONE;
				btManager.btLinked_env[1].hfpState = BT_HFP_STATE_NONE;
				btManager.btLinked_env[1].btLinkState = 0;
				APP_DBG("BtLink[1] DisConnect\n");
			}
			else
			{
				btManager.btLinked_env[0].avrcpState = BT_AVRCP_STATE_NONE;
				btManager.btLinked_env[0].a2dpState = BT_A2DP_STATE_NONE;
				btManager.btLinked_env[0].hfpState = BT_HFP_STATE_NONE;
				btManager.btLinked_env[0].btLinkState = 0;
				APP_DBG("BtLink[0] DisConnect\n");
			}
			return;
		}
#endif

		if((param->params.bd_addr)[0] || (param->params.bd_addr)[1] || (param->params.bd_addr)[2]
		|| (param->params.bd_addr)[3] || (param->params.bd_addr)[4] || (param->params.bd_addr)[5])
		{
			memcpy(GetBtManager()->remoteAddr, param->params.bd_addr, 6);
			memcpy(GetBtManager()->btLinked_env[ConnectIndex].remoteAddr, param->params.bd_addr, 6);
		}
	}
	if(btManager.btLinked_env[ConnectIndex].a2dp_index >= BT_LINK_DEV_NUM)//û�б�ռ��
		btManager.btLinked_env[ConnectIndex].a2dp_index = param->index;
	SetA2dpState(ConnectIndex, BT_A2DP_STATE_CONNECTED);

	SetBtConnectedProfile(ConnectIndex, BT_CONNECTED_A2DP_FLAG);

	if(!btManager.btReconnectTimer.timerFlag)
	{
		//Remote Device��������BP10,A2DP���ӳɹ�,AVRCPδ������,��������1��avrcp����
		if(GetAvrcpState(ConnectIndex) < BT_AVRCP_STATE_CONNECTED)
		{
			//btEventListB2Count = 500;//50;//��ʱ50ms
			//btCheckEventList |= BT_EVENT_AVRCP_CONNECT;
			BtStack_BtAvrcpConRegister(param->index);
		}
	}

	//��δ�����ֻ�,����index
	if(btManager.linkedNumber == 0)
	{
		//btManager.cur_index = param->index;
		BtCurIndex_Set(ConnectIndex);
	}

	BtLinkStateConnect(0, ConnectIndex);
}


/*****************************************************************************************
* A2DP�Ͽ�����
****************************************************************************************/
void BtA2dpDisconnectedDev(BT_A2DP_CALLBACK_PARAMS * param)
{
	APP_DBG("A2dp disconnect,index:%d\n",param->index);
	uint8_t index = GetBtManagerA2dpIndex(param->index);

	if(index < BT_LINK_DEV_NUM)
	{
		SetA2dpState(index, BT_A2DP_STATE_NONE);
		SetBtDisconnectProfile(index,BT_CONNECTED_A2DP_FLAG);
		BtLinkStateDisconnect(index);
	}
	
	//��������,HFP�Ѿ����ӳɹ�,A2DP���ܾ�,�ٷ���һ������,�绹�Ǳ��ܾ�,����Ϊϵͳ���ӳɹ�
	if((BtReconnectDevIsExcute()))//remote terminated disconnect
	{
		uint8_t ConnectIndex = 0;
		for(ConnectIndex = 0; ConnectIndex < BT_LINK_DEV_NUM; ConnectIndex++)
		{
			if(memcmp(btManager.btReconPhoneSt.RemoteDevAddr,btManager.btLinked_env[ConnectIndex].remoteAddr,BT_ADDR_SIZE) == 0)
			{
				break;
			}
		}
		if((GetHfpState(ConnectIndex) >= BT_HFP_STATE_CONNECTED))
		{
			if(btManager.btReconExcuteSt->TryCount)
			{
				//��ʱ1s�ٴγ���;
				btManager.btReconExcuteSt->TryCount = 0;
				BtReconnectDevAgain(1000);//��ʱ1s�ٴη�������
				printf("============ delay 1000ms, reconnect again\n");
			}
			else
			{
				BtReconnectDevStop();
				BtLinkStateConnect(1, param->index);
				printf("=============hfp cannot connect, finished..\n");
			}
		}
	}
	#ifdef RECON_ADD
	btManager.btReconnectunusual = TRUE;
	#endif

	//A2DP�Ͽ��󣬿������AVRCP�Ͽ�����(3S��ʱ)
	/*if(IsAvrcpConnected())
	{
		btEventListB0Count = btEventListCount;
		btEventListB0Count += 5000;//��ʱ5s
		btCheckEventList |= BT_EVENT_AVRCP_DISCONNECT;
	}*/
	BtStack_BtAvrcpDisconRegister(param->index);

	SetA2dpState(param->index,BT_A2DP_STATE_NONE);
}

/*****************************************************************************************
* A2DP���ӳ�ʱ����
****************************************************************************************/
void BtA2dpConnectTimeout(BT_A2DP_CALLBACK_PARAMS * param)
{
	APP_DBG("A2dp connect timeout,index:%d\n",param->index);
	uint8_t index = GetBtManagerA2dpIndex(param->index);

	if(index < BT_LINK_DEV_NUM && GetA2dpState(index) > BT_A2DP_STATE_NONE)
	{
		SetA2dpState(index, BT_A2DP_STATE_NONE);
		SetBtDisconnectProfile(index,BT_CONNECTED_A2DP_FLAG);

		//���¸�������decoder��ز���
		a2dp_sbc_decoer_init();

		BtLinkStateDisconnect(index);

		//A2DP�Ͽ��󣬿������AVRCP�Ͽ�����(5S��ʱ)
		/*if(IsAvrcpConnected())
		{
			btEventListB0Count = btEventListCount;
			btEventListB0Count += 5000;//��ʱ5s
			btCheckEventList |= BT_EVENT_AVRCP_DISCONNECT;
		}*/
	}
	SetA2dpState(param->index,BT_A2DP_STATE_NONE);
}

/*****************************************************************************************
* A2DP Stream ��ʼ����
****************************************************************************************/
extern uint32_t		a2dp_unmute_delay_cnt;
uint32_t a2dp_pause_delay_cnt = 0; //delay pause
void BtA2dpStreamStart(BT_A2DP_CALLBACK_PARAMS * param)
{
	APP_DBG("A2dp streaming %d...\n", param->index);
	APP_MUTE_CMD(FALSE);


	#if (BT_LINK_DEV_NUM == 2)
		uint8_t index = GetBtManagerA2dpIndex(param->index);
		if(GetSystemMode() == ModeBtHfPlay && (index < BT_LINK_DEV_NUM))//ͨ��ģʽ�� �����ֻ����ڲ��� �ҶϺ���ת�����ڲ��ŵ��ֻ�
		{
			if(btManager.cur_index != param->index)
			{
				if(btManager.HfpCurIndex != 0xff)
				{
					APP_DBG("talking now,arvcp pause index %d\n",index);
					//AvrcpCtrlPause(index);
					a2dp_pause_delay_cnt = 500;
				}
			}
			
			//APP_DBG("ModeBtHfPlay param->index[%d]->>>>> BT_A2DP_STATE_STREAMING %d \n",index);
			SetA2dpState(index, BT_A2DP_STATE_STREAMING);
			return ;
		}
	#endif

	#if (BT_HFP_SUPPORT == ENABLE)
		if(GetSystemMode() != ModeBtAudioPlay)
		{
			if(GetSystemMode() == ModeBtHfPlay)
			{
				extern bool GetDelayExitBtHfMode(void);
				APP_DBG("--A2dp streaming unusual set status is %d,%d\n", GetSysModeState(ModeBtHfPlay),GetDelayExitBtHfMode());

				if((GetSysModeState(ModeBtHfPlay) != ModeStateSusend) && !GetDelayExitBtHfMode())
				{
					return; // �˳�BTCALL֮���Ƚ�MODE�����Ǵ�ʱ��û�н���BTAUDIO
				}
			}else{
				return;
			}
		}
	#endif

	if(IsAudioPlayerMute() == FALSE
		#ifdef CFG_FUNC_REMIND_SOUND_EN
		&& (!RemindSoundIsPlay())
		#endif
		)
	{
		#if(BT_LINK_DEV_NUM == 2)
			if((btManager.btLinked_env[0].a2dpState == BT_A2DP_STATE_STREAMING) || (btManager.btLinked_env[1].a2dpState == BT_A2DP_STATE_STREAMING))
			{
				; 
			}
			else
		#endif
		{
			HardWareMuteOrUnMute();
			a2dp_unmute_delay_cnt = 0;
		}
	}

	#if (BT_LINK_DEV_NUM == 2)
		if(index < BT_LINK_DEV_NUM)
		{
			SetA2dpState(index, BT_A2DP_STATE_STREAMING);
		}

		if(btManager.HfpCurIndex != 0xff)
		{
			APP_DBG("talking now,arvcp pause index %d\n",index);
			AvrcpCtrlPause(index);
			return;
		}

		APP_DBG("A2dp streaming...cur %d, index %d, ,param->index %d\n",btManager.cur_index,index,param->index);

		if(btManager.cur_index == index)
		{
			extern uint32_t AvrcpStateSuspendCount;
			AvrcpStateSuspendCount = 0;
		}
		else
		{
			#ifdef LAST_PLAY_PRIORITY
			extern uint32_t AvrcpStateSuspendCount;
			AvrcpStateSuspendCount++;
			#endif

			if(GetA2dpState(btManager.cur_index) != BT_A2DP_STATE_STREAMING)
			{
				BtCurIndex_Set(index);
				BT_DBG("channel[%d]:start playing\n", btManager.cur_index);
			}
			else
			{
				return;
			}
		}
	#endif

	btManager.aacFrameNumber = 0;//AAC
	BtMidMessageSend(MSG_BT_MID_PLAY_STATE_CHANGE, 1);
	
	#ifndef BT_TWS_SUPPORT
		#if (BT_HFP_SUPPORT == ENABLE)
			extern uint32_t gSpecificDevice;
			if(gSpecificDevice)
			{
				extern void SpecialDeviceFunc(uint8_t index);
				SpecialDeviceFunc(param->index);
			}
		#endif
	#endif

	#if(BT_LINK_DEV_NUM == 2)
		if((btManager.btLinked_env[0].a2dpState == BT_A2DP_STATE_STREAMING)
			|| (btManager.btLinked_env[1].a2dpState == BT_A2DP_STATE_STREAMING)
		)
		{
			; //���ֻ��ڲ����� ���ܳ�ʼ����������ֹ���ֶ���
		}
		else
	#endif
	{
		a2dp_sbc_decoer_init();
	}

	SetA2dpState(param->index,BT_A2DP_STATE_STREAMING);
}

/*****************************************************************************************
* A2DP Stream ������ͣ
****************************************************************************************/
void BtA2dpStreamSuspend(BT_A2DP_CALLBACK_PARAMS * param)
{

	APP_MUTE_CMD(TRUE);

	uint8_t index = GetBtManagerA2dpIndex(param->index);
	APP_DBG("A2dp suspend\n");
	if(GetSystemMode() == ModeBtHfPlay && index < BT_LINK_DEV_NUM)//ͨ��ʱ �����ֻ�����ͣ״̬����
	{
		APP_DBG("ModeBtHfPlay param->index[%d]->>>>> BT_A2DP_STATE_CONNECTED \n",index);
		SetA2dpState(index, BT_A2DP_STATE_CONNECTED);
		return ;
	}

	if(GetSystemMode() != ModeBtAudioPlay)
		return;

#if (BT_LINK_DEV_NUM == 2)
	if( (btManager.btLinked_env[0].btLinkState == 1 && btManager.btLinked_env[1].btLinkState == 0)
	   || (btManager.btLinked_env[0].btLinkState == 0 && btManager.btLinked_env[1].btLinkState == 1) )//��ֹ�������ҵ�����
	{
		sPlayingIndex = 0xff;
	}
#endif

	if(index < BT_LINK_DEV_NUM)
		SetA2dpState(index, BT_A2DP_STATE_CONNECTED);

	if(IsAudioPlayerMute() == FALSE
#ifdef CFG_FUNC_REMIND_SOUND_EN
		&& (!RemindSoundIsPlay())
#endif
	)
	{
		#if (BT_LINK_DEV_NUM == 2)
		if( (GetA2dpState(0) != BT_A2DP_STATE_STREAMING) && (GetA2dpState(1) != BT_A2DP_STATE_STREAMING) )//��ֹ�������ܻ���ֵĶ���
		#endif
		{
			#if defined CFG_FUNC_I2S_MIX_MODE || defined CFG_FUNC_LINE_MIX_MODE || defined CFG_FUNC_SPDIF_MIX_MODE
			#else
			HardWareMuteOrUnMute();
			a2dp_unmute_delay_cnt = 0;
			#endif
		}
	}

#ifdef BT_TWS_SUPPORT
	if(IsBtAudioMode())
	{
#if (BT_LINK_DEV_NUM == 2)
		if( GetA2dpState(0) != BT_A2DP_STATE_STREAMING && GetA2dpState(1) != BT_A2DP_STATE_STREAMING )//��ֹ�������ܻ���ֵĶ���
#endif
			AudioCoreSourceDisable(1);
	}
#endif

	if(btManager.cur_index != index)
		return;
	
	//a2dp pause�� 500ms�����л�
	extern uint32_t AvrcpStateSuspendCount;
	if(AvrcpStateSuspendCount<22)
		AvrcpStateSuspendCount = 22;

	BtMidMessageSend(MSG_BT_MID_PLAY_STATE_CHANGE, 2);
	SetA2dpState(param->index,BT_A2DP_STATE_CONNECTED);
}

/*****************************************************************************************
* A2DP Stream ���յ�ý������
****************************************************************************************/
void BtA2dpStreamDataRecevied(BT_A2DP_CALLBACK_PARAMS * param)
{
#if (BT_LINK_DEV_NUM == 2)
	uint8_t index = GetBtManagerA2dpIndex(param->index);
#endif
#ifdef BT_HFP_MODE_DISABLE
	if(gHfpCallNoneWaiting)
		return;// bkd change
#endif

#if (BT_LINK_DEV_NUM == 2)
	if(btManager.cur_index != index)
		return;

	if(GetSystemMode() == ModeBtHfPlay && index < BT_LINK_DEV_NUM)//ͨ�����ܻ������ֲ��� ��Ҫ�Ӵ�����
	{
		AvrcpCtrlPause(index);
	}
	//��������Դ�и���
	if((btManager.btLinked_env[btManager.cur_index].btLinkState == 1)&&(sPlayingIndex != btManager.cur_index)&&(btManager.btDisConnectingFlag == 0)/*&&(GetSystemMode() == AppModeBtAudioPlay)???*/)
	{
		sPlayingIndex = btManager.cur_index;
		APP_DBG("playing index:%d\n",sPlayingIndex);
		btManager.btDdbLastProfile = btManager.btLinked_env[sPlayingIndex].btLinkedProfile;
		//SpiFlashErase(SECTOR_ERASE, (BTDB_ALIVE_RECORD_ADDR) /4096 , 1);
		BtDdb_UpgradeLastBtAddr(GetBtManager()->btLinked_env[sPlayingIndex].remoteAddr, btManager.btDdbLastProfile);
		//BtDdb_UpgradeLastBtProfile(GetBtManager()->btLinked_env[sPlayingIndex].remoteAddr, btManager.btDdbLastProfile);
		if(GetBtPlayState() != BT_PLAYER_STATE_PLAYING)
		{
			SetBtPlayState(BT_PLAYER_STATE_PLAYING);
		}
		a2dp_sbc_decoer_init();
	}
#endif

	a2dp_sbc_save(param->params.a2dpStreamParams.a2dpStreamData,param->params.a2dpStreamParams.a2dpStreamDataLen);
}

/*****************************************************************************************
* A2DP Stream ���ݸ�ʽ
****************************************************************************************/
void BtA2dpStreamDataType(BT_A2DP_CALLBACK_PARAMS * param)
{
	APP_DBG("A2dp stream %d type:",param->index);
	if(param->index < BT_LINK_DEV_NUM)
	{
		if(param->params.a2dpStreamDataType)
		{
			APP_DBG("AAC\n");
			btManager.a2dpStreamType[param->index] = BT_A2DP_STREAM_TYPE_AAC;
		}
		else
		{
			APP_DBG("SBC\n");
			btManager.a2dpStreamType[param->index] = BT_A2DP_STREAM_TYPE_SBC;
		}
		if(btManager.cur_index == GetBtManagerA2dpIndex(param->index))
			a2dp_sbc_decoer_init();
	}
}


