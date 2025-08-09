/**
 **************************************************************************************
 * @file    bluetooth_common.c
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
#include "type.h"
#include "delay.h"
#include "debug.h"
#include "app_config.h"

#include "chip_info.h"

#include "bt_config.h"
#include "bt_interface.h"
#include "bt_manager.h"
#include "bt_common_api.h"
#include "bt_app_ddb_info.h"
#include "bt_a2dp_api.h"
#include "bt_avrcp_api.h"
#include "bt_config.h"
#include "bb_api.h"
#include "rtos_api.h"
#include "bt_app_common.h"
#include "bt_app_connect.h"
#include "bt_app_tws_connect.h"
#include "bt_app_sniff.h"
#include "mode_task.h"
#include "bt_stack_service.h"
#include "bt_play_mode.h"
#include "spi_flash.h"
#include "sys_param.h"

//��������������״̬
#define BT_CON_REJECT		0
#define BT_CON_MASTER		1
#define BT_CON_SLAVE		2
#define BT_CON_UNKNOW		3

extern BT_CONFIGURATION_PARAMS		*btStackConfigParams;
#ifdef BT_TWS_SUPPORT
extern uint32_t gBtEnterDeepSleepFlag;
#endif

//extern api function
extern void Set_rwip_sleep_enable(bool flag);
extern bool is_tws_device(uint8_t *remote_addr);

/*****************************************************************************************
 * ���������ж�
 * ����1��˫�ֻ�+TWS
 ****************************************************************************************/ 
#if ((BT_DEVICE_NUMBER == 2)&&(BT_TWS_LINK_NUM == 1))
static uint32_t BtRemoteDeviceConnecting_DualPhoneAndTws(uint8_t *addr)
{
	if(btManager.btLastAddrUpgradeIgnored 
#if (BT_HFP_SUPPORT == ENABLE)
		|| (GetHfpState(BtCurIndex_Get()) > BT_HFP_STATE_CONNECTED)
#endif
		)
	{
		APP_DBG("****** mv test box is connected ***\n");
		return BT_CON_REJECT; //1. ���Ժ��������豸��,�����������豸�ٴ����ӽ��в��� 2. �����ڽ������������ͨ�����ܾ������豸����
	}

//������/��ɫ���
#if ((TWS_PAIRING_MODE == CFG_TWS_ROLE_RANDOM)||(TWS_PAIRING_MODE == CFG_TWS_PEER_MASTER) || (TWS_PAIRING_MODE == CFG_TWS_ROLE_MASTER))
	if(is_tws_device(addr))
	{
		if(btManager.twsState == BT_TWS_STATE_CONNECTED)
		{
			APP_DBG("tws state is connected, reject the new tws device\n");
			return BT_CON_REJECT;
		}

		if((btManager.btLinkState == 1)
			&&(btManager.linkedNumber >= 2))
		{
			APP_DBG("2phones are connected, reject the new tws device\n");
			return BT_CON_REJECT;
		}
			
		if((btManager.btLinkState == 1)
			||(GetA2dpState(BtCurIndex_Get()) >= BT_A2DP_STATE_CONNECTED)
			||(GetAvrcpState(BtCurIndex_Get()) >= BT_AVRCP_STATE_CONNECTED)
#if (BT_HFP_SUPPORT == ENABLE)
			|| (GetHfpState(BtCurIndex_Get()) >= BT_HFP_STATE_CONNECTED)
#endif
			)
		{
			if((memcmp(addr, btManager.btTwsDeviceAddr, 6) == 0)&&(btManager.twsFlag))
			{
				if(btManager.twsRole == BT_TWS_MASTER)
				{
					BT_DBG("phone connected, tws Role:Master\n");
					return BT_CON_MASTER;
				}
				else
				{
					BT_DBG("role not match, reject\n");
					return BT_CON_REJECT;
				}
			}
			else
			{
				BT_DBG("phone connected, reject\n");
				return BT_CON_REJECT;
			}
		}

		if((btManager.twsFlag)&&(btManager.twsRole == BT_TWS_MASTER))
		{
			if(memcmp(addr, btManager.btTwsDeviceAddr, 6) == 0)
			{
				APP_DBG("BT_TWS_MASTER: tws device connecting, Cur_Role:Master\n");
				return BT_CON_MASTER;
			}
			else
			{
				APP_DBG("BT_TWS_MASTER: tws device connecting, Cur_Role:Slave\n");
				return BT_CON_SLAVE;
			}
		}
		else if((btManager.twsFlag)&&(btManager.twsRole == BT_TWS_SLAVE))
		{
			APP_DBG("BT_TWS_SLAVE: tws device connecting, Cur_Role:Slave\n");
			BtReconnectTwsStop();
			return BT_CON_SLAVE;
		}
		else
		{
			APP_DBG("BT_TWS_SLAVE: tws device connecting, Cur_Role:unknow\n");
			return BT_CON_UNKNOW;
		}
	}
	else
	{
		if(!IsBtAudioMode() && sys_parameter.Bt_BackgroundType == BT_BACKGROUND_FAST_POWER_ON_OFF)
		{
			return BT_CON_REJECT;
		}
		else
		if((btManager.twsState == BT_TWS_STATE_CONNECTED))
		{
			if(btManager.twsRole == BT_TWS_SLAVE)
			{
				return BT_CON_REJECT;
			}

			if(btManager.btLinkState == 1)
			{
				return BT_CON_REJECT;
			}

			return BT_CON_MASTER;
		}
		else
		{
			if((btManager.btLinkState == 1)
				&&(btManager.linkedNumber >= 2)
				)
			{
				//˫�ֻ�������,�ܾ����ֻ�����
				return BT_CON_REJECT;
			}
			else
			{
				return BT_CON_MASTER;
			}
		}
	}

#elif(TWS_PAIRING_MODE == CFG_TWS_PEER_SLAVE)
	if(is_tws_device(addr))
	{
		if(btManager.twsState == BT_TWS_STATE_CONNECTED)
		{
			APP_DBG("tws state is connected, reject the new tws device\n");
			return BT_CON_REJECT;
		}
		else if(btManager.btLinkState == 1)
		{
			APP_DBG("phone connected, reject tws connect\n");
			return BT_CON_REJECT;
		}
		else
		{
			APP_DBG("tws device connecting\n");
			return BT_CON_SLAVE;
		}
	}
	else
	{
		if(!IsBtAudioMode() && sys_parameter.Bt_BackgroundType == BT_BACKGROUND_FAST_POWER_ON_OFF)
		{
			return BT_CON_REJECT;
		}
		else 
			if(btManager.twsState == BT_TWS_STATE_CONNECTED)
		{
			APP_DBG("tws state is connected, reject the new tws device\n");
			return BT_CON_REJECT;
		}
		else
		{
			if((btManager.btLinkState == 1)
				&&(btManager.linkedNumber >= 2)
				)
			{
				return BT_CON_REJECT;
			}
			else 
			{
				return BT_CON_MASTER;
			}
		}
	}
#else
	return BT_CON_SLAVE;
#endif
}
#endif

/*****************************************************************************************
 * ���������ж�
 * ����2�����ֻ�+TWS
 ****************************************************************************************/ 
#if ((BT_DEVICE_NUMBER == 1)&&(BT_TWS_LINK_NUM == 1))
static uint32_t BtRemoteDeviceConnecting_SinglePhoneAndTws(uint8_t *addr)
{
	if(btManager.btLastAddrUpgradeIgnored 
#if (BT_HFP_SUPPORT == ENABLE)
		|| (GetHfpState(BtCurIndex_Get()) > BT_HFP_STATE_CONNECTED)
#endif
		)
	{
		APP_DBG("****** mv test box is connected ***\n");
		return BT_CON_REJECT; //1. ���Ժ��������豸��,�����������豸�ٴ����ӽ��в��� 2. �����ڽ������������ͨ�����ܾ������豸����
	}

//������/��ɫ���
#if ((TWS_PAIRING_MODE == CFG_TWS_ROLE_RANDOM)||(TWS_PAIRING_MODE == CFG_TWS_PEER_MASTER) || (TWS_PAIRING_MODE == CFG_TWS_ROLE_MASTER))
	if(is_tws_device(addr))
	{
		if(btManager.twsState == BT_TWS_STATE_CONNECTED)
		{
			APP_DBG("tws state is connected, reject the new tws device\n");
			return BT_CON_REJECT;
		}

		#if (defined(BT_TWS_SUPPORT) && ((CFG_TWS_ONLY_IN_BT_MODE == ENABLE)))
		if(!IsBtAudioMode())
		{
			APP_DBG("no bt mode, reject tws device\n");
			return BT_CON_REJECT;
		}
		#endif

		if((btManager.btLinkState == 1)
			||(GetA2dpState(BtCurIndex_Get()) >= BT_A2DP_STATE_CONNECTED)
			||(GetAvrcpState(BtCurIndex_Get()) >= BT_AVRCP_STATE_CONNECTED)
#if (BT_HFP_SUPPORT == ENABLE)
			|| (GetHfpState(BtCurIndex_Get()) >= BT_HFP_STATE_CONNECTED)
#endif
			)
		{
			if((memcmp(addr, btManager.btTwsDeviceAddr, 6) == 0)&&(btManager.twsFlag))
			{
				if(btManager.twsRole == BT_TWS_MASTER)
				{
					BT_DBG("phone connected, tws Role:Master\n");
					return BT_CON_MASTER;
				}
				else
				{
					#if (TWS_PAIRING_MODE != CFG_TWS_ROLE_RANDOM)
					BT_DBG("role not match, reject\n");
					return BT_CON_REJECT;
					#else
					BT_DBG("phone connected2, tws Role:Master\n");
					return BT_CON_MASTER;
					#endif
				}
			}
			else
			{
				#if (TWS_PAIRING_MODE != CFG_TWS_ROLE_RANDOM)
				BT_DBG("role not match, reject\n");
				return BT_CON_REJECT;
				#else
				BT_DBG("phone connected2, tws Role:Master\n");
				return BT_CON_MASTER;
				#endif
			}
		}

		if((btManager.twsFlag)&&(btManager.twsRole == BT_TWS_MASTER))
		{
			if(memcmp(addr, btManager.btTwsDeviceAddr, 6) == 0)
			{
				APP_DBG("BT_TWS_MASTER: tws device connecting, Cur_Role:Master\n");
				return BT_CON_MASTER;
			}
			else
			{
				APP_DBG("BT_TWS_MASTER: tws device connecting, Cur_Role:Slave\n");
				return BT_CON_SLAVE;
			}
		}
		else if((btManager.twsFlag)&&(btManager.twsRole == BT_TWS_SLAVE))
		{
			APP_DBG("BT_TWS_SLAVE: tws device connecting, Cur_Role:Slave\n");
			BtReconnectTwsStop();
			return BT_CON_SLAVE;
		}
		else
		{
			APP_DBG("BT_TWS_SLAVE: tws device connecting, Cur_Role:unknow\n");
			return BT_CON_UNKNOW;
		}
	}
	else
	{
		if(!IsBtAudioMode() && sys_parameter.Bt_BackgroundType == BT_BACKGROUND_FAST_POWER_ON_OFF)
		{
			return BT_CON_REJECT;
		}
		else
		{
			extern unsigned char get_tws_device_number(void);
			extern uint8_t lc_link_number(void);

			if((lc_link_number()>=2) && (get_tws_device_number()==0))
			{
				printf("2 phone connected, disconnect current link req. \n");
				return BT_CON_REJECT;
			}
		}

		if((btManager.twsState == BT_TWS_STATE_CONNECTED))
		{
			if(btManager.twsRole == BT_TWS_SLAVE)
			{
				return BT_CON_REJECT;
			}

			if(btManager.btLinkState == 1)
			{
				return BT_CON_REJECT;
			}

			return BT_CON_MASTER;
		}
		else
		{
			if(btManager.btLinkState == 1)
			{
				//�ֻ�������,�ܾ����ֻ�����
				return BT_CON_REJECT;
			}
			else
			{
				return BT_CON_MASTER;
			}
		}
	}

#elif(TWS_PAIRING_MODE == CFG_TWS_PEER_SLAVE)
	if(is_tws_device(addr))
	{
		if(btManager.twsState == BT_TWS_STATE_CONNECTED)
		{
			APP_DBG("tws state is connected, reject the new tws device\n");
			return BT_CON_REJECT;
		}
		else if(btManager.btLinkState == 1)
		{
			APP_DBG("phone connected, reject tws connect\n");
			return BT_CON_REJECT;
		}
		else
		{
			APP_DBG("tws device connecting\n");
			return BT_CON_MASTER;//BT_CON_SLAVE;
		}
	}
	else
	{
		if(!IsBtAudioMode() && sys_parameter.Bt_BackgroundType == BT_BACKGROUND_FAST_POWER_ON_OFF)
		{
			return BT_CON_REJECT;
		}
		else if(btManager.twsState == BT_TWS_STATE_CONNECTED)
		{
			return BT_CON_MASTER;
		}
		else
		{
			if(btManager.btLinkState == 1)
			{
				return BT_CON_REJECT;
			}
			else 
			{
				return BT_CON_MASTER;
			}
		}
	}

#else
	return BT_CON_SLAVE;
#endif
}

#endif

/*****************************************************************************************
 * ���������ж�
 * ����3��˫�ֻ�
 ****************************************************************************************/ 
#if ((BT_DEVICE_NUMBER == 2)&&(BT_TWS_LINK_NUM == 0))
static uint32_t BtRemoteDeviceConnecting_DualPhone(uint8_t *addr)
{
	if(btManager.btLastAddrUpgradeIgnored 
#if (BT_HFP_SUPPORT == ENABLE)
		|| (GetHfpState(BtCurIndex_Get()) > BT_HFP_STATE_CONNECTED)
#endif
		)
	{
		APP_DBG("****** mv test box is connected ***\n");
		return BT_CON_REJECT; //1. ���Ժ��������豸��,�����������豸�ٴ����ӽ��в��� 2. �����ڽ������������ͨ�����ܾ������豸����
	}
	
	if(!IsBtAudioMode() && sys_parameter.Bt_BackgroundType == BT_BACKGROUND_FAST_POWER_ON_OFF)
	{
		return BT_CON_REJECT;
	}
	else
	{
		if((btManager.btLinkState == 1)
			&&(btManager.linkedNumber>=2)
			)
		{
			return BT_CON_REJECT;
		}
		else 
		{
			return BT_CON_MASTER;
		}
	}
}
#endif

/*****************************************************************************************
 * ���������ж�
 * ����4�����ֻ�
 ****************************************************************************************/ 
#if ((BT_DEVICE_NUMBER == 1)&&(BT_TWS_LINK_NUM == 0))
static uint32_t BtRemoteDeviceConnecting_SinglePhone(uint8_t *addr)
{
	if(btManager.btLastAddrUpgradeIgnored 
#if (BT_HFP_SUPPORT == ENABLE)
		|| (GetHfpState(BtCurIndex_Get()) > BT_HFP_STATE_CONNECTED)
#endif
		)
	{
		APP_DBG("****** mv test box is connected ***\n");
		return BT_CON_REJECT; //1. ���Ժ��������豸��,�����������豸�ٴ����ӽ��в��� 2. �����ڽ������������ͨ�����ܾ������豸����
	}
	
	if(!IsBtAudioMode() && sys_parameter.Bt_BackgroundType == BT_BACKGROUND_FAST_POWER_ON_OFF)
	{
		return BT_CON_REJECT;
	}
	else
	{
		if((btManager.btLinkState == 1)
			||(GetA2dpState(BtCurIndex_Get()) > BT_A2DP_STATE_NONE)
			||(GetAvrcpState(BtCurIndex_Get()) > BT_AVRCP_STATE_NONE)
#if (BT_HFP_SUPPORT == ENABLE)
			||(GetHfpState(BtCurIndex_Get()) > BT_HFP_STATE_NONE)
#endif
			)
		{
			return BT_CON_REJECT;
		}
		else 
		{
			return BT_CON_SLAVE;
		}
	}
}
#endif

/***********************************************************************************
 * ������ʱ,ȷ����������
 * return: 
 * 0=reject
 * 1=master
 * 2=slave
 **********************************************************************************/
uint32_t comfirm_connecting_condition(uint8_t *addr)
{
#if ((BT_DEVICE_NUMBER == 2)&&(BT_TWS_LINK_NUM == 1))
	return BtRemoteDeviceConnecting_DualPhoneAndTws(addr);
#elif ((BT_DEVICE_NUMBER == 1)&&(BT_TWS_LINK_NUM == 1))
	return BtRemoteDeviceConnecting_SinglePhoneAndTws(addr);
#elif ((BT_DEVICE_NUMBER == 2)&&(BT_TWS_LINK_NUM == 0))
	return BtRemoteDeviceConnecting_DualPhone(addr);
#else //((BT_DEVICE_NUMBER == 1)&&(BT_TWS_LINK_NUM == 0))
	return BtRemoteDeviceConnecting_SinglePhone(addr);
#endif
}




/*****************************************************************************************
 * ������ǰ�����ɼ���
 ****************************************************************************************/ 
void BtAccessModeUpdate(BtAccessMode accessMode)
{
	BT_DBG("$$$ access mode %d->%d\n", btManager.deviceConMode, accessMode);
	SetBtDeviceConnState(accessMode);
}

/*****************************************************************************************
 * ����ʵ�ʵ�Ӧ���������������������ɼ���
 * ����1��˫�ֻ�+TWS
 ****************************************************************************************/ 
#if ((BT_DEVICE_NUMBER == 2)&&(BT_TWS_LINK_NUM == 1))
static void BtAccessMode_DualPhoneAndTws(void)
{
	printf("BtAccessModeSetting, DualPhoneAndTws \n");
	if(btManager.twsState == BT_TWS_STATE_CONNECTED)
	{
		if(btManager.btLinkState)
		{
			BtSetAccessMode_NoDisc_NoCon();
		}
		else
		{
			if(sys_parameter.Bt_BackgroundType == BT_BACKGROUND_FAST_POWER_ON_OFF)
			{
				if(IsBtAudioMode())
				{
					BtSetAccessMode_Disc_Con();
				}
				else
				{
					BtSetAccessMode_NoDisc_NoCon();
				}
			}
			else
				BtSetAccessMode_Disc_Con();
		}
	}
	else
	{
		if(btManager.btLinkState)
		{
			if(IsIdleModeReady())
			{
				BtSetAccessMode_NoDisc_NoCon();
			}
			else if(btManager.linkedNumber >= 2)
			{
				BtSetAccessMode_NoDisc_NoCon();
			}
			else
			{
			#ifdef BT_LINK_2DEV_ACCESS_DIS_CON
				BtSetAccessMode_Disc_Con();
			#else
				BtSetAccessMode_NoDisc_Con();
			#endif
			}
		}
		else
		{
			if(IsIdleModeReady())
			{
				BtSetAccessMode_NoDisc_NoCon();
			}
			else if(sys_parameter.Bt_BackgroundType == BT_BACKGROUND_FAST_POWER_ON_OFF)
			{
				if(IsBtAudioMode())
				{
					BtSetAccessMode_Disc_Con();
				}
				else if(IsBtTwsSlaveMode())
				{
					BtSetAccessMode_NoDisc_NoCon();
				}
				else
				{
					#if (CFG_TWS_ONLY_IN_BT_MODE == ENABLE)
					BtSetAccessMode_NoDisc_NoCon();
					#else
					BtSetAccessMode_NoDisc_Con();
					#endif
				}
			}
			else
			{
				BtSetAccessMode_Disc_Con();
			}
		}
	}
}
#endif
/*****************************************************************************************
 * ����ʵ�ʵ�Ӧ���������������������ɼ���
 * ����2�����ֻ�+TWS
 ****************************************************************************************/ 
#if ((BT_DEVICE_NUMBER == 1)&&(BT_TWS_LINK_NUM == 1))
static void BtAccessMode_SinglePhoneAndTws(void)
{
	printf("BtAccessModeSetting, SinglePhoneAndTws \n");
	if(btManager.twsState == BT_TWS_STATE_CONNECTED)
	{
		if(btManager.btLinkState)
		{
			BtSetAccessMode_NoDisc_NoCon();
		}
		else
		{
			if(sys_parameter.Bt_BackgroundType == BT_BACKGROUND_FAST_POWER_ON_OFF)
			{
				if(IsBtAudioMode())
				{
					BtSetAccessMode_Disc_Con();
				}
				else
				{
					BtSetAccessMode_NoDisc_NoCon();
				}
			}
			else
				BtSetAccessMode_Disc_Con();
		}
	}
	else
	{
		if(btManager.btLinkState)
		{
			if(IsIdleModeReady())
			{
				BtSetAccessMode_NoDisc_NoCon();
			}
			else if((btManager.twsFlag) && (btManager.twsRole == BT_TWS_SLAVE))
			{
				BtSetAccessMode_NoDisc_NoCon();
				BtReconnectTwsStop();
			}
			else
			{
				BtSetAccessMode_NoDisc_Con();
			}
		}
		else
		{
			if(IsIdleModeReady())
			{
				BtSetAccessMode_NoDisc_NoCon();
			}
			else if(sys_parameter.Bt_BackgroundType == BT_BACKGROUND_FAST_POWER_ON_OFF)
			{
				if(IsBtAudioMode())
				{
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
				}
				else if(IsBtTwsSlaveMode())
				{
					BtSetAccessMode_NoDisc_NoCon();
				}
				else
				{
					#if (CFG_TWS_ONLY_IN_BT_MODE == ENABLE)
						BtSetAccessMode_NoDisc_NoCon();
					#else
						BtSetAccessMode_NoDisc_Con();
					#endif
				}
			}
			else
			{
				#if (TWS_PAIRING_MODE == CFG_TWS_ROLE_SLAVE)
				BtSetAccessMode_NoDisc_Con();
				#else
				BtSetAccessMode_Disc_Con();
				#endif
			}
		}
	}
}
#endif
/*****************************************************************************************
 * ����ʵ�ʵ�Ӧ���������������������ɼ���
 * ����3��˫�ֻ�
 ****************************************************************************************/ 
#if ((BT_DEVICE_NUMBER == 2)&&(BT_TWS_LINK_NUM == 0))
static void BtAccessMode_DualPhone(void)
{
	printf("BtAccessModeSetting, DualPhone \n");

	if(btManager.btLinkState)
	{
		if(IsIdleModeReady())
		{
			BtSetAccessMode_NoDisc_NoCon();
		}
		else if(btManager.linkedNumber >= 2)
		{
			BtSetAccessMode_NoDisc_NoCon();
		}
		else
		{
#ifdef BT_LINK_2DEV_ACCESS_DIS_CON
			BtSetAccessMode_Disc_Con();
#else
			BtSetAccessMode_NoDisc_Con();
#endif
		}
	}
	else
	{
		if(IsIdleModeReady())
		{
			BtSetAccessMode_NoDisc_NoCon();
		}
		else if(sys_parameter.Bt_BackgroundType == BT_BACKGROUND_FAST_POWER_ON_OFF)
		{
			if(IsBtAudioMode())
			{
				BtSetAccessMode_Disc_Con();
			}
			else
			{
				BtSetAccessMode_NoDisc_NoCon();
			}
		}
		else
		{
			BtSetAccessMode_Disc_Con();
		}
	}
}
#endif
/*****************************************************************************************
 * ����ʵ�ʵ�Ӧ���������������������ɼ���
 * ����4�����ֻ�
 ****************************************************************************************/ 
#if ((BT_DEVICE_NUMBER == 1)&&(BT_TWS_LINK_NUM == 0))
static void BtAccessMode_SinglePhone(void)
{
	printf("BtAccessModeSetting, SinglePhone \n");
	
	if(btManager.btLinkState)
	{
		BtSetAccessMode_NoDisc_NoCon();
	}
	else
	{
		if(IsIdleModeReady())
		{
			BtSetAccessMode_NoDisc_NoCon();
		}
		else if(sys_parameter.Bt_BackgroundType == BT_BACKGROUND_FAST_POWER_ON_OFF)
		{
			if(IsBtAudioMode())
			{
				BtSetAccessMode_Disc_Con();
			}
			else
			{
				BtSetAccessMode_NoDisc_NoCon();
			}
		}
		else
		{
			BtSetAccessMode_Disc_Con();
		}
	}
}
#endif
/*****************************************************************************************
 * ����ʵ�ʵ�Ӧ���������������������ɼ���
 ****************************************************************************************/ 
void BtAccessModeSetting(void)
{
#if ((BT_DEVICE_NUMBER == 2)&&(BT_TWS_LINK_NUM == 1))
	BtAccessMode_DualPhoneAndTws();
#elif ((BT_DEVICE_NUMBER == 1)&&(BT_TWS_LINK_NUM == 1))
	BtAccessMode_SinglePhoneAndTws();
#elif ((BT_DEVICE_NUMBER == 2)&&(BT_TWS_LINK_NUM == 0))
	BtAccessMode_DualPhone();
#else //((BT_DEVICE_NUMBER == 1)&&(BT_TWS_LINK_NUM == 0))
	BtAccessMode_SinglePhone();
#endif
}

/*****************************************************************************************
 * ����Э��ջ��ʼ���ɹ�����
 ****************************************************************************************/
void BtStackInitialized(void)
{
	APP_DBG("BT_STACK_EVENT_COMMON_STACK_INITIALIZED\n");
	SetBtDeviceConnState(BT_DEVICE_CONNECTION_MODE_NONE);
/*#ifdef BT_SNIFF_ENABLE
	//enable bb enter sleep
	Set_rwip_sleep_enable(1);
#else
	Set_rwip_sleep_enable(0);
#endif*/
	
	Set_rwip_sleep_enable(0);
	
	BtMidMessageSend(MSG_BT_MID_STACK_INIT, 0);
}

/*****************************************************************************************
 * ����remote devie����״̬ sniff/active
 ****************************************************************************************/
void BtLinkModeChanged(BT_STACK_CALLBACK_PARAMS * param)
{
#ifdef BT_TWS_SUPPORT
	extern bool tws_connect_cmp(uint8_t * addr);
	if(tws_connect_cmp(param->params.modeChange.addr) == 0)
	{
		btManager.twsMode = param->params.modeChange.mode;
#endif
		
#ifdef BT_SNIFF_ENABLE
		if(param->params.modeChange.mode == BTLinkModeSniff)
		{
			Bt_sniff_sniff_start();
		}
		else
		{
			Bt_sniff_sniff_stop();
		}
#endif

#ifdef BT_TWS_SUPPORT
	}
#endif
}

/*****************************************************************************************
 * �������Ʒ�������
 ****************************************************************************************/
void BtGetRemoteName(BT_STACK_CALLBACK_PARAMS * param)
{
	memset(btManager.remoteName, 0, 40);
	btManager.remoteNameLen = 0;
	
	if((param->params.remDevName.nameLen) && (param->params.remDevName.name))
	{
		if(param->params.remDevName.nameLen < 40)
		{
			btManager.remoteNameLen = param->params.remDevName.nameLen;
		}
		else
		{
			btManager.remoteNameLen = 40;
		}
		memcpy(btManager.remoteName, param->params.remDevName.name, btManager.remoteNameLen);
	}
	APP_DBG("\t nameLen = %d , name = %s \n",btManager.remoteNameLen, btManager.remoteName);
	
	
	if((param->params.remDevName.name[0] == 'M')
		&& (param->params.remDevName.name[1] == 'V')
		&& (param->params.remDevName.name[2] == '_')
		&& (param->params.remDevName.name[3] == 'B')
		&& (param->params.remDevName.name[4] == 'T')
		&& (param->params.remDevName.name[5] == 'B')
		&& (param->params.remDevName.name[6] == 'O')
		&& (param->params.remDevName.name[7] == 'X')
		)
	{
        if(btManager.btLinkState)
        {
    	    printf("device is connected, disconnect btbox\n");
    		btManager.btLastAddrUpgradeIgnored = 0;
            BTHciDisconnectCmd(param->params.remDevName.addr);
        }
        else
        {
    		printf("connect btbox\n");
    		btManager.btLastAddrUpgradeIgnored = 1;
        }
	}

}

/*****************************************************************************************
 * ����page���ӳ�ʱ
 ****************************************************************************************/
void BtLinkPageTimeout(BT_STACK_CALLBACK_PARAMS * param)
{
	BtStackServiceMsgSend(MSG_BTSTACK_RECONNECT_REMOTE_PAGE_TIMEOUT);
	BtStackServiceMsgSend(MSG_BTSTACK_RECONNECT_TWS_PAGE_TIMEOUT);
	
#if (defined(BT_TWS_SUPPORT) && (TWS_PAIRING_MODE == CFG_TWS_ROLE_SLAVE))
	if(!btManager.twsSbSlaveDisable)
	{
		if((btManager.twsEnterPairingFlag)&&(gBtTwsAppCt->btTwsEvent&BT_TWS_EVENT_SIMPLE_PAIRING))
			tws_slave_start_simple_pairing();
		else
			tws_slave_simple_pairing_ready();
	}
#endif// defined(BT_TWS_SUPPORT) && (TWS_PAIRING_MODE == CFG_TWS_ROLE_SLAVE)
}

/*****************************************************************************************
 * ���������쳣�Ͽ�
 ****************************************************************************************/
/*void BtDevConnectionAborted(void)
{
	//��L2CAP�쳣�Ͽ�����Э��ʱ����ʼ������������,30s��ʱ
	btEventListB1State = 0;
	{
		if(GetBtPlayState() == 1)
			btEventListB1State = 1;
	}

	btCheckEventList |= BT_EVENT_L2CAP_LINK_DISCONNECT;
	btEventListB1Count = btEventListCount;
	btEventListB1Count += 30000;
}*/

/*****************************************************************************************
 * ������������ʧ��
 * ����: �ֻ���ȡ����Լ�¼,�ֻ��˷�����֤ʧ��
 * ������ʽ: ȡ����������
 ****************************************************************************************/
void BtPairingFail(void)
{
	//�ֻ���ȡ����ԣ�ϵͳ�����������ֻ��˻ᷴ����msg���ظ���֤ʧ��
	//����ֹͣ����������ѡ�������Լ�¼
	BtReconnectDevStop();
}

/*****************************************************************************************
 * �������Ӷ�ʧ
 * ����: �ֻ�����Զ��
 ****************************************************************************************/
void BtDevConnectionLinkLoss(BT_STACK_CALLBACK_PARAMS * param)
{
#ifdef BT_TWS_SUPPORT
	APP_DBG("LINK LOST\n");
#else
	//connection timeout
	if(param->errorCode == 0x08)
	{
		APP_DBG("BB LOST\n");
#endif //#ifdef BT_TWS_SUPPORT

	if(!btManager.btLastAddrUpgradeIgnored)
	{
	
#ifdef BT_TWS_SUPPORT
		//tws�Ѿ�����,����Ϊ��,���ܻ�������
		if((btManager.twsState == BT_TWS_STATE_CONNECTED)&&(btManager.twsRole == BT_TWS_SLAVE))
			return;
#endif

		if(bt_BBLostReconnectionEnable)
		{
			//bb lost: reconnect device
			BtReconnectDevCreate(btManager.btDdbLastAddr, bt_BBLostTryCounts, bt_BBLostInternalTime, 1000, btManager.btDdbLastProfile);
		}
	}

	btManager.btLastAddrUpgradeIgnored = 0;

#ifndef BT_TWS_SUPPORT
	}
#endif //#ifndef BT_TWS_SUPPORT
}

/*****************************************************************************************
 * TWS���Ӷ�ʧ
 ****************************************************************************************/
#ifdef BT_TWS_SUPPORT
void BtTwsConnectionLinkLoss(void)
{
	APP_DBG("[TWS]LINK LOST\n");
	
#ifdef BT_SNIFF_ENABLE
	{
		extern void sniff_lmpsend_set(uint8_t set);
		sniff_lmpsend_set(0);
	}
#endif

#if (TWS_PAIRING_MODE == CFG_TWS_PEER_SLAVE)
	if(GetSystemMode() == ModeTwsSlavePlay)
	{
		MessageContext		msgSend;
		msgSend.msgId		= MSG_BT_TWS_LINKLOSS;
		//MessageSend(GetSysModeMsgHandle(), &msgSend);
		return;
	}
#endif

#if (TWS_PAIRING_MODE == CFG_TWS_PEER_MASTER)	
	if(btManager.twsRole == BT_TWS_SLAVE)
		BtTwsLinkLoss();
#else
	BtTwsLinkLoss();
#endif
}
/*****************************************************************************************
 * TWS���ӳ�ʱ(����page����)
 ****************************************************************************************/
void BtTwsPairingTimeout(void)
{
	CheckBtTwsPairing();
}
#endif

/***********************************************************************************
 * ��ȡ��ǰϵͳ���õ�֧������Э��
 **********************************************************************************/
uint32_t GetSupportProfiles(void)
{
	uint32_t		profiles = 0;
	
#if BT_HFP_SUPPORT == ENABLE
	profiles |= BT_PROFILE_SUPPORTED_HFP;
#endif

#if BT_A2DP_SUPPORT == ENABLE
	profiles |= BT_PROFILE_SUPPORTED_A2DP;
	profiles |= BT_PROFILE_SUPPORTED_AVRCP;
#endif

#if (BT_SPP_SUPPORT == ENABLE ||(defined(CFG_FUNC_BT_OTA_EN)))
	profiles |= BT_PROFILE_SUPPORTED_SPP;
#endif
	
#if BT_HID_SUPPORT == ENABLE
	profiles |= BT_PROFILE_SUPPORTED_HID;
#endif
	
#if BT_MFI_SUPPORT == ENABLE
	profiles |= BT_PROFILE_SUPPORTED_MFI;
#endif
	
#if BT_OBEX_SUPPORT == ENABLE
	profiles |= BT_PROFILE_SUPPORTED_OBEX;
#endif

#if BT_PBAP_SUPPORT == ENABLE
	profiles |= BT_PROFILE_SUPPORTED_PBAP;
#endif

#ifdef BT_TWS_SUPPORT
	profiles |= BT_PROFILE_SUPPORTED_TWS;
#endif

	return profiles;
}
#if 0
/***********************************************************************************
 * ����������Э�����
 * ���������ӵ�Э��
 **********************************************************************************/
void SetBtConnectedProfile(uint16_t connectedProfile)
{
	btManager.btConnectedProfile |= connectedProfile;

	SetBtCurConnectFlag(1);

	if(btManager.btLastAddrUpgradeIgnored)
		return;
	
	//���ӵ�һ�������豸��һ��profileʱ,�͸������1�����ӵ�������¼��Ϣ
	if(GetNumOfBtConnectedProfile() == 1)
	{
		//��Ե�ַһ��,����Ҫ�ظ�����
		if(memcmp(btManager.btDdbLastAddr, btManager.remoteAddr, 6) == 0)
		{
			if(btManager.btDdbLastProfile & connectedProfile)
				return ;
		}
		
		btManager.btDdbLastProfile = (uint8_t)btManager.btConnectedProfile;

		//BP10���������ϴ����ӵ��豸�����µ��ֻ����ӣ�����MAC��ַ��һ�£���ֹͣ����
		BtReconnectDevStop();

		memcpy(btManager.btDdbLastAddr, btManager.remoteAddr, 6);
		BtDdb_UpgradeLastBtAddr(btManager.remoteAddr, btManager.btDdbLastProfile);
	}
	else
	{
		if((btManager.btDdbLastProfile & connectedProfile) == 0 )
		{
			btManager.btDdbLastProfile |= connectedProfile;
#ifdef CFG_FUNC_OPEN_SLOW_DEVICE_TASK
			{
			extern void SlowDevice_MsgSend(uint16_t msgId);
			SlowDevice_MsgSend(MSG_DEVICE_BT_LINKED_PROFILE_UPDATE);
			}
#else
			BtDdb_UpgradeLastBtProfile(btManager.remoteAddr, btManager.btDdbLastProfile);
#endif
		}
	}
}


/***********************************************************************************
 * ��ȡ�ѶϿ���Э��
 **********************************************************************************/
void SetBtDisconnectProfile(uint16_t disconnectProfile)
{
	btManager.btConnectedProfile &= ~disconnectProfile;

	if(!btManager.btConnectedProfile)
	{
		SetBtCurConnectFlag(0);
	}
}
#endif
/***********************************************************************************
 * ��ȡ�����ӵ�Э����Ϣ
 **********************************************************************************/
uint16_t GetBtConnectedProfile(void)
{
	return btManager.btConnectedProfile;
}

/***********************************************************************************
 * ��ȡ�����ӵ�Э�����
 **********************************************************************************/
uint8_t GetNumOfBtConnectedProfile(void)
{
	uint8_t number_of_profile = 0;
	uint8_t i;

	//���ݳ��õ�A2DP/AVRCP/HFP����Э��,ȷ�ϵ�ǰ�����ӵ�Э��״̬
	for(i=0;i<3;i++)
	{
		if((btManager.btConnectedProfile >> i)&0x01)
		{
			number_of_profile++;
		}
	}

	return 	number_of_profile;
}

/***********************************************************************************
 * get device name (max 40bytes)
 * ��ȡ���ش�ͳ�����豸����
 **********************************************************************************/
uint8_t* BtDeviceNameGet(void)
{
	return &btStackConfigParams->bt_LocalDeviceName[0];
}

/***********************************************************************************
 * ���� �������Ƶ�flash
 **********************************************************************************/
int32_t BtDeviceNameSet(void)
{
	APP_DBG("device name set!\n");

	return BtDeviceSaveNameToFlash(sys_parameter.BT_Name,strlen(sys_parameter.BT_Name),0);

}

/***********************************************************************************
 * pin code get (max: 16bytes)
 * ��ȡpin code��Ϣ
 **********************************************************************************/
uint8_t* BtPinCodeGet(void)
{
	return btStackConfigParams->bt_pinCode;
}

/***********************************************************************************
 * save pin code to flash
 **********************************************************************************/
int32_t BtPinCodeSet(uint8_t *pinCode)
{
	BT_CONFIGURATION_PARAMS 	*btParams = NULL;
	int8_t ret=0;
	APP_DBG("pin code set!\n");

	//1.�Ƿ����������
	if(btStackConfigParams->bt_simplePairingFunc)
		return -3;//ģʽ����

	//2.����RAM
	btParams = (BT_CONFIGURATION_PARAMS*)osPortMalloc(sizeof(BT_CONFIGURATION_PARAMS));
	if(btParams == NULL)
	{
		APP_DBG("ERROR: Ram is not enough!\n");
		return -2;//RAM����
	}
	memcpy(btParams, btStackConfigParams, sizeof(BT_CONFIGURATION_PARAMS));
	
	//3.��pin code����
	memcpy(btStackConfigParams->bt_pinCode, pinCode, 4);
	memcpy(btParams->bt_pinCode, pinCode, 4);

	//4.���������ݱ��浽flash
	BtDdb_SaveBtConfigurationParams(btParams);
	memset(btParams, 0, sizeof(BT_CONFIGURATION_PARAMS));

	//5.���´�flash�ж�ȡ���ݣ��ٴν��жԱ�
	ret = BtDdb_LoadBtConfigurationParams(btParams);
	if(ret == -3)
	{
		//��ȡ�쳣��read again
		ret = BtDdb_LoadBtConfigurationParams(btParams);
		if(ret == -3)
		{
			APP_DBG("bt database read error!\n");
			osPortFree(btParams);
			return -3;//��ȡʧ��
		}
	}

	ret = memcmp(btStackConfigParams, btParams,sizeof(BT_CONFIGURATION_PARAMS));
	if(ret == 0)
	{
		APP_DBG("save ok!\n");
		osPortFree(btParams);
		return 0;
	}
	else
	{
		APP_DBG("save NG!\n");
		osPortFree(btParams);
		return -4;//����ʧ��
	}
}

/***********************************************************************************
 * @brief:  �����¼�����
 **********************************************************************************/
void BtEventFlagRegister(uint32_t SoftEvent, uint32_t SoftTimeOut)
{
	btManager.btEventFlagMask = SoftEvent;
	btManager.btEventFlagCount = SoftTimeOut;
}

void BtEventFlagDeregister(uint32_t SoftEvent)
{
	if(btManager.btEventFlagMask == SoftEvent)
	{
		btManager.btEventFlagMask = BT_EVENT_FLAG_NONE;
		btManager.btEventFlagCount = 0;
	}
}

void BtEventFlagProcess(void)
{
	if(btManager.btEventFlagMask)
	{
		if(BT_EVENT_FLAG_AVRCP_CONNECT == btManager.btEventFlagMask)
		{
			if(btManager.btEventFlagCount)
			{
				btManager.btEventFlagCount--;
				if(btManager.btEventFlagCount == 0)
				{
					if((GetAvrcpState(BtCurIndex_Get()) < BT_AVRCP_STATE_CONNECTED)&&(GetA2dpState(BtCurIndex_Get()) >= BT_A2DP_STATE_CONNECTED))
					{
						printf("+++ avrcp connect \n");
						BtAvrcpConnect(BtCurIndex_Get(),btManager.remoteAddr);
					}
					btManager.btEventFlagMask = BT_EVENT_FLAG_NONE;
				}
			}
			else
			{
				btManager.btEventFlagCount = 0;
				btManager.btEventFlagMask = BT_EVENT_FLAG_NONE;
			}
		}
		else if(BT_EVENT_FLAG_AVRCP_DISCONNECT == btManager.btEventFlagMask)
		{
			if(btManager.btEventFlagCount)
			{
				btManager.btEventFlagCount--;
				if(btManager.btEventFlagCount == 0)
				{
					if(GetAvrcpState(BtCurIndex_Get()) > BT_AVRCP_STATE_NONE)
					{
						printf("--- avrcp disconnect \n");
						AvrcpDisconnect(BtCurIndex_Get());
					}
					btManager.btEventFlagMask = BT_EVENT_FLAG_NONE;
				}
			}
			else
			{
				btManager.btEventFlagCount = 0;
				btManager.btEventFlagMask = BT_EVENT_FLAG_NONE;
			}
		}
		else if(BT_EVENT_FLAG_A2DP_DISCONNECT  == btManager.btEventFlagMask)
		{
			if(btManager.btEventFlagCount)
			{
				btManager.btEventFlagCount--;
				if(btManager.btEventFlagCount == 0)
				{
					if(GetA2dpState(BtCurIndex_Get()) > BT_A2DP_STATE_NONE)
					{
						printf("--- a2dp disconnect \n");
						A2dpDisconnect(BtCurIndex_Get());
					}
					btManager.btEventFlagMask = BT_EVENT_FLAG_NONE;
				}
			}
			else
			{
				btManager.btEventFlagCount = 0;
				btManager.btEventFlagMask = BT_EVENT_FLAG_NONE;
			}
		}
	}
}

/***********************************************************************************
 * ���� ����BLE���Ƶ�flash
 **********************************************************************************/
int32_t BtDeviceBleNameSet(uint8_t* deviceName, uint8_t deviceLen)
{

	uint32_t addr = get_sys_parameter_addr();

	SPI_FLASH_ERR_CODE ret=0;

	uint8_t len = deviceLen;
	
	if(deviceName == NULL || len == 0)
	{
		APP_DBG("write ble name error:params is null\n");
		return -1;
	}
	APP_DBG("write ble name :%s\n", deviceName);
	
	if(len > BLE_NAME_SIZE)
		len = BLE_NAME_SIZE;
	memset(sys_parameter.BT_Name,0,BLE_NAME_SIZE);
	memcpy(sys_parameter.BT_Name,deviceName,len);
	memcpy(btStackConfigParams->ble_LocalDeviceName, deviceName, len);

	//1.erase
	SpiFlashErase(SECTOR_ERASE, (addr/4096), 1);

	//2.write params
	ret = SpiFlashWrite(addr, (uint8_t*)&sys_parameter, sizeof(SYS_PARAMETER), 1);
	if(ret != FLASH_NONE_ERR)
	{
		APP_DBG("write ble name error:%d\n", ret);
		return -2;
	}

	APP_DBG("write ble name success\n");
	
	return 0;
}

