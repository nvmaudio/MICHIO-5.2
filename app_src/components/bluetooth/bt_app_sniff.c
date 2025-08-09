/**
 **************************************************************************************
 * @file    bluetooth_sniff.c
 * @brief   bluetooth sniff��غ������ܽӿ�
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
//application
#include "bt_app_sniff.h"
#include "bt_app_connect.h"

#ifdef BT_SNIFF_ENABLE
typedef enum
{
	SNIFF_EXIT,
	SNIFF_READY,
	SNIFF_READY_EXIT,
	SNIFF_ENTER
}_SNIFF_STATE_t;

_SNIFF_STATE_t	sniff_state = SNIFF_EXIT;
static _BT_SNIFF_CTRL Bt_sniff_state_data;

uint8_t sniffaddaready_f = 0;//bit1:master ok!
							 //bit2:slaver ok!
uint32_t btEnterSniffCnt = 0;

extern uint8_t sniff_wakeup_get(void);
extern void sniff_wakeup_set(uint8_t set);
#endif
#ifdef BT_SNIFF_ENABLE
/*****************************************************************************************
 * 
 ****************************************************************************************/
void Bt_sniff_state_init(void)
{
	Bt_sniff_state_data.bt_sleep_enter = 0;
	Bt_sniff_state_data.bt_sniff_start = 0;
}

/*****************************************************************************************
 * ����stack deepsleep flag
 ****************************************************************************************/
void Bt_sniff_sleep_enter(void)
{
	Bt_sniff_state_data.bt_sleep_enter = 1;
}

/*****************************************************************************************
 * 
 ****************************************************************************************/
void Bt_sniff_sleep_exit(void)
{
	Bt_sniff_state_data.bt_sleep_enter = 0;
}

/*****************************************************************************************
 * 
 ****************************************************************************************/
uint8_t Bt_sniff_sleep_state_get(void)
{
	return Bt_sniff_state_data.bt_sleep_enter;
}

/*****************************************************************************************
 * ����sniff flag
 ****************************************************************************************/
void Bt_sniff_sniff_start(void)
{
#ifndef BT_TWS_SUPPORT
	Bt_sniff_fastpower_dis();
#endif
	Bt_sniff_state_data.bt_sniff_start = 1;
}

/*****************************************************************************************
 * 
 ****************************************************************************************/
void Bt_sniff_sniff_stop(void)
{
	Bt_sniff_state_data.bt_sniff_start = 0;
}

/*****************************************************************************************
 * 
 ****************************************************************************************/
uint8_t Bt_sniff_sniff_start_state_get(void)
{
	return Bt_sniff_state_data.bt_sniff_start;
}

#ifndef BT_TWS_SUPPORT
/*****************************************************************************************
 * ��������ʱ�ò���fastpower//����sniff EN��K29 change mode���ر�
 ****************************************************************************************/
void Bt_sniff_fastpower_dis(void)
{
	Bt_sniff_state_data.bt_sleep_fastpower_f = 1;
}

/*****************************************************************************************
 * 
 ****************************************************************************************/
void Bt_sniff_fastpower_en(void)
{
	Bt_sniff_state_data.bt_sleep_fastpower_f = 0;
}

/*****************************************************************************************
 * 
 ****************************************************************************************/
uint8_t Bt_sniff_fastpower_get(void)
{
	return Bt_sniff_state_data.bt_sleep_fastpower_f;
}
#endif//#ifndef BT_TWS_SUPPORT


#ifdef BT_SNIFF_ENABLE
/*****************************************************************************************
 * 
 ****************************************************************************************/
void SniffStateSet(_SNIFF_STATE_t state)
{
	sniff_state = state;
}

/*****************************************************************************************
 * 
 ****************************************************************************************/
_SNIFF_STATE_t SniffStateGet()
{
	return sniff_state;
}

/*****************************************************************************************
 * ������������deepsleep����Ϣ��btlib��ʹ��
 ****************************************************************************************/
void SendDeepSleepMsg(void)
{
	if(Bt_sniff_sniff_start_state_get())
	{
		Bt_sniff_sleep_enter();
	}
}

/*****************************************************************************************
 * 
 ****************************************************************************************/
void SysDeepsleepStandbyStatus(void)
{
	MessageContext		msgSend;

	if(sniff_wakeup_get() == 0)
	{
		sniff_wakeup_set(1);//����Ϊ����sniffģʽ
		msgSend.msgId		= MSG_BTSTACK_DEEPSLEEP;
		MessageSend(mainAppCt.msgHandle, &msgSend);
	}
}

/*****************************************************************************************
 * 
 ****************************************************************************************/
void tws_slave_ready_callback(uint32_t twsRole)
{
	switch(twsRole)
	{

	case 1://s
		{
			uint8_t flag = BtSniffADDAReadyGet();
			BtSniffADDAReadySet(flag | (1<<1));
		}
		break;
	case 0://m
		{
			uint8_t flag = BtSniffADDAReadyGet();
			BtSniffADDAReadySet(flag | (1<<1));
		}
		break;
	}

}

/*****************************************************************************************
 * 
 ****************************************************************************************/
void tws_master_ready_callback(uint32_t twsRole)
{
	switch(twsRole)
	{

	case 1://s
		{
			uint8_t flag = BtSniffADDAReadyGet();
			BtSniffADDAReadySet(flag | (1));
		}
		break;
	case 0://m
		{
			uint8_t flag = BtSniffADDAReadyGet();
			BtSniffADDAReadySet(flag | (1));
		}
		break;

	}
}

/*****************************************************************************************
 * 
 ****************************************************************************************/
void send_sniff_msg(void)
{
	MessageContext		msgSend;
	msgSend.msgId		= MSG_BT_SNIFF;
	MessageSend(GetMainMessageHandle(), &msgSend);
}

/*****************************************************************************************
 * 
 ****************************************************************************************/
void DisconnectFromPhone(void)
{
	if((GetA2dpState(BtCurIndex_Get()) < BT_A2DP_STATE_CONNECTED)
#if (BT_HFP_SUPPORT == ENABLE)
		||(GetHfpState(BtCurIndex_Get()) < BT_HFP_STATE_CONNECTED)
#endif
		)//�Ѿ��Ͽ�
		return;

	if(GetA2dpState(BtCurIndex_Get())>BT_A2DP_STATE_NONE)
	{
		A2dpDisconnect(BtCurIndex_Get());//���������ֻ��Ļ����ȶϿ�
	}
#if (BT_HFP_SUPPORT == ENABLE)
	if(GetHfpState(BtCurIndex_Get()) >= BT_HFP_STATE_CONNECTED)
	{
		BtHfpDisconnect(BtCurIndex_Get());//���������ֻ��Ļ����ȶϿ�
	}
#endif
	while((GetA2dpState(BtCurIndex_Get()) >= BT_A2DP_STATE_CONNECTED)
#if (BT_HFP_SUPPORT == ENABLE)
		||(GetHfpState(BtCurIndex_Get()) >= BT_HFP_STATE_CONNECTED)
#endif
		)//�ȴ��Ͽ��ɹ�
	{
		vTaskDelay(2);
	}
	send_sniff_msg();
//	BTSetRemDevIntoSniffMode(btManager.btTwsDeviceAddr);//�Ͽ��˳�sniff��Ȼ���ٽ�sniff
}

/*****************************************************************************************
 * 
 ****************************************************************************************/
void BTWakeupIOSet()
{
	extern uint8_t sniffiocnt;

	sniffiocnt = 0;//�����������Ѽ�����
	GPIO_PortAModeSet(GPIOA23,0);
	GPIO_RegOneBitSet(GPIO_A_IE,GPIOA23);
	GPIO_RegOneBitClear(GPIO_A_OE,GPIOA23);
	GPIO_RegOneBitSet(GPIO_A_PU,GPIOA23);
	GPIO_RegOneBitClear(GPIO_A_PD,GPIOA23);
}

/*****************************************************************************************
 * 
 ****************************************************************************************/
void BTSniffSet()
{
	if((Bt_sniff_sniff_start_state_get()))//&&(Bt_sniff_sleep_state_get()))
	{
		Bt_sniff_sleep_exit();

		DisconnectFromPhone();//���������ֻ��Ļ����ȶϿ�

//		BTWakeupIOSet();//���û���Դ��
	}
}

/*****************************************************************************************
 * 
 ****************************************************************************************/
void BtSniffExit_process(void)
{
#ifdef BT_TWS_SUPPORT
	if(tws_get_role() == BT_TWS_MASTER)
		BTSetAccessMode(BtAccessModeGeneralAccessible);

//	BTSetRemDevExitSniffMode(btManager.btTwsDeviceAddr);
	if(btManager.twsState > BT_TWS_STATE_NONE)
		BTSetRemDevExitSniffMode(btManager.btTwsDeviceAddr);
#endif
	Bt_sniff_sniff_stop();

}

/*****************************************************************************************
 * 
 ****************************************************************************************/
void BtSniffADDAReadySet(uint8_t set)
{
	sniffaddaready_f = set;
}

/*****************************************************************************************
 * 
 ****************************************************************************************/
uint8_t BtSniffADDAReadyGet()
{
	return sniffaddaready_f;
}

/*****************************************************************************************
 * 
 ****************************************************************************************/
void BtStartEnterSniffMode(void)
{
#ifdef BT_TWS_SUPPORT
	//ȡ��Master�����ֻ�����
	BtReconnectDevStop();
	
	/*if((GetA2dpState() >= BT_A2DP_STATE_CONNECTED) 
				|| (GetHfpState() >= BT_HFP_STATE_CONNECTED) 
				|| (GetAvrcpState() >= BT_AVRCP_STATE_CONNECTED))
				*/
	if(GetBtCurConnectFlag())
	{
		//�ֶ��Ͽ�
		BtDisconnectCtrl();

		btManager.btEnterSniffStep = 1;
	}
	else
	{
		if(tws_get_role() == BT_TWS_MASTER)
		{
			if(Bt_sniff_sniff_start_state_get() == 0)
			{
				APP_DBG("remote device enter sniff mode.\n");
				//DisconnectFromPhone();
				BTSetRemDevIntoSniffMode(btManager.btTwsDeviceAddr);
			}
			else
			{
				APP_DBG("remote device exit sniff mode.\n");
				//DisconnectFromPhone();//��modeΪ0ʱ�����ܽ���˴�˵�����ֻ�����ӽ�����sniffmode
									  //����ֱ�ӶϿ����ӡ�

			}
		}
	}
#endif
}

/*****************************************************************************************
 * 
 ****************************************************************************************/
void BtStartEnterSniffStep(void)
{
	if(btManager.btEnterSniffStep)
	{
		if((btManager.btLinkState == 0)&&(GetBtDeviceConnState() == BT_DEVICE_CONNECTION_MODE_ALL))
		{
			BTSetAccessMode(BtAccessModeNotAccessible);
			btManager.btExitSniffReconPhone=0;
			btManager.btEnterSniffStep = 0;
#ifdef BT_TWS_SUPPORT
			if(tws_get_role() == BT_TWS_MASTER)
			{
				if(Bt_sniff_sniff_start_state_get() == 0)
				{
					APP_DBG("remote device enter sniff mode.\n");
					BTSetRemDevIntoSniffMode(btManager.btTwsDeviceAddr);
					BTSetAccessMode(BtAccessModeNotAccessible);
				}
				else
				{
					APP_DBG("remote device exit sniff mode.\n");
					//DisconnectFromPhone();//��modeΪ0ʱ�����ܽ���˴�˵�����ֻ�����ӽ�����sniffmode
										  //����ֱ�ӶϿ����ӡ�

				}
			}
#endif
		}
		else
		{
			btEnterSniffCnt++;
			if(btEnterSniffCnt>=250)
			{
				btEnterSniffCnt=0;
				if((GetA2dpState(BtCurIndex_Get()) >= BT_A2DP_STATE_CONNECTED)
				|| (GetAvrcpState(BtCurIndex_Get()) >= BT_AVRCP_STATE_CONNECTED)
#if (BT_HFP_SUPPORT == ENABLE)
				|| (GetHfpState(BtCurIndex_Get()) >= BT_HFP_STATE_CONNECTED) 
#endif
				)
				{
					BtDisconnectCtrl();
				}
			}
		}
	}
}

/*****************************************************************************************
 * 
 ****************************************************************************************/
void BtExitSniffReconnectFlagSet(void)
{
	btManager.btExitSniffReconPhone = 1;
}

/*****************************************************************************************
 * 
 ****************************************************************************************/
void BtExitSniffReconnectPhone(void)
{
	if(btManager.btExitSniffReconPhone)
	{
		btManager.btExitSniffReconPhone++;
		if(btManager.btExitSniffReconPhone>=500)
		{
			btManager.btExitSniffReconPhone=0;
			
			if(sys_parameter.Bt_BackgroundType != BT_BACKGROUND_FAST_POWER_ON_OFF)
				BtConnectCtrl();
		}
	}
}

#else
void SendDeepSleepMsg(void)
{
}

void BtStartEnterSniffMode(void)
{
}

void BtStartEnterSniffStep(void)
{
}


void BtExitSniffReconnectPhone(void)
{
}

void send_sniff_msg()
{
}

void tws_slave_ready_callback(uint32_t twsRole)
{
}

void tws_master_ready_callback(uint32_t twsRole)
{
}
#endif	//BT_SNIFF_ENABLE

#else
uint8_t Bt_sniff_sniff_start_state_get(void)
{
	return 0;
}

void Bt_sniff_sniff_stop(void)
{
}

void send_sniff_msg()
{
}

void tws_slave_ready_callback(uint32_t twsRole)
{
}

void tws_master_ready_callback(uint32_t twsRole)
{
}

void SendDeepSleepMsg(void)
{
}

#endif //BT_SNIFF_ENABLE

