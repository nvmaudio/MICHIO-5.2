/**
 **************************************************************************************
 * @file    bluetooth_tws_connect.c
 * @brief   tws ����������غ������ܽӿ�
 *
 * @author  KK
 * @version V1.0.0
 *
 * $Created: 2021-4-18 18:00:00$
 *
 * @Copyright (C) Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#include <string.h>
#include "type.h"
#include "app_config.h"
#include "bt_config.h"
//driver
#include "chip_info.h"
#include "debug.h"
//middleware
#include "main_task.h"
#include "bt_manager.h"
#include "bt_tws_api.h"
//application
#include "bt_app_tws_connect.h"
#include "bt_app_ddb_info.h"
#include "bt_stack_service.h"
#include "bt_app_connect.h"
#include "tws_mode.h"


#ifdef BT_TWS_SUPPORT

BtTwsAppCt *gBtTwsAppCt = NULL;
static uint32_t gBtTwsCount = 0;

extern uint32_t gBtTwsDelayConnectCnt;

extern int8_t ME_CancelInquiry(void);


/***********************************************************************************
 * TWS�ڴ�����
 **********************************************************************************/
void BtTwsAppInit(void)
{
	if(gBtTwsAppCt == NULL)
	{
		gBtTwsAppCt = (BtTwsAppCt *)osPortMalloc(sizeof(BtTwsAppCt));
	}

	if(gBtTwsAppCt)
	{
		memset(gBtTwsAppCt, 0, sizeof(BtTwsAppCt));
	}
}

void BtTwsAppDeinit(void)
{
	if(gBtTwsAppCt)
	{
		osPortFree(gBtTwsAppCt);
	}
	gBtTwsAppCt = NULL;
}
/***********************************************************************************
 * tws ���������ӶϿ���Ϣ
 **********************************************************************************/
void BtTwsConnectApi(void)
{
	MessageContext		msgSend;
	msgSend.msgId		= MSG_BTSTACK_TWS_CONNECT;
	MessageSend(GetBtStackServiceMsgHandle(), &msgSend);
}

void BtTwsDisconnectApi(void)
{
	MessageContext		msgSend;
	msgSend.msgId		= MSG_BTSTACK_TWS_DISCONNECT;
	MessageSend(GetBtStackServiceMsgHandle(), &msgSend);
}

/***********************************************************************************
 * 
 **********************************************************************************/
void BtTwsDeviceReconnect(void)
{
	if(btManager.twsState == BT_TWS_STATE_NONE)
	{
		if(btManager.twsFlag) 
		{
			APP_DBG("tws device connect\n");
			BtTwsConnectApi();
		}
		else
		{
			APP_DBG("no tws paired list\n");
		}
	}
	else
	{
		APP_DBG("tws is connected\n");
	}
}

/**************************************************************************
 *  �Ͽ�TWS���ӵĺ���
 *************************************************************************/
void BtTwsDeviceDisconnectExt(void)
{
	if(btManager.twsState == BT_TWS_STATE_CONNECTED)
	{
		APP_DBG("tws disconnect\n");
		tws_link_disconnect();
	}
}

/**************************************************************************
 *  Ҳ�ǶϿ�TWS���ӵĺ��������������溯����������ǶϿ�����Լ�������ص�TWS��Ϣ�����ᷢ��Ϣ����һ̨����Ҳ���TWS��Ϣ
 *************************************************************************/
void BtTwsDeviceDisconnect(void)
{
	if(btManager.twsState == BT_TWS_STATE_CONNECTED)
	{
		APP_DBG("tws disconnect\n");
		//tws_link_disconnect();
		BtTwsDisconnectApi();
		if(bt_TwsConnectedWhenActiveDisconSupport)
		{
			tws_active_disconnection();
			BtDdb_ClrTwsDevInfor();
		}
	}
}

/**************************************************************************
 *  
 *************************************************************************/
void BtTwsLinkLoss(void)
{
#if (TWS_PAIRING_MODE == CFG_TWS_ROLE_SLAVE)
	if(!btManager.twsSbSlaveDisable)
		tws_slave_simple_pairing_ready();
#elif ((TWS_PAIRING_MODE == CFG_TWS_PEER_MASTER)||(TWS_PAIRING_MODE == CFG_TWS_PEER_SLAVE)||(TWS_PAIRING_MODE == CFG_TWS_ROLE_RANDOM))
	{
		//delay reconnect master device
		gBtTwsAppCt->btTwsEvent |= BT_TWS_EVENT_LINKLOSS;
		gBtTwsAppCt->btTwsLinkLossTimeoutCnt = 25;
	}
#endif
}

/**************************************************************************
 *  
 *************************************************************************/
void CheckBtTwsPairing(void)
{
	if((gBtTwsAppCt->btTwsPairingTimeroutCnt)&&(gBtTwsAppCt->btTwsPairingStart))
	{
		APP_DBG("again...\n");
		gBtTwsAppCt->btTwsEvent |= BT_TWS_EVENT_PAIRING;
		
#if (TWS_PAIRING_MODE == CFG_TWS_ROLE_RANDOM)
		BtSetAccessMode_Disc_Con();
#endif
	}
	else
	{
#if (TWS_PAIRING_MODE == CFG_TWS_ROLE_RANDOM)
		BtTwsExitPairingMode();
#endif
	}
}

/**************************************************************************
 *  
 *************************************************************************/
bool BtTwsPairingState(void)
{
	if(gBtTwsAppCt)
		return gBtTwsAppCt->btTwsPairingStart;
	else
		return 0;
}

/**************************************************************************
 *  
 *************************************************************************/
void BtTwsEnterPairingMode(void)
{
	//1.�˳�����״̬(TWS�������ֻ�����)
	BtReconnectDevStop();
	BtReconnectTwsStop();
	
	//2.�Ͽ���ǰ�����ӵ��豸
	if(btManager.twsState > BT_TWS_STATE_NONE)
	{
		BtTwsDisconnectApi();
	}
#if (TWS_PAIRING_MODE == CFG_TWS_ROLE_RANDOM)
	if(btManager.btLinkState)//�����ֻ���tws����ֻ�㲥
	{
		printf("doublekey pair start!!!!!!!\n");
		extern void tws_start_pairing_doubleKey(void);
		extern uint32_t doubleKeyCnt;
		tws_start_pairing_doubleKey();

		doubleKeyCnt = 30000;//30s ��ʱ
		BTSetAccessMode(BtAccessModeGeneralAccessible);
	}
	else
#endif
	{
		gBtTwsAppCt->btTwsEvent |= BT_TWS_EVENT_PAIRING;
		gBtTwsAppCt->btTwsPairingStart = 1;
		gBtTwsAppCt->btTwsPairingTimeroutCnt = BT_TWS_RECONNECT_TIMEOUT;

		tws_start_pairing_radom();
	}
}

void BtTwsExitPairingMode(void)
{
	tws_stop_pairing_radom();
	if(gBtTwsAppCt->btTwsPairingStart == 0)
		return;
	
	gBtTwsAppCt->btTwsEvent &= ~BT_TWS_EVENT_PAIRING;
	gBtTwsAppCt->btTwsEvent &= ~BT_TWS_EVENT_REPAIR_CLEAR;
	gBtTwsAppCt->btTwsPairingStart = 0;
	gBtTwsAppCt->btTwsPairingTimeroutCnt = 0;
}

/**************************************************************************
 *  
 *************************************************************************/
void BtTwsPeerEnterPairingMode(void)
{
	//BtDdb_ClearTwsDeviceAddrList();
	btManager.twsRole = BT_TWS_UNKNOW;
}

/**************************************************************************
 *  
 *************************************************************************/
void BtTwsRepairDiscon(void)
{
	if(gBtTwsAppCt->btTwsEvent & BT_TWS_EVENT_REPAIR_CLEAR)
		gBtTwsAppCt->btTwsRepairTimerout = 1;
}

/**************************************************************************
 *  peer
 *************************************************************************/
void BtTwsEnterPeerPairingMode(void)
{
	APP_DBG("BtTwsEnterPeerPairingMode\n");
	gBtTwsAppCt->btTwsEvent |= BT_TWS_EVENT_PAIRING;
	gBtTwsAppCt->btTwsPairingStart = 1;
	gBtTwsAppCt->btTwsPairingTimeroutCnt = BT_TWS_RECONNECT_TIMEOUT;

	BtReconnectDevStop();
}

/**************************************************************************
 *  
 *************************************************************************/
void BtTwsExitPeerPairingMode(void)
{
	if(gBtTwsAppCt->btTwsPairingStart == 0)
		return;

	APP_DBG("BtTwsExitPeerPairingMode\n");
	gBtTwsAppCt->btTwsEvent &= ~BT_TWS_EVENT_PAIRING;
	gBtTwsAppCt->btTwsEvent &= ~BT_TWS_EVENT_REPAIR_CLEAR;
	gBtTwsAppCt->btTwsPairingStart = 0;
	gBtTwsAppCt->btTwsPairingTimeroutCnt = 0;

	BTInquiryCancel();
}

/**************************************************************************
 *  soundbar
 *************************************************************************/
void BtTwsEnterSimplePairingMode(void)
{
	APP_DBG("enter tws pairing mode...\n");
	//���TWS������Լ�¼
	//BtDdb_ClearTwsDeviceAddrList();

	//�Ͽ��Ѿ����ӵ�TWS����
	if(btManager.twsState > BT_TWS_STATE_NONE)
	{
		gBtTwsAppCt->btTwsEvent |= BT_TWS_EVENT_PAIRING_READY;
		//tws_link_disconnect();
		BtTwsDisconnectApi();
	}

	gBtTwsAppCt->btTwsEvent |= BT_TWS_EVENT_SIMPLE_PAIRING;
	gBtTwsAppCt->btTwsPairingStart = 1;
	gBtTwsAppCt->btTwsPairingTimeroutCnt = BT_TWS_RECONNECT_TIMEOUT;

	btManager.twsRole = BT_TWS_UNKNOW;
	btManager.twsEnterPairingFlag = 1;
	
#if (TWS_PAIRING_MODE == CFG_TWS_ROLE_SLAVE)
	if(((gBtTwsAppCt->btTwsEvent & BT_TWS_EVENT_PAIRING_READY)==0)&&(!btManager.twsSbSlaveDisable))
		tws_slave_start_simple_pairing();
#elif (TWS_PAIRING_MODE == CFG_TWS_ROLE_MASTER)
	if((gBtTwsAppCt->btTwsEvent & BT_TWS_EVENT_PAIRING_READY)==0)
		ble_advertisement_data_update();
#endif
}

/**************************************************************************
 *  
 *************************************************************************/
void BtTwsExitSimplePairingMode(void)
{
	//ֹͣ�㲥
	gBtTwsAppCt->btTwsEvent &= ~BT_TWS_EVENT_SIMPLE_PAIRING;
	gBtTwsAppCt->btTwsEvent &= ~BT_TWS_EVENT_PAIRING_READY;

	gBtTwsAppCt->btTwsPairingStart = 0;
	gBtTwsAppCt->btTwsPairingTimeroutCnt = 0;
	btManager.twsEnterPairingFlag = 0;
	
#if (TWS_PAIRING_MODE == CFG_TWS_ROLE_SLAVE)
	if((btManager.twsState == BT_TWS_STATE_NONE)&&(btManager.btLinkState == 0))
		tws_slave_stop_simple_pairing();
#elif (TWS_PAIRING_MODE == CFG_TWS_ROLE_MASTER)
	if((btManager.twsState == BT_TWS_STATE_NONE))
		ble_advertisement_data_update();
#endif
}

/**************************************************************************
 *  
 *************************************************************************/
void BtTwsRunLoop(void)
{
	//20ms
	gBtTwsCount++;
	if(gBtTwsCount<20)
		return;

	if(gBtTwsDelayConnectCnt)
	{
		gBtTwsDelayConnectCnt--;
		if(gBtTwsDelayConnectCnt==0)
		{
			if((gBtTwsAppCt->btTwsPairingStart)&&(gBtTwsAppCt->btTwsPairingTimeroutCnt))
			{
				gBtTwsAppCt->btTwsEvent |= BT_TWS_EVENT_PAIRING;
				printf("pairing again...\n");
			}
		}
	}

	gBtTwsCount=0;
	//���������ش���
	if(gBtTwsAppCt->btTwsEvent & BT_TWS_EVENT_PAIRING)
	{
		if(gBtTwsAppCt->btTwsEvent & BT_TWS_EVENT_REPAIR_CLEAR)
		{
			if(btManager.twsState != BT_TWS_STATE_NONE)
			{
				gBtTwsAppCt->btTwsRepairTimerout++;
				if(gBtTwsAppCt->btTwsRepairTimerout >= 20)
				{
					gBtTwsAppCt->btTwsRepairTimerout = 0;
					gBtTwsAppCt->btTwsEvent &= ~BT_TWS_EVENT_REPAIR_CLEAR;
					//tws_link_disconnect();
					BtTwsDisconnectApi();
				}
			}
			else
			{
				gBtTwsAppCt->btTwsRepairTimerout = 0;
				gBtTwsAppCt->btTwsEvent &= ~BT_TWS_EVENT_REPAIR_CLEAR;
			}
		}
		else
		{
			if(btManager.twsState == BT_TWS_STATE_NONE)
			{
			#if (TWS_PAIRING_MODE != CFG_TWS_PEER_SLAVE)
				//BtDdb_ClearTwsDeviceAddrList();
			#endif
				APP_DBG("tws pairing start\n");
				btManager.twsRole = BT_TWS_UNKNOW;
#if (TWS_PAIRING_MODE == CFG_TWS_ROLE_RANDOM)
				tws_start_pairing(TWS_ROLE_RANDOM);
				if(btManager.btLinkState == 1)
				{
					APP_DBG("NoDisc_NoCon\n");
					BtSetAccessMode_NoDisc_NoCon();
				}
				else
				{
					APP_DBG("Disc_Con\n");
					BtSetAccessMode_Disc_Con();
				}
#elif (TWS_PAIRING_MODE == CFG_TWS_PEER_MASTER)
				tws_start_pairing(TWS_ROLE_MASTER);
				if(btManager.btLinkState == 1)
				{
					APP_DBG("NoDisc_NoCon\n");
					BtSetAccessMode_NoDisc_NoCon();
				}
				else
				{
					APP_DBG("Disc_Con\n");
					BtSetAccessMode_Disc_Con();
				}
#elif (TWS_PAIRING_MODE == CFG_TWS_PEER_SLAVE)
				tws_start_pairing(TWS_ROLE_SLAVE);
				BtSetAccessMode_NoDisc_Con();
#endif
				gBtTwsAppCt->btTwsEvent &= ~BT_TWS_EVENT_PAIRING;
			}
		}
	}
	if(gBtTwsAppCt->btTwsPairingTimeroutCnt)
	{
		gBtTwsAppCt->btTwsPairingTimeroutCnt--;

#if (TWS_PAIRING_MODE == CFG_TWS_PEER_MASTER)
		if(gBtTwsAppCt->btTwsPairingTimeroutCnt == 0)
			BtTwsExitPeerPairingMode();
#elif (TWS_PAIRING_MODE == CFG_TWS_ROLE_RANDOM)
		if(gBtTwsAppCt->btTwsPairingTimeroutCnt == 0)
			BtTwsExitPairingMode();
#endif
	}

//only master and slave
	if(gBtTwsAppCt->btTwsEvent & BT_TWS_EVENT_PAIRING_READY)
	{
		if((btManager.twsState == BT_TWS_STATE_NONE)&&(btManager.btLinkState == 0))
		{
			gBtTwsAppCt->btTwsEvent &= ~BT_TWS_EVENT_PAIRING_READY;
	#if (TWS_PAIRING_MODE == CFG_TWS_ROLE_MASTER)
			{
				ble_advertisement_data_update();
			}
	#elif (TWS_PAIRING_MODE == CFG_TWS_ROLE_SLAVE)
			{
				if(!btManager.twsSbSlaveDisable)
					tws_slave_start_simple_pairing();
			}
	#endif
		}
	}
	
	if(gBtTwsAppCt->btTwsEvent & BT_TWS_EVENT_SIMPLE_PAIRING)
	{
		if((gBtTwsAppCt->btTwsPairingTimeroutCnt == 0)||((btManager.twsState == BT_TWS_STATE_CONNECTED)&&((gBtTwsAppCt->btTwsEvent & BT_TWS_EVENT_PAIRING_READY)==0)))
		{
			APP_DBG("auto exit tws simple pairing\n");
			//gBtTwsAppCt->btTwsEvent &= ~BT_TWS_EVENT_SIMPLE_PAIRING;

			BtTwsExitSimplePairingMode();
		}
	}

	if(bt_TwsBBLostReconnectionEnable)
	{
		//linkloss����
		if(gBtTwsAppCt->btTwsLinkLossTimeoutCnt)
			gBtTwsAppCt->btTwsLinkLossTimeoutCnt--;
		if((gBtTwsAppCt->btTwsEvent & BT_TWS_EVENT_LINKLOSS)&&(gBtTwsAppCt->btTwsLinkLossTimeoutCnt == 0))
		{
			gBtTwsAppCt->btTwsEvent &= ~BT_TWS_EVENT_LINKLOSS;
			BtStartReconnectTws(bt_TwsBBLostTryCounts, bt_TwsBBLostInternalTime);
		}
	}

	//slave����������ʱ�����ɱ������ɱ�����״̬
	if(gBtTwsAppCt->btTwsEvent & BT_TWS_EVENT_SLAVE_TIMEOUT)
	{
		//30s timeout
		if(gBtTwsAppCt->btTwsSlaveTimeoutCount)
			gBtTwsAppCt->btTwsSlaveTimeoutCount--;
		
		if(gBtTwsAppCt->btTwsSlaveTimeoutCount == 0)
		{
			gBtTwsAppCt->btTwsEvent &= ~BT_TWS_EVENT_SLAVE_TIMEOUT;
			//gBtTwsAppCt->btTwsSlaveTimeoutCount = 0;

			if((btManager.twsState == BT_TWS_STATE_NONE)&&(btManager.btLinkState == 0))
			{
				#if (defined(BT_TWS_SUPPORT) && (TWS_PAIRING_MODE == CFG_TWS_ROLE_SLAVE))
				if (!btManager.twsSoundbarSlaveTestFlag)
				{
					BtSetAccessMode_NoDisc_Con();
				}
				else
				#endif
				{
					BtSetAccessMode_Disc_Con(); //TWS������ʱ,����ɱ������ɱ�����״̬
				}
			}
		}
	}
}

/**************************************************************************
 *  
 *************************************************************************/
extern uint8_t BB_link_state_get(uint8_t type);
extern uint8_t tws_connect_state_get(void);
bool TwsPeerSlave(void)
{
	if(GetSystemMode() == ModeTwsSlavePlay)
	{					
		if(tws_connect_state_get() == TWS_PAIR_PAIRING)
		{
			return TRUE;
		}
		
		if(gBtTwsAppCt->btTwsPairingStart == 1)
		{
			APP_DBG("CFG_TWS_PEER_SLAVE: Exit pairing loop\n");
			extern unsigned char gLmpCmdStart;
			BtTwsExitPeerPairingMode();
			if(gLmpCmdStart == 0)
			{
				gLmpCmdStart = 1;
			}
			if(tws_connect_state_get() == TWS_PAIR_INQUIRY)
			{
				ME_CancelInquiry();
			}
		}
		TwsSlaveModeExit();
		return TRUE;
	}
	else
	{	
		if(!btManager.btLinkState)
		{
			if(btManager.twsState == BT_TWS_STATE_NONE)
			{
				#if (TWS_PAIRING_MODE == CFG_TWS_ROLE_SLAVE)
				BtSetAccessMode_NoDisc_Con();
				#else
				BtSetAccessMode_Disc_Con();
				#endif

				TwsSlaveModeEnter();
				BtTwsEnterPeerPairingMode();
				return TRUE;
			}
		}
		
		if(btManager.twsState == BT_TWS_STATE_CONNECTED)
		{
			tws_link_disconnect();
		}
	}

	return FALSE;
}

bool TwsRoleSlave(void)
{
#if (TWS_SIMPLE_PAIRING_SUPPORT==ENABLE)
	if(!btManager.twsSbSlaveDisable)
	{
		//1.�Ͽ�TWS
		if(btManager.twsState == BT_TWS_STATE_CONNECTED)
		{
			tws_link_disconnect();
		}
		//2.�ر�BLE SCAN
		tws_slave_stop_simple_pairing();
		tws_slave_simple_pairing_end();
		btManager.twsSbSlaveDisable = 1;
	}
	else
	{
		btManager.twsSbSlaveDisable = 0;
		//1.����BLE SCAN
		tws_slave_start_simple_pairing();
	}
#else
	BtTwsEnterSimplePairingMode();
#endif

	return FALSE;
}

void BtTwsPairingStart(void)
{

	


	BtReconnectTwsStop();
	BtReconnectDevStop();



	#if (TWS_PAIRING_MODE == CFG_TWS_PEER_SLAVE)
		if (TRUE == TwsPeerSlave())
		{
			return;
		}
	#elif (TWS_PAIRING_MODE == CFG_TWS_ROLE_SLAVE)
		TwsRoleSlave();
	#else  

	if(gBtTwsAppCt->btTwsEvent || gBtTwsAppCt->btTwsPairingStart)
		return;
	
	if(BB_link_state_get(1)||btManager.btTwsPairingStartDelayCnt)
	{
		//btManager.btTwsPairingStartDelayCnt = 1;
		return;
	}


	if(GetBtManager()->twsState == BT_TWS_STATE_CONNECTED)
	{
		BtTwsDeviceDisconnect();
		return;
	}

	if(GetSystemMode() != ModeBtAudioPlay)
		return;
	
	if(!bt_TwsPairingWhenPhoneConnectedSupport && btManager.btLinkState)
	{
		return;
	}

/*#if (TWS_PAIRING_MODE == CFG_TWS_PEER_MASTER)
    //����ȡ�����ڷ����������Ϊ��  �û�������ɴ�
	if(gBtTwsAppCt->btTwsPairingStart == 1)
	{
		APP_DBG("CFG_TWS_PEER_MASTER: Exit pairing loop\n");
		GetBtManager()->twsStopConnect = 1;
		extern unsigned char gLmpCmdStart;
		BtTwsExitPeerPairingMode();
		if(gLmpCmdStart == 0)
		{
			gLmpCmdStart = 1;
		}
		else
		{
			ME_CancelInquiry();
		}
		return;
	}
#endif*/


	BtSetAccessMode_Disc_Con();

	#if (TWS_PAIRING_MODE == CFG_TWS_ROLE_RANDOM)
		BtTwsEnterPairingMode();
	#elif (TWS_PAIRING_MODE == CFG_TWS_ROLE_MASTER)
		BtTwsEnterSimplePairingMode();
	#elif (TWS_PAIRING_MODE == CFG_TWS_PEER_MASTER)
		BtTwsEnterPeerPairingMode();
	#endif

	BtDdb_ClearTwsDeviceAddrList();

#endif
}

/**************************************************************************
 *  Note: �˺�������main_task�н��д���;
 *        �ڲ�������ʱ,��Ҫע��,����ֱ�ӵ��ÿ��ƺ���,������������
 *************************************************************************/
void tws_msg_process(uint16_t msg)
{
	switch(msg)
	{	
		case MSG_BT_TWS_MASTER_CONNECTED:
		case MSG_BT_TWS_SLAVE_CONNECTED:
			break;
		case MSG_BT_TWS_PAIRING:
			BtStackServiceMsgSend(MSG_BT_STACK_TWS_PAIRING_START);//bkd change
			break;

		case MSG_BT_TWS_RECONNECT:
			BtTwsDeviceReconnect();
			break;
		
		case MSG_BT_TWS_DISCONNECT:
			BtTwsDeviceDisconnect();
			break;

		case MSG_BT_TWS_CLEAR_LIST:
			BtDdb_ClearTwsDeviceAddrList();
			break;

	#if (TWS_PAIRING_MODE == CFG_TWS_ROLE_SLAVE)
		//Soundbar Slave�������УƵƫ״̬
		//У׼��Ƶƫ��,��Ҫ����ϵͳ
		case MSG_BT_SOUNDBAR_SLAVE_TEST_MODE:
			if(btManager.twsState == BT_TWS_STATE_NONE)
			{
				if(!btManager.twsSoundbarSlaveTestFlag)
				{
					btManager.twsSoundbarSlaveTestFlag = 1;
					APP_DBG("Soundbar Slave Enter Test State\n");
				}
				else
				{
					btManager.twsSoundbarSlaveTestFlag = 0;
					APP_DBG("Soundbar Slave Exit Test State\n");
				}
				BtStackServiceMsgSend(MSG_BTSTACK_ACCESS_MODE_SET);
			}
			break;
	#endif

		case MSG_BT_SNIFF:
	#if(( (TWS_PAIRING_MODE == CFG_TWS_ROLE_MASTER)||(TWS_PAIRING_MODE == CFG_TWS_ROLE_SLAVE) ) && defined(BT_SNIFF_ENABLE))
			if(GetBtManager()->twsState > BT_TWS_STATE_NONE)
			{
				if(tws_audio_state_get() == TWS_DISCONNECT)
					break;
			}
			if(GetBtManager()->twsState == BT_TWS_STATE_NONE)
			{
				sniff_lmpsend_set(0);
				MessageContext		msgSend;
				msgSend.msgId		= MSG_DEEPSLEEP;
				MessageSend(GetMainMessageHandle(), &msgSend);
				break;
			}
				
			if(sniff_lmpsend_get() == 0)
			{
				sniff_lmpsend_set(1);
				if(GetBtManager()->twsState > BT_TWS_STATE_NONE)
				{
					if(GetBtManager()->twsRole == BT_TWS_MASTER)
					{
						//if(sniff_lmpsend_get() == 0)
						{
							//����sniff��������ֹͣTWS���䣬���Ӷ������ tws_stop_callback().
							//sniff_lmpsend_set(1);
							tws_stop_transfer();
						}
					}
					else
					{
						tws_slave_send_cmd_sniff();
					}
				}
				else
				{
					sniff_lmpsend_set(0);
					MessageContext		msgSend;
					msgSend.msgId		= MSG_DEEPSLEEP;
					MessageSend(GetMainMessageHandle(), &msgSend);
				}
			}
		break;
	#else
		{
			MessageContext		msgSend;
			msgSend.msgId		= MSG_DEEPSLEEP;
			MessageSend(GetMainMessageHandle(), &msgSend);
		}
		break;
	#endif
	}
}
#else
bool BtTwsPairingState(void)
{
	return 0;
}
#endif


