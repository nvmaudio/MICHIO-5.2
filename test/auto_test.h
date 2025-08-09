/**
 **************************************************************************************
 * @file    auto_test.h
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
#ifndef __AUTO_TEST_H__
#define __AUTO_TEST_H__


#include "type.h"
#include "rtos_api.h"
#include "app_config.h"
#include "audio_core_api.h"
#include "app_message.h"
#include "timeout.h"
#include "mode_task.h"
#include "flash_boot.h"

#ifdef AUTO_TEST_ENABLE
enum
{
	TEST_NONE,  
	TEST_BT_LINK_PHONE,
	TEST_BT_LINK_TWS,
	TEST_MODE,
	TEST_REMIND,
	TEST_HFP_MODE,
	TEST_ALL,
	TEST_END,
}auto_test;


typedef struct AutoContext
{
	TIMER				TestScanTimer;
	uint16_t            TimerRand;
	uint16_t 			AutoTestMode;
	uint16_t 			StateBack;
}AutoContext;

AutoContext TestApp;

extern int rand (void);

#endif
#endif /*__AUTO_TEST_H__*/

