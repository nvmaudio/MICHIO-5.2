/**
 **************************************************************************************
 * @file    bt_manager.c
 * @brief   management of all bluetooth event and apis
 *
 * @author  Halley
 * @version V1.1.0
 *
 * $Created: 2016-07-18 16:24:11$
 *
 * @Copyright (C) 2016, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#ifndef _BT_MANAGER_H_
#define _BT_MANAGER_H_

#include <string.h>
#include "bt_common_api.h"
#include "bt_config.h"
#include "timeout.h"
#ifdef CFG_APP_CONFIG
#include "app_config.h"
#endif
#include "sys_param.h"
#ifdef BT_TWS_SUPPORT
#include "bt_tws_api.h"
#endif
#define MAX_PHONE_NUMBER_LENGTH		20

//typedef uint32_t BT_CHECK_EVENT_LIST;
//#define BT_EVENT_NONE					0x00
//#define BT_EVENT_L2CAP_LINK_DISCONNECT	0x02//bit1:��L2CAP�����쳣ʱ�Ͽ�����Ҫ��������������������ʾ�������ӳɹ����Զ�����
//extern BT_CHECK_EVENT_LIST btCheckEventList;
//extern uint32_t btEventListB1Count;
//extern uint32_t btEventListB1State;//�쳣ǰ����״̬�ı��棬ȷ���ڻ������Ƿ���Ҫ���Ͳ�������
//extern uint32_t btEventListCount;


//btManager.btEventFlagMask
#define BT_EVENT_FLAG_NONE					0x00
#define BT_EVENT_FLAG_AVRCP_DISCONNECT		0x01	//��A2DP�Ͽ���,����AVRCP��ʱ�Ͽ�(5s)
#define BT_EVENT_FLAG_L2CAP_LINK_DISCONNECT	0x02	//��L2CAP�����쳣ʱ�Ͽ�����Ҫ��������������������ʾ�������ӳɹ����Զ�����
#define BT_EVENT_FLAG_AVRCP_CONNECT			0x03	//������A2DP��,��������AVRCP����(50ms)
#define BT_EVENT_FLAG_A2DP_DISCONNECT		0x04	//��AVRCP��,��ʱ�Ͽ�(1s)


/***********************************************************************************
 *
 **********************************************************************************/
typedef uint8_t (*Name_Confirm_Callback_t)(uint8_t *name);
typedef uint8_t (*Other_Confirm_Callback_t)(void);
// �����Ҫ����ĳЩ�ֻ����������������ǰ��ӣ�bt_stack_server��ʼ��ʱ�����ú���ָ����ײ�ȥ���ж�����
// ������о������ں���BtConnectDecide��ʵ��
extern uint8_t Name_confirm_Callback_Set(Name_Confirm_Callback_t callback);
// CFG_TWS_ONLY_IN_BT_MODE�򿪺���Կ��ٲ忨�п��ܵ��´ӻ��������������лص�ʵ�ֹ���
// ������о������ں���BtConnectConfirm��ʵ��
extern uint8_t Other_confirm_Callback_Set(Other_Confirm_Callback_t callback);

/***********************************************************************************
 * ��Ȩ��غ����ӿ�
 **********************************************************************************/
#ifndef BT_TWS_SUPPORT
//������Ȩ���ӵĵײ㺯���ӿ�
typedef void (*SEC_Confirm_Callback_t)(uint32_t val);
extern void SEC_Confirm_Callback_Set(SEC_Confirm_Callback_t callback);

// ʹ�òο�:
// 1. �򿪼�Ȩ���ܣ�������BtStackServiceEntrance()
// BtNumericalDisplayFuncEnable(1)
// 2. ����callback��������BtStackServiceEntrance()
//SEC_Confirm_Callback_Set(BtNumericalVerify);
// 3. ʵ�ֻص�����BtNumericalVerify�����¾��Ǵ�ӡ��Զ���豸�������ļ�Ȩ����

//����Զ���豸�ļ�Ȩ����
extern void BtNumericalAccecpt(void);
//�ܾ�Զ���豸�ļ�Ȩ����
extern void BtNumericalReject(void);
#endif

/***********************************************************************************
 *
 **********************************************************************************/
#define MAX_FLASH_DDB_DEVICE_NUM	9

typedef enum
{
	BT_STACK_STATE_NONE,
	BT_STACK_STATE_INITAILIZING,
	BT_STACK_STATE_READY	
} BT_STACK_STATE;

//bt reset state
typedef enum
{
	BT_RST_STATE_NONE,
	BT_RST_STATE_START,
	BT_RST_STATE_WAITING,
	BT_RST_STATE_FINISHED,
} BT_RST_STATE;

//scan page state 
typedef enum
{
	BT_SCAN_PAGE_STATE_NONE,
	BT_SCAN_PAGE_STATE_CLOSING,
	BT_SCAN_PAGE_STATE_DISCONNECTING,
	BT_SCAN_PAGE_STATE_DISABLE,
	BT_SCAN_PAGE_STATE_OPENING,
	BT_SCAN_PAGE_STATE_ENABLE,
	BT_SCAN_PAGE_STATE_SNIFF,	
	BT_SCAN_PAGE_STATE_OPEN_WAITING,
} BT_SCAN_PAGE_STATE;

/**
* Bt device connectable mode. 
*/
typedef enum
{
	BT_DEVICE_CONNECTION_MODE_NONE,				/* cann't be discovered and connected*/
	BT_DEVICE_CONNECTION_MODE_DISCOVERIBLE, 	/* can be discovered*/
	BT_DEVICE_CONNECTION_MODE_CONNECTABLE,		/* can be connected*/
	BT_DEVICE_CONNECTION_MODE_ALL,				/* can be discovered and connected*/
} BT_DEVICE_CONNECTION_MODE;

typedef enum 
{
	BT_HFP_STATE_NONE,
	BT_HFP_STATE_CONNECTING,
	BT_HFP_STATE_CONNECTED,
	BT_HFP_STATE_INCOMING,
	BT_HFP_STATE_OUTGOING,
	BT_HFP_STATE_ACTIVE,
	BT_HFP_STATE_3WAY_INCOMING_CALL,		//1CALL ACTIVE, 1CALL INCOMING
	BT_HFP_STATE_3WAY_OUTGOING_CALL,		//1CALL ACTIVE, 1CALL OUTGOING
	BT_HFP_STATE_3WAY_ATCTIVE_CALL			//2CALL ACTIVE
} BT_HFP_STATE;

typedef enum
{
	BT_A2DP_STATE_NONE,
	BT_A2DP_STATE_CONNECTING,
	BT_A2DP_STATE_CONNECTED,
	BT_A2DP_STATE_STREAMING,
} BT_A2DP_STATE;

typedef enum
{
	BT_AVRCP_STATE_NONE,
	BT_AVRCP_STATE_CONNECTING,
	BT_AVRCP_STATE_CONNECTED,	
} BT_AVRCP_STATE;

typedef uint8_t TIMER_FLAG;
#define TIMER_UNUSED					0x00
#define TIMER_USED						0x01
#define TIMER_STARTED					0x02
#define TIMER_WAITING					0x04

typedef struct _BT_TIMER
{
	TIMER_FLAG	timerFlag;
	TIMER		timerHandle;
}BT_TIMER;


//�ϵ�ʱ����FLASH�����ò���
typedef struct _BT_CONFIGURATION_PARAMS
{
	//BT������������
	uint8_t			bt_LocalDeviceAddr[BT_ADDR_SIZE];//flash mac˳��: NAP-UAP-LAP (�˱������ڱ���flash�Ĳ���,���������޸�)
	uint8_t			ble_LocalDeviceAddr[BT_ADDR_SIZE];
	uint8_t			bt_LocalDeviceName[BT_NAME_SIZE];
	uint8_t			ble_LocalDeviceName[BLE_NAME_SIZE];	//Ԥ��:��BLE name��ͨ��BLE�������ɵĹ㲥���а���

	uint8_t			bt_ConfigHeader[4];
	
	uint8_t			bt_trimValue;
	uint8_t			bt_TxPowerValue;
	
	uint32_t		bt_SupportProfile;
	uint8_t			bt_simplePairingFunc;
	uint8_t			bt_pinCode[17];//max len: 16
	uint8_t			bt_pinCodeLen;
	
	uint8_t			bt_reconFunc;
	uint8_t			bt_reconDevNum;
	uint8_t			bt_reconCount;
	uint8_t			bt_reconIntervalTime;
	uint8_t			bt_reconTimeout;
	uint8_t			bt_accessMode;
}BT_CONFIGURATION_PARAMS;

typedef struct _BT_DB_RECORD
{
	uint8_t			bdAddr[BT_ADDR_SIZE];
	bool			trusted;
	uint8_t			linkKey[16];
	uint8_t			keyType;
	uint8_t			pinLen;
}BT_DB_RECORD;

#ifdef BT_USER_VISIBILITY_STATE
typedef enum{
	BT_USER_STATE_NONE,
	BT_USER_STATE_RECON_BEGIAN,
	BT_USER_STATE_RECON_END,
	BT_USER_STATE_PREPAIR,
	BT_USER_STATE_PAIRING,
	BT_USER_STATE_CONNECTED,
	BT_USER_STATE_DISCONNECTED,
} BT_USER_STATE;

BT_USER_STATE GetBtUserState(void);
void SetBtUserState(BT_USER_STATE bt_state);
void SetBtUserVsibilityLock(bool visibility);
bool GetBtUserVsibilityLock(void);
#endif

typedef struct _BT_LINK_DEVICE_INFO
{
	BT_DB_RECORD	device;
	uint8_t			UsedFlag;

	uint8_t			tws_role;
	uint8_t			remote_profile;

}BT_LINK_DEVICE_INFO;

//a2dp stream type
#define  BT_A2DP_STREAM_TYPE_SBC	0 //defined
#define  BT_A2DP_STREAM_TYPE_AAC	1

/*******************************************************************************
 *  reconnect struct
 *******************************************************************************/
#include "bt_engine_utility.h"
//��������ִ�к���ָ��
typedef void (*FUNC_RECONNECTION_EXCUTE)(void);
void BtAppiFunc_ReconnectionExcute(FUNC_RECONNECTION_EXCUTE CallbackFunc);
extern FUNC_RECONNECTION_EXCUTE BtReconnectExcute;
typedef struct _BT_RECONNECT_ST
{
	btstack_item_t				item;
	
	FUNC_RECONNECTION_EXCUTE	excute;				//ִ�к���ָ��
	BT_TIMER					ConnectionTimer;	//��ʱ��
	uint32_t					profile;			//��Ҫ���ӵ�Э��
	uint8_t						RemoteDevAddr[BT_ADDR_SIZE];	//���ӵĵ�ַ
	uint8_t						TryCount;			//�ظ�����
	uint8_t						IntervalTime;		//���ʱ��
	uint32_t					DelayStartTime;		//��ʱ��ʼ����,ֻ��Чһ�� unit:ms
}BT_RECONNECT_ST;


/*******************************************************************************
 * ����˫�ֻ����ӽṹ��
 *******************************************************************************/
typedef struct _BT_LINKED_ENV_TAG
{
	uint8_t					btLinkState; //0=disconnect; 1=connected
	uint8_t					btLinkedProfile; //�����ӵ�Э��
	uint8_t					remoteAddr[BT_ADDR_SIZE];
	uint8_t					remoteName[BT_NAME_SIZE];
	uint8_t					remoteNameLen;

	//A2DP
	BT_A2DP_STATE			a2dpState;
	uint8_t					a2dp_index;

	//AVRCP
	BT_AVRCP_STATE			avrcpState;
	uint8_t					avrcp_index;
	uint8_t					avrcpPlayStatus;	//ͨ�� BT_STACK_EVENT_AVRCP_ADV_PLAY_STATUS ���²���״̬

	//HFP
	BT_HFP_STATE			hfpState;
	uint8_t					hf_index;
	uint8_t					appleDeviceFlag;	//1=apple device; 0=other device
	
}BT_LINKED_ENV_TAG;

/*******************************************************************************
 * ����ͨ���б��ṹ��
 *******************************************************************************/
typedef struct _BT_HF_CALL_LIST_ST
{
	/* Used: 0=unused, 1=used */
	uint8_t				 used;
	
    /* Dir: 0=outgoing, 1=incoming */
    uint8_t              dir;

    /* Call state. */
    uint8_t				state;
}BT_HF_CALL_LIST_ST;

//2=���뵽���ɱ������ɱ�����״̬;1=���뵽�ɱ������ɱ�����״̬;  0=���뵽���ɱ��������ɱ�����״̬;
typedef enum {
	USER_ANYNOT,
	USER_ACCESSIBLECONNECT,
	USER_NOTACCESSIBLE,
}USER_VISIBILITY;

/*******************************************************************************
 * 
 *******************************************************************************/
typedef struct _BT_MANAGER_ST
{
	uint8_t					cur_index;//��ǰʹ�õ���·
	uint8_t					back_index;//���õ���·
	btstack_list_t			btReconHandle;		//��������������
	BT_RECONNECT_ST*		btReconExcuteSt;	//��ǰִ�л������̽ṹ��ָ��
	BT_RECONNECT_ST			btReconPhoneSt;		//�����ֻ����ݽṹ
	BT_RECONNECT_ST			btReconTwsSt;		//����TWS���ݽṹ
	uint32_t				btReconStopDelay;	//�������� btReconStopDelayʱ���ڲ�ֹͣ����
	
	uint8_t 				ddbUpdate;	//��ʶremote device information�Ƿ���Ҫд�뵽FLASH
	uint8_t					btDevAddr[BT_ADDR_SIZE];//ϵͳӦ�õ�����mac˳��: LAP-UAP-NAP
	BT_LINK_DEVICE_INFO		btLinkDeviceInfo[MAX_FLASH_DDB_DEVICE_NUM];//1(tws)+ 8(device)
	BT_STACK_STATE			stackState;
	BT_DEVICE_CONNECTION_MODE	deviceConMode;

	uint8_t 				btLinkState; //0=disconnect; 1=connected
	//����·
	uint8_t					linkedNumber;
	BT_LINKED_ENV_TAG		btLinked_env[BT_LINK_DEV_NUM];	//2����·���ݽṹ
	
	uint8_t					remoteAddr[BT_ADDR_SIZE];
	uint8_t					remoteName[BT_NAME_SIZE];
	uint8_t					remoteNameLen;
	uint8_t					btDdbLastAddr[BT_ADDR_SIZE];
	uint8_t					btDdbLastProfile;
	uint32_t				btDdbLastInfoOffset;

	USER_VISIBILITY			btAccessModeEnable; //2=���뵽���ɱ������ɱ�����״̬;1=���뵽�ɱ������ɱ�����״̬;  0=���뵽���ɱ��������ɱ�����״̬;
	bool					keysetAccessModeEnable;
	uint32_t				btConStateProtectCnt;//10s��ʱ,���ڱ�����ǰ������������״̬
	uint8_t					btLastAddrUpgradeIgnored; //1:���ڲ��Ժ�У׼Ƶƫ��,���������һ�������豸��Ϣ	
	//remote device
	uint8_t					btReconnectTryCount;
	uint8_t					btReconnectIntervalTime;
	BT_TIMER				btReconnectTimer;
	bool					BtPowerOnFlag;
	bool					btReconnectedFlag;
	uint32_t				btReconnectDelayCount;//�ϵ��������ʼ��,�ڽ���ģʽ���ٷ������
	uint32_t				btReconProfilePriority;//Ĭ�ϻ������� HFP->A2DP->AVRCP, �˼Ĵ�������ע���޸Ļ���˳��
	#ifdef RECON_ADD
	bool 					btReconnectunusual;	
	#endif
#ifdef BT_TWS_SUPPORT
	bool					TwsPowerOnFlag;
	uint8_t					btTwsDeviceAddr[BT_ADDR_SIZE];
#endif //defined(BT_TWS_SUPPORT)	

	uint16_t				btConnectedProfile;

	uint8_t					btCurConnectFlag;
	uint8_t					btDisConnectingFlag;
	BT_SCAN_PAGE_STATE		btScanPageState;

#ifdef BT_TWS_SUPPORT
//tws
	uint8_t					TwsFilterInfor[7];	//tws����������Ϣ
	BT_TWS_STATE			twsState;
	BT_TWS_ROLE				twsRole;
	uint8_t					twsFlag;//��TWS��Լ�¼
	uint32_t				twsSimplePairingCfg;
	uint8_t					twsEnterPairingFlag;//tws������ʼ��־ //flag:0=normal; 1=pairing
	uint8_t					twsMode;//0=active mode; 2=sniff mode
	uint8_t					twsSbSlaveDisable;//1=�ӻ��ر�TWS��������;
	uint8_t					twsStopConnect;	// 1 = ֹͣTWS�������
#if (TWS_PAIRING_MODE == CFG_TWS_ROLE_SLAVE)
	uint8_t					twsSoundbarSlaveTestFlag; //1 = soundbar slave����У׼���Ա�־
#endif
#endif

	BT_RST_STATE			btRstState; //BT�ָ���������
	uint32_t				btRstWaitingCount;

	uint8_t					appleDeviceFlag;	//Note:�˱�־ͨ��HFP�������ж�;           1=apple device; 0=other device
	
	//hfp
//	BT_HFP_STATE			hfpState;
	bool					scoConnected;
	uint8_t					phoneNumber[MAX_PHONE_NUMBER_LENGTH];
	bool					callWaitingFlag;
	uint8_t					batteryLevel;
	uint8_t					signalLevel;
	bool					voiceRecognition;
	uint8_t					volGain;
	//uint8_t					hfpScoCodecType;
	uint8_t					hfpScoCodecType[BT_LINK_DEV_NUM];
	uint8_t					hfpVoiceState;		//1:��ͨ��,����Ӧ��ʹ����ͨ����·������Ƶ 0:����ͨ��
	uint8_t					hfpScoDiscIndex;	//sco����index
	uint16_t				hfpScoDiscDelay;	//sco������ʱ����
#ifdef BT_HFP_BATTERY_SYNC
	uint8_t					HfpBatLevel;
#endif
	uint8_t		 			Esco_RePack[60];
	uint16_t				Esco_RePackLen;

	BT_HF_CALL_LIST_ST		hfpCallListParams[2];	//����ͨ��,�ֻ�������ǰ��ͨ����·״̬
	uint8_t					HfpCurIndex;			//��ǰͨ����index

	//a2dp
//	BT_A2DP_STATE			a2dpState;
	uint8_t					a2dpStreamType[BT_LINK_DEV_NUM];//=sbc; =aac
	uint32_t				aacFrameNumber;//������յ���aac֡��,���ڹ���decoder

	//avrcp
//	BT_AVRCP_STATE			avrcpState;
//	bool					avrcpConnectStart;
	BT_TIMER				avrcpPlayStatusTimer;	
	uint32_t				avrcpMediaInfoGetCnt;
#if (BT_AVRCP_VOLUME_SYNC == ENABLE)
	uint8_t					avrcpSyncEnable;
	uint8_t					avrcpSyncVol;
#endif

	uint32_t				btDutModeEnable;
	uint32_t				btEnterSniffStep;
	uint32_t				btExitSniffReconPhone;
	uint32_t				btTwsPairingStartDelayCnt;
	uint32_t				btEventFlagMask;		
	uint32_t				btEventFlagCount;		
	uint8_t					hfp_CallFalg;
#ifdef BT_TWS_SUPPORT
	bool                    TwsDacNeedTimeout;
	bool                    TwsDacStatus; 
#endif
#ifdef BT_USER_VISIBILITY_STATE
	BT_USER_STATE           btuserstate;
	bool                    btuservisibility;
#endif
} BT_MANAGER_ST;

extern BT_MANAGER_ST		btManager;

/**
 * @brief  Clear Bt Manager Register
 * @param  NONE
 * @return NONE
 * @Note
 *
 */
void ClearBtManagerReg(void);

/**
 * @brief  Get Bt Manager
 * @param  NONE
 * @return ptr is returned
 * @Note
 *
 */
BT_MANAGER_ST * GetBtManager(void);

/**
 * @brief  Set Bt Stack State
 * @param  state: The state of bt stack.
 * @return NONE
 * @Note
 *
 */
void SetBtStackState(BT_STACK_STATE state);

/**
 * @brief  Get Bt Stack State
 * @param  NONE
 * @return The state of bt stack.
 * @Note
 *
 */
BT_STACK_STATE GetBtStackState(void);

/**
 * @brief  Set Bt Device Connection Mode
 * @param  mode: connection mode
 * @return TRUE: success
 * @Note
 *
 */
bool SetBtDeviceConnState(BT_DEVICE_CONNECTION_MODE mode);

/**
 * @brief  Get Bt Device Connection State
 * @param  NONE
 * @return connection mode
 * @Note
 *
 */
BT_DEVICE_CONNECTION_MODE GetBtDeviceConnState(void);


/**
* Bt device connected profile. 
*/
#define BT_CONNECTED_A2DP_FLAG			0x0001
#define BT_CONNECTED_AVRCP_FLAG			0x0002
#define BT_CONNECTED_HFP_FLAG			0x0004
#define BT_CONNECTED_MFI_FLAG			0x0008
#define BT_CONNECTED_SPP_FLAG			0x0010
#define BT_CONNECTED_OPP_FLAG			0x0020
#define BT_CONNECTED_HID_FLAG			0x0040

void SetBtConnectedProfile(uint8_t index, uint16_t connectedProfile);
void SetBtDisconnectProfile(uint8_t index, uint16_t disconnectProfile);
uint16_t GetBtConnectedProfile(void);
uint8_t GetNumOfBtConnectedProfile(void);

void SetBtCurConnectFlag(uint8_t state);
uint8_t GetBtCurConnectFlag(void);

uint8_t BtConnectDecide(uint8_t *Addr);
uint8_t BtConnectConfirm(void);

/*****************************************************************************
******************************************************************************/
/*HFP*/
/**
 * @brief  connect the hfp profile 
 * @param  addr - the remote address
 * @return NONE
 * @Note
 *		
 */
signed char  BtHfpConnect(uint8_t index,uint8_t * addr);

/**
 * @brief  disconnect the hfp profile
 * @param  NONE
 * @return NONE
 * @Note
 *		
 */
void BtHfpDisconnect(uint8_t index);

/**
 * @brief  set the hfp profile state
 * @param  state
 * @return NONE
 * @Note
 *		
 */
void SetHfpState(uint8_t index,BT_HFP_STATE state);

/**
 * @brief  get the hfp profile state
 * @param  NONE
 * @return state
 * @Note
 *		
 */
BT_HFP_STATE GetHfpState(uint8_t index);

/**
 * @brief  get the sco connect flag
 * @param  NONE
 * @return TRUE - connected
 * @Note
 *		
 */
bool GetScoConnectFlag(void);

/**
 * @brief  get callin phone number
 * @param  phone number
 * @return state
 * @Note
 *		
 */
int16_t GetBtCallInPhoneNumber(uint8_t index,uint8_t * number);

/**
 * @brief  set the voice recognition flag
 * @param  flag
 * @return void
 * @Note
 *		
 */
void SetBtHfpVoiceRecognition(bool flag);

/**
 * @brief  use the voice recognition funciton
 * @param  
 * @return 
 * @Note
 * 
 */
void OpenBtHfpVoiceRecognitionFunc(uint8_t index);

/**
 * @brief  Get bt hfp voice recognition flag
 * @param  void
 * @return state
 * @Note
 * 
 */
bool GetBtHfpVoiceRecognition(void);

/**
 * @brief  
 * @param  
 * @return 
 * @Note
 * 
 */
int16_t SetBtHfpSpeakerVolume(uint8_t gain);

/**
 * @brief  
 * @param  
 * @return 
 * @Note
 * 
 */void SetScoConnectFlag(bool flag);


/*A2DP*/
/**
 * @brief  get the a2dp profile state
 * @param  NONE
 * @return state
 * @Note
 *		
 */
BT_A2DP_STATE GetA2dpState(uint8_t index);
int16_t GetA2dpConnectedAddr(uint8_t * addr);

/**
 * @brief  connect the a2dp profile 
 * @param  addr - the remote address
 * @return NONE
 * @Note
 *		
 */
void BtA2dpConnect(uint8_t index,uint8_t *addr);

/*AVRCP*/
/**
 * @brief  get the avrcp profile state
 * @param  NONE
 * @return state
 * @Note
 *		
 */
BT_AVRCP_STATE GetAvrcpState(uint8_t index);
int16_t GetAvrcpConnectedAddr(uint8_t index,uint8_t * addr);

/**
 * @brief  connect the avrcp profile 
 * @param  addr - the remote address
 * @return NONE
 * @Note
 *		
 */
void BtAvrcpConnect(uint8_t index, uint8_t *addr);

/**
 * @brief  avrcp profile state
 * @param  NULL
 * @return 0: avrcp disconnect
 *         1: avrcp is connected
 * @Note
 *		
 */
 bool IsAvrcpConnected(uint8_t index);

/**
 * @brief  timer start - get bt audio play status
 * @param  NONE
 * @return NONE
 * @Note
 *		
 */
void TimerStart_BtPlayStatus(void);

/**
 * @brief  timer stop - get bt audio play status
 * @param  NONE
 * @return NONE
 * @Note
 *		
 */
void TimerStop_BtPlayStatus(void);

/**
 * @brief  check timer start flag - get bt audio play status
 * @param  NONE
 * @return TRUE/FALSE
 * @Note
 *		
 */
uint8_t CheckTimerStart_BtPlayStatus(void);


//�������ӶϿ�����״̬����
void BtLinkStateConnect(uint8_t flag, uint8_t index);
void BtLinkStateDisconnect(uint8_t index);
bool GetBtLinkState(uint8_t index);

//�ֶ�����/�Ͽ�����
void BtConnectCtrl(void);
void BtDisconnectCtrl(void);


//app
void BtSetAccessMode_NoDisc_NoCon(void);
void BtSetAccessMode_NoDisc_Con(void);
void BtSetAccessMode_Disc_Con(void);

int32_t BtAddrIsValid(uint8_t *addr);

bool GetBtHfpVoiceRecognition(void);
void SetBtHfpVoiceRecognition(bool flag);
uint8_t BtCurIndex_Get(void);
void BtCurIndex_Set(uint8_t index);


void BtManageInit(void);

//hfp
void FirstTalkingPhoneIndexSet(uint8_t Index);
uint8_t FirstTalkingPhoneIndexGet(void);
void SecondTalkingPhoneIndexSet(uint8_t Index);
uint8_t SecondTalkingPhoneIndexGet(void);

//a2dp
void SetA2dpState(uint8_t index, BT_A2DP_STATE state);

//avrcp
void SetAvrcpState(uint8_t index, BT_AVRCP_STATE state);
void ChangePlayer(void);

uint8_t GetBtManagerA2dpIndex(uint8_t index);
uint8_t GetBtManagerAvrcpIndex(uint8_t index);
uint8_t GetBtManagerHfpIndex(uint8_t index);

#endif

