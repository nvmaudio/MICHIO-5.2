/**
 **************************************************************************************
 * @file    bluetooth_connect.c
 * @brief   bluetooth ����/����  ��غ������ܽӿ�
 *
 * @author  KK
 * @version V1.0.0
 *
 * $Created: 2019-10-28 18:00:00$
 *
 * @Copyright (C) 2016, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#include <string.h>
#include "type.h"
#include "app_config.h"
#include "bt_config.h"
//driver
#include "debug.h"
//middleware
#include "main_task.h"
#include "bt_manager.h"
//application
#include "bt_app_common.h"
#include "bt_app_connect.h"
#include "bt_app_ddb_info.h"
#include "bt_app_tws_connect.h"
#include "bt_engine_utility.h"
#include "bt_stack_service.h"

extern bool IsBtReconnectReady(void);

extern uint8_t BB_link_state_is_used(uint8_t *addr);
extern int32_t lm_env_link_state(uint8_t *addr);


/*****************************************************************************************
 * 
 ****************************************************************************************/
void BtConnectCtrl(void)
{
	APP_DBG("bt connect ctrl\n");
	
	BtDdb_GetLastBtAddr(GetBtManager()->btDdbLastAddr, &GetBtManager()->btDdbLastProfile);

	if(BtAddrIsValid(btManager.btDdbLastAddr))
	{
		APP_DBG("Last Addr is NULL\n");
		return ;
	}
	
	//reconnect remote device
	APP_DBG("Reconnect:");
	{
		uint8_t i;
		for(i=0; i<6; i++)
		{
			APP_DBG("%x ", btManager.btDdbLastAddr[i]);
		}
	}
	APP_DBG("\n");

	//��������2��
	BtReconnectDevCreate(btManager.btDdbLastAddr, 2, 3, 0, btManager.btDdbLastProfile);
}

/*****************************************************************************************
 * 
 ****************************************************************************************/
void BtDisconnectCtrl(void)
{
	uint8_t i;
	APP_DBG("bt disconnect ctrl\n");

	//1.��ֹ������Ϊ
	BtReconnectDevStop();

	//2.�Ͽ��������ϵ�profile
	for(i=0;i<BT_LINK_DEV_NUM;i++)
	{
		if(btManager.btLinked_env[i].btLinkState)
		{
			BTDisconnect(i);
			//BTHciDisconnectCmd(btManager.btLinked_env[i].remoteAddr);
		}
	}
	
//#if (BT_LINK_DEV_NUM == 2) //Ŀǰ����secondindex��δʹ��
//	SecondTalkingPhoneIndexSet(0xff);//�Ͽ����Ӻ����־
//#endif
}

/*****************************************************************************************
 * 
 ****************************************************************************************/
void BtCancelConnect(void)
{
	//�˺����ڲ��ܼӴ�ӡ��Ϣ
	//BtReconnectDevStop();	
	//���ڴ�����,����ֹͣ
	if(btManager.btReconExcuteSt == (&btManager.btReconPhoneSt))
	{
		btManager.btReconExcuteSt = NULL;
	}
	else
	{
		//���������Ƴ�
		btstack_list_remove(&btManager.btReconHandle, &btManager.btReconPhoneSt.item);
	}
	btManager.btReconPhoneSt.excute = NULL;
	btManager.btReconPhoneSt.ConnectionTimer.timerFlag = TIMER_UNUSED;
	btManager.btReconPhoneSt.profile = 0;
	#ifdef BT_USER_VISIBILITY_STATE
		SetBtUserState(BT_USER_STATE_RECON_END);
	#endif
}

/*****************************************************************************************
 * 
 ****************************************************************************************/
//�˺����ײ����
void BtCancelReconnect(void)
{
	//BtReconnectDevStop();
	
	//���ڴ�����,��ʱ�˳�
	if(btManager.btReconExcuteSt == (&btManager.btReconPhoneSt))
	{
	//	btManager.btReconExcuteSt = NULL;
	//	btManager.btReconPhoneSt.excute = NULL;
	//	btManager.btReconPhoneSt.ConnectionTimer.timerFlag = TIMER_UNUSED;
		btManager.btReconPhoneSt.TryCount = 0;
	}
	else
	{
		btManager.btReconPhoneSt.DelayStartTime = 3000;

		//���������Ƴ�
//		btstack_list_remove(&btManager.btReconHandle, &btManager.btReconPhoneSt.item);
//		btManager.btReconPhoneSt.excute = NULL;
//		btManager.btReconPhoneSt.ConnectionTimer.timerFlag = TIMER_UNUSED;
//		btManager.btReconPhoneSt.profile = 0;
	}
}

/*********************************************************************
 * ע�ᡢע�� ����profile���ȼ�˳��
 * default: hfp -> a2dp -> avrcp
 *********************************************************************/
void BtReconProfilePrioRegister(uint32_t profile)
{
	btManager.btReconProfilePriority |= profile;
}

void BtReconProfilePrioDeregister(uint32_t profile)
{
	btManager.btReconProfilePriority &= ~profile;
}

uint32_t BtReconProfilePrioFlagGet(void)
{
	return btManager.btReconProfilePriority;
}

/*****************************************************************************************
 * 
 ****************************************************************************************/
void BtReconnectDevExcute(void)
{
	printf("BtReconnectDevExcute, %d\n", btManager.btReconPhoneSt.TryCount);
	uint8_t i;
	for(i=0;i<BT_LINK_DEV_NUM;i++)
	{
		if(memcmp(btManager.btDdbLastAddr, btManager.btLinked_env[i].remoteAddr, 6)==0)
			break;
	}

	if(i<BT_LINK_DEV_NUM)
	{
			if((GetA2dpState(i) == BT_A2DP_STATE_NONE)&&(GetAvrcpState(i) == BT_AVRCP_STATE_NONE)
#if (BT_HFP_SUPPORT == ENABLE)
			&&(GetHfpState(i) == BT_HFP_STATE_NONE)
#endif
			)
	    {
	        //��ǰ��·����,��Ӧ�ò�ֹͣ�����µ�����,�ȴ���ʱ���ٷ�������
	        if(lm_env_link_state(btManager.btDdbLastAddr))
	        {
		        printf("link exist, delay2s reconnect device...\n");
	            BtReconnectDevStop();
	            BtReconnectDevCreate(btManager.btDdbLastAddr, bt_ReconnectionTryCounts, bt_ReconnectionInternalTime, 2000, btManager.btDdbLastProfile);
	            return;
	        }
	    }
	}
	
	btManager.btReconPhoneSt.profile &= (BT_PROFILE_SUPPORTED_HFP | BT_PROFILE_SUPPORTED_A2DP | BT_PROFILE_SUPPORTED_AVRCP);

	//��ǰ��·�ѱ�ʹ��,���Ҳ��ǵ�ǰ���豸,��ֹͣ��������
	if(BB_link_state_is_used(btManager.btDdbLastAddr))
	{
		btManager.btReconPhoneSt.profile &= ~(BT_PROFILE_SUPPORTED_HFP | BT_PROFILE_SUPPORTED_A2DP | BT_PROFILE_SUPPORTED_AVRCP);
		APP_DBG("!!!  lm link used, stop reconnect...\n");
		BtStackServiceMsgSend(MSG_BTSTACK_RECONNECT_REMOTE_STOP);
		return;
	}

	//�޸�����Э�����˳��
	if(btManager.btReconProfilePriority)
	{
		if((btManager.btReconProfilePriority & BT_PROFILE_SUPPORTED_A2DP)&&(GetA2dpState(0) >= BT_A2DP_STATE_CONNECTED))
			BtReconProfilePrioDeregister(BT_PROFILE_SUPPORTED_A2DP);
		
		if((btManager.btReconProfilePriority & BT_PROFILE_SUPPORTED_AVRCP)&&(GetAvrcpState(0) >= BT_AVRCP_STATE_CONNECTED))
			BtReconProfilePrioDeregister(BT_PROFILE_SUPPORTED_AVRCP);
		
		if((btManager.btReconProfilePriority & BT_PROFILE_SUPPORTED_A2DP))
		{
			APP_DBG("A2DP connect priority...\n");
			BtA2dpConnect(0, btManager.btDdbLastAddr);
			return;
		}
		else if((btManager.btReconProfilePriority & BT_PROFILE_SUPPORTED_AVRCP))
		{
			APP_DBG("AVRCP connect priority...\n");
			BtAvrcpConnect(0, btManager.btDdbLastAddr);
			return;
		}
	}

	//�ڻ����ֻ�ʱ,���ֻ���������;��׿�ֻ����Զ�����;
	//ĳЩ��׿�ֻ������ǳ���,�ᵼ��sdp channel����,�����쳣;
	//�ڷ������ʱ,ȷ��sdp server�ǲ��Ǳ�ʹ��,δʹ����ʼ���л���
#if (BT_HFP_SUPPORT == ENABLE)
	if((btManager.btReconPhoneSt.profile & BT_PROFILE_SUPPORTED_HFP)&&(GetHfpState(BtCurIndex_Get()) < BT_HFP_STATE_CONNECTED))
	{
		signed char status = 0;
		/*if(!IsBtReconnectReady())
		{
			BtReconnectDelay();
			return;
		}*/
		APP_DBG("connect hfp\n");
		status = BtHfpConnect(0,btManager.btDdbLastAddr);
		if(status > 2)
		{
	        printf("link exist, delay2s reconnect device...\n");
            BtReconnectDevStop();
            BtReconnectDevCreate(btManager.btDdbLastAddr, bt_ReconnectionTryCounts, bt_ReconnectionInternalTime, 2000, btManager.btDdbLastProfile);
            return;
		}
	}
	else 
#endif
	if((btManager.btReconPhoneSt.profile & BT_PROFILE_SUPPORTED_A2DP)&&(GetA2dpState(BtCurIndex_Get()) < BT_A2DP_STATE_CONNECTED))
	{
		btManager.btReconPhoneSt.profile &= ~(BT_PROFILE_SUPPORTED_HFP);
		
		/*if(!IsBtReconnectReady())
		{
			BtReconnectDelay();
			return;
		}*/
		APP_DBG("connect a2dp\n");
		BtA2dpConnect(0,btManager.btDdbLastAddr);
	}
	else if((btManager.btReconPhoneSt.profile & BT_PROFILE_SUPPORTED_AVRCP)&&(GetAvrcpState(BtCurIndex_Get()) != BT_AVRCP_STATE_CONNECTED))
	{
		btManager.btReconPhoneSt.profile &= ~(BT_PROFILE_SUPPORTED_HFP | BT_PROFILE_SUPPORTED_A2DP);
		
		/*if(!IsBtReconnectReady())
		{
			BtReconnectDelay();
			return;
		}*/
		APP_DBG("connect avrcp\n");
		BtAvrcpConnect(0,btManager.btDdbLastAddr);
	}
	else
	{
		btManager.btReconPhoneSt.profile &= ~(BT_PROFILE_SUPPORTED_HFP | BT_PROFILE_SUPPORTED_A2DP | BT_PROFILE_SUPPORTED_AVRCP);
		//btManager.btReconPhoneSt.ConnectionTimer.timerFlag = TIMER_UNUSED;
		APP_DBG("connect end\n");
		BtStackServiceMsgSend(MSG_BTSTACK_RECONNECT_REMOTE_STOP);
	}
}

/*****************************************************************************************
 * @brief : bt reconnect device 
 * @params: profile
 * @params: addr
 * @params: try count
 * @params: interval time
 * @params: delay time
 * @note  : 1.�ڿ�����ʱ����Ҫ�������ӹ����豸
 *          2.��BB���Ӷ�ʧ(�豸��Զ��)����Ҫ�����豸
 ****************************************************************************************/
void BtReconnectDevCreate(uint8_t *addr, uint8_t tryCount, uint8_t interval, uint32_t delayMs, uint32_t profile)
{
	if(btManager.btReconExcuteSt == (&btManager.btReconPhoneSt))
	{
		APP_DBG("BtReconnectDev is running..., stop\n");
		BtReconnectDevStop();
		return;
	}

    if(tryCount == 0)
    {
		APP_DBG("BtReconnectDevCreate fail, count = 0\n");
		return;
    }
	
	APP_DBG("BtReconnectDevCreate\n");
#ifdef BT_USER_VISIBILITY_STATE
	SetBtUserState(BT_USER_STATE_RECON_BEGIAN);
#endif
	btManager.btReconPhoneSt.ConnectionTimer.timerFlag = TIMER_UNUSED;
	memcpy(btManager.btReconPhoneSt.RemoteDevAddr, addr, BT_ADDR_SIZE);
	btManager.btReconPhoneSt.TryCount = tryCount;
	btManager.btReconPhoneSt.IntervalTime = interval;
	btManager.btReconPhoneSt.DelayStartTime = delayMs;
	btManager.btReconPhoneSt.profile = profile;
	btManager.btReconPhoneSt.excute = (FUNC_RECONNECTION_EXCUTE)BtReconnectDevExcute;

	btstack_list_add_tail(&btManager.btReconHandle, &btManager.btReconPhoneSt.item);

	btManager.btReconStopDelay = 5;
}

/*****************************************************************************************
 * 
 ****************************************************************************************/
//1.�������豸�ɹ�����Ҫֹͣ����
//2.�����Ӵ�����ʱ����Ҫֹͣ����
//3.���������豸���ӳɹ���ֹͣ����
void BtReconnectDevStop(void)
{
	if(btManager.btReconStopDelay > 0)
	{
		return;
	}
	APP_DBG("BtReconnectDevStop\n");
	BtReconProfilePrioDeregister(BT_PROFILE_SUPPORT_GENERAL);
	
	//���ڴ�����,����ֹͣ
	if(btManager.btReconExcuteSt == (&btManager.btReconPhoneSt))
	{
		btManager.btReconExcuteSt = NULL;
	}
	else
	{
		//���������Ƴ�
		btstack_list_remove(&btManager.btReconHandle, &btManager.btReconPhoneSt.item);
	}
	btManager.btReconPhoneSt.excute = NULL;
	btManager.btReconPhoneSt.ConnectionTimer.timerFlag = TIMER_UNUSED;
	btManager.btReconPhoneSt.profile = 0;
}

/*****************************************************************************************
 * 
 ****************************************************************************************/
void BtReconnectDevice(void)
{
	btManager.BtPowerOnFlag = 1;

	//reconnect last bluetooth device
	BtDdb_GetLastBtAddr(GetBtManager()->btDdbLastAddr, &GetBtManager()->btDdbLastProfile);
	if(BtAddrIsValid(btManager.btDdbLastAddr))
	{
		APP_DBG("Last Addr is NULL\n");
		return ;
	}
	
	//reconnect remote device
	APP_DBG("Reconnect:");
	{
		uint8_t i;
		for(i=0; i<6; i++)
		{
			APP_DBG("%x ", btManager.btDdbLastAddr[i]);
		}
	}
	APP_DBG("\n");

    if(GetBtCurConnectFlag())
    {
        BtReconnectDevCreate(btManager.btDdbLastAddr, bt_ReconnectionTryCounts, bt_ReconnectionInternalTime, 2000, btManager.btDdbLastProfile);
    }
    else
    {
        BtReconnectDevCreate(btManager.btDdbLastAddr, bt_ReconnectionTryCounts, bt_ReconnectionInternalTime, 1000, btManager.btDdbLastProfile);
    }
}

uint8_t BtReconnectDevIsUsed(void)
{
	if(btManager.btReconPhoneSt.excute)
		return 1;
	else
		return 0;
}

uint8_t BtReconnectDevIsExcute(void)
{
	if(btManager.btReconExcuteSt == (&btManager.btReconPhoneSt))
		return 1;
	else
		return 0;
}

//��ʱ���ٴη���Э������
void BtReconnectDevAgain(uint32_t delay)
{
	if(btManager.btReconExcuteSt == (&btManager.btReconPhoneSt))
	{
		TimeOutSet(&btManager.btReconExcuteSt->ConnectionTimer.timerHandle, delay);
		btManager.btReconExcuteSt->ConnectionTimer.timerFlag = (TIMER_USED | TIMER_WAITING);
	}
}
/*****************************************************************************************
 * 
 ****************************************************************************************/
#ifdef BT_TWS_SUPPORT
/*****************************************************************************************
 * 
 ****************************************************************************************/
void BtReconnectTwsExcute(void)
{
	APP_DBG("BtReconnectTwsExcute\n");

	if(btManager.twsState != BT_TWS_STATE_CONNECTED)
	{
		APP_DBG("tws connect...\n");
		tws_connect(btManager.btReconTwsSt.RemoteDevAddr);
	}
	else
	{
		//btManager.btReconTwsSt.ConnectionTimer.timerFlag = TIMER_UNUSED;
		BtReconnectTwsStop();
	}
}

/*****************************************************************************************
 * 
 ****************************************************************************************/
void BtReconnectTwsCreate(uint8_t *addr, uint8_t tryCount, uint8_t interval, uint32_t delayMs, uint32_t profile)
{
	if(btManager.twsState == BT_TWS_STATE_CONNECTED)
		return;
	
	if(btManager.btReconExcuteSt == (&btManager.btReconTwsSt))
	{
		APP_DBG("BtReconnectTws is running...\n");
		return;
	}

    if(tryCount == 0)
    {
		APP_DBG("BtReconnectTwsCreate fail, count = 0\n");
		return;
    }
	
	if(((addr[0]==0x00)&&(addr[1]==0x00)&&(addr[2]==0x00)&&(addr[3]==0x00)&&(addr[4]==0x00)&&(addr[5]==0x00))
			|| ((addr[0]==0xff)&&(addr[1]==0xff)&&(addr[2]==0xff)&&(addr[3]==0xff)&&(addr[4]==0xff)&&(addr[5]==0xff)))
	{
		printf("tws addr is null, return\n");
		return;
	}
			
	APP_DBG("BtReconnectTwsCreate\n");
	btManager.btReconTwsSt.ConnectionTimer.timerFlag = TIMER_UNUSED;
	memcpy(btManager.btReconTwsSt.RemoteDevAddr, addr, BT_ADDR_SIZE);
	btManager.btReconTwsSt.TryCount = tryCount;
	btManager.btReconTwsSt.IntervalTime = interval;
	btManager.btReconTwsSt.DelayStartTime = delayMs;
	btManager.btReconTwsSt.profile = profile;
	btManager.btReconTwsSt.excute = (FUNC_RECONNECTION_EXCUTE)BtReconnectTwsExcute;

	btstack_list_add_tail(&btManager.btReconHandle, &btManager.btReconTwsSt.item);
}

/*****************************************************************************************
 * 
 ****************************************************************************************/
void BtReconnectTws_Slave(void)
{
	if(btManager.twsState == BT_TWS_STATE_CONNECTED)
		return;
	
	if(btManager.TwsPowerOnFlag)
		return;
	btManager.TwsPowerOnFlag = 1;

	//reconnect remote device
	APP_DBG("tws slave Reconnect:");
	{
		uint8_t i;
		for(i=0; i<6; i++)
		{
			APP_DBG("%x ", btManager.btTwsDeviceAddr[i]);
		}
	}
	APP_DBG("\n");

	BtReconnectTwsCreate(btManager.btTwsDeviceAddr, bt_TwsReconnectionTryCounts, bt_TwsReconnectionInternalTime, 5000, BT_PROFILE_SUPPORTED_TWS);
}

/*****************************************************************************************
 * 
 ****************************************************************************************/
void BtReconnectTws(void)
{
	if(btManager.twsState == BT_TWS_STATE_CONNECTED)
		return;
	
	if(btManager.TwsPowerOnFlag)
		return;
	btManager.TwsPowerOnFlag = 1;

	//reconnect remote device
	APP_DBG("tws Reconnect:");
	{
		uint8_t i;
		for(i=0; i<6; i++)
		{
			APP_DBG("%x ", btManager.btTwsDeviceAddr[i]);
		}
	}
	APP_DBG("\n");

	BtStartReconnectTws(bt_TwsReconnectionTryCounts, bt_TwsReconnectionInternalTime);
}

/*****************************************************************************************
 * 
 ****************************************************************************************/
//1.�ڿ�����ʱ����Ҫ�������ӹ����豸
//2.��BB���Ӷ�ʧ(�豸��Զ��)����Ҫ�����豸
void BtStartReconnectTws(uint8_t tryCount, uint8_t interval)
{
	BtReconnectTwsCreate(btManager.btTwsDeviceAddr, tryCount, interval, 900, BT_PROFILE_SUPPORTED_TWS);
}
#endif

/*****************************************************************************************
 * 
 ****************************************************************************************/
//1.�������豸�ɹ�����Ҫֹͣ����
//2.�����Ӵ�����ʱ����Ҫֹͣ����
//3.���������豸���ӳɹ���ֹͣ����
void BtReconnectTwsStop(void)
{
	APP_DBG("BtReconnectTwsStop\n");
	
	//���ڴ�����,����ֹͣ
	if(btManager.btReconExcuteSt == (&btManager.btReconTwsSt))
	{
		btManager.btReconExcuteSt = NULL;
	}
	else
	{
		//���������Ƴ�
		btstack_list_remove(&btManager.btReconHandle, &btManager.btReconTwsSt.item);
	}
	btManager.btReconTwsSt.excute = NULL;
	btManager.btReconTwsSt.ConnectionTimer.timerFlag = TIMER_UNUSED;
	btManager.btReconTwsSt.profile = 0;
}

/*****************************************************************************************
 * 
 ****************************************************************************************/
#ifdef BT_TWS_SUPPORT
void BtTwsCancelReconnect(void)
{
	//�˺����ڲ��ܼӴ�ӡ��Ϣ
	BtReconnectTwsStop();

	BtDdb_ClearTwsDeviceRecord();
}
#endif //#ifdef BT_TWS_SUPPORT

uint32_t delay_time = 0;
/***********************************************************************************
 * @breif  : bt reconnect process
 * @note   : �����������̴�������
 **********************************************************************************/
void BtReconnectProcess(void)
{
	if(btManager.btReconStopDelay > 0)
		btManager.btReconStopDelay--;
	//ִ�е�ǰ���������� phone or tws
	if(btManager.btReconExcuteSt)
	{
		if(btManager.btReconExcuteSt->ConnectionTimer.timerFlag == TIMER_UNUSED)
		{
			//��ǰ�Ļ���ִ�н��̽���
			if(!btstack_list_empty(&btManager.btReconHandle))
			{
				//�ȴ�ִ����һ�������Ļ�������,����ǰ�Ļ������̲������
				if(btManager.btReconExcuteSt->TryCount)
				{
					btManager.btReconExcuteSt->TryCount--;
					btstack_list_add_tail(&btManager.btReconHandle, &btManager.btReconExcuteSt->item);
					APP_DBG("reconnect: insert list\n");
				}
				// else
				// {
				// 	APP_DBG("reconnect: end\n");
				// }
				btManager.btReconExcuteSt = NULL;
			}
			else
			{
				//�����������������ȴ��¼�,��ʱ�ȴ���һ�����ӵ���
				if(btManager.btReconExcuteSt->TryCount)
				{
					btManager.btReconExcuteSt->TryCount--;
					TimeOutSet(&btManager.btReconExcuteSt->ConnectionTimer.timerHandle, (btManager.btReconExcuteSt->IntervalTime * 1000));
					btManager.btReconExcuteSt->ConnectionTimer.timerFlag = (TIMER_USED | TIMER_WAITING);
					//APP_DBG("reconnect: end -> waiting\n");
				}
				else
				{
					btManager.btReconExcuteSt = NULL;
					#ifdef BT_USER_VISIBILITY_STATE
					SetBtUserState(BT_USER_STATE_RECON_END);
					#endif
					//APP_DBG("reconnect: end\n");
				}
			}
		}
		else if(btManager.btReconExcuteSt->ConnectionTimer.timerFlag & TIMER_STARTED)
		{
			btManager.btReconExcuteSt->ConnectionTimer.timerFlag &= ~TIMER_STARTED;
			
			if(btManager.btReconExcuteSt->DelayStartTime)
			{
				//��ʱ����
				TimeOutSet(&btManager.btReconExcuteSt->ConnectionTimer.timerHandle, (btManager.btReconExcuteSt->DelayStartTime));
				btManager.btReconExcuteSt->DelayStartTime = 0;
				btManager.btReconExcuteSt->ConnectionTimer.timerFlag |= TIMER_WAITING;
				//APP_DBG("reconnect: start -> waiting\n");
			}
			else if(btManager.btReconExcuteSt->excute)
			{
				//����ִ�к���
				btManager.btReconExcuteSt->excute();
				//APP_DBG("reconnect: excute\n");
			}
			else
			{
				//�쳣�˳�
				btManager.btReconExcuteSt->ConnectionTimer.timerFlag = TIMER_UNUSED;
				//APP_DBG("reconnect: excute is null, start -> unused\n");
			}
		}
		else if(btManager.btReconExcuteSt->ConnectionTimer.timerFlag & TIMER_WAITING)
		{
			//��ʱ�ȴ�����
			if(IsTimeOut(&btManager.btReconExcuteSt->ConnectionTimer.timerHandle))
			{
				btManager.btReconExcuteSt->ConnectionTimer.timerFlag &= ~TIMER_WAITING;
				btManager.btReconExcuteSt->ConnectionTimer.timerFlag |= TIMER_STARTED;
				//APP_DBG("reconnect: waiting -> start\n");
			}
		}
	}
	//�ӻ������������л�ȡ��Ҫִ�е�����
	else if(!btstack_list_empty(&btManager.btReconHandle))
	{
		if (delay_time < 100)
		{
			delay_time++;
		}
		else
		{
			delay_time = 0;
			btManager.btReconExcuteSt = (BT_RECONNECT_ST*)btstack_list_pop(&btManager.btReconHandle);
			//APP_DBG("reconnect: pop list...\n");
		
			if(btManager.btReconExcuteSt)
			{
				btManager.btReconExcuteSt->ConnectionTimer.timerFlag = (TIMER_USED | TIMER_STARTED);
				//APP_DBG("reconnect: start...\n");
			}
		}
	}
}

/***********************************************************************************
 * ���ٿ�������
 * �Ͽ��������ӣ��������벻�ɱ����������ɱ�����״̬
 **********************************************************************************/
void BtScanPageStateSet(BT_SCAN_PAGE_STATE state)
{
	btManager.btScanPageState = state;
}

BT_SCAN_PAGE_STATE BtScanPageStateGet(void)
{
	return btManager.btScanPageState;
}

static uint32_t g_bt_discon_fail_cnt = 0; //��ͳ�ĶϿ���ʽ,����4�ζϿ����ɹ�,��ֱ�ӵ���HCI�Ͽ�
void BtScanPageStateCheck(void)
{
	static uint32_t bt_disconnect_count = 0;
	switch(btManager.btScanPageState)
	{
		case BT_SCAN_PAGE_STATE_CLOSING:
			//APP_DBG("BT_SCAN_PAGE_STATE_CLOSING\n");
			// If there is a reconnectiong process, stop it
			BtReconnectDevStop();

#if (defined(BT_TWS_SUPPORT) && ((CFG_TWS_ONLY_IN_BT_MODE == ENABLE))) 
			BtReconnectTwsStop();
			
			BtTwsDeviceDisconnectExt();
#endif
			if(sys_parameter.Bt_BackgroundType == BT_BACKGROUND_FAST_POWER_ON_OFF)
			{
				// If there is a bt link, disconnect it
				if(GetBtCurConnectFlag())
				{
					BtDisconnectCtrl();
					BtScanPageStateSet(BT_SCAN_PAGE_STATE_DISCONNECTING);
					g_bt_discon_fail_cnt = 0;
					break;
				}
			}

#ifdef BT_TWS_SUPPORT
#if (CFG_TWS_ONLY_IN_BT_MODE == DISABLE)
			if((btManager.twsFlag) && (btManager.twsState == BT_TWS_STATE_NONE))
			{
				if(IsIdleModeReady())
					BtSetAccessMode_NoDisc_NoCon();	
				else
					BtSetAccessMode_NoDisc_Con();
			}
			else
#endif
#endif
			{
				BtSetAccessMode_NoDisc_NoCon();
			}
			BtScanPageStateSet(BT_SCAN_PAGE_STATE_DISABLE);
			break;
			
		case BT_SCAN_PAGE_STATE_DISCONNECTING:
			if(bt_disconnect_count > 500)	// wait about 200ms
			{
				/*if(GetBtDeviceConnState() != BT_DEVICE_CONNECTION_MODE_NONE)
				{
					BtStackServiceMsgSend(MSG_BTSTACK_ACCESS_MODE_SET);
					BtScanPageStateSet(BT_SCAN_PAGE_STATE_DISABLE);
				}*/
				if(GetBtCurConnectFlag() == 0)
                {
					BtStackServiceMsgSend(MSG_BTSTACK_ACCESS_MODE_SET);
					BtScanPageStateSet(BT_SCAN_PAGE_STATE_DISABLE);

					g_bt_discon_fail_cnt = 0;
                }
				else
				{
					if(GetBtCurConnectFlag())
					{
						//ǰ4��,�Ͽ�������Э��Ͽ���ʽ,������,ֱ�ӷ���HCI�Ͽ�CMD
						g_bt_discon_fail_cnt++;
						if(g_bt_discon_fail_cnt >= 5)
							BTHciDisconnectCmd(btManager.btDdbLastAddr);
						else
							BtDisconnectCtrl();
						BtScanPageStateSet(BT_SCAN_PAGE_STATE_DISCONNECTING);
					}
				}
				bt_disconnect_count = 0;
			}
			else
				bt_disconnect_count++;
			break;
			
		case BT_SCAN_PAGE_STATE_DISABLE:
			if(sys_parameter.Bt_BackgroundType == BT_BACKGROUND_FAST_POWER_ON_OFF)
			{
				// double check wether there is a bt link, if any, disconnect again
				if(GetBtCurConnectFlag())
				{
					BtDisconnectCtrl();
					BtScanPageStateSet(BT_SCAN_PAGE_STATE_DISCONNECTING);
				}
			}
			break;

		case BT_SCAN_PAGE_STATE_OPENING:
			APP_DBG("BT_SCAN_PAGE_STATE_OPENING\n");

			BtScanPageStateSet(BT_SCAN_PAGE_STATE_ENABLE);
			btManager.BtPowerOnFlag = 0;
			#ifdef BT_TWS_SUPPORT
			btManager.TwsPowerOnFlag = 0;
			#endif

#if ((CFG_TWS_ONLY_IN_BT_MODE == ENABLE) || defined(TWS_SLAVE_MODE_SWITCH_EN))
		#if ((TWS_PAIRING_MODE == CFG_TWS_PEER_MASTER)||(TWS_PAIRING_MODE == CFG_TWS_ROLE_RANDOM))
			if(bt_TwsReconnectionEnable && btManager.twsFlag)
			{
				if (IsBtAudioMode())
				{
					btManager.btConStateProtectCnt = 1;
					if(btManager.twsRole == BT_TWS_SLAVE)
						BtReconnectTws_Slave();
					else
						BtReconnectTws();
				}
			}
			if(bt_ReconnectionEnable)
			{
				BtReconnectDevice();
			}

			BtSetAccessMode_Disc_Con();
			break;
		#endif
#else
		#if ((TWS_PAIRING_MODE == CFG_TWS_ROLE_MASTER)||(TWS_PAIRING_MODE == CFG_TWS_PEER_SLAVE))
			if(bt_ReconnectionEnable)
			{
				//BtReconnectDevice();
				GetBtManager()->btReconnectDelayCount = 1;
			}
		#elif (TWS_PAIRING_MODE != CFG_TWS_ROLE_SLAVE)
			if(bt_TwsReconnectionEnable && btManager.twsFlag)
			{
				if(btManager.twsRole == BT_TWS_SLAVE)
				{
			#ifndef CFG_AUTO_ENTER_TWS_SLAVE_MODE
					BtReconnectTws_Slave();
					BtSetAccessMode_NoDisc_NoCon();
					break;
			#endif
            
					if(bt_ReconnectionEnable && GetCurTotaBtRecNum())
					{
						BtReconnectDevice();
						break;
					}
				}
			}
			if(bt_ReconnectionEnable)
			{
				GetBtManager()->btReconnectDelayCount = 1;
			}
		#endif
#endif
			#if (defined(BT_TWS_SUPPORT) && (TWS_PAIRING_MODE == CFG_TWS_ROLE_SLAVE))
			if (!btManager.twsSoundbarSlaveTestFlag)
			{
				BtSetAccessMode_NoDisc_Con();
			}
			else
			#endif
			{
				BtSetAccessMode_Disc_Con();
			}
			break;
			
		case BT_SCAN_PAGE_STATE_ENABLE:
			break;

		case BT_SCAN_PAGE_STATE_OPEN_WAITING:
			//sleep���Ѻ���ʱ�����ֻ�
			if(btManager.btExitSniffReconPhone == 0)
			{
				APP_DBG("BT_SCAN_PAGE_STATE_OPEN_WAITING_END\n");
				BtScanPageStateSet(BT_SCAN_PAGE_STATE_OPENING);
			}
			break;

		default:
			break;
	}
}

