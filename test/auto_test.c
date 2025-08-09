/**
 **************************************************************************************
 * @file    auto_test.c
 * @brief   
 *
 * @author 
 * @version V1.0.0
 *
 * $Created: 2021-10-25 13:06:47$
 *
 * @Copyright (C) 2016, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#include <string.h>
#include "type.h"
#include "app_config.h"
#include "timeout.h"
#include "rtos_api.h"
#include "app_message.h"
#include "debug.h"
#include "dma.h"
#include "clk.h"
#include "main_task.h"
#include "timer.h"
#include "irqn.h"
#include "watchdog.h"
#include "dac.h"
#include "ctrlvars.h"
#include "delay.h"
#include "efuse.h"
#include "uarts_interface.h"
//functions
#include "powercontroller.h"
#include "deepsleep.h"
#include "flash_boot.h"
#include "breakpoint.h"
#include "remind_sound.h"
#include "rtc_alarm.h"
#include "rtc.h"
#include "audio_vol.h"
#include "device_detect.h"
#include "otg_detect.h"
#include "sadc_interface.h"
#include "adc.h"
#include "adc_key.h"
#include "ir_key.h"
#include "otg_device_hcd.h"
#include "otg_device_standard_request.h"
//services
#include "shell.h"
#include "audio_core_service.h"
#include "audio_core_api.h"
#include "bt_stack_service.h"
#include "bb_api.h"
#include "bt_config.h"
#include "bt_app_ddb_info.h"
#ifdef BT_TWS_SUPPORT
#include "bt_tws_api.h"
#endif
#include "bt_hf_mode.h"
#include "idle_mode.h"
#include "bt_app_tws_connect.h"
#include "tws_mode.h"
#include "bt_app_connect.h"
#include "auto_test.h"

#ifdef AUTO_TEST_ENABLE

void AutoTestMsgSend(uint16_t msg)
{
	MessageContext		msgSend;
	msgSend.msgId		= msg;
	MessageSend(GetMainMessageHandle(), &msgSend);
}

void TwsLinkTest(void)
{
	TestApp.TimerRand = rand() % (10001) + 50;//[50ms ~ 10050ms]

	DBG("GetBtManager()->twsState: %d twsRole: %d twsFlag:  %d btTwsPairingStart: %d\n", GetBtManager()->twsState, GetBtManager()->twsRole, GetBtManager()->twsFlag, gBtTwsAppCt->btTwsPairingStart);

	if(GetBtManager()->twsState == BT_TWS_STATE_CONNECTED)
	{
		//master�Ͽ�����
		if (GetBtManager()->twsRole == BT_TWS_MASTER)
			AutoTestMsgSend(MSG_BT_TWS_PAIRING);
	}
	else if(GetBtManager()->twsFlag)
	{
		//slave��������
		if ((GetBtManager()->twsRole == BT_TWS_SLAVE)&&(gBtTwsAppCt->btTwsPairingStart == 0))
			AutoTestMsgSend(MSG_BT_TWS_PAIRING);
	}
	else
	{
		//��������¼ֱ�ӷ�������
		if(gBtTwsAppCt->btTwsPairingStart == 0)
			AutoTestMsgSend(MSG_BT_TWS_PAIRING);
	}
}

void AutoTestMsgDel(uint16_t msg)
{
	switch(msg)
	{
		case MSG_AUTO_TEST_START:
		if (TestApp.AutoTestMode == TEST_NONE)
		{
			TestApp.AutoTestMode = TEST_HFP_MODE;
			DBG("auto test start\n");
		}
		else
		{
			TestApp.AutoTestMode = TEST_NONE;
			DBG("auto test stop\n");
		}
		break;	

		default:
		break;
	}
}

void AutoTestMain(uint16_t test_msg)
{
	AutoTestMsgDel(test_msg);
	
	if(TestApp.AutoTestMode != TEST_NONE)
	{
		if(IsTimeOut(&TestApp.TestScanTimer))
		{
			switch(TestApp.AutoTestMode)
			{
				case TEST_NONE:
				break;
				
				//������Ͽ��ֻ�
				case TEST_BT_LINK_PHONE:
				TestApp.TimerRand = rand() % (5001) + 50; //[50ms ~ 5050ms]
				AutoTestMsgSend(MSG_BT_PAIRING);
				break;
				
				//TWS ��������
				case TEST_BT_LINK_TWS:
				TwsLinkTest();
				break;

				//ģʽ�л�
				case TEST_MODE:
				TestApp.TimerRand = rand() % (2001) + 200; //[50ms ~ 1050ms]
				AutoTestMsgSend(MSG_MODE);
				break;
				
				//��ʾ������
				case TEST_REMIND:
				AutoTestMsgSend(MSG_REMIND1);
				break;
				
				//siri����
				case TEST_HFP_MODE:
				TestApp.TimerRand = rand() % (1001) + 20; //[50ms ~ 1050ms]
				if (GetSystemMode() == ModeBtHfPlay)
				{
					AutoTestMsgSend(MSG_PLAY_PAUSE);
				}
				else
				{
					AutoTestMsgSend(MSG_BT_HF_VOICE_RECOGNITION);
				}
				break;

				default:
				break;
			}
			
			DBG("TimerRand: %d    AutoTestMode: %d\n",TestApp.TimerRand, TestApp.AutoTestMode);
			TimeOutSet(&TestApp.TestScanTimer, TestApp.TimerRand);
		}
	}
}

#endif
