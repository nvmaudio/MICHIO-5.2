/**
 **************************************************************************************
 * @file    bt_app_interface.c
 * @brief   application interface
 *			����middleware����������ģʽ��ͨ��ģʽ�����ӿ�
 *
 * @author  KK
 * @version V1.0.0
 *
 * $Created: 2018-03-22 16:24:11$
 *
 * @Copyright (C) 2016, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#include "type.h"
#include "debug.h"
#if FUNC_OS_EN
#include "rtos_api.h"
#endif
#include "bt_common_api.h"
#include "bt_interface.h"
#include "bt_config.h"
#include "bt_manager.h"
#include "bt_app_ddb_info.h"
#ifdef CFG_APP_CONFIG
#include "app_config.h"
#endif

//OS
static void * osMalloc(uint32_t size);
static void osFree(void * ptr);
//DDB
bool OpenBtRecord(const uint8_t * localBdAddr);
bool CloseBtRecord(void);
bool AddBtRecord(const BT_DB_RECORD * btDbRecord);
bool DeleteBtRecord(const uint8_t * bdAddr);
bool FindBtRecord(const uint8_t * bdAddr, BT_DB_RECORD * btDbRecord);
bool FindBtRecordByIndex(uint8_t index, BT_DB_RECORD * btDbRecord);

/*****************************************************************************
 *OS
 ****************************************************************************/
PLATFORM_INTERFACE_OS_T		pfiOS = {
	osMalloc,
	osFree,
	NULL,
	NULL
};


#if FUNC_OS_EN
static void * osMalloc(uint32_t size)
{
	return osPortMalloc(size);
}

static void osFree(void * ptr)
{
	osPortFree(ptr);
}
#else
static void * osMalloc(uint32_t size)
{
	return NULL;
}

static void osFree(void * ptr)
{
	;
}
#endif
/*****************************************************************************
 *DDB
 ****************************************************************************/
PLATFORM_INTERFACE_BT_DDB_T	pfiBtDdb = {
		OpenBtRecord,
		CloseBtRecord,
		AddBtRecord,
		DeleteBtRecord,
		FindBtRecord,
		FindBtRecordByIndex
};

/*---------------------------------------------------------------------------
 *            OpenBtRecord()
 *---------------------------------------------------------------------------
 */
bool OpenBtRecord(const uint8_t * localBdAddr)
{
	//load bt ddb record
	BtDdb_Open(localBdAddr);
	
	/*
	* Get the last BtAddr and ready to connect
	*/
	BtDdb_GetLastBtAddr(btManager.btDdbLastAddr, &btManager.btDdbLastProfile);
	if(((btManager.btDdbLastAddr[0]==0)
		&&(btManager.btDdbLastAddr[1]==0)
		&&(btManager.btDdbLastAddr[2]==0)
		&&(btManager.btDdbLastAddr[3]==0)
		&&(btManager.btDdbLastAddr[4]==0)
		&&(btManager.btDdbLastAddr[5]==0))
		||
		((btManager.btDdbLastAddr[0]==0xff)
		&&(btManager.btDdbLastAddr[1]==0xff)
		&&(btManager.btDdbLastAddr[2]==0xff)
		&&(btManager.btDdbLastAddr[3]==0xff)
		&&(btManager.btDdbLastAddr[4]==0xff)
		&&(btManager.btDdbLastAddr[5]==0xff)))
	{
		APP_DBG("--- last device is not!\n");
#ifdef BT_USER_VISIBILITY_STATE
		SetBtUserState(BT_USER_STATE_PREPAIR);
#endif
	}
	else
	{
#ifdef BT_USER_VISIBILITY_STATE
		SetBtUserState(BT_USER_STATE_NONE);
#endif
		APP_DBG("last device addr %x:%x:%x:%x:%x:%x, profile:0x%02x\n", btManager.btDdbLastAddr[0],btManager.btDdbLastAddr[1],btManager.btDdbLastAddr[2],
			btManager.btDdbLastAddr[3],btManager.btDdbLastAddr[4],btManager.btDdbLastAddr[5], btManager.btDdbLastProfile);
	}
#ifdef BT_USER_VISIBILITY_STATE
	SetBtUserVsibilityLock(FALSE);
#endif
#ifdef BT_TWS_SUPPORT
	btManager.twsFlag = BtDdb_GetTwsDeviceAddr(btManager.btTwsDeviceAddr);

	/*
	* Get the tws BtAddr and ready to connect
	*/
	if(!btManager.twsFlag)
	{
		APP_DBG("no tws paired list\n");
		btManager.twsRole = BT_TWS_UNKNOW;
	}
	else
	{
		APP_DBG("tws role: %d\n", btManager.twsRole);

		if(btManager.twsRole == 0)
		{
			//tws���� page timeoutʱ��Ϊ2.5s,�ӻ� page timeoutĬ��Ϊ5s
			APP_DBG("page timeout 2.5s\n");
			BTStatckSetPageTimeOutValue(BT_PAGE_TIMEOUT); 
		}

	#if (TWS_PAIRING_MODE == CFG_TWS_ROLE_MASTER)
		extern void ble_advertisement_data_update(void);
		ble_advertisement_data_update();
	#endif
	}
#endif
	return TRUE;
}

/*---------------------------------------------------------------------------
 *            CloseBtRecord()
 *---------------------------------------------------------------------------
 */
bool CloseBtRecord(void)
{
	return TRUE;
}

/*---------------------------------------------------------------------------
 *            AddBtRecord()
 *---------------------------------------------------------------------------
 */
bool AddBtRecord(const BT_DB_RECORD * btDbRecord)
{
	BtDdb_AddOneRecord(btDbRecord);
	return TRUE;
}

/*---------------------------------------------------------------------------
 *            DeleteBtRecord()
 *---------------------------------------------------------------------------
 */
bool DeleteBtRecord(const uint8_t * remoteBdAddr)
{
	uint32_t count;
	
	count = DdbFindRecord(remoteBdAddr);
	
	if (count != DDB_NOT_FOUND) 
	{
		DdbDeleteRecord(count);
		return TRUE;
	}
	return TRUE;
}


/*---------------------------------------------------------------------------
 *            FindBtRecord()
 *---------------------------------------------------------------------------
 */
bool FindBtRecord(const uint8_t * remoteBdAddr, BT_DB_RECORD * btDbRecord)
{
	uint32_t count;
	
	count = DdbFindRecord(remoteBdAddr);

	if (count != DDB_NOT_FOUND) 
	{
		*btDbRecord = btManager.btLinkDeviceInfo[count].device;
		return TRUE;
	}

	return FALSE;
}

/*---------------------------------------------------------------------------
 *            FindBtRecordByIndex()
 *---------------------------------------------------------------------------
*/
bool FindBtRecordByIndex(uint8_t index, BT_DB_RECORD * btDbRecord)
{
	return FALSE;
}

/*---------------------------------------------------------------------------
 *
 *---------------------------------------------------------------------------
*/

FUNC_MESSAGE_SEND	BtMidMessageSend = NULL;
void BtAppiFunc_MessageSend(FUNC_MESSAGE_SEND MessageSendFunc)
{
	BtMidMessageSend = MessageSendFunc;
}

FUNC_GET_MEDIA_INFO GetMediaInfo = NULL;
void BtAppiFunc_GetMediaInfo(FUNC_GET_MEDIA_INFO CallbackFunc)
{
	GetMediaInfo = CallbackFunc;
}

FUNC_SAVE_SCO_DATA SaveHfpScoDataToBuffer = NULL;
void BtAppiFunc_SaveScoData(FUNC_SAVE_SCO_DATA CallbackFunc)
{
	SaveHfpScoDataToBuffer = CallbackFunc;
}

//����SCO���ݷ��ͺ����ӿ�
FUNC_BT_SCO_SEND_PROCESS BtScoSendProcess = NULL;
void BtAppiFunc_BtScoSendProcess(FUNC_BT_SCO_SEND_PROCESS CallbackFunc)
{
	BtScoSendProcess = CallbackFunc;
}

//�����Ͽ����̺����ӿ�
FUNC_BT_DISCONNECT_PROCESS BtDisconnectProcess = NULL;
void BtAppiFunc_BtDisconnectProcess(FUNC_BT_DISCONNECT_PROCESS CallbackFunc)
{
	BtDisconnectProcess = CallbackFunc;
}

//����AVRCP�������Ӻ����ӿ�
FUNC_BT_AVRCP_CON_PROCESS BtAvrcpConProcess = NULL;
void BtAppiFunc_BtAvrcpConProcess(FUNC_BT_AVRCP_CON_PROCESS CallbackFunc)
{
	BtAvrcpConProcess = CallbackFunc;
}

//����AVRCP�Ͽ����Ӻ����ӿ�
FUNC_BT_AVRCP_DISCON_PROCESS BtAvrcpDisconProcess = NULL;
void BtAppiFunc_BtAvrcpDisconProcess(FUNC_BT_AVRCP_DISCON_PROCESS CallbackFunc)
{
	BtAvrcpDisconProcess = CallbackFunc;
}

//����ͨ�����Զ���ȡ�ֻ��˵�ͨ��״̬
FUNC_BT_HFP_GET_CUR_CALL_STATE_PROCESS BtHfpGetCurCallStateProcess = NULL;
void BtAppiFunc_BtHfpGetCurCallStateProcess(FUNC_BT_HFP_GET_CUR_CALL_STATE_PROCESS CallbackFunc)
{
	BtHfpGetCurCallStateProcess = CallbackFunc;
}

