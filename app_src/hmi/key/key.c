/**
 **************************************************************************************
 * @file    Key.c
 * @brief   
 *
 * @author  pi
 * @version V1.0.0
 *
 * $Created: 2018-01-11 17:30:47$
 *
 * @Copyright (C) 2018, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#include "adc_levels.h"
#include "app_config.h"
#include "app_message.h"
#include "debug.h"
#include "key.h"
#include "ctrlvars.h"
#include "timeout.h"
#include "beep.h"
#include "mode_task.h"
#ifdef CFG_RES_ADC_KEY_SCAN
	#include "adc_key.h"
#endif
#ifdef CFG_RES_IR_KEY_SCAN
	#include "ir_key.h"
#endif
#ifdef CFG_RES_CODE_KEY_USE
	#include "code_key.h"
#endif
#ifdef CFG_RES_IO_KEY_SCAN
	#include "io_key.h"
#endif
#ifdef CFG_APP_IDLE_MODE_EN
	#include "idle_mode.h"
#endif


#include "sys_param.h"


extern void GIE_DISABLE(void); 
extern void GIE_ENABLE(void);

uint16_t gFuncID = 0;
uint16_t gKeyValue;

#ifdef CFG_FUNC_DBCLICK_MSG_EN
	KEYBOARD_MSG dbclick_msg;
	TIMER	DBclicTimer;
#endif




#ifdef KEY_TEST
    uint8_t ADKEY_TAB[11][5] =
    {
        //KEY_PRESS       SHORT_RELEASE                 LONG_PRESS                  KEY_HOLD                    LONG_PRESS_RELEASE

		{MSG_NONE,        MSG_MODE,                 MSG_BT_TWS_PAIRING,  		MSG_NONE,                   MSG_NONE},  			// 100R
        {MSG_NONE,        MSG_EFFECTMODE,     		MSG_NONE,    		        MSG_NONE,   				MSG_NONE},  			// 2K2
        {MSG_NONE,        MSG_PRE,	    			MSG_MUSIC_VOLDOWN,    		MSG_MUSIC_VOLDOWN,   		MSG_MUSIC_VOLDOWN},  	// 4K7 
        {MSG_NONE,        MSG_NEXT,					MSG_MUSIC_VOLUP,    		MSG_MUSIC_VOLUP,   	MSG_MUSIC_VOLUP},  				// 8K2
        {MSG_NONE,        MSG_PLAY_PAUSE,			MSG_BT_PAIRING_CLEAR_DATA, 	MSG_NONE,   				MSG_NONE},  			// 12K
        {MSG_NONE,        MSG_NONE,	        		MSG_NONE,          			MSG_NONE,          			MSG_NONE},  						// 18K
        {MSG_NONE,        MSG_NONE,            		MSG_NONE,          			MSG_NONE,          			MSG_NONE},  						// 27K
        {MSG_NONE,        MSG_NONE,   				MSG_NONE,  					MSG_NONE,                   MSG_NONE},  						// 39K
        {MSG_NONE,        MSG_NONE,  				MSG_NONE,                   MSG_NONE,                   MSG_NONE},  						// 58K
        {MSG_NONE,        MSG_NONE,     			MSG_NONE,   				MSG_NONE,            		MSG_NONE},  						// 100K
        {MSG_NONE,        MSG_NONE,  				MSG_NONE,  					MSG_NONE,                   MSG_NONE},  						// 220K
    };
#else
    uint8_t ADKEY_TAB[11][5] =
    {
        //KEY_PRESS       SHORT_RELEASE             LONG_PRESS                  KEY_HOLD                    LONG_PRESS_RELEASE

        {MSG_NONE,        MSG_MODE,                 MSG_BT_TWS_PAIRING,  		MSG_NONE,                   MSG_NONE},  			// 100R
        {MSG_NONE,        MSG_NONE,     			MSG_NONE,    		        MSG_NONE,   				MSG_NONE},  						// 2K2
        {MSG_NONE,        MSG_EFFECTMODE,	    	MSG_VB_DAC0,    		    MSG_NONE,   			    MSG_NONE},  			// 4K7 
        {MSG_NONE,        MSG_PRE,					MSG_MUSIC_VOLDOWN,    		MSG_MUSIC_VOLDOWN,   		MSG_MUSIC_VOLDOWN},  	// 8K2
        {MSG_NONE,        MSG_NONE,				    MSG_NONE,    				MSG_NONE,   				MSG_NONE},  						// 12K
        {MSG_NONE,        MSG_NONE,	        		MSG_NONE,          			MSG_NONE,          			MSG_NONE},  						// 18K
        {MSG_NONE,        MSG_NONE,            		MSG_NONE,          			MSG_NONE,          			MSG_NONE},  						// 27K
        {MSG_NONE,        MSG_NONE,   				MSG_NONE,  					MSG_NONE,                   MSG_NONE},  						// 39K
        {MSG_NONE,        MSG_NONE,  				MSG_NONE,                   MSG_NONE,                   MSG_NONE},  						// 58K
        {MSG_NONE,        MSG_NEXT,     			MSG_MUSIC_VOLUP,   			MSG_MUSIC_VOLUP,            MSG_MUSIC_VOLUP},  		// 100K
        {MSG_NONE,        MSG_PLAY_PAUSE,  			MSG_BT_PAIRING_CLEAR_DATA,  MSG_NONE,                   MSG_NONE},  			// 220K
    };
#endif


static const uint16_t IOKEY_TAB[][KEY_MSG_DEFAULT_NUM] =
{
    //KEY_PRESS       SHORT_RELEASE                 LONG_PRESS                  KEY_HOLD                    LONG_PRESS_RELEASE

	{MSG_NONE,        MSG_EFFECTMODE,               MSG_SOFT_POWER,  			MSG_NONE,                   MSG_NONE},
	{MSG_NONE,        MSG_NONE,      				MSG_NONE,    				MSG_NONE,   				MSG_NONE},
	{MSG_NONE,        MSG_NONE,	    				MSG_NONE,    				MSG_NONE,   				MSG_NONE},
};

static const uint16_t CODEKEY_TAB[][KEY_MSG_DEFAULT_NUM] =
{
	//KEY_PRESS       SHORT_RELEASE                 LONG_PRESS                  KEY_HOLD                    LONG_PRESS_RELEASE

	{MSG_NONE,        MSG_MUSIC_VOLDOWN,        	MSG_MUSIC_VOLDOWN,   		MSG_MUSIC_VOLDOWN,  		MSG_NONE},
	{MSG_NONE,        MSG_MUSIC_VOLUP,          	MSG_MUSIC_VOLUP,     		MSG_MUSIC_VOLUP,    		MSG_NONE},
};

static const uint16_t IRKEY_TAB[][KEY_MSG_DEFAULT_NUM] =
{
	//KEY_PRESS       SHORT_RELEASE                 LONG_PRESS                  KEY_HOLD                    LONG_PRESS_RELEASE

#if(defined(BT_SNIFF_ENABLE) || defined(BT_TWS_SUPPORT))
	{MSG_NONE,        MSG_BT_SNIFF,    				MSG_BT_SNIFF,  				MSG_NONE,                   MSG_NONE},
#else
	{MSG_NONE,        MSG_DEEPSLEEP,    			MSG_DEEPSLEEP,  			MSG_NONE,                   MSG_NONE},
#endif
	{MSG_NONE,        MSG_MODE,  					MSG_NONE,                   MSG_NONE,                   MSG_NONE},
	{MSG_NONE,        MSG_MUTE,	        			MSG_NONE,                   MSG_NONE,                   MSG_NONE},
	{MSG_NONE,        MSG_PLAY_PAUSE,   		    MSG_NONE,                   MSG_NONE,                   MSG_NONE},
	{MSG_NONE,        MSG_PRE,          			MSG_FB_START,         		MSG_FB_START,               MSG_FF_FB_END},
	{MSG_NONE,        MSG_NEXT,	        			MSG_FF_START,         		MSG_FF_START,               MSG_FF_FB_END},
	{MSG_NONE,        MSG_EQ,           			MSG_NONE,                   MSG_NONE,                   MSG_NONE},
	{MSG_NONE,        MSG_MUSIC_VOLUP,	    		MSG_MUSIC_VOLUP,      		MSG_MUSIC_VOLUP,     		MSG_NONE},
	{MSG_NONE,        MSG_MUSIC_VOLDOWN,	    	MSG_MUSIC_VOLDOWN,    		MSG_MUSIC_VOLDOWN,   		MSG_NONE},
	{MSG_NONE,        MSG_REPEAT,                   MSG_NONE,                   MSG_NONE,                   MSG_NONE},
	{MSG_NONE,        MSG_RADIO_PLAY_SCAN,          MSG_NONE,                   MSG_NONE,                   MSG_NONE},
	{MSG_NONE,        MSG_NUM_1,            		MSG_NONE,                   MSG_NONE,                   MSG_NONE},
	{MSG_NONE,        MSG_NUM_2,            		MSG_NONE,                   MSG_NONE,                   MSG_NONE},
	{MSG_NONE,        MSG_NUM_3,	        		MSG_NONE,                   MSG_NONE,                   MSG_NONE},
	{MSG_NONE,        MSG_NUM_4,	        		MSG_NONE,                   MSG_NONE,                   MSG_NONE},
	{MSG_NONE,        MSG_NUM_5,	        		MSG_NONE,                   MSG_NONE,                   MSG_NONE},
	{MSG_NONE,        MSG_NUM_6,	        		MSG_NONE,                   MSG_NONE,                   MSG_NONE},
	{MSG_NONE,        MSG_NUM_7,            		MSG_NONE,                   MSG_NONE,                   MSG_NONE},
	{MSG_NONE,        MSG_NUM_8,            		MSG_NONE,                   MSG_NONE,                   MSG_NONE},
	{MSG_NONE,        MSG_NUM_9,            		MSG_NONE,                   MSG_NONE,                   MSG_NONE},
	{MSG_NONE,        MSG_NUM_0,            		MSG_NONE,                   MSG_NONE,                   MSG_NONE},
};

static const uint16_t PWRKey_TAB = MSG_PLAY_PAUSE;

uint32_t GetGlobalKeyValue(void)
{
    return gFuncID;
}

void ClrGlobalKeyValue(void)
{
    gFuncID = 0;
}





#if (defined(CFG_RES_ADC_KEY_SCAN) ||defined(CFG_RES_IO_KEY_SCAN)|| defined(CFG_RES_IR_KEY_SCAN) || defined(CFG_RES_CODE_KEY_USE) ||  defined(CFG_ADC_LEVEL_KEY_EN))


void SetGlobalKeyValue(uint8_t KeyType, uint16_t KeyValue)
{
}

void SetIrKeyValue(uint8_t KeyType, uint16_t KeyValue)
{
	gFuncID = IRKEY_TAB[KeyValue][KeyType - 1];
}



void SetAdcKeyValue(uint8_t KeyType, uint16_t KeyValue)
{
	gFuncID = ADKEY_TAB[KeyValue][KeyType - 1];
}

uint16_t GetIrKeyValue()
{
	return gFuncID;
}

void KeyInit(void)
{
	#ifdef CFG_RES_ADC_KEY_SCAN
		AdcKeyInit();
	#endif

	#ifdef CFG_ADC_LEVEL_KEY_EN
		ADCLevelsKeyInit();
	#endif

	#ifdef CFG_FUNC_DBCLICK_MSG_EN
		DbclickInit();
	#endif
}

bool GIE_STATE_GET(void);

MessageId KeyScan(void)
{
	MessageId KeyMsg = MSG_NONE;

	#ifdef CFG_RES_ADC_KEY_SCAN
		AdcKeyMsg AdcKeyMsg;
	#endif

	#ifdef CFG_RES_ADC_KEY_SCAN
		if(sys_parameter.Key_en){
			AdcKeyMsg = AdcKeyScan();
			if(AdcKeyMsg.index != ADC_CHANNEL_EMPTY && AdcKeyMsg.type != ADC_KEY_UNKOWN_TYPE)
			{
				gFuncID = ADKEY_TAB[AdcKeyMsg.index][AdcKeyMsg.type - 1];
				//APP_DBG("AdcKeyMsg = %d, %d\n", AdcKeyMsg.index, AdcKeyMsg.type);
			}
		}
	#endif

	#ifdef CFG_ADC_LEVEL_KEY_EN
		if(sys_parameter.Vr_en){
			KeyMsg = AdcLevelKeyProcess();
			if(KeyMsg != MSG_NONE)
			{
				gFuncID = KeyMsg;
			}		
		}		
	#endif

	#if	defined(CFG_APP_IDLE_MODE_EN)
		KeyMsg = GetEnterIdleModeScanKey();
		if(KeyMsg != MSG_NONE)
		{
			gFuncID = KeyMsg;
		}
	#endif

	KeyMsg = GetGlobalKeyValue();
	ClrGlobalKeyValue();

	return KeyMsg;
}

#endif


void DbclickInit(void)
{
	#ifdef CFG_FUNC_DBCLICK_MSG_EN
		dbclick_msg.dbclick_en            = 1;
		dbclick_msg.dbclick_counter       = 0;
		dbclick_msg.dbclick_timeout       = 0;

		dbclick_msg.KeyMsg                = CFG_PARA_CLICK_MSG;////Single click msg
		dbclick_msg.dbclick_msg           = CFG_PARA_DBCLICK_MSG;//double  click msg
		TimeOutSet(&DBclicTimer, 0);
	#endif
}

void DbclickProcess(void)
{
	#ifdef CFG_FUNC_DBCLICK_MSG_EN
		if(!IsTimeOut(&DBclicTimer))
		{
			return;
		}
		TimeOutSet(&DBclicTimer, 4);

		if(dbclick_msg.dbclick_en)
		{
			if(dbclick_msg.dbclick_timeout)
			{
				dbclick_msg.dbclick_timeout--;
				if(dbclick_msg.dbclick_timeout == 0)
				{
					gFuncID = dbclick_msg.KeyMsg;//Single click msg
					dbclick_msg.dbclick_counter = 0;
					dbclick_msg.dbclick_timeout = 0;
					APP_DBG("shot click_msg \n");
				}
			}
		}
	#endif ///#ifdef CFG_FUNC_DBCLICK_MSG_EN
}

uint8_t DbclickGetMsg(uint16_t Msg)
{
	#ifdef CFG_FUNC_DBCLICK_MSG_EN
		if(dbclick_msg.dbclick_en == 0)   return 0;
		//if( WORK_MODE != BT_MODE) return 0
		if((dbclick_msg.KeyMsg == Msg))
		{
			gFuncID = 0;

			if(dbclick_msg.dbclick_timeout == 0)
			{
			dbclick_msg.dbclick_timeout =  CFG_PARA_DBCLICK_DLY_TIME;///4ms*20=80ms
				dbclick_msg.dbclick_counter += 1;
			APP_DBG("start double click_msg \n");
			}
			else
			{
				gFuncID =  dbclick_msg.dbclick_msg;//double  click msg
				dbclick_msg.dbclick_counter = 0;
				dbclick_msg.dbclick_timeout = 0;
				APP_DBG("double click_msg \n");
			}
			return 1;
		}
		else
		{
			if(Msg != MSG_NONE)
			{
				dbclick_msg.dbclick_timeout = 0;
				dbclick_msg.dbclick_counter = 0;
			}
			return 0;
		}
	#else
		return 0;
	#endif
}

